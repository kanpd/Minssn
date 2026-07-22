#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QLayout>

class ConfigManager;

class AddAccountDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AddAccountDialog(ConfigManager* configManager, QWidget *parent = nullptr);
    
private slots:
    void addAccount();
    
private:
    ConfigManager* configManager;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QCheckBox* offlineCheck;
};

#endif // ADDACCOUNTDIALOG_H