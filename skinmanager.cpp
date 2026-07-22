#include "skinmanager.h"

#include <QFile>
#include <QDir>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QImage>
#include <QCryptographicHash>

SkinManager::SkinManager(const QString& gameDir, QObject *parent)
    : QObject(parent), gameDir(gameDir)
{
    skinsDir = gameDir + "/assets/skins";
    capesDir = gameDir + "/assets/capes";
    
    QDir().mkpath(skinsDir);
    QDir().mkpath(capesDir);
    
    networkManager = new QNetworkAccessManager(this);
}

QList<QJsonObject> SkinManager::getLocalSkins()
{
    QList<QJsonObject> skins;
    QDir dir(skinsDir);
    
    QStringList files = dir.entryList(QStringList() << "*.png", QDir::Files);
    for (auto file : files) {
        QJsonObject skin;
        skin["name"] = file.left(file.lastIndexOf('.'));
        skin["path"] = dir.filePath(file);
        skin["type"] = "local";
        skins.append(skin);
    }
    
    return skins;
}

bool SkinManager::downloadSkin(const QString& url, const QString& name)
{
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, 
                  "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36");
    
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return false;
    }
    
    QString skinPath = skinsDir + "/" + name + ".png";
    QFile file(skinPath);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->deleteLater();
        return false;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    if (!validateSkin(skinPath)) {
        QFile::remove(skinPath);
        return false;
    }
    
    return true;
}

bool SkinManager::validateSkin(const QString& path)
{
    QImage img(path);
    if (img.isNull()) return false;
    
    int w = img.width();
    int h = img.height();
    
    return (w == 64 && h == 32) || (w == 64 && h == 64) || 
           (w == 128 && h == 64) || (w == 128 && h == 128);
}

bool SkinManager::setSkin(const QString& username, const QString& skinPath)
{
    if (!QFile::exists(skinPath)) return false;
    
    QString targetPath = skinsDir + "/" + username + ".png";
    return QFile::copy(skinPath, targetPath);
}

bool SkinManager::fetchMojangSkin(const QString& username)
{
    QString url = "https://api.mojang.com/users/profiles/minecraft/" + username;
    QNetworkRequest req(url);
    
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() != QNetworkReply::NoError) {
        reply->deleteLater();
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    reply->deleteLater();
    
    QString uuid = doc.object()["id"].toString();
    if (uuid.isEmpty()) return false;
    
    QString textureUrl = "https://sessionserver.mojang.com/session/minecraft/profile/" + uuid;
    QNetworkRequest req2(textureUrl);
    
    QNetworkReply* reply2 = networkManager->get(req2);
    QEventLoop loop2;
    QObject::connect(reply2, &QNetworkReply::finished, &loop2, &QEventLoop::quit);
    loop2.exec();
    
    if (reply2->error() != QNetworkReply::NoError) {
        reply2->deleteLater();
        return false;
    }
    
    QJsonDocument doc2 = QJsonDocument::fromJson(reply2->readAll());
    reply2->deleteLater();
    
    QJsonArray properties = doc2.object()["properties"].toArray();
    for (auto propItem : properties) {
        QJsonObject prop = propItem.toObject();
        if (prop["name"].toString() == "textures") {
            QString value = prop["value"].toString();
            QByteArray decoded = QByteArray::fromBase64(value.toUtf8());
            QJsonDocument texturesDoc = QJsonDocument::fromJson(decoded);
            
            QString skinUrl = texturesDoc.object()["textures"].toObject()["SKIN"].toObject()["url"].toString();
            if (!skinUrl.isEmpty()) {
                return downloadSkin(skinUrl, username);
            }
        }
    }
    
    return false;
}