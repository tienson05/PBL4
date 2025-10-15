#pragma once
#include <QMainWindow>

class QStackedWidget;
class HeaderWidget;
class WelcomePage;
class CapturePage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void showCapturePage();  // khi double-click interface

private:
    HeaderWidget *header;
    QStackedWidget *stackedWidget;
    WelcomePage *welcomePage;
    CapturePage *capturePage;
};
