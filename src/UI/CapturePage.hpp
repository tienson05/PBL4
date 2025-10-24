#ifndef CAPTUREPAGE_HPP
#define CAPTUREPAGE_HPP

#include <QWidget>
#include <QList>
#include "PacketInfo.hpp"

// Khai báo trước (Forward declarations) để giảm thời gian biên dịch
class QPushButton;
class QLabel;
class NetworkViewer;
class PacketSniffer;

class CapturePage : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePage(QWidget *parent = nullptr);

    // --- CÁC HÀM ĐIỀU KHIỂN CÔNG KHAI (do MainWindow gọi) ---
    void startInitialCapture(const QString &interfaceName);
    void startCaptureFromFile(const QString &filePath);
    void saveCurrentCapture();

private slots:
    void onRestartCaptureClicked();
    void onStopCaptureClicked();
    void onPauseCaptureClicked();

private:
    void startCaptureInternal();

    // --- BIẾN THÀNH VIÊN ---
    QLabel *sourceNameLabel;
    QString currentCaptureSource;
    bool isLiveCapture;

    QPushButton *restartBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    NetworkViewer *viewer;
    PacketSniffer *scanner;
    bool isPaused;

    // Bộ nhớ đệm để lưu các gói tin, giống như Wireshark
    QList<PacketInfo> capturedPackets;
};

#endif // CAPTUREPAGE_HPP
