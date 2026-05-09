#include "MainWindow.h"
#include "NetworkManager.h"
#include "CryptoManager.h"

#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QStatusBar>
#include <QSplitter>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>
#include <QDateTime>
#include <QClipboard>

class MainWindowUiBuilder {
public:
    static QWidget *buildCentralWidget(MainWindow *window,
                                       QLineEdit **serverUrlEdit,
                                       QPushButton **connectButton,
                                       QPushButton **fetchParamsButton,
                                       QPlainTextEdit **paramsView,
                                       QLineEdit **userIdEdit,
                                       QPushButton **genLocalKeyButton,
                                       QPushButton **requestPartialKeyButton,
                                       QPushButton **uploadXButton,
                                       QLineEdit **localXEdit,
                                       QLineEdit **partialDEdit,
                                       QLineEdit **partialREdit,
                                       QLineEdit **targetIdEdit,
                                       QPushButton **fetchPeerButton,
                                       QLineEdit **peerXEdit,
                                       QLineEdit **peerREdit,
                                       QTextEdit **messageEdit,
                                       QPushButton **encryptButton,
                                       QPushButton **decryptButton,
                                       QPlainTextEdit **ciphertextView,
                                       QPlainTextEdit **plaintextView,
                                       QPlainTextEdit **logView)
    {
        auto *central = new QWidget(window);
        auto *root = new QVBoxLayout(central);

        auto *serverBox = new QGroupBox("服务器连接");
        auto *serverLayout = new QFormLayout(serverBox);
        *serverUrlEdit = new QLineEdit("http://127.0.0.1:8080");
        *connectButton = new QPushButton("保存地址");
        *fetchParamsButton = new QPushButton("获取公共参数");
        auto *serverButtonRow = new QWidget;
        auto *serverButtonLayout = new QHBoxLayout(serverButtonRow);
        serverButtonLayout->setContentsMargins(0, 0, 0, 0);
        serverButtonLayout->addWidget(*connectButton);
        serverButtonLayout->addWidget(*fetchParamsButton);
        serverLayout->addRow("服务器地址", *serverUrlEdit);
        serverLayout->addRow(serverButtonRow);

        auto *paramsBox = new QGroupBox("公共参数");
        auto *paramsLayout = new QVBoxLayout(paramsBox);
        *paramsView = new QPlainTextEdit;
        (*paramsView)->setReadOnly(true);
        paramsLayout->addWidget(*paramsView);

        auto *accountBox = new QGroupBox("本地账号");
        auto *accountLayout = new QFormLayout(accountBox);
        *userIdEdit = new QLineEdit;
        *genLocalKeyButton = new QPushButton("生成本地公钥 X");
        *requestPartialKeyButton = new QPushButton("请求部分私钥 d/R");
        *uploadXButton = new QPushButton("上传本地公钥 X");
        *localXEdit = new QLineEdit;
        *partialDEdit = new QLineEdit;
        *partialREdit = new QLineEdit;
        (*localXEdit)->setReadOnly(true);
        (*partialDEdit)->setReadOnly(true);
        (*partialREdit)->setReadOnly(true);
        auto *accountButtonRow = new QWidget;
        auto *accountButtonLayout = new QHBoxLayout(accountButtonRow);
        accountButtonLayout->setContentsMargins(0, 0, 0, 0);
        accountButtonLayout->addWidget(*genLocalKeyButton);
        accountButtonLayout->addWidget(*requestPartialKeyButton);
        accountButtonLayout->addWidget(*uploadXButton);
        accountLayout->addRow("用户 ID", *userIdEdit);
        accountLayout->addRow("本地公钥 X", *localXEdit);
        accountLayout->addRow("服务器下发 d", *partialDEdit);
        accountLayout->addRow("服务器下发 R", *partialREdit);
        accountLayout->addRow(accountButtonRow);

        auto *peerBox = new QGroupBox("对方公钥查询");
        auto *peerLayout = new QFormLayout(peerBox);
        *targetIdEdit = new QLineEdit;
        *fetchPeerButton = new QPushButton("获取对方公钥");
        *peerXEdit = new QLineEdit;
        *peerREdit = new QLineEdit;
        (*peerXEdit)->setReadOnly(true);
        (*peerREdit)->setReadOnly(true);
        peerLayout->addRow("目标用户 ID", *targetIdEdit);
        peerLayout->addRow("对方公钥 X", *peerXEdit);
        peerLayout->addRow("对方公钥 R", *peerREdit);
        peerLayout->addRow(*fetchPeerButton);

        auto *cryptoBox = new QGroupBox("加解密");
        auto *cryptoLayout = new QVBoxLayout(cryptoBox);
        *messageEdit = new QTextEdit;
        *encryptButton = new QPushButton("加密");
        *decryptButton = new QPushButton("解密");
        *ciphertextView = new QPlainTextEdit;
        *plaintextView = new QPlainTextEdit;
        (*ciphertextView)->setPlaceholderText("加密后的 JSON 密文会显示在这里");
        (*plaintextView)->setPlaceholderText("解密结果会显示在这里");
        (*ciphertextView)->setMinimumHeight(120);
        (*plaintextView)->setMinimumHeight(100);

        auto *cryptoBtnRow = new QWidget;
        auto *cryptoBtnLayout = new QHBoxLayout(cryptoBtnRow);
        cryptoBtnLayout->setContentsMargins(0, 0, 0, 0);
        cryptoBtnLayout->addWidget(*encryptButton);
        cryptoBtnLayout->addWidget(*decryptButton);

        cryptoLayout->addWidget(new QLabel("消息"));
        cryptoLayout->addWidget(*messageEdit);
        cryptoLayout->addWidget(cryptoBtnRow);
        cryptoLayout->addWidget(new QLabel("密文 JSON"));
        cryptoLayout->addWidget(*ciphertextView);
        cryptoLayout->addWidget(new QLabel("明文"));
        cryptoLayout->addWidget(*plaintextView);

        auto *logBox = new QGroupBox("日志");
        auto *logLayout = new QVBoxLayout(logBox);
        *logView = new QPlainTextEdit;
        (*logView)->setReadOnly(true);
        (*logView)->setMinimumHeight(150);
        logLayout->addWidget(*logView);

        root->addWidget(serverBox);
        root->addWidget(paramsBox);
        root->addWidget(accountBox);
        root->addWidget(peerBox);
        root->addWidget(cryptoBox);
        root->addWidget(logBox);

        return central;
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(nullptr),
      m_network(new NetworkManager(this)),
      m_crypto(new CryptoManager(this))
{
    QLineEdit *serverUrlEdit = nullptr;
    QPushButton *connectButton = nullptr;
    QPushButton *fetchParamsButton = nullptr;
    QPlainTextEdit *paramsView = nullptr;
    QLineEdit *userIdEdit = nullptr;
    QPushButton *genLocalKeyButton = nullptr;
    QPushButton *requestPartialKeyButton = nullptr;
    QPushButton *uploadXButton = nullptr;
    QLineEdit *localXEdit = nullptr;
    QLineEdit *partialDEdit = nullptr;
    QLineEdit *partialREdit = nullptr;
    QLineEdit *targetIdEdit = nullptr;
    QPushButton *fetchPeerButton = nullptr;
    QLineEdit *peerXEdit = nullptr;
    QLineEdit *peerREdit = nullptr;
    QTextEdit *messageEdit = nullptr;
    QPushButton *encryptButton = nullptr;
    QPushButton *decryptButton = nullptr;
    QPlainTextEdit *ciphertextView = nullptr;
    QPlainTextEdit *plaintextView = nullptr;
    QPlainTextEdit *logView = nullptr;

    auto *central = MainWindowUiBuilder::buildCentralWidget(
        this,
        &serverUrlEdit,
        &connectButton,
        &fetchParamsButton,
        &paramsView,
        &userIdEdit,
        &genLocalKeyButton,
        &requestPartialKeyButton,
        &uploadXButton,
        &localXEdit,
        &partialDEdit,
        &partialREdit,
        &targetIdEdit,
        &fetchPeerButton,
        &peerXEdit,
        &peerREdit,
        &messageEdit,
        &encryptButton,
        &decryptButton,
        &ciphertextView,
        &plaintextView,
        &logView
    );

    setCentralWidget(central);
    statusBar()->showMessage("客户端已启动");

    
    central->setProperty("serverUrlEdit", QVariant::fromValue<void*>(serverUrlEdit));
    central->setProperty("userIdEdit", QVariant::fromValue<void*>(userIdEdit));
    central->setProperty("targetIdEdit", QVariant::fromValue<void*>(targetIdEdit));
    central->setProperty("localXEdit", QVariant::fromValue<void*>(localXEdit));
    central->setProperty("partialDEdit", QVariant::fromValue<void*>(partialDEdit));
    central->setProperty("partialREdit", QVariant::fromValue<void*>(partialREdit));
    central->setProperty("peerXEdit", QVariant::fromValue<void*>(peerXEdit));
    central->setProperty("peerREdit", QVariant::fromValue<void*>(peerREdit));
    central->setProperty("messageEdit", QVariant::fromValue<void*>(messageEdit));
    central->setProperty("paramsView", QVariant::fromValue<void*>(paramsView));
    central->setProperty("ciphertextView", QVariant::fromValue<void*>(ciphertextView));
    central->setProperty("plaintextView", QVariant::fromValue<void*>(plaintextView));
    central->setProperty("logView", QVariant::fromValue<void*>(logView));

    // 保存服务器地址
    connect(connectButton, &QPushButton::clicked, this, [this, serverUrlEdit]() {
        m_network->setBaseUrl(QUrl(serverUrlEdit->text().trimmed()));
        appendLog(QString("服务器地址已设置为：%1").arg(serverUrlEdit->text().trimmed()));
        statusBar()->showMessage("服务器地址已设置");
    });

    connect(fetchParamsButton, &QPushButton::clicked, this, &MainWindow::onFetchParamsClicked);
    connect(genLocalKeyButton, &QPushButton::clicked, this, &MainWindow::onGenerateLocalKeyClicked);
    connect(requestPartialKeyButton, &QPushButton::clicked, this, &MainWindow::onRequestPartialKeyClicked);
    connect(uploadXButton, &QPushButton::clicked, this, &MainWindow::onUploadPublicKeyClicked);
    connect(fetchPeerButton, &QPushButton::clicked, this, &MainWindow::onFetchPeerKeysClicked);
    connect(encryptButton, &QPushButton::clicked, this, &MainWindow::onEncryptClicked);
    connect(decryptButton, &QPushButton::clicked, this, &MainWindow::onDecryptClicked);

    // 网络层 -> UI / 逻辑层
    connect(m_network, &NetworkManager::publicParamsReady, this, &MainWindow::onPublicParamsReady);
    connect(m_network, &NetworkManager::partialKeyReady, this, &MainWindow::onPartialKeyReady);
    connect(m_network, &NetworkManager::peerPubkeysReady, this, &MainWindow::onPeerPubkeysReady);
    connect(m_network, &NetworkManager::requestSucceeded, this, &MainWindow::onRequestSucceeded);
    connect(m_network, &NetworkManager::requestFailed, this, &MainWindow::onRequestFailed);

    // 密码学层 -> UI
    connect(m_crypto, &CryptoManager::cryptoInfo, this, &MainWindow::onCryptoInfo);
    connect(m_crypto, &CryptoManager::ciphertextReady, this, &MainWindow::onCiphertextReady);
    connect(m_crypto, &CryptoManager::plaintextReady, this, &MainWindow::onPlaintextReady);

    setWindowTitle("CryptoComm Client");
    resize(1100, 900);
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::serverBaseUrl() const {
    auto *central = centralWidget();
    auto *edit = static_cast<QLineEdit *>(central->property("serverUrlEdit").value<void*>());
    return edit ? edit->text().trimmed() : QString();
}

void MainWindow::setupConnections() {}

void MainWindow::appendLog(const QString &message) {
    auto *view = static_cast<QPlainTextEdit *>(centralWidget()->property("logView").value<void*>());
    if (!view) return;
    const QString line = QString("[%1] %2").arg(QDateTime::currentDateTime().toString("HH:mm:ss")).arg(message);
    view->appendPlainText(line);
}

void MainWindow::onConnectServerClicked() {
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    appendLog("已更新服务器地址");
}

void MainWindow::onFetchParamsClicked() {
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->fetchPublicParams();
    appendLog("正在请求公共参数...");
}

void MainWindow::onGenerateLocalKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit) return;
    m_crypto->setCurrentUserId(userEdit->text().trimmed());

    QString err;
    if (!m_crypto->generateLocalKeys(&err)) {
        appendLog("生成本地公钥失败：" + err);
        return;
    }

    auto *localXEdit = static_cast<QLineEdit *>(centralWidget()->property("localXEdit").value<void*>());
    if (localXEdit) {
        localXEdit->setText(m_crypto->serializeLocalPublicKey());
    }
    appendLog("本地公钥已生成");
}

void MainWindow::onRequestPartialKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit || userEdit->text().trimmed().isEmpty()) {
        appendLog("user_id 为空");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->requestPartialKey(userEdit->text().trimmed());
    appendLog("正在请求部分私钥...");
}

void MainWindow::onUploadPublicKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit || userEdit->text().trimmed().isEmpty()) {
        appendLog("user_id 为空");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());
    const QString json = m_crypto->serializeLocalPublicKey();
    if (json.isEmpty()) {
        appendLog("请先生成本地公钥 X");
        return;
    }

    const auto doc = QJsonDocument::fromJson(json.toUtf8());
    const QString xStr = doc.object().value("X").toString();
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->uploadPublicKey(userEdit->text().trimmed(), xStr);
    appendLog("正在上传本地公钥 X...");
}

void MainWindow::onFetchPeerKeysClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    auto *targetEdit = static_cast<QLineEdit *>(centralWidget()->property("targetIdEdit").value<void*>());
    if (!userEdit || !targetEdit) return;

    if (userEdit->text().trimmed().isEmpty() || targetEdit->text().trimmed().isEmpty()) {
        appendLog("user_id 或 target_id 为空");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());
    m_crypto->setCurrentTargetId(targetEdit->text().trimmed());
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->fetchPeerPublicKeys(userEdit->text().trimmed(), targetEdit->text().trimmed());
    appendLog("正在请求对方公钥...");
}

void MainWindow::onEncryptClicked() {
    auto *msgEdit = static_cast<QTextEdit *>(centralWidget()->property("messageEdit").value<void*>());
    auto *cipherView = static_cast<QPlainTextEdit *>(centralWidget()->property("ciphertextView").value<void*>());
    if (!msgEdit || !cipherView) return;

    QString err;
    const QString ciphertextJson = m_crypto->encryptMessageToJson(msgEdit->toPlainText(), &err);
    if (ciphertextJson.isEmpty()) {
        appendLog("加密失败：" + err);
        return;
    }

    cipherView->setPlainText(ciphertextJson);
    appendLog("加密完成，密文已生成");
}

void MainWindow::onDecryptClicked() {
    auto *cipherView = static_cast<QPlainTextEdit *>(centralWidget()->property("ciphertextView").value<void*>());
    auto *plainView = static_cast<QPlainTextEdit *>(centralWidget()->property("plaintextView").value<void*>());
    if (!cipherView || !plainView) return;

    QString err;
    const QString plaintext = m_crypto->decryptMessageFromJson(cipherView->toPlainText(), &err);
    if (plaintext.isEmpty()) {
        appendLog("解密失败：" + err);
        return;
    }

    plainView->setPlainText(plaintext);
    appendLog("解密完成");
}

void MainWindow::onPublicParamsReady(const QString &paramStr, const QString &gStr, const QString &pPubStr) {
    auto *paramsView = static_cast<QPlainTextEdit *>(centralWidget()->property("paramsView").value<void*>());
    if (paramsView) {
        paramsView->setPlainText(
            "param_str:\n" + paramStr + "\n\nG:\n" + gStr + "\n\nP_pub:\n" + pPubStr
        );
    }
    m_crypto->setServerParams(paramStr, gStr, pPubStr);
    appendLog("公共参数已写入本地上下文");
}

void MainWindow::onPartialKeyReady(const QString &userId, const QString &dStr, const QString &rStr) {
    auto *partialDEdit = static_cast<QLineEdit *>(centralWidget()->property("partialDEdit").value<void*>());
    auto *partialREdit = static_cast<QLineEdit *>(centralWidget()->property("partialREdit").value<void*>());
    if (partialDEdit) partialDEdit->setText(dStr);
    if (partialREdit) partialREdit->setText(rStr);

    QString err;
    if (!m_crypto->applyPartialKey(dStr, rStr, &err)) {
        appendLog("写入部分私钥失败：" + err);
        return;
    }

    appendLog(QString("用户 %1 的部分私钥已更新").arg(userId));
}

void MainWindow::onPeerPubkeysReady(const QString &userId, const QString &targetId, const QString &xStr, const QString &rStr) {
    auto *peerXEdit = static_cast<QLineEdit *>(centralWidget()->property("peerXEdit").value<void*>());
    auto *peerREdit = static_cast<QLineEdit *>(centralWidget()->property("peerREdit").value<void*>());
    if (peerXEdit) peerXEdit->setText(xStr);
    if (peerREdit) peerREdit->setText(rStr);

    m_crypto->setPeerPublicKeys(targetId, xStr, rStr);
    appendLog(QString("用户 %1 已获取 %2 的公钥").arg(userId, targetId));
}

void MainWindow::onRequestSucceeded(const QString &message) {
    appendLog(message);
    statusBar()->showMessage(message, 3000);
}

void MainWindow::onRequestFailed(const QString &message) {
    appendLog("网络错误：" + message);
    QMessageBox::warning(this, "请求失败", message);
}

void MainWindow::onCryptoInfo(const QString &message) {
    appendLog(message);
}

void MainWindow::onCiphertextReady(const QString &ciphertextJson) {
    auto *cipherView = static_cast<QPlainTextEdit *>(centralWidget()->property("ciphertextView").value<void*>());
    if (cipherView) {
        cipherView->setPlainText(ciphertextJson);
    }
}

void MainWindow::onPlaintextReady(const QString &plaintext) {
    auto *plainView = static_cast<QPlainTextEdit *>(centralWidget()->property("plaintextView").value<void*>());
    if (plainView) {
        plainView->setPlainText(plaintext);
    }
}
