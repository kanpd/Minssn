#ifndef VERSIONMANAGER_H
#define VERSIONMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QNetworkAccessManager>

class VersionManager : public QObject
{
    Q_OBJECT
public:
    explicit VersionManager(const QString& dataDir, QObject *parent = nullptr);
    
    QList<QJsonObject> loadVersions();
    void saveVersions();
    
    void fetchMcbbsVersions();
    void fetchOfficialVersions();
    
    QJsonObject getVersionDetails(const QString& versionName);
    
    QList<QJsonObject> getInstalledVersions(const QString& gameDir);
    
signals:
    void versionsFetched(const QList<QJsonObject>& versions);
    void fetchError(const QString& error);

private:
    QString dataDir;
    QString versionsFile;
    QList<QJsonObject> versions;
    QNetworkAccessManager* networkManager;
    
    QJsonObject getOfficialVersionDetails(const QJsonObject& version);
    QJsonObject getMcbbsVersionDetails(const QJsonObject& version);
    QNetworkRequest createRequest(const QString& url);
};

#endif // VERSIONMANAGER_H