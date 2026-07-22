#include "gamedownloader.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QDir>
#include <QCryptographicHash>

GameDownloader::GameDownloader(const QString& gameDir, QObject *parent)
    : QObject(parent), gameDir(gameDir)
{
    assetsDir = gameDir + "/assets";
    librariesDir = gameDir + "/libraries";
    versionsDir = gameDir + "/versions";
    networkManager = new QNetworkAccessManager(this);
}

void GameDownloader::downloadVersion(const QJsonObject& versionInfo)
{
    QString versionId = versionInfo["id"].toString();
    QString versionDir = versionsDir + "/" + versionId;
    
    QDir().mkpath(versionDir);
    
    QString jsonPath = versionDir + "/" + versionId + ".json";
    QString jarPath = versionDir + "/" + versionId + ".jar";
    
    emit progressChanged(10, "正在下载版本JSON...");
    
    QJsonDocument doc(versionInfo);
    QFile jsonFile(jsonPath);
    if (jsonFile.open(QIODevice::WriteOnly)) {
        jsonFile.write(doc.toJson(QJsonDocument::Indented));
        jsonFile.close();
    }
    
    QJsonObject downloads = versionInfo["downloads"].toObject();
    QJsonObject client = downloads["client"].toObject();
    QString clientUrl = client["url"].toString();
    QString clientHash = client["sha1"].toString();
    
    emit progressChanged(20, "正在下载客户端JAR...");
    if (!downloadFile(clientUrl, jarPath, clientHash)) {
        emit downloadError("Failed to download client JAR");
        emit downloadFinished(false);
        return;
    }
    
    emit progressChanged(50, "正在下载依赖库...");
    downloadLibraries(versionInfo["libraries"].toArray());
    
    emit progressChanged(80, "正在下载资源文件...");
    downloadAssets(versionInfo["assetIndex"].toObject());
    
    emit progressChanged(100, "下载完成");
    emit downloadFinished(true);
}

void GameDownloader::downloadLibraries(const QJsonArray& libraries)
{
    int total = libraries.size();
    int completed = 0;
    
    for (auto libItem : libraries) {
        QJsonObject lib = libItem.toObject();
        if (!shouldDownloadLibrary(lib)) {
            completed++;
            continue;
        }
        
        QJsonObject downloads = lib["downloads"].toObject();
        QJsonObject artifact = downloads["artifact"].toObject();
        QString url = artifact["url"].toString();
        QString path = artifact["path"].toString();
        QString sha1 = artifact["sha1"].toString();
        
        if (url.isEmpty() || path.isEmpty()) {
            completed++;
            continue;
        }
        
        QString libPath = librariesDir + "/" + path;
        QDir().mkpath(QFileInfo(libPath).absolutePath());
        
        downloadFile(url, libPath, sha1);
        completed++;
        
        emit progressChanged(50 + (completed * 30) / total, "下载依赖库");
    }
}

bool GameDownloader::shouldDownloadLibrary(const QJsonObject& lib)
{
    QJsonArray rules = lib["rules"].toArray();
    QString osName = QSysInfo::productType();
    
    for (auto ruleItem : rules) {
        QJsonObject rule = ruleItem.toObject();
        QString action = rule["action"].toString("allow");
        QJsonObject osInfo = rule["os"].toObject();
        
        if (!osInfo.isEmpty()) {
            QString targetOs = osInfo["name"].toString();
            if ((targetOs == "windows" && osName == "windows") ||
                (targetOs != "windows" && osName != "windows")) {
                if (action == "disallow") return false;
            } else {
                if (action == "allow") return false;
            }
        }
    }
    
    return true;
}

void GameDownloader::downloadAssets(const QJsonObject& assetIndex)
{
    QString url = assetIndex["url"].toString();
    if (url.isEmpty()) return;
    
    QString indexId = assetIndex["id"].toString("unknown");
    QString indexPath = assetsDir + "/indexes/" + indexId + ".json";
    
    QDir().mkpath(QFileInfo(indexPath).absolutePath());
    
    downloadFile(url, indexPath);
    
    QFile indexFile(indexPath);
    if (!indexFile.open(QIODevice::ReadOnly)) return;
    
    QJsonDocument doc = QJsonDocument::fromJson(indexFile.readAll());
    indexFile.close();
    
    QJsonObject objects = doc.object()["objects"].toObject();
    int total = objects.size();
    int completed = 0;
    
    for (auto it = objects.begin(); it != objects.end(); ++it) {
        QString hash = it.value().toObject()["hash"].toString();
        QString firstTwo = hash.left(2);
        QString assetPath = assetsDir + "/objects/" + firstTwo + "/" + hash;
        
        if (QFile::exists(assetPath)) {
            completed++;
            continue;
        }
        
        QString assetUrl = "https://resources.download.minecraft.net/" + firstTwo + "/" + hash;
        QDir().mkpath(QFileInfo(assetPath).absolutePath());
        
        downloadFile(assetUrl, assetPath);
        completed++;
        
        emit progressChanged(80 + (completed * 20) / total, "下载资源文件");
    }
}

bool GameDownloader::downloadFile(const QString& url, const QString& path, const QString& expectedHash)
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
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        reply->deleteLater();
        return false;
    }
    
    file.write(reply->readAll());
    file.close();
    reply->deleteLater();
    
    if (!expectedHash.isEmpty()) {
        QString actualHash = calculateSha1(path);
        if (actualHash != expectedHash) {
            QFile::remove(path);
            return false;
        }
    }
    
    return true;
}

QString GameDownloader::calculateSha1(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return "";
    
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(&file);
    
    return hash.result().toHex();
}