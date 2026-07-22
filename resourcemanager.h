#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QString>
#include <QList>
#include <QJsonObject>

class ResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit ResourceManager(const QString& gameDir, QObject *parent = nullptr);
    
    QList<QJsonObject> getResourcePacks();
    QList<QJsonObject> getMods();
    QList<QJsonObject> getShaderPacks();
    
    bool installResourcePack(const QString& filePath);
    bool installMod(const QString& filePath);
    bool installShaderPack(const QString& filePath);
    
    bool removeResourcePack(const QString& name);
    bool removeMod(const QString& name);
    bool removeShaderPack(const QString& name);

private:
    QString gameDir;
    QString resourcePacksDir;
    QString modsDir;
    QString shaderPacksDir;
    
    QJsonObject parsePackMeta(const QString& content);
};

#endif // RESOURCEMANAGER_H