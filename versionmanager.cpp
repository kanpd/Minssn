#include "versionmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QDir>

VersionManager::VersionManager(const QString& dataDir, QObject *parent)
    : QObject(parent), dataDir(dataDir)
{
    versionsFile = dataDir + "/versions.json";
    networkManager = new QNetworkAccessManager(this);
}

QList<QJsonObject> VersionManager::loadVersions()
{
    if (QFile::exists(versionsFile)) {
        QFile file(versionsFile);
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            if (doc.isArray()) {
                QJsonArray arr = doc.array();
                for (auto item : arr) {
                    if (item.isObject()) {
                        versions.append(item.toObject());
                    }
                }
            }
            file.close();
        }
    }
    return versions;
}

void VersionManager::saveVersions()
{
    QJsonArray arr;
    for (auto v : versions) {
        arr.append(v);
    }
    
    QJsonDocument doc(arr);
    QFile file(versionsFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void VersionManager::fetchOfficialVersions()
{
    QString url = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
    QNetworkReply* reply = networkManager->get(createRequest(url));
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonObject manifest = doc.object();
            QJsonArray versArray = manifest["versions"].toArray();
            
            for (auto item : versArray) {
                QJsonObject v = item.toObject();
                v["source"] = "Official";
                versions.append(v);
            }
            
            saveVersions();
            emit versionsFetched(versions);
        } else {
            emit fetchError("Failed to fetch official versions: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void VersionManager::fetchMcbbsVersions()
{
    QString url = "https://www.mcbbs.net/forum.php?mod=viewthread&tid=83422&extra=page%3D1";
    QNetworkReply* reply = networkManager->get(createRequest(url));
    
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString html = reply->readAll();
            
            QList<QJsonObject> mcbbsVersions;
            int start = 0;
            while ((start = html.indexOf("<a href=\"", start)) != -1) {
                int hrefEnd = html.indexOf("\"", start + 9);
                int textStart = html.indexOf(">", hrefEnd);
                int textEnd = html.indexOf("</a>", textStart);
                
                if (hrefEnd != -1 && textStart != -1 && textEnd != -1) {
                    QString href = html.mid(start + 9, hrefEnd - start - 9);
                    QString text = html.mid(textStart + 1, textEnd - textStart - 1);
                    
                    if (text.contains("Minecraft") || (text.contains("版本") && text.contains("."))) {
                        QJsonObject v;
                        v["name"] = text.trimmed();
                        v["url"] = href;
                        v["source"] = "MCBBS";
                        mcbbsVersions.append(v);
                    }
                }
                start = textEnd;
            }
            
            versions.append(mcbbsVersions);
            saveVersions();
            emit versionsFetched(versions);
        } else {
            emit fetchError("Failed to fetch MCBBS versions: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

QJsonObject VersionManager::getVersionDetails(const QString& versionName)
{
    for (auto v : versions) {
        if (v["name"].toString() == versionName) {
            if (v["source"].toString() == "Official") {
                return getOfficialVersionDetails(v);
            } else {
                return getMcbbsVersionDetails(v);
            }
        }
    }
    return QJsonObject();
}

QJsonObject VersionManager::getOfficialVersionDetails(const QJsonObject& version)
{
    QString url = version["url"].toString();
    QNetworkRequest req = createRequest(url);
    
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        reply->deleteLater();
        return doc.object();
    }
    reply->deleteLater();
    return QJsonObject();
}

QJsonObject VersionManager::getMcbbsVersionDetails(const QJsonObject& version)
{
    QString url = version["url"].toString();
    QNetworkRequest req = createRequest(url);
    
    QEventLoop loop;
    QNetworkReply* reply = networkManager->get(req);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QJsonObject result;
        result["html"] = QString(reply->readAll());
        reply->deleteLater();
        return result;
    }
    reply->deleteLater();
    return QJsonObject();
}

QNetworkRequest VersionManager::createRequest(const QString& url)
{
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, 
                  "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    return req;
}

QList<QJsonObject> VersionManager::getInstalledVersions(const QString& gameDir)
{
    QList<QJsonObject> installed;
    QDir versionsDir(gameDir + "/versions");
    
    if (!versionsDir.exists()) return installed;
    
    QStringList entries = versionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto entry : entries) {
        QString jsonPath = versionsDir.filePath(entry + "/" + entry + ".json");
        if (QFile::exists(jsonPath)) {
            QFile file(jsonPath);
            if (file.open(QIODevice::ReadOnly)) {
                QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
                QJsonObject info = doc.object();
                
                QJsonObject v;
                v["name"] = entry;
                v["id"] = info.value("id").toString(entry);
                v["type"] = info.value("type").toString("release");
                installed.append(v);
                
                file.close();
            }
        }
    }
    
    return installed;
}