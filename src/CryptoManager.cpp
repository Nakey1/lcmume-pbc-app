#include "CryptoManager.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

CryptoManager::CryptoManager(QObject *parent): QObject(parent) {}

bool CryptoManager::hasContext() const {
    return static_cast<bool>(m_ctx);
}

QString CryptoManager::currentUserId() const {
    return m_userId;
}

QString CryptoManager::currentTargetId() const {
    return m_targetId;
}

void CryptoManager::setServerParams(const QString &paramStr, const QString &gStr, const QString &pPubStr) {
    try {
        m_ctx = std::make_unique<CryptoContext>(paramStr.toStdString());
        m_localKeys = UserKeys();
        m_peerKeys = PeerUserKeys();
        m_peerKeysLoaded = false;
        m_peerXStr.clear();
        m_peerRStr.clear();
        m_recipients.clear();

        if (!setElementFromString(m_ctx->G, gStr, true, nullptr)) {
            emit cryptoInfo("Failed to parse G");
        }
        if (!setElementFromString(m_ctx->P_pub, pPubStr, true, nullptr)) {
            emit cryptoInfo("Failed to parse P_pub");
        }

        emit cryptoInfo("Crypto context initialized");
    } catch (const std::exception &e) {
        emit cryptoInfo(QString("Context init failed: %1").arg(e.what()));
    }
}

void CryptoManager::setCurrentUserId(const QString &userId) {
    m_userId = userId.trimmed();
}

void CryptoManager::setCurrentTargetId(const QString &targetId) {
    m_targetId = targetId.trimmed();
}

QString CryptoManager::elementToString(Element &e) const {
    char buf[2048];
    element_snprintf(buf, sizeof(buf), "%B", e.get());
    return QString::fromUtf8(buf);
}

bool CryptoManager::setElementFromString(Element &e, const QString &s, bool isG1, QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Context not initialized";
        return false;
    }

    if (!e.is_initialized()) {
        if (isG1) {
            e.init_G1(*m_ctx->guard_);
        } else {
            e.init_Zr(*m_ctx->guard_);
        }
    }

    const QByteArray bytes = s.toUtf8();
    if (element_set_str(e.get(), bytes.constData(), 10) == 0) {
        if (errorMessage) *errorMessage = QStringLiteral("element_set_str failed: %1").arg(s);
        return false;
    }

    return true;
}

void CryptoManager::ensureLocalKeyInit(QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Init context first";
        return;
    }

    if (!m_localKeys.x.is_initialized()) m_localKeys.x.init_Zr(*m_ctx->guard_);
    if (!m_localKeys.d.is_initialized()) m_localKeys.d.init_Zr(*m_ctx->guard_);
    if (!m_localKeys.X.is_initialized()) m_localKeys.X.init_G1(*m_ctx->guard_);
    if (!m_localKeys.R.is_initialized()) m_localKeys.R.init_G1(*m_ctx->guard_);
}

void CryptoManager::ensurePeerKeyInit(QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Init context first";
        return;
    }

    if (!m_peerKeys.X.is_initialized()) m_peerKeys.X.init_G1(*m_ctx->guard_);
    if (!m_peerKeys.R.is_initialized()) m_peerKeys.R.init_G1(*m_ctx->guard_);
}

PeerUserKeys *CryptoManager::findRecipient(const QString &targetId) {
    for (auto &recipient : m_recipients) {
        if (QString::fromStdString(recipient->id) == targetId) {
            return recipient.get();
        }
    }
    return nullptr;
}

const PeerUserKeys *CryptoManager::findRecipient(const QString &targetId) const {
    for (const auto &recipient : m_recipients) {
        if (QString::fromStdString(recipient->id) == targetId) {
            return recipient.get();
        }
    }
    return nullptr;
}

std::unique_ptr<PeerUserKeys> CryptoManager::buildPeerKeys(const QString &targetId,
                                                           const QString &xStr,
                                                           const QString &rStr,
                                                           QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Context not initialized";
        return nullptr;
    }

    auto peer = std::make_unique<PeerUserKeys>();
    peer->id = targetId.toStdString();
    peer->X.init_G1(*m_ctx->guard_);
    peer->R.init_G1(*m_ctx->guard_);

    if (!setElementFromString(peer->X, xStr, true, errorMessage)) {
        return nullptr;
    }
    if (!setElementFromString(peer->R, rStr, true, errorMessage)) {
        return nullptr;
    }

    return peer;
}

void CryptoManager::setPeerPublicKeys(const QString &targetId, const QString &xStr, const QString &rStr) {
    if (!m_ctx) {
        emit cryptoInfo("Init context before loading peer keys");
        return;
    }

    m_targetId = targetId.trimmed();
    m_peerKeys.id = m_targetId.toStdString();

    QString error;
    ensurePeerKeyInit(&error);
    if (!error.isEmpty()) {
        emit cryptoInfo(error);
        return;
    }

    if (!setElementFromString(m_peerKeys.X, xStr, true, &error)) {
        emit cryptoInfo(error);
        return;
    }
    if (!setElementFromString(m_peerKeys.R, rStr, true, &error)) {
        emit cryptoInfo(error);
        return;
    }

    m_peerKeysLoaded = true;
    m_peerXStr = xStr;
    m_peerRStr = rStr;
    emit peerKeysUpdated(targetId, xStr, rStr);
    emit cryptoInfo(QString("Peer keys loaded: %1").arg(targetId));
}

bool CryptoManager::addCurrentPeerToRecipients(QString *errorMessage) {
    if (!m_peerKeysLoaded || m_targetId.isEmpty()) {
        if (errorMessage) *errorMessage = "Load peer keys first";
        return false;
    }

    auto peer = buildPeerKeys(m_targetId, m_peerXStr, m_peerRStr, errorMessage);
    if (!peer) {
        return false;
    }

    for (auto &recipient : m_recipients) {
        if (QString::fromStdString(recipient->id) == m_targetId) {
            recipient = std::move(peer);
            emit cryptoInfo(QString("Recipient updated: %1").arg(m_targetId));
            return true;
        }
    }

    m_recipients.push_back(std::move(peer));
    emit cryptoInfo(QString("Recipient added: %1").arg(m_targetId));
    return true;
}

bool CryptoManager::removeRecipient(const QString &targetId, QString *errorMessage) {
    for (auto it = m_recipients.begin(); it != m_recipients.end(); ++it) {
        if (QString::fromStdString((*it)->id) == targetId) {
            m_recipients.erase(it);
            emit cryptoInfo(QString("Recipient removed: %1").arg(targetId));
            return true;
        }
    }

    if (errorMessage) *errorMessage = QString("Recipient not found: %1").arg(targetId);
    return false;
}

QStringList CryptoManager::recipientIds() const {
    QStringList ids;
    for (const auto &recipient : m_recipients) {
        ids << QString::fromStdString(recipient->id);
    }
    return ids;
}

int CryptoManager::recipientCount() const {
    return int(m_recipients.size());
}

bool CryptoManager::generateLocalKeys(QString *errorMessage) {
    ensureLocalKeyInit(errorMessage);
    if (errorMessage && !errorMessage->isEmpty()) return false;
    if (!m_ctx) return false;

    if (m_userId.trimmed().isEmpty()) {
        if (errorMessage) *errorMessage = "userId is empty";
        return false;
    }

    m_localKeys.id = m_userId.toStdString();
    element_random(m_localKeys.x);

    if (!m_localKeys.X.is_initialized()) m_localKeys.X.init_G1(*m_ctx->guard_);
    element_mul_zn(m_localKeys.X, m_ctx->G, m_localKeys.x);

    if (m_localKeys.d.is_initialized()) {
        element_set0(m_localKeys.d);
    }
    if (m_localKeys.R.is_initialized()) {
        element_set0(m_localKeys.R);
    }

    emit localKeysUpdated(
        m_userId,
        elementToString(m_localKeys.x),
        elementToString(m_localKeys.d),
        elementToString(m_localKeys.X),
        elementToString(m_localKeys.R)
    );
    emit cryptoInfo("Local key pair generated");
    return true;
}

bool CryptoManager::applyPartialKey(const QString &dStr, const QString &rStr, QString *errorMessage) {
    ensureLocalKeyInit(errorMessage);
    if (errorMessage && !errorMessage->isEmpty()) return false;
    if (!m_ctx) return false;

    if (!setElementFromString(m_localKeys.d, dStr, false, errorMessage)) return false;
    if (!setElementFromString(m_localKeys.R, rStr, true, errorMessage)) return false;

    emit localKeysUpdated(
        m_userId,
        elementToString(m_localKeys.x),
        elementToString(m_localKeys.d),
        elementToString(m_localKeys.X),
        elementToString(m_localKeys.R)
    );
    emit cryptoInfo("Partial key applied");
    return true;
}

QString CryptoManager::serializeLocalPublicKey() const {
    if (!m_localKeys.X.is_initialized() || !m_localKeys.R.is_initialized()) {
        return {};
    }

    QJsonObject obj;
    obj["user_id"] = m_userId;
    obj["X"] = const_cast<CryptoManager *>(this)->elementToString(const_cast<Element &>(m_localKeys.X));
    obj["R"] = const_cast<CryptoManager *>(this)->elementToString(const_cast<Element &>(m_localKeys.R));
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QString CryptoManager::serializeLocalPartialKey() const {
    if (!m_localKeys.d.is_initialized() || !m_localKeys.R.is_initialized()) {
        return {};
    }

    QJsonObject obj;
    obj["user_id"] = m_userId;
    obj["d"] = const_cast<CryptoManager *>(this)->elementToString(const_cast<Element &>(m_localKeys.d));
    obj["R"] = const_cast<CryptoManager *>(this)->elementToString(const_cast<Element &>(m_localKeys.R));
    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QString CryptoManager::serializeCiphertext(const Ciphertext &ct) const {
    QJsonObject obj;

    auto toStr = [this](Element &e) {
        return const_cast<CryptoManager *>(this)->elementToString(e);
    };

    obj["C1"] = toStr(const_cast<Element &>(ct.C1));
    obj["C2"] = toStr(const_cast<Element &>(ct.C2));
    obj["C3"] = QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(ct.C3.data()), int(ct.C3.size())).toBase64());
    obj["C4"] = QString::fromLatin1(QByteArray(reinterpret_cast<const char *>(ct.C4.data()), int(ct.C4.size())).toBase64());

    QJsonArray vArr;
    for (auto &e : const_cast<std::vector<Element>&>(ct.poly_coeffsV)) {
        vArr.append(toStr(e));
    }
    obj["poly_coeffsV"] = vArr;

    QJsonArray zArr;
    for (auto &e : const_cast<std::vector<Element>&>(ct.poly_coeffsZ)) {
        zArr.append(toStr(e));
    }
    obj["poly_coeffsZ"] = zArr;

    return QString::fromUtf8(QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

bool CryptoManager::deserializeCiphertext(const QString &jsonText, Ciphertext &ct, QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Context not initialized";
        return false;
    }

    const auto doc = QJsonDocument::fromJson(jsonText.toUtf8());
    if (!doc.isObject()) {
        if (errorMessage) *errorMessage = "Ciphertext JSON is not an object";
        return false;
    }

    const QJsonObject obj = doc.object();
    QString err;

    ct.clear();
    ct.C1.init_G1(*m_ctx->guard_);
    ct.C2.init_G1(*m_ctx->guard_);

    if (!setElementFromString(ct.C1, obj.value("C1").toString(), true, &err)) { if (errorMessage) *errorMessage = err; return false; }
    if (!setElementFromString(ct.C2, obj.value("C2").toString(), true, &err)) { if (errorMessage) *errorMessage = err; return false; }

    const QByteArray c3 = QByteArray::fromBase64(obj.value("C3").toString().toLatin1());
    const QByteArray c4 = QByteArray::fromBase64(obj.value("C4").toString().toLatin1());
    if (c3.size() != CryptoContext::H2_LEN || c4.size() != CryptoContext::H3_LEN) {
        if (errorMessage) *errorMessage = "Invalid C3/C4 length";
        return false;
    }
    std::memcpy(ct.C3.data(), c3.constData(), c3.size());
    std::memcpy(ct.C4.data(), c4.constData(), c4.size());

    ct.poly_coeffsV.clear();
    for (const auto &v : obj.value("poly_coeffsV").toArray()) {
        Element e;
        e.init_Zr(*m_ctx->guard_);
        if (!setElementFromString(e, v.toString(), false, &err)) {
            if (errorMessage) *errorMessage = err;
            return false;
        }
        ct.poly_coeffsV.push_back(std::move(e));
    }

    ct.poly_coeffsZ.clear();
    for (const auto &v : obj.value("poly_coeffsZ").toArray()) {
        Element e;
        e.init_Zr(*m_ctx->guard_);
        if (!setElementFromString(e, v.toString(), false, &err)) {
            if (errorMessage) *errorMessage = err;
            return false;
        }
        ct.poly_coeffsZ.push_back(std::move(e));
    }

    return true;
}

QString CryptoManager::encryptMessageToJson(const QString &message, QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Init public params first";
        return {};
    }
    if (m_recipients.empty()) {
        if (errorMessage) *errorMessage = "Add at least one recipient first";
        return {};
    }
    if (m_localKeys.id.empty()) {
        if (errorMessage) *errorMessage = "Sender id is empty";
        return {};
    }
    if (!m_localKeys.x.is_initialized() || !m_localKeys.d.is_initialized() ||
        !m_localKeys.X.is_initialized() || !m_localKeys.R.is_initialized()) {
        if (errorMessage) *errorMessage = "Local keys are incomplete";
        return {};
    }

    std::vector<PeerUserKeys*> receivers;
    receivers.reserve(m_recipients.size());
    for (auto &recipient : m_recipients) {
        receivers.push_back(recipient.get());
    }

    std::vector<unsigned char> msg(CryptoContext::MSG_LEN, 0);
    const QByteArray raw = message.toUtf8();
    const int copyLen = qMin<int>(raw.size(), CryptoContext::MSG_LEN);
    std::memcpy(msg.data(), raw.constData(), copyLen);

    Ciphertext ct;
    try {
        Encrypt(ct, m_localKeys, receivers, msg, *m_ctx);
    } catch (const std::exception &e) {
        if (errorMessage) *errorMessage = QString("Encrypt failed: %1").arg(e.what());
        return {};
    }

    const QString json = serializeCiphertext(ct);
    emit ciphertextReady(json);
    emit cryptoInfo(QString("Encrypt done, recipients=%1").arg(m_recipients.size()));
    return json;
}

QString CryptoManager::decryptMessageFromJson(const QString &ciphertextJson, QString *errorMessage) {
    if (!m_ctx) {
        if (errorMessage) *errorMessage = "Init public params first";
        return {};
    }
    if (!m_peerKeysLoaded) {
        if (errorMessage) *errorMessage = "Load sender public key first";
        return {};
    }

    Ciphertext ct;
    if (!deserializeCiphertext(ciphertextJson, ct, errorMessage)) {
        return {};
    }

    if (m_localKeys.id.empty()) {
        if (errorMessage) *errorMessage = "Current user id is empty";
        return {};
    }

    std::vector<unsigned char> out;
    if (!Decrypt(out, m_localKeys, m_peerKeys, ct, *m_ctx)) {
        if (errorMessage) *errorMessage = "Decrypt failed";
        return {};
    }

    QString plaintext = QString::fromUtf8(reinterpret_cast<const char *>(out.data()), int(out.size()));
    plaintext = plaintext.trimmed();
    emit plaintextReady(plaintext);
    emit cryptoInfo("Decrypt done");
    return plaintext;
}
