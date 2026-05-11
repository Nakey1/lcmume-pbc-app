#include "MainWindow.h"
#include "NetworkManager.h"
#include "CryptoManager.h"

#include <QDateTime>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QVariant>
#include <QVBoxLayout>

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
                                       QPushButton **addRecipientButton,
                                       QPushButton **removeRecipientButton,
                                       QLineEdit **peerXEdit,
                                       QLineEdit **peerREdit,
                                       QPlainTextEdit **recipientListView,
                                       QTextEdit **messageEdit,
                                       QPushButton **encryptButton,
                                       QPushButton **decryptButton,
                                       QPlainTextEdit **ciphertextView,
                                       QPlainTextEdit **plaintextView,
                                       QPlainTextEdit **logView) {
        auto *central = new QWidget(window);
        auto *root = new QVBoxLayout(central);

        auto *serverBox = new QGroupBox("Server");
        auto *serverLayout = new QFormLayout(serverBox);
        *serverUrlEdit = new QLineEdit("http://127.0.0.1:8080");
        *connectButton = new QPushButton("Save URL");
        *fetchParamsButton = new QPushButton("Fetch Params");
        auto *serverButtonRow = new QWidget;
        auto *serverButtonLayout = new QHBoxLayout(serverButtonRow);
        serverButtonLayout->setContentsMargins(0, 0, 0, 0);
        serverButtonLayout->addWidget(*connectButton);
        serverButtonLayout->addWidget(*fetchParamsButton);
        serverLayout->addRow("Server URL", *serverUrlEdit);
        serverLayout->addRow(serverButtonRow);

        auto *paramsBox = new QGroupBox("Public Params");
        auto *paramsLayout = new QVBoxLayout(paramsBox);
        *paramsView = new QPlainTextEdit;
        (*paramsView)->setReadOnly(true);
        paramsLayout->addWidget(*paramsView);

        auto *accountBox = new QGroupBox("Local Account");
        auto *accountLayout = new QFormLayout(accountBox);
        *userIdEdit = new QLineEdit;
        *genLocalKeyButton = new QPushButton("Gen Local X");
        *requestPartialKeyButton = new QPushButton("Request d/R");
        *uploadXButton = new QPushButton("Upload X");
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
        accountLayout->addRow("User ID", *userIdEdit);
        accountLayout->addRow("Local X", *localXEdit);
        accountLayout->addRow("Partial d", *partialDEdit);
        accountLayout->addRow("Partial R", *partialREdit);
        accountLayout->addRow(accountButtonRow);

        auto *peerBox = new QGroupBox("Peer And Recipients");
        auto *peerLayout = new QFormLayout(peerBox);
        *targetIdEdit = new QLineEdit;
        *fetchPeerButton = new QPushButton("Fetch Peer Key");
        *addRecipientButton = new QPushButton("Add Recipient");
        *removeRecipientButton = new QPushButton("Remove Recipient");
        *peerXEdit = new QLineEdit;
        *peerREdit = new QLineEdit;
        *recipientListView = new QPlainTextEdit;
        (*peerXEdit)->setReadOnly(true);
        (*peerREdit)->setReadOnly(true);
        (*recipientListView)->setReadOnly(true);
        (*recipientListView)->setPlaceholderText("Recipients will be listed here");
        (*recipientListView)->setMinimumHeight(90);
        auto *recipientButtonRow = new QWidget;
        auto *recipientButtonLayout = new QHBoxLayout(recipientButtonRow);
        recipientButtonLayout->setContentsMargins(0, 0, 0, 0);
        recipientButtonLayout->addWidget(*addRecipientButton);
        recipientButtonLayout->addWidget(*removeRecipientButton);
        peerLayout->addRow("Target ID", *targetIdEdit);
        peerLayout->addRow(*fetchPeerButton);
        peerLayout->addRow("Current Peer X", *peerXEdit);
        peerLayout->addRow("Current Peer R", *peerREdit);
        peerLayout->addRow(recipientButtonRow);
        peerLayout->addRow("Recipients", *recipientListView);

        auto *cryptoBox = new QGroupBox("Crypto");
        auto *cryptoLayout = new QVBoxLayout(cryptoBox);
        *messageEdit = new QTextEdit;
        *encryptButton = new QPushButton("Encrypt");
        *decryptButton = new QPushButton("Decrypt");
        *ciphertextView = new QPlainTextEdit;
        *plaintextView = new QPlainTextEdit;
        (*ciphertextView)->setPlaceholderText("Ciphertext JSON");
        (*plaintextView)->setPlaceholderText("Plaintext");
        (*ciphertextView)->setMinimumHeight(120);
        (*plaintextView)->setMinimumHeight(100);

        auto *cryptoBtnRow = new QWidget;
        auto *cryptoBtnLayout = new QHBoxLayout(cryptoBtnRow);
        cryptoBtnLayout->setContentsMargins(0, 0, 0, 0);
        cryptoBtnLayout->addWidget(*encryptButton);
        cryptoBtnLayout->addWidget(*decryptButton);

        cryptoLayout->addWidget(new QLabel("Message"));
        cryptoLayout->addWidget(*messageEdit);
        cryptoLayout->addWidget(cryptoBtnRow);
        cryptoLayout->addWidget(new QLabel("Ciphertext JSON"));
        cryptoLayout->addWidget(*ciphertextView);
        cryptoLayout->addWidget(new QLabel("Plaintext"));
        cryptoLayout->addWidget(*plaintextView);

        auto *logBox = new QGroupBox("Log");
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
      m_crypto(new CryptoManager(this)) {
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
    QPushButton *addRecipientButton = nullptr;
    QPushButton *removeRecipientButton = nullptr;
    QLineEdit *peerXEdit = nullptr;
    QLineEdit *peerREdit = nullptr;
    QPlainTextEdit *recipientListView = nullptr;
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
        &addRecipientButton,
        &removeRecipientButton,
        &peerXEdit,
        &peerREdit,
        &recipientListView,
        &messageEdit,
        &encryptButton,
        &decryptButton,
        &ciphertextView,
        &plaintextView,
        &logView
    );

    setCentralWidget(central);
    statusBar()->showMessage("Client started");

    central->setProperty("serverUrlEdit", QVariant::fromValue<void*>(serverUrlEdit));
    central->setProperty("userIdEdit", QVariant::fromValue<void*>(userIdEdit));
    central->setProperty("targetIdEdit", QVariant::fromValue<void*>(targetIdEdit));
    central->setProperty("localXEdit", QVariant::fromValue<void*>(localXEdit));
    central->setProperty("partialDEdit", QVariant::fromValue<void*>(partialDEdit));
    central->setProperty("partialREdit", QVariant::fromValue<void*>(partialREdit));
    central->setProperty("peerXEdit", QVariant::fromValue<void*>(peerXEdit));
    central->setProperty("peerREdit", QVariant::fromValue<void*>(peerREdit));
    central->setProperty("recipientListView", QVariant::fromValue<void*>(recipientListView));
    central->setProperty("messageEdit", QVariant::fromValue<void*>(messageEdit));
    central->setProperty("paramsView", QVariant::fromValue<void*>(paramsView));
    central->setProperty("ciphertextView", QVariant::fromValue<void*>(ciphertextView));
    central->setProperty("plaintextView", QVariant::fromValue<void*>(plaintextView));
    central->setProperty("logView", QVariant::fromValue<void*>(logView));

    connect(connectButton, &QPushButton::clicked, this, [this, serverUrlEdit]() {
        m_network->setBaseUrl(QUrl(serverUrlEdit->text().trimmed()));
        appendLog(QString("Server URL set: %1").arg(serverUrlEdit->text().trimmed()));
        statusBar()->showMessage("Server URL saved");
    });

    connect(fetchParamsButton, &QPushButton::clicked, this, &MainWindow::onFetchParamsClicked);
    connect(genLocalKeyButton, &QPushButton::clicked, this, &MainWindow::onGenerateLocalKeyClicked);
    connect(requestPartialKeyButton, &QPushButton::clicked, this, &MainWindow::onRequestPartialKeyClicked);
    connect(uploadXButton, &QPushButton::clicked, this, &MainWindow::onUploadPublicKeyClicked);
    connect(fetchPeerButton, &QPushButton::clicked, this, &MainWindow::onFetchPeerKeysClicked);
    connect(addRecipientButton, &QPushButton::clicked, this, &MainWindow::onAddRecipientClicked);
    connect(removeRecipientButton, &QPushButton::clicked, this, &MainWindow::onRemoveRecipientClicked);
    connect(encryptButton, &QPushButton::clicked, this, &MainWindow::onEncryptClicked);
    connect(decryptButton, &QPushButton::clicked, this, &MainWindow::onDecryptClicked);

    connect(m_network, &NetworkManager::publicParamsReady, this, &MainWindow::onPublicParamsReady);
    connect(m_network, &NetworkManager::partialKeyReady, this, &MainWindow::onPartialKeyReady);
    connect(m_network, &NetworkManager::peerPubkeysReady, this, &MainWindow::onPeerPubkeysReady);
    connect(m_network, &NetworkManager::requestSucceeded, this, &MainWindow::onRequestSucceeded);
    connect(m_network, &NetworkManager::requestFailed, this, &MainWindow::onRequestFailed);

    connect(m_crypto, &CryptoManager::cryptoInfo, this, &MainWindow::onCryptoInfo);
    connect(m_crypto, &CryptoManager::ciphertextReady, this, &MainWindow::onCiphertextReady);
    connect(m_crypto, &CryptoManager::plaintextReady, this, &MainWindow::onPlaintextReady);

    setWindowTitle("CryptoComm Client");
    resize(1100, 900);
    refreshRecipientListView();
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
    const QString line = QString("[%1] %2")
                             .arg(QDateTime::currentDateTime().toString("HH:mm:ss"))
                             .arg(message);
    view->appendPlainText(line);
}

void MainWindow::refreshRecipientListView() {
    auto *view = static_cast<QPlainTextEdit *>(centralWidget()->property("recipientListView").value<void*>());
    if (!view) return;

    const QStringList ids = m_crypto->recipientIds();
    QString text = QString("Recipients: %1").arg(ids.size());
    if (!ids.isEmpty()) {
        text += "\n";
        for (const QString &id : ids) {
            text += "- " + id + "\n";
        }
        text.chop(1);
    }
    view->setPlainText(text);
}

void MainWindow::onConnectServerClicked() {
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    appendLog("Server URL refreshed");
}

void MainWindow::onFetchParamsClicked() {
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->fetchPublicParams();
    appendLog("Fetching public params...");
}

void MainWindow::onGenerateLocalKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit) return;

    m_crypto->setCurrentUserId(userEdit->text().trimmed());

    QString err;
    if (!m_crypto->generateLocalKeys(&err)) {
        appendLog("Generate local key failed: " + err);
        return;
    }

    auto *localXEdit = static_cast<QLineEdit *>(centralWidget()->property("localXEdit").value<void*>());
    if (localXEdit) {
        localXEdit->setText(m_crypto->serializeLocalPublicKey());
    }
    appendLog("Local key generated");
}

void MainWindow::onRequestPartialKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit || userEdit->text().trimmed().isEmpty()) {
        appendLog("user_id is empty");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->requestPartialKey(userEdit->text().trimmed());
    appendLog("Requesting partial key...");
}

void MainWindow::onUploadPublicKeyClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    if (!userEdit || userEdit->text().trimmed().isEmpty()) {
        appendLog("user_id is empty");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());

    const QString json = m_crypto->serializeLocalPublicKey();
    if (json.isEmpty()) {
        appendLog("Generate local X first");
        return;
    }

    const auto doc = QJsonDocument::fromJson(json.toUtf8());
    const QString xStr = doc.object().value("X").toString();
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->uploadPublicKey(userEdit->text().trimmed(), xStr);
    appendLog("Uploading local X...");
}

void MainWindow::onFetchPeerKeysClicked() {
    auto *userEdit = static_cast<QLineEdit *>(centralWidget()->property("userIdEdit").value<void*>());
    auto *targetEdit = static_cast<QLineEdit *>(centralWidget()->property("targetIdEdit").value<void*>());
    if (!userEdit || !targetEdit) return;

    if (userEdit->text().trimmed().isEmpty() || targetEdit->text().trimmed().isEmpty()) {
        appendLog("user_id or target_id is empty");
        return;
    }

    m_crypto->setCurrentUserId(userEdit->text().trimmed());
    m_crypto->setCurrentTargetId(targetEdit->text().trimmed());
    m_network->setBaseUrl(QUrl(serverBaseUrl()));
    m_network->fetchPeerPublicKeys(userEdit->text().trimmed(), targetEdit->text().trimmed());
    appendLog("Fetching peer keys...");
}

void MainWindow::onAddRecipientClicked() {
    QString err;
    if (!m_crypto->addCurrentPeerToRecipients(&err)) {
        appendLog("Add recipient failed: " + err);
        return;
    }

    refreshRecipientListView();
    appendLog(QString("Recipient added: %1").arg(m_crypto->currentTargetId()));
}

void MainWindow::onRemoveRecipientClicked() {
    auto *targetEdit = static_cast<QLineEdit *>(centralWidget()->property("targetIdEdit").value<void*>());
    if (!targetEdit) return;

    const QString targetId = targetEdit->text().trimmed();
    if (targetId.isEmpty()) {
        appendLog("target_id is empty");
        return;
    }

    QString err;
    if (!m_crypto->removeRecipient(targetId, &err)) {
        appendLog("Remove recipient failed: " + err);
        return;
    }

    refreshRecipientListView();
    appendLog(QString("Recipient removed: %1").arg(targetId));
}

void MainWindow::onEncryptClicked() {
    auto *msgEdit = static_cast<QTextEdit *>(centralWidget()->property("messageEdit").value<void*>());
    auto *cipherView = static_cast<QPlainTextEdit *>(centralWidget()->property("ciphertextView").value<void*>());
    if (!msgEdit || !cipherView) return;

    QString err;
    const QString ciphertextJson = m_crypto->encryptMessageToJson(msgEdit->toPlainText(), &err);
    if (ciphertextJson.isEmpty()) {
        appendLog("Encrypt failed: " + err);
        return;
    }

    cipherView->setPlainText(ciphertextJson);
    appendLog("Encrypt done");
}

void MainWindow::onDecryptClicked() {
    auto *cipherView = static_cast<QPlainTextEdit *>(centralWidget()->property("ciphertextView").value<void*>());
    auto *plainView = static_cast<QPlainTextEdit *>(centralWidget()->property("plaintextView").value<void*>());
    if (!cipherView || !plainView) return;

    QString err;
    const QString plaintext = m_crypto->decryptMessageFromJson(cipherView->toPlainText(), &err);
    if (plaintext.isEmpty()) {
        appendLog("Decrypt failed: " + err);
        return;
    }

    plainView->setPlainText(plaintext);
    appendLog("Decrypt done");
}

void MainWindow::onPublicParamsReady(const QString &paramStr, const QString &gStr, const QString &pPubStr) {
    auto *paramsView = static_cast<QPlainTextEdit *>(centralWidget()->property("paramsView").value<void*>());
    if (paramsView) {
        paramsView->setPlainText(
            "param_str:\n" + paramStr + "\n\nG:\n" + gStr + "\n\nP_pub:\n" + pPubStr
        );
    }
    m_crypto->setServerParams(paramStr, gStr, pPubStr);
    refreshRecipientListView();
    appendLog("Public params loaded");
}

void MainWindow::onPartialKeyReady(const QString &userId, const QString &dStr, const QString &rStr) {
    auto *partialDEdit = static_cast<QLineEdit *>(centralWidget()->property("partialDEdit").value<void*>());
    auto *partialREdit = static_cast<QLineEdit *>(centralWidget()->property("partialREdit").value<void*>());
    if (partialDEdit) partialDEdit->setText(dStr);
    if (partialREdit) partialREdit->setText(rStr);

    QString err;
    if (!m_crypto->applyPartialKey(dStr, rStr, &err)) {
        appendLog("Apply partial key failed: " + err);
        return;
    }

    appendLog(QString("Partial key updated for %1").arg(userId));
}

void MainWindow::onPeerPubkeysReady(const QString &userId, const QString &targetId, const QString &xStr, const QString &rStr) {
    auto *peerXEdit = static_cast<QLineEdit *>(centralWidget()->property("peerXEdit").value<void*>());
    auto *peerREdit = static_cast<QLineEdit *>(centralWidget()->property("peerREdit").value<void*>());
    if (peerXEdit) peerXEdit->setText(xStr);
    if (peerREdit) peerREdit->setText(rStr);

    m_crypto->setPeerPublicKeys(targetId, xStr, rStr);
    appendLog(QString("Peer keys ready: user=%1 target=%2").arg(userId, targetId));
}

void MainWindow::onRequestSucceeded(const QString &message) {
    appendLog(message);
    statusBar()->showMessage(message, 3000);
}

void MainWindow::onRequestFailed(const QString &message) {
    appendLog("Network error: " + message);
    QMessageBox::warning(this, "Request failed", message);
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
