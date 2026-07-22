#ifndef GAMEDOWNLOADER_H
#define GAMEDOWNLOADER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QNetworkAccessManager>

class GameDownloader : public QObject
{
    Q_OBJECT
public:
    explicit GameDownloader(const QString& gameDir, QObject *parent = nullptr);
    
    void downloadVersion(const QJsonObject& versionInfo);
    
signals:
    void progressChanged(int progress, const QString& message);
    void downloadFinished(bool success);
    void downloadError(const QString& error);

private:
    QString gameDir;
    QString assetsDir;
    QString librariesDir;
    QString versionsDir;
    QNetworkAccessManager* networkManager;
    
    void downloadLibraries(const QJsonArray& libraries);
    void downloadAssets(const QJsonObject& assetIndex);
    bool downloadFile(const QString& url, const QString& path, const QString& expectedHash = "");
    QString calculateSha1(const QString& filePath);
    bool shouldDownloadLibrary(const QJsonObject& lib);
};

#endif // GAMEDOWNLOADER_H