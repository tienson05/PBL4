#ifndef CAPTUREPAGE_HPP
#define CAPTUREPAGE_HPP

#include <QWidget>
#include <QList>
#include "PacketInfo.hpp"
#include "StatisticsManager.hpp" // <-- THÊM MỚI

// Khai báo trước (Forward declarations)
class QPushButton;
class QLabel;
class NetworkViewer;
class PacketSniffer;
class QLineEdit;
class StatisticsManager; // <-- THÊM MỚI

class CapturePage : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePage(QWidget *parent = nullptr);

    // --- CÁC HÀM ĐIỀU KHIỂN CÔNG KHAI (do MainWindow gọi) ---
    void startInitialCapture(const QString &interfaceName, const QString &captureFilter);
    void startCaptureFromFile(const QString &filePath);
    void saveCurrentCapture();

private slots:
    void onRestartCaptureClicked();
    void onStopCaptureClicked();
    void onPauseCaptureClicked();
    void onApplyFilterClicked();
    void onStatisticsClicked();

private:
    void startCaptureInternal();
    void refreshPacketView();
    bool packetMatchesFilter(const PacketInfo& packet, const QString& filter);

    // --- BIẾN THÀNH VIÊN ---
    QLabel *sourceNameLabel;
    QString currentCaptureSource;
    bool isLiveCapture;

    QPushButton *restartBtn;
    QPushButton *stopBtn;
    QPushButton *pauseBtn;
    QPushButton *statisticsBtn;
    NetworkViewer *viewer;
    PacketSniffer *scanner;
    bool isPaused;

    // --- Các widget cho thanh Filter ---
    QLineEdit *filterLineEdit;
    QPushButton *applyFilterButton;

    // --- Biến lưu trữ filter (vẫn giữ) ---
    QString m_displayFilter;
    QString m_captureFilter;

    // --- THÊM MỚI: Trình quản lý Thống kê ---
    StatisticsManager *m_statsManager;

    // Bộ nhớ đệm để lưu các gói tin, giống như WiShark
    QList<PacketInfo> capturedPackets;
};

#endif // CAPTUREPAGE_HPP
