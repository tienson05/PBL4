#ifndef CAPTUREPAGE_HPP
#define CAPTUREPAGE_HPP

#include <QWidget>

// Khai báo trước
class QPushButton;
class QLabel; // THÊM MỚI: Dùng để hiển thị tên interface
class NetworkViewer;
class NetworkScanner;

class CapturePage : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePage(QWidget *parent = nullptr);

    // Hàm này nhận tên interface từ MainWindow
    void startInitialCapture(const QString &interfaceName);

private slots:
    void startCapture();
    void stopCapture();
    void pauseCapture();

private:
    // XÓA BỎ: QComboBox *interfaceBox;

    // THAY THẾ: Bằng một QLabel để hiển thị và một QString để lưu trữ
    QLabel *interfaceNameLabel;
    QString currentInterfaceName;

    // Các thành phần còn lại giữ nguyên
    QPushButton *startBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    NetworkViewer *viewer;
    NetworkScanner *scanner;
    bool isPaused;
};

#endif // CAPTUREPAGE_HPP
