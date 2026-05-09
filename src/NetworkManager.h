#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QUrl>

class NetworkManager : public QObject {
    Q_OBJECT
public:
    explicit NetworkManager(QObject *parent = nullptr);

    void setBaseUrl(const QUrl &url);
    QUrl baseUrl() const;

public slots:
    void fetchPublicParams();
    void requestPartialKey(const QString &userId);
    void uploadPublicKey(const QString &userId, const QString &xStr);
    void fetchPeerPublicKeys(const QString &userId, const QString &targetId);

signals:
    void publicParamsReady(const QString &paramStr, const QString &gStr, const QString &pPubStr);
    void partialKeyReady(const QString &userId, const QString &dStr, const QString &rStr);
    void peerPubkeysReady(const QString &userId, const QString &targetId, const QString &xStr, const QString &rStr);
    void requestSucceeded(const QString &message);
    void requestFailed(const QString &message);

private:
    QNetworkRequest makeGetRequest(const QString &pathWithQuery = QString()) const;
    QNetworkRequest makePostRequest(const QString &path) const;
    void emitParseError(const QString &prefix, const QByteArray &body);

private:
    QNetworkAccessManager m_manager;
    QUrl m_baseUrl;
};
