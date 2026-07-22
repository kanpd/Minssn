#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QTabWidget>
#include <QLayout>

class VersionManager;
class GameDownloader;
class GameLauncher;
class ConfigManager;
class SettingsTab;
class SkinTab;
class ResourceTab;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    void loadVersions();
    void refreshVersions();
    void onVersionSelected(QListWidgetItem* item);
    void downloadVersion();
    void onDownloadProgress(int progress, const QString& message);
    void launchGame();
    void loadAccounts();
    void onAccountChanged(const QString& username);
    void showAddAccountDialog();
    void log(const QString& message);
    
private:
    QString dataDir;
    QString gameDir;
    
    VersionManager* versionManager;
    ConfigManager* configManager;
    
    QListWidget* versionList;
    QPushButton* refreshBtn;
    QPushButton* downloadBtn;
    QPushButton* launchBtn;
    
    QComboBox* accountCombo;
    QPushButton* addAccountBtn;
    
    QComboBox* memoryCombo;
    QLineEdit* resolutionEdit;
    
    QTextEdit* logText;
    
    SettingsTab* settingsTab;
    SkinTab* skinTab;
    ResourceTab* resourceTab;
    
    QJsonObject currentVersion;
};

#endif // MAINWINDOW_H