#include "mainwindow.h"
#include "core/versionmanager.h"
#include "core/gamedownloader.h"
#include "core/gamelauncher.h"
#include "core/configmanager.h"
#include "ui/settingstab.h"
#include "ui/skintab.h"
#include "ui/resourcetab.h"
#include "ui/addaccountdialog.h"
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("我的世界启动器");
    setGeometry(100, 100, 900, 600);
    setMinimumSize(800, 500);
    
    dataDir = QDir::homePath() + "/.minecraft_launcher";
    QDir().mkpath(dataDir);
    
    configManager = new ConfigManager(dataDir, this);
    versionManager = new VersionManager(dataDir, this);
    gameDir = configManager->getGameDir();
    
    initUI();
    loadVersions();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUI()
{
    QWidget* centralWidget = new QWidget();
    setCentralWidget(centralWidget);
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    
    QWidget* leftPanel = new QWidget();
    leftPanel->setFixedWidth(250);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    
    leftLayout->addWidget(new QLabel("版本列表"));
    
    versionList = new QListWidget();
    versionList->setSelectionMode(QListWidget::SingleSelection);
    connect(versionList, &QListWidget::itemClicked, this, &MainWindow::onVersionSelected);
    leftLayout->addWidget(versionList);
    
    refreshBtn = new QPushButton("刷新版本");
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::refreshVersions);
    leftLayout->addWidget(refreshBtn);
    
    downloadBtn = new QPushButton("下载版本");
    downloadBtn->setEnabled(false);
    connect(downloadBtn, &QPushButton::clicked, this, &MainWindow::downloadVersion);
    leftLayout->addWidget(downloadBtn);
    
    QWidget* rightPanel = new QWidget();
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    
    QWidget* topSection = new QWidget();
    QHBoxLayout* topLayout = new QHBoxLayout(topSection);
    
    topLayout->addWidget(new QLabel("账号:"));
    
    accountCombo = new QComboBox();
    accountCombo->addItem("选择账号");
    connect(accountCombo, &QComboBox::currentTextChanged, this, &MainWindow::onAccountChanged);
    topLayout->addWidget(accountCombo);
    
    addAccountBtn = new QPushButton("添加账号");
    connect(addAccountBtn, &QPushButton::clicked, this, &MainWindow::showAddAccountDialog);
    topLayout->addWidget(addAccountBtn);
    
    topLayout->addStretch();
    rightLayout->addWidget(topSection);
    
    QWidget* middleSection = new QWidget();
    QHBoxLayout* middleLayout = new QHBoxLayout(middleSection);
    
    middleLayout->addWidget(new QLabel("内存:"));
    
    memoryCombo = new QComboBox();
    memoryCombo->addItems({"1G", "2G", "3G", "4G", "6G", "8G"});
    memoryCombo->setCurrentText(configManager->getMemory());
    middleLayout->addWidget(memoryCombo);
    
    middleLayout->addWidget(new QLabel("分辨率:"));
    
    resolutionEdit = new QLineEdit(configManager->get("resolution").toString("1280x720"));
    middleLayout->addWidget(resolutionEdit);
    
    middleLayout->addStretch();
    rightLayout->addWidget(middleSection);
    
    launchBtn = new QPushButton("启动游戏");
    launchBtn->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-size: 18px; padding: 10px; }");
    connect(launchBtn, &QPushButton::clicked, this, &MainWindow::launchGame);
    rightLayout->addWidget(launchBtn);
    
    rightLayout->addWidget(new QLabel("日志:"));
    
    logText = new QTextEdit();
    logText->setReadOnly(true);
    logText->setMaximumHeight(150);
    rightLayout->addWidget(logText);
    
    QTabWidget* tabWidget = new QTabWidget();
    
    settingsTab = new SettingsTab(configManager);
    skinTab = new SkinTab(gameDir);
    resourceTab = new ResourceTab(gameDir);
    
    tabWidget->addTab(settingsTab, "设置");
    tabWidget->addTab(skinTab, "皮肤");
    tabWidget->addTab(resourceTab, "资源");
    
    rightLayout->addWidget(tabWidget);
    
    mainLayout->addWidget(leftPanel);
    mainLayout->addWidget(rightPanel);
    
    loadAccounts();
}

void MainWindow::loadVersions()
{
    versionList->clear();
    QList<QJsonObject> versions = versionManager->loadVersions();
    
    if (versions.isEmpty()) {
        log("正在获取版本列表...");
        versions = versionManager->loadVersions();
    }
    
    QList<QJsonObject> installed = versionManager->getInstalledVersions(gameDir);
    QStringList installedNames;
    for (auto v : installed) {
        installedNames.append(v["name"].toString());
    }
    
    for (auto v : versions) {
        QString name = v["name"].toString();
        QString source = v["source"].toString();
        QString status = installedNames.contains(name) ? "已安装" : "未安装";
        
        QListWidgetItem* item = new QListWidgetItem(name + " [" + source + "] - " + status);
        item->setData(Qt::UserRole, QVariant::fromValue(v));
        versionList->addItem(item);
    }
}

void MainWindow::refreshVersions()
{
    log("正在刷新版本列表...");
    
    QList<QJsonObject> newVersions;
    QEventLoop loop;
    
    connect(versionManager, &VersionManager::versionsFetched, [&](const QList<QJsonObject>& vs) {
        newVersions = vs;
        loop.quit();
    });
    
    versionManager->fetchOfficialVersions();
    loop.exec();
    
    loadVersions();
    log("版本列表刷新完成");
}

void MainWindow::onVersionSelected(QListWidgetItem* item)
{
    downloadBtn->setEnabled(true);
    currentVersion = item->data(Qt::UserRole).value<QJsonObject>();
    log("选中版本: " + currentVersion["name"].toString());
}

void MainWindow::downloadVersion()
{
    if (currentVersion.isEmpty()) {
        log("请先选择一个版本");
        return;
    }
    
    QString versionName = currentVersion["name"].toString();
    log("开始下载版本: " + versionName);
    
    QJsonObject details = versionManager->getVersionDetails(versionName);
    if (details.isEmpty()) {
        log("无法获取版本详情: " + versionName);
        return;
    }
    
    GameDownloader* downloader = new GameDownloader(gameDir, this);
    connect(downloader, &GameDownloader::progressChanged, this, &MainWindow::onDownloadProgress);
    connect(downloader, &GameDownloader::downloadFinished, [this](bool success) {
        if (success) {
            log("下载完成");
            loadVersions();
        } else {
            log("下载失败");
        }
    });
    
    downloader->downloadVersion(details);
}

void MainWindow::onDownloadProgress(int progress, const QString& message)
{
    log("[" + QString::number(progress) + "%] " + message);
    QApplication::processEvents();
}

void MainWindow::launchGame()
{
    QListWidgetItem* currentItem = versionList->currentItem();
    if (!currentItem) {
        log("请先选择一个版本");
        return;
    }
    
    QJsonObject version = currentItem->data(Qt::UserRole).value<QJsonObject>();
    QString versionId = version["name"].toString();
    
    QString account = accountCombo->currentText();
    if (account == "选择账号") {
        log("请先添加账号");
        return;
    }
    
    QString memory = memoryCombo->currentText();
    
    log("正在启动 " + versionId + "...");
    log("玩家: " + account);
    log("内存: " + memory);
    
    GameLauncher* launcher = new GameLauncher(gameDir, configManager->getJavaPath(), this);
    
    connect(launcher, &GameLauncher::launchStarted, [this]() {
        log("游戏启动成功!");
    });
    
    connect(launcher, &GameLauncher::launchError, [this](const QString& error) {
        log("启动失败: " + error);
    });
    
    if (!launcher->launch(versionId, account, memory)) {
        log("游戏启动失败");
    }
}

void MainWindow::loadAccounts()
{
    accountCombo->clear();
    accountCombo->addItem("选择账号");
    
    QList<QJsonObject> accounts = configManager->getAccounts();
    for (auto acc : accounts) {
        accountCombo->addItem(acc["username"].toString());
    }
    
    QJsonObject current = configManager->getCurrentAccount();
    if (!current.isEmpty()) {
        int index = accountCombo->findText(current["username"].toString());
        if (index > 0) {
            accountCombo->setCurrentIndex(index);
        }
    }
}

void MainWindow::onAccountChanged(const QString& username)
{
    if (username != "选择账号") {
        configManager->setCurrentAccount(username);
    }
}

void MainWindow::showAddAccountDialog()
{
    AddAccountDialog* dialog = new AddAccountDialog(configManager, this);
    dialog->exec();
    loadAccounts();
}

void MainWindow::log(const QString& message)
{
    logText->append(message);
    logText->verticalScrollBar()->setValue(logText->verticalScrollBar()->maximum());
}