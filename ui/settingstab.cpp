#include "settingstab.h"
#include "../core/configmanager.h"
#include <QFileDialog>
#include <QMessageBox>

SettingsTab::SettingsTab(ConfigManager* configManager, QWidget *parent)
    : QWidget(parent), configManager(configManager)
{
    QFormLayout* layout = new QFormLayout(this);
    
    QWidget* gameDirWidget = new QWidget();
    QHBoxLayout* gameDirLayout = new QHBoxLayout(gameDirWidget);
    gameDirEdit = new QLineEdit(configManager->getGameDir());
    QPushButton* gameDirBtn = new QPushButton("浏览...");
    connect(gameDirBtn, &QPushButton::clicked, this, &SettingsTab::browseGameDir);
    gameDirLayout->addWidget(gameDirEdit);
    gameDirLayout->addWidget(gameDirBtn);
    layout->addRow("游戏目录:", gameDirWidget);
    
    QWidget* javaPathWidget = new QWidget();
    QHBoxLayout* javaPathLayout = new QHBoxLayout(javaPathWidget);
    javaPathEdit = new QLineEdit(configManager->getJavaPath());
    QPushButton* javaPathBtn = new QPushButton("浏览...");
    connect(javaPathBtn, &QPushButton::clicked, this, &SettingsTab::browseJavaPath);
    javaPathLayout->addWidget(javaPathEdit);
    javaPathLayout->addWidget(javaPathBtn);
    layout->addRow("Java路径:", javaPathWidget);
    
    memoryCombo = new QComboBox();
    memoryCombo->addItems({"1G", "2G", "3G", "4G", "6G", "8G"});
    memoryCombo->setCurrentText(configManager->getMemory());
    layout->addRow("内存大小:", memoryCombo);
    
    fullscreenCheck = new QCheckBox("全屏模式");
    fullscreenCheck->setChecked(configManager->get("fullscreen").toBool());
    layout->addRow("全屏模式:", fullscreenCheck);
    
    QPushButton* saveBtn = new QPushButton("保存设置");
    connect(saveBtn, &QPushButton::clicked, this, &SettingsTab::saveSettings);
    layout->addRow("", saveBtn);
    
    setLayout(layout);
}

void SettingsTab::browseGameDir()
{
    QString path = QFileDialog::getExistingDirectory(this, "选择游戏目录");
    if (!path.isEmpty()) {
        gameDirEdit->setText(path);
    }
}

void SettingsTab::browseJavaPath()
{
    QString path = QFileDialog::getOpenFileName(this, "选择Java可执行文件", "", "Java Executable (*.exe)");
    if (!path.isEmpty()) {
        javaPathEdit->setText(path);
    }
}

void SettingsTab::saveSettings()
{
    configManager->setGameDir(gameDirEdit->text());
    configManager->setJavaPath(javaPathEdit->text());
    configManager->setMemory(memoryCombo->currentText());
    configManager->set("fullscreen", fullscreenCheck->isChecked());
    QMessageBox::information(this, "成功", "设置已保存");
}