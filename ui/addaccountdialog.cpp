#include "addaccountdialog.h"
#include "../core/configmanager.h"
#include <QMessageBox>

AddAccountDialog::AddAccountDialog(ConfigManager* configManager, QWidget *parent)
    : QDialog(parent), configManager(configManager)
{
    setWindowTitle("添加账号");
    setFixedSize(300, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    layout->addWidget(new QLabel("用户名"));
    usernameEdit = new QLineEdit();
    usernameEdit->setPlaceholderText("用户名");
    layout->addWidget(usernameEdit);
    
    layout->addWidget(new QLabel("密码"));
    passwordEdit = new QLineEdit();
    passwordEdit->setPlaceholderText("密码（离线模式可不填）");
    passwordEdit->setEchoMode(QLineEdit::Password);
    layout->addWidget(passwordEdit);
    
    offlineCheck = new QCheckBox("离线模式");
    offlineCheck->setChecked(true);
    layout->addWidget(offlineCheck);
    
    QPushButton* addBtn = new QPushButton("添加");
    connect(addBtn, &QPushButton::clicked, this, &AddAccountDialog::addAccount);
    layout->addWidget(addBtn);
    
    setLayout(layout);
}

void AddAccountDialog::addAccount()
{
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入用户名");
        return;
    }
    
    configManager->addAccount(username, password, offlineCheck->isChecked());
    QMessageBox::information(this, "成功", "账号添加成功");
    close();
}