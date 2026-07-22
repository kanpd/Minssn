#include "resourcetab.h"
#include "../core/resourcemanager.h"
#include <QFileDialog>
#include <QMessageBox>

ResourceTab::ResourceTab(const QString& gameDir, QWidget *parent)
    : QWidget(parent)
{
    resourceManager = new ResourceManager(gameDir, this);
    
    tabWidget = new QTabWidget();
    
    QWidget* rpTab = new QWidget();
    QVBoxLayout* rpLayout = new QVBoxLayout(rpTab);
    rpList = new QListWidget();
    
    QPushButton* rpRefreshBtn = new QPushButton("刷新");
    connect(rpRefreshBtn, &QPushButton::clicked, this, &ResourceTab::loadResourcePacks);
    
    QPushButton* rpInstallBtn = new QPushButton("安装资源包");
    connect(rpInstallBtn, &QPushButton::clicked, this, &ResourceTab::installResourcePack);
    
    QPushButton* rpRemoveBtn = new QPushButton("删除");
    connect(rpRemoveBtn, &QPushButton::clicked, this, &ResourceTab::removeResourcePack);
    
    rpLayout->addWidget(rpList);
    rpLayout->addWidget(rpRefreshBtn);
    rpLayout->addWidget(rpInstallBtn);
    rpLayout->addWidget(rpRemoveBtn);
    tabWidget->addTab(rpTab, "资源包");
    
    QWidget* modTab = new QWidget();
    QVBoxLayout* modLayout = new QVBoxLayout(modTab);
    modList = new QListWidget();
    
    QPushButton* modRefreshBtn = new QPushButton("刷新");
    connect(modRefreshBtn, &QPushButton::clicked, this, &ResourceTab::loadMods);
    
    QPushButton* modInstallBtn = new QPushButton("安装模组");
    connect(modInstallBtn, &QPushButton::clicked, this, &ResourceTab::installMod);
    
    QPushButton* modRemoveBtn = new QPushButton("删除");
    connect(modRemoveBtn, &QPushButton::clicked, this, &ResourceTab::removeMod);
    
    modLayout->addWidget(modList);
    modLayout->addWidget(modRefreshBtn);
    modLayout->addWidget(modInstallBtn);
    modLayout->addWidget(modRemoveBtn);
    tabWidget->addTab(modTab, "模组");
    
    QWidget* shaderTab = new QWidget();
    QVBoxLayout* shaderLayout = new QVBoxLayout(shaderTab);
    shaderList = new QListWidget();
    
    QPushButton* shaderRefreshBtn = new QPushButton("刷新");
    connect(shaderRefreshBtn, &QPushButton::clicked, this, &ResourceTab::loadShaders);
    
    QPushButton* shaderInstallBtn = new QPushButton("安装光影");
    connect(shaderInstallBtn, &QPushButton::clicked, this, &ResourceTab::installShader);
    
    QPushButton* shaderRemoveBtn = new QPushButton("删除");
    connect(shaderRemoveBtn, &QPushButton::clicked, this, &ResourceTab::removeShader);
    
    shaderLayout->addWidget(shaderList);
    shaderLayout->addWidget(shaderRefreshBtn);
    shaderLayout->addWidget(shaderInstallBtn);
    shaderLayout->addWidget(shaderRemoveBtn);
    tabWidget->addTab(shaderTab, "光影");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tabWidget);
    setLayout(layout);
    
    loadResourcePacks();
    loadMods();
    loadShaders();
}

void ResourceTab::loadResourcePacks()
{
    rpList->clear();
    QList<QJsonObject> packs = resourceManager->getResourcePacks();
    
    for (auto pack : packs) {
        QString name = pack["name"].toString();
        QString desc = pack["description"].toString();
        QListWidgetItem* item = new QListWidgetItem(name + " - " + desc);
        item->setData(Qt::UserRole, QVariant::fromValue(pack));
        rpList->addItem(item);
    }
}

void ResourceTab::loadMods()
{
    modList->clear();
    QList<QJsonObject> mods = resourceManager->getMods();
    
    for (auto mod : mods) {
        QListWidgetItem* item = new QListWidgetItem(mod["name"].toString());
        item->setData(Qt::UserRole, QVariant::fromValue(mod));
        modList->addItem(item);
    }
}

void ResourceTab::loadShaders()
{
    shaderList->clear();
    QList<QJsonObject> shaders = resourceManager->getShaderPacks();
    
    for (auto shader : shaders) {
        QListWidgetItem* item = new QListWidgetItem(shader["name"].toString());
        item->setData(Qt::UserRole, QVariant::fromValue(shader));
        shaderList->addItem(item);
    }
}

void ResourceTab::installResourcePack()
{
    QString path = QFileDialog::getOpenFileName(this, "选择资源包", "", "ZIP Files (*.zip)");
    if (!path.isEmpty()) {
        if (resourceManager->installResourcePack(path)) {
            QMessageBox::information(this, "成功", "资源包安装成功");
            loadResourcePacks();
        }
    }
}

void ResourceTab::installMod()
{
    QString path = QFileDialog::getOpenFileName(this, "选择模组", "", "JAR Files (*.jar)");
    if (!path.isEmpty()) {
        if (resourceManager->installMod(path)) {
            QMessageBox::information(this, "成功", "模组安装成功");
            loadMods();
        }
    }
}

void ResourceTab::installShader()
{
    QString path = QFileDialog::getOpenFileName(this, "选择光影", "", "ZIP Files (*.zip)");
    if (!path.isEmpty()) {
        if (resourceManager->installShaderPack(path)) {
            QMessageBox::information(this, "成功", "光影安装成功");
            loadShaders();
        }
    }
}

void ResourceTab::removeResourcePack()
{
    QListWidgetItem* current = rpList->currentItem();
    if (current) {
        QJsonObject pack = current->data(Qt::UserRole).value<QJsonObject>();
        if (resourceManager->removeResourcePack(pack["name"].toString())) {
            loadResourcePacks();
        }
    }
}

void ResourceTab::removeMod()
{
    QListWidgetItem* current = modList->currentItem();
    if (current) {
        QJsonObject mod = current->data(Qt::UserRole).value<QJsonObject>();
        if (resourceManager->removeMod(mod["name"].toString())) {
            loadMods();
        }
    }
}

void ResourceTab::removeShader()
{
    QListWidgetItem* current = shaderList->currentItem();
    if (current) {
        QJsonObject shader = current->data(Qt::UserRole).value<QJsonObject>();
        QString name = shader["name"].toString();
        
        QFileInfo info(shader["path"].toString());
        QString dirPath = shader["path"].toString();
        
        if (info.isDir()) {
            QDir().removeRecursively(dirPath);
        } else {
            QFile::remove(dirPath);
        }
        
        loadShaders();
    }
}