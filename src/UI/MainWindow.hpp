#pragma once

#include <QMainWindow>

// Khai báo trước
class HeaderWidget;
class WelcomePage;
class CapturePage;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    // --- THAY ĐỔI: Thêm tham số captureFilter ---
    void showCapturePage(const QString &interfaceName, const QString &captureFilter);
    void handleOpenFileRequest();
    void handleSaveFileRequest();

private:
    HeaderWidget *header;
    QStackedWidget *stackedWidget;
    WelcomePage *welcomePage;
    CapturePage *capturePage;
};
