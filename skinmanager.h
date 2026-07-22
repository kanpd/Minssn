#ifndef SKINMANAGER_H
#define SKINMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>
#include <QNetworkAccessManager>

class SkinManager : public QObject
{
    Q_OBJECT
public:
    explicit SkinManager(const QString& gameDir, QObject *parent = nullptr);
    
    QList<QJsonObject> getLocalSkins();
    bool downloadSkin(const QString& url, const QString& name);
    bool setSkin(const QString& username, const QString& skinPath);
    bool fetchMojangSkin(const QString& username);
    
signals:
    void skinDownloaded(const QString& name, bool success);
    void error(const QString& message);

private:
    QString gameDir;
    QString skinsDir;
    QString capesDir;
    QNetworkAccessManager* networkManager;
    
    bool validateSkin(const QString& path);
};

#endif // SKINMANAGER_H