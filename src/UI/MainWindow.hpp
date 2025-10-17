#pragma once

#include <QMainWindow> // THÊM MỚI: Dòng quan trọng nhất để sửa lỗi

// Khai báo trước để giảm thời gian biên dịch
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
    void showCapturePage(const QString &interfaceName);
    void handleOpenFileRequest();
    void handleSaveFileRequest();

private:
    HeaderWidget *header;
    QStackedWidget *stackedWidget;
    WelcomePage *welcomePage;
    CapturePage *capturePage;
};
