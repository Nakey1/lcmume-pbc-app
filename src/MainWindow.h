#pragma once
#include <QMainWindow>
#include <memory>

QT_BEGIN_NAMESPACE
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QTextEdit;
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class NetworkManager;
class CryptoManager;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onConnectServerClicked();
    void onFetchParamsClicked();
    void onGenerateLocalKeyClicked();
    void onRequestPartialKeyClicked();
    void onUploadPublicKeyClicked();
    void onFetchPeerKeysClicked();
    void onAddRecipientClicked();
    void onRemoveRecipientClicked();
    void onEncryptClicked();
    void onDecryptClicked();

    void appendLog(const QString &message);

    void onPublicParamsReady(const QString &paramStr, const QString &gStr, const QString &pPubStr);
    void onPartialKeyReady(const QString &userId, const QString &dStr, const QString &rStr);
    void onPeerPubkeysReady(const QString &userId, const QString &targetId, const QString &xStr, const QString &rStr);
    void onRequestSucceeded(const QString &message);
    void onRequestFailed(const QString &message);

    void onCryptoInfo(const QString &message);
    void onCiphertextReady(const QString &ciphertextJson);
    void onPlaintextReady(const QString &plaintext);

private:
    void setupConnections();
    QString serverBaseUrl() const;
    void refreshRecipientListView();

private:
    Ui::MainWindow *ui = nullptr;
    NetworkManager *m_network = nullptr;
    CryptoManager *m_crypto = nullptr;
};
