#pragma once

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>

class HeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HeaderWidget(QWidget *parent = nullptr);

private slots:
    void minimizeRequested();
    void maximizeRequested();
    void closeRequested();
    void onFileNew();
    void onFileOpen();
    void onEditCut();
    void onEditCopy();

private:
    void setupTitleBar(QWidget *parent, QVBoxLayout *mainLayout);
    void setupMenuBar(QWidget *parent, QVBoxLayout *mainLayout);
};
