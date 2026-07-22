#include "skintab.h"
#include "../core/skinmanager.h"
#include <QInputDialog>
#include <QMessageBox>

SkinTab::SkinTab(const QString& gameDir, QWidget *parent)
    : QWidget(parent)
{
    skinManager = new SkinManager(gameDir, this);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    skinList = new QListWidget();
    skinList->setSelectionMode(QListWidget::SingleSelection);
    layout->addWidget(skinList);
    
    QPushButton* loadBtn = new QPushButton("加载本地皮肤");
    connect(loadBtn, &QPushButton::clicked, this, &SkinTab::loadSkins);
    layout->addWidget(loadBtn);
    
    QPushButton* downloadBtn = new QPushButton("下载皮肤");
    connect(downloadBtn, &QPushButton::clicked, this, &SkinTab::downloadSkin);
    layout->addWidget(downloadBtn);
    
    QPushButton* setBtn = new QPushButton("应用皮肤");
    connect(setBtn, &QPushButton::clicked, this, &SkinTab::setSkin);
    layout->addWidget(setBtn);
    
    setLayout(layout);
    loadSkins();
}

void SkinTab::loadSkins()
{
    skinList->clear();
    QList<QJsonObject> skins = skinManager->getLocalSkins();
    
    for (auto skin : skins) {
        QListWidgetItem* item = new QListWidgetItem(skin["name"].toString());
        item->setData(Qt::UserRole, QVariant::fromValue(skin));
        skinList->addItem(item);
    }
}

void SkinTab::downloadSkin()
{
    bool ok;
    QString username = QInputDialog::getText(this, "下载皮肤", "输入Minecraft用户名:", QLineEdit::Normal, "", &ok);
    
    if (ok && !username.isEmpty()) {
        if (skinManager->fetchMojangSkin(username)) {
            QMessageBox::information(this, "成功", "皮肤下载成功");
            loadSkins();
        } else {
            QMessageBox::warning(this, "失败", "无法下载皮肤");
        }
    }
}

void SkinTab::setSkin()
{
    QListWidgetItem* current = skinList->currentItem();
    if (!current) {
        QMessageBox::warning(this, "警告", "请选择一个皮肤");
        return;
    }
    
    QJsonObject skin = current->data(Qt::UserRole).value<QJsonObject>();
    
    bool ok;
    QString username = QInputDialog::getText(this, "应用皮肤", "输入要应用皮肤的用户名:", QLineEdit::Normal, "", &ok);
    
    if (ok && !username.isEmpty()) {
        if (skinManager->setSkin(username, skin["path"].toString())) {
            QMessageBox::information(this, "成功", "皮肤应用成功");
        } else {
            QMessageBox::warning(this, "失败", "皮肤应用失败");
        }
    }
}