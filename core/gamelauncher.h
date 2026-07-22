#ifndef GAMELAUNCHER_H
#define GAMELAUNCHER_H

#include <QObject>
#include <QString>
#include <QProcess>

class GameLauncher : public QObject
{
    Q_OBJECT
public:
    explicit GameLauncher(const QString& gameDir, const QString& javaPath = "", QObject *parent = nullptr);
    
    void setJavaPath(const QString& path);
    bool launch(const QString& versionId, const QString& username, const QString& memory = "2G");
    
    bool isRunning() const;
    void kill();
    
signals:
    void launchStarted();
    void launchError(const QString& error);
    void processFinished(int exitCode);

private:
    QString gameDir;
    QString javaPath;
    QProcess* process;
    
    QString findJava();
    QStringList getLibraryPaths(const QJsonArray& libraries);
    QString generateUuid(const QString& username);
    bool shouldUseLibrary(const QJsonObject& lib);
};

#endif // GAMELAUNCHER_H