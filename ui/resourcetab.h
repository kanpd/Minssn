#ifndef RESOURCETAB_H
#define RESOURCETAB_H

#include <QWidget>
#include <QTabWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLayout>

class ResourceManager;

class ResourceTab : public QWidget
{
    Q_OBJECT
public:
    explicit ResourceTab(const QString& gameDir, QWidget *parent = nullptr);
    
private slots:
    void loadResourcePacks();
    void loadMods();
    void loadShaders();
    
    void installResourcePack();
    void installMod();
    void installShader();
    
    void removeResourcePack();
    void removeMod();
    void removeShader();
    
private:
    ResourceManager* resourceManager;
    
    QTabWidget* tabWidget;
    QListWidget* rpList;
    QListWidget* modList;
    QListWidget* shaderList;
};

#endif // RESOURCETAB_H