#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QList>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(const QString& configDir, QObject *parent = nullptr);
    
    void saveConfig();
    void saveAccounts();
    
    QVariant get(const QString& key, const QVariant& defaultValue = QVariant());
    void set(const QString& key, const QVariant& value);
    
    void addAccount(const QString& username, const QString& password = "", bool isOffline = true);
    void removeAccount(const QString& username);
    QList<QJsonObject> getAccounts();
    QJsonObject getCurrentAccount();
    void setCurrentAccount(const QString& username);
    
    QString getGameDir();
    void setGameDir(const QString& path);
    
    QString getJavaPath();
    void setJavaPath(const QString& path);
    
    QString getMemory();
    void setMemory(const QString& memory);
    
private:
    QString configDir;
    QString configFile;
    QString accountsFile;
    QJsonObject config;
    QJsonObject accounts;
    
    void loadConfig();
    void loadAccounts();
};

#endif // CONFIGMANAGER_H