#ifndef SETTINGSTAB_H
#define SETTINGSTAB_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLayout>

class ConfigManager;

class SettingsTab : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsTab(ConfigManager* configManager, QWidget *parent = nullptr);
    
private slots:
    void browseGameDir();
    void browseJavaPath();
    void saveSettings();
    
private:
    ConfigManager* configManager;
    QLineEdit* gameDirEdit;
    QLineEdit* javaPathEdit;
    QComboBox* memoryCombo;
    QCheckBox* fullscreenCheck;
};

#endif // SETTINGSTAB_H