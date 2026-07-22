#include "configmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>

ConfigManager::ConfigManager(const QString& configDir, QObject *parent)
    : QObject(parent), configDir(configDir)
{
    QDir().mkpath(configDir);
    configFile = configDir + "/config.json";
    accountsFile = configDir + "/accounts.json";
    
    loadConfig();
    loadAccounts();
}

void ConfigManager::loadConfig()
{
    QJsonObject defaultConfig;
    defaultConfig["game_dir"] = QDir::homePath() + "/.minecraft";
    defaultConfig["java_path"] = "";
    defaultConfig["memory"] = "2G";
    defaultConfig["theme"] = "dark";
    defaultConfig["auto_update"] = true;
    defaultConfig["show_console"] = false;
    defaultConfig["resolution"] = "1280x720";
    defaultConfig["fullscreen"] = false;
    defaultConfig["language"] = "zh_CN";
    
    if (QFile::exists(configFile)) {
        QFile file(configFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            config = doc.object();
            file.close();
            
            for (auto it = defaultConfig.begin(); it != defaultConfig.end(); ++it) {
                if (!config.contains(it.key())) {
                    config[it.key()] = it.value();
                }
            }
        } else {
            config = defaultConfig;
        }
    } else {
        config = defaultConfig;
    }
}

void ConfigManager::loadAccounts()
{
    QJsonObject defaultAccounts;
    defaultAccounts["current_account"] = QJsonValue::Null;
    
    if (QFile::exists(accountsFile)) {
        QFile file(accountsFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            accounts = doc.object();
            file.close();
        } else {
            accounts = defaultAccounts;
        }
    } else {
        accounts = defaultAccounts;
    }
}

void ConfigManager::saveConfig()
{
    QJsonDocument doc(config);
    QFile file(configFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void ConfigManager::saveAccounts()
{
    QJsonDocument doc(accounts);
    QFile file(accountsFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

QVariant ConfigManager::get(const QString& key, const QVariant& defaultValue)
{
    return config.value(key, defaultValue);
}

void ConfigManager::set(const QString& key, const QVariant& value)
{
    config[key] = value.toJsonValue();
    saveConfig();
}

void ConfigManager::addAccount(const QString& username, const QString& password, bool isOffline)
{
    QJsonArray accountArray = accounts["accounts"].toArray();
    
    QJsonObject account;
    account["username"] = username;
    account["password"] = password;
    account["is_offline"] = isOffline;
    account["access_token"] = QJsonValue::Null;
    account["uuid"] = QJsonValue::Null;
    
    accountArray.append(account);
    accounts["accounts"] = accountArray;
    accounts["current_account"] = username;
    
    saveAccounts();
}

void ConfigManager::removeAccount(const QString& username)
{
    QJsonArray accountArray = accounts["accounts"].toArray();
    QJsonArray newArray;
    
    for (auto item : accountArray) {
        if (item.toObject()["username"].toString() != username) {
            newArray.append(item);
        }
    }
    
    accounts["accounts"] = newArray;
    
    if (accounts["current_account"].toString() == username) {
        accounts["current_account"] = QJsonValue::Null;
    }
    
    saveAccounts();
}

QList<QJsonObject> ConfigManager::getAccounts()
{
    QList<QJsonObject> result;
    QJsonArray array = accounts["accounts"].toArray();
    
    for (auto item : array) {
        result.append(item.toObject());
    }
    
    return result;
}

QJsonObject ConfigManager::getCurrentAccount()
{
    QString current = accounts["current_account"].toString();
    if (current.isEmpty()) return QJsonObject();
    
    QJsonArray array = accounts["accounts"].toArray();
    for (auto item : array) {
        QJsonObject acc = item.toObject();
        if (acc["username"].toString() == current) {
            return acc;
        }
    }
    
    return QJsonObject();
}

void ConfigManager::setCurrentAccount(const QString& username)
{
    accounts["current_account"] = username;
    saveAccounts();
}

QString ConfigManager::getGameDir()
{
    return config["game_dir"].toString(QDir::homePath() + "/.minecraft");
}

void ConfigManager::setGameDir(const QString& path)
{
    config["game_dir"] = path;
    saveConfig();
}

QString ConfigManager::getJavaPath()
{
    return config["java_path"].toString();
}

void ConfigManager::setJavaPath(const QString& path)
{
    config["java_path"] = path;
    saveConfig();
}

QString ConfigManager::getMemory()
{
    return config["memory"].toString("2G");
}

void ConfigManager::setMemory(const QString& memory)
{
    config["memory"] = memory;
    saveConfig();
}