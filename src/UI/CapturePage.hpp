#ifndef CAPTUREPAGE_HPP
#define CAPTUREPAGE_HPP

#include <QWidget>
#include <QList>
#include "PacketInfo.hpp"

// Khai báo trước (Forward declarations)
class QPushButton;
class QLabel;
class NetworkViewer;
class PacketSniffer;
class QLineEdit;

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
    void onApplyFilterClicked(); // Slot này vẫn được dùng

private:
    void startCaptureInternal();

    // --- THÊM MỚI: Logic cho Display Filter ---
    /**
     * @brief Lặp lại toàn bộ 'capturedPackets' và hiển thị lại chúng
     * trong 'viewer' dựa trên 'currentFilter'.
     */
    void refreshPacketView();

    /**
     * @brief Kiểm tra xem một gói tin có khớp với chuỗi filter không.
     * @param packet Gói tin cần kiểm tra.
     * @param filter Chuỗi filter (đã chuyển sang chữ thường).
     * @return true nếu khớp, false nếu không.
     */
    bool packetMatchesFilter(const PacketInfo& packet, const QString& filter);
    // --- KẾT THÚC PHẦN THÊM MỚI ---

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

    // --- Các widget cho thanh Filter ---
    QLineEdit *filterLineEdit;
    QPushButton *applyFilterButton;

    // --- Biến lưu trữ filter (vẫn giữ) ---
    QString m_displayFilter;
    QString m_captureFilter;

    // Bộ nhớ đệm để lưu các gói tin, giống như WiShark
    QList<PacketInfo> capturedPackets;
};

#endif // CAPTUREPAGE_HPP
