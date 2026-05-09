#include "NetworkManager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QUrlQuery>

NetworkManager::NetworkManager(QObject *parent)  : QObject(parent) {}

void NetworkManager::setBaseUrl(const QUrl &url) {
    m_baseUrl = url;
}

QUrl NetworkManager::baseUrl() const {
    return m_baseUrl;
}

QNetworkRequest NetworkManager::makeGetRequest(const QString &pathWithQuery) const {
    QUrl url = m_baseUrl;
    url.setPath(pathWithQuery);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return req;
}

QNetworkRequest NetworkManager::makePostRequest(const QString &path) const {
    QUrl url = m_baseUrl;
    url.setPath(path);
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    return req;
}

void NetworkManager::emitParseError(const QString &prefix, const QByteArray &body)  {
    emit requestFailed(prefix + "\nRaw body:\n" + QString::fromUtf8(body));
}

void NetworkManager::fetchPublicParams() {
    if (!m_baseUrl.isValid()) {
        emit requestFailed("服务器地址无效");
        return;
    }

    QNetworkReply *reply = m_manager.get(makeGetRequest("/get_public_params"));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        const auto doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            emitParseError("解析 /get_public_params 失败", body);
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        if (obj.value("status").toString() != "success") {
            emit requestFailed(obj.value("message").toString("获取公共参数失败"));
            reply->deleteLater();
            return;
        }

        emit publicParamsReady(
            obj.value("param_str").toString(),
            obj.value("G").toString(),
            obj.value("P_pub").toString()
        );
        emit requestSucceeded("公共参数已获取");
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError) {
        emit requestFailed("网络错误: " + reply->errorString());
    });
}

void NetworkManager::requestPartialKey(const QString &userId) {
    if (!m_baseUrl.isValid()) {
        emit requestFailed("服务器地址无效");
        return;
    }

    QUrl url = m_baseUrl;
    url.setPath("/generate_partial_key");
    QUrlQuery query;
    query.addQueryItem("user_id", userId);
    url.setQuery(query);

    QNetworkReply *reply = m_manager.get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [this, reply, userId]() {
        const QByteArray body = reply->readAll();
        const auto doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            emitParseError("解析 /generate_partial_key 失败", body);
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        if (obj.value("status").toString() != "success") {
            emit requestFailed(obj.value("message").toString("生成部分私钥失败"));
            reply->deleteLater();
            return;
        }

        emit partialKeyReady(
            userId,
            obj.value("partial_key_d").toString(),
            obj.value("partial_key_R").toString()
        );
        emit requestSucceeded("部分私钥已获取");
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError) {
        emit requestFailed("网络错误: " + reply->errorString());
    });
}

void NetworkManager::uploadPublicKey(const QString &userId, const QString &xStr) {
    if (!m_baseUrl.isValid()) {
        emit requestFailed("服务器地址无效");
        return;
    }

    QUrl url = m_baseUrl;
    url.setPath("/upload_pubkey");

    QJsonObject obj;
    obj["user_id"] = userId;
    obj["X"] = xStr;

    QNetworkReply *reply = m_manager.post(makePostRequest("/upload_pubkey"), QJsonDocument(obj).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const QByteArray body = reply->readAll();
        const auto doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            emitParseError("解析 /upload_pubkey 失败", body);
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        if (obj.value("status").toString() != "success") {
            emit requestFailed(obj.value("message").toString("上传公钥失败"));
            reply->deleteLater();
            return;
        }

        emit requestSucceeded(obj.value("message").toString("公钥已上传"));
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError) {
        emit requestFailed("网络错误: " + reply->errorString());
    });
}

void NetworkManager::fetchPeerPublicKeys(const QString &userId, const QString &targetId) {
    if (!m_baseUrl.isValid()) {
        emit requestFailed("服务器地址无效");
        return;
    }

    QJsonObject obj;
    obj["user_id"] = userId;
    obj["target_id"] = targetId;

    QNetworkReply *reply = m_manager.post(makePostRequest("/get_user_pubkeys"), QJsonDocument(obj).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, userId, targetId]() {
        const QByteArray body = reply->readAll();
        const auto doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            emitParseError("解析 /get_user_pubkeys 失败", body);
            reply->deleteLater();
            return;
        }

        const QJsonObject obj = doc.object();
        if (obj.value("status").toString() != "success") {
            emit requestFailed(obj.value("message").toString("获取对方公钥失败"));
            reply->deleteLater();
            return;
        }

        emit peerPubkeysReady(
            userId,
            targetId,
            obj.value("target_public_key_X").toString(),
            obj.value("target_public_key_R").toString()
        );
        emit requestSucceeded("对方公钥已获取");
        reply->deleteLater();
    });

    connect(reply, &QNetworkReply::errorOccurred, this, [this, reply](QNetworkReply::NetworkError) {
        emit requestFailed("网络错误: " + reply->errorString());
    });
}
