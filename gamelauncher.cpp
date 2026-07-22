#include "gamelauncher.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDir>
#include <QCryptographicHash>
#include <QByteArray>

GameLauncher::GameLauncher(const QString& gameDir, const QString& javaPath, QObject *parent)
    : QObject(parent), gameDir(gameDir), javaPath(javaPath), process(nullptr)
{
    if (this->javaPath.isEmpty()) {
        this->javaPath = findJava();
    }
}

void GameLauncher::setJavaPath(const QString& path)
{
    if (QFile::exists(path)) {
        javaPath = path;
    }
}

bool GameLauncher::launch(const QString& versionId, const QString& username, const QString& memory)
{
    QString versionDir = gameDir + "/versions/" + versionId;
    QString jsonPath = versionDir + "/" + versionId + ".json";
    QString jarPath = versionDir + "/" + versionId + ".jar";
    
    if (!QFile::exists(jsonPath) || !QFile::exists(jarPath)) {
        emit launchError("Version files not found: " + versionId);
        return false;
    }
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit launchError("Failed to open version file");
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    QJsonObject versionInfo = doc.object();
    
    QStringList libraries = getLibraryPaths(versionInfo["libraries"].toArray());
    QString classpath = libraries.join(";") + ";" + jarPath;
    
    QString mainClass = versionInfo["mainClass"].toString("net.minecraft.client.main.Main");
    QString assetIndexId = versionInfo["assetIndex"].toObject()["id"].toString(versionId);
    
    QStringList args;
    args << javaPath;
    args << "-Xmx" + memory;
    args << "-Xms" + memory;
    args << "-Djava.library.path=" + versionDir + "/natives";
    args << "-cp" << classpath;
    args << mainClass;
    args << "--username" << username;
    args << "--version" << versionId;
    args << "--gameDir" << gameDir;
    args << "--assetsDir" << gameDir + "/assets";
    args << "--assetIndex" << assetIndexId;
    args << "--uuid" << generateUuid(username);
    args << "--accessToken" << "0";
    args << "--userType" << "mojang";
    
    process = new QProcess(this);
    process->setWorkingDirectory(gameDir);
    
    connect(process, &QProcess::started, this, &GameLauncher::launchStarted);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &GameLauncher::processFinished);
    connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        emit launchError("Process error: " + QVariant::fromValue(error).toString());
    });
    
    process->start(args.join(" "));
    
    return true;
}

bool GameLauncher::isRunning() const
{
    return process != nullptr && process->state() == QProcess::Running;
}

void GameLauncher::kill()
{
    if (process) {
        process->kill();
        process->deleteLater();
        process = nullptr;
    }
}

QString GameLauncher::findJava()
{
    QString javaHome = qgetenv("JAVA_HOME");
    if (!javaHome.isEmpty()) {
        QString path = javaHome + "/bin/java.exe";
        if (QFile::exists(path)) return path;
    }
    
    QStringList commonPaths = {
        "C:/Program Files/Java/jre1.8.0_301/bin/java.exe",
        "C:/Program Files/Java/jdk1.8.0_301/bin/java.exe",
        "C:/Program Files (x86)/Java/jre1.8.0_301/bin/java.exe"
    };
    
    for (auto path : commonPaths) {
        if (QFile::exists(path)) return path;
    }
    
    return "java";
}

QStringList GameLauncher::getLibraryPaths(const QJsonArray& libraries)
{
    QStringList paths;
    QString libDir = gameDir + "/libraries";
    
    for (auto libItem : libraries) {
        QJsonObject lib = libItem.toObject();
        if (!shouldUseLibrary(lib)) continue;
        
        QJsonObject downloads = lib["downloads"].toObject();
        QJsonObject artifact = downloads["artifact"].toObject();
        QString path = artifact["path"].toString();
        
        if (!path.isEmpty()) {
            QString fullPath = libDir + "/" + path;
            if (QFile::exists(fullPath)) {
                paths.append(fullPath);
            }
        }
    }
    
    return paths;
}

bool GameLauncher::shouldUseLibrary(const QJsonObject& lib)
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

QString GameLauncher::generateUuid(const QString& username)
{
    QString namespaceStr = "6ba7b811-9dad-11d1-80b4-00c04fd430c8";
    QString input = namespaceStr + username;
    
    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(input.toUtf8());
    QByteArray result = hash.result();
    
    QString hex = result.toHex();
    return QString("%1-%2-%3-%4-%5")
            .arg(hex.left(8))
            .arg(hex.mid(8, 4))
            .arg(hex.mid(12, 4))
            .arg(hex.mid(16, 4))
            .arg(hex.mid(20));
}