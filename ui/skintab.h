#ifndef SKINTAB_H
#define SKINTAB_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLayout>

class SkinManager;

class SkinTab : public QWidget
{
    Q_OBJECT
public:
    explicit SkinTab(const QString& gameDir, QWidget *parent = nullptr);
    
private slots:
    void loadSkins();
    void downloadSkin();
    void setSkin();
    
private:
    SkinManager* skinManager;
    QListWidget* skinList;
};

#endif // SKINTAB_H