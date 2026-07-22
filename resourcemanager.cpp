#include "resourcemanager.h"

#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFileInfo>
#include <QStandardPaths>
#include <QtWidgets>

ResourceManager::ResourceManager(const QString& gameDir, QObject *parent)
    : QObject(parent), gameDir(gameDir)
{
    resourcePacksDir = gameDir + "/resourcepacks";
    modsDir = gameDir + "/mods";
    shaderPacksDir = gameDir + "/shaderpacks";
    
    QDir().mkpath(resourcePacksDir);
    QDir().mkpath(modsDir);
    QDir().mkpath(shaderPacksDir);
}

QList<QJsonObject> ResourceManager::getResourcePacks()
{
    QList<QJsonObject> packs;
    QDir dir(resourcePacksDir);
    
    QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (auto entry : entries) {
        QString path = dir.filePath(entry);
        bool isZip = entry.endsWith(".zip");
        
        if (isZip) {
            QZipReader zipReader(path);
            QByteArray metaData = zipReader.fileData("pack.mcmeta");
            
            if (!metaData.isEmpty()) {
                QJsonObject info = parsePackMeta(QString(metaData));
                QJsonObject pack;
                pack["name"] = entry;
                pack["path"] = path;
                pack["type"] = "resourcepack";
                pack["format"] = info["pack"].toObject()["pack_format"].toInt(0);
                pack["description"] = info["pack"].toObject()["description"].toString();
                packs.append(pack);
            }
        } else if (QFileInfo(path).isDir()) {
            QString metaPath = path + "/pack.mcmeta";
            if (QFile::exists(metaPath)) {
                QFile file(metaPath);
                if (file.open(QIODevice::ReadOnly)) {
                    QJsonObject info = parsePackMeta(QString(file.readAll()));
                    file.close();
                    
                    QJsonObject pack;
                    pack["name"] = entry;
                    pack["path"] = path;
                    pack["type"] = "resourcepack";
                    pack["format"] = info["pack"].toObject()["pack_format"].toInt(0);
                    pack["description"] = info["pack"].toObject()["description"].toString();
                    packs.append(pack);
                }
            }
        }
    }
    
    return packs;
}

QList<QJsonObject> ResourceManager::getMods()
{
    QList<QJsonObject> mods;
    QDir dir(modsDir);
    
    QStringList files = dir.entryList(QStringList() << "*.jar", QDir::Files);
    for (auto file : files) {
        QJsonObject mod;
        mod["name"] = file;
        mod["path"] = dir.filePath(file);
        mod["type"] = "mod";
        mod["size"] = QFileInfo(dir.filePath(file)).size();
        mods.append(mod);
    }
    
    return mods;
}

QList<QJsonObject> ResourceManager::getShaderPacks()
{
    QList<QJsonObject> packs;
    QDir dir(shaderPacksDir);
    
    QStringList entries = dir.entryList(QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
    for (auto entry : entries) {
        QString path = dir.filePath(entry);
        bool isZip = entry.endsWith(".zip");
        
        if (isZip || QFileInfo(path).isDir()) {
            QJsonObject pack;
            pack["name"] = entry;
            pack["path"] = path;
            pack["type"] = "shaderpack";
            packs.append(pack);
        }
    }
    
    return packs;
}

bool ResourceManager::installResourcePack(const QString& filePath)
{
    if (!QFile::exists(filePath)) return false;
    
    QFileInfo fileInfo(filePath);
    QString destPath = resourcePacksDir + "/" + fileInfo.fileName();
    
    if (fileInfo.isDir()) {
        return QDir().rename(filePath, destPath);
    } else {
        return QFile::copy(filePath, destPath);
    }
}

bool ResourceManager::installMod(const QString& filePath)
{
    if (!QFile::exists(filePath) || !filePath.endsWith(".jar")) return false;
    
    QFileInfo fileInfo(filePath);
    QString destPath = modsDir + "/" + fileInfo.fileName();
    
    return QFile::copy(filePath, destPath);
}

bool ResourceManager::installShaderPack(const QString& filePath)
{
    if (!QFile::exists(filePath)) return false;
    
    QFileInfo fileInfo(filePath);
    QString destPath = shaderPacksDir + "/" + fileInfo.fileName();
    
    if (fileInfo.isDir()) {
        return QDir().rename(filePath, destPath);
    } else {
        return QFile::copy(filePath, destPath);
    }
}

bool ResourceManager::removeResourcePack(const QString& name)
{
    QString path = resourcePacksDir + "/" + name;
    QFileInfo info(path);
    
    if (info.isDir()) {
        return QDir().removeRecursively(path);
    } else {
        return QFile::remove(path);
    }
}

bool ResourceManager::removeMod(const QString& name)
{
    QString path = modsDir + "/" + name;
    return QFile::remove(path);
}

bool ResourceManager::removeShaderPack(const QString& name)
{
    QString path = shaderPacksDir + "/" + name;
    QFileInfo info(path);
    
    if (info.isDir()) {
        return QDir().removeRecursively(path);
    } else {
        return QFile::remove(path);
    }
}

QJsonObject ResourceManager::parsePackMeta(const QString& content)
{
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
    return doc.object();
}