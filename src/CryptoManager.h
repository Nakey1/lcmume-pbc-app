#pragma once
#include <QObject>
#include <QString>
#include <QStringList>
#include <QByteArray>
#include <memory>
#include <vector>
#include "models.h"
#include "lcmume.hpp"

class CryptoManager : public QObject {
    Q_OBJECT
public:
    explicit CryptoManager(QObject *parent = nullptr);

    bool hasContext() const;
    QString currentUserId() const;
    QString currentTargetId() const;

public slots:
    void setServerParams(const QString &paramStr, const QString &gStr, const QString &pPubStr);
    void setCurrentUserId(const QString &userId);
    void setCurrentTargetId(const QString &targetId);

    bool generateLocalKeys(QString *errorMessage = nullptr);
    bool applyPartialKey(const QString &dStr, const QString &rStr, QString *errorMessage = nullptr);

    void setPeerPublicKeys(const QString &targetId, const QString &xStr, const QString &rStr);
    bool addCurrentPeerToRecipients(QString *errorMessage = nullptr);
    bool removeRecipient(const QString &targetId, QString *errorMessage = nullptr);

    QString serializeLocalPublicKey() const;
    QString serializeLocalPartialKey() const;
    QStringList recipientIds() const;
    int recipientCount() const;

    QString encryptMessageToJson(const QString &message, QString *errorMessage = nullptr);
    QString decryptMessageFromJson(const QString &ciphertextJson, QString *errorMessage = nullptr);

signals:
    void cryptoInfo(const QString &message);
    void localKeysUpdated(const QString &userId, const QString &xStr, const QString &dStr, const QString &xPublicStr, const QString &rStr);
    void peerKeysUpdated(const QString &targetId, const QString &xStr, const QString &rStr);
    void ciphertextReady(const QString &ciphertextJson);
    void plaintextReady(const QString &plaintext);

private:
    QString elementToString(Element &e) const;
    bool setElementFromString(Element &e, const QString &s, bool isG1, QString *errorMessage);
    QString serializeCiphertext(const Ciphertext &ct) const;
    bool deserializeCiphertext(const QString &jsonText, Ciphertext &ct, QString *errorMessage);

    void ensureLocalKeyInit(QString *errorMessage);
    void ensurePeerKeyInit(QString *errorMessage);
    PeerUserKeys *findRecipient(const QString &targetId);
    const PeerUserKeys *findRecipient(const QString &targetId) const;
    std::unique_ptr<PeerUserKeys> buildPeerKeys(const QString &targetId,
                                                const QString &xStr,
                                                const QString &rStr,
                                                QString *errorMessage);

private:
    std::unique_ptr<CryptoContext> m_ctx;
    UserKeys m_localKeys;
    PeerUserKeys m_peerKeys;
    bool m_peerKeysLoaded = false;
    QString m_peerXStr;
    QString m_peerRStr;
    std::vector<std::unique_ptr<PeerUserKeys>> m_recipients;

    QString m_userId;
    QString m_targetId;
};
