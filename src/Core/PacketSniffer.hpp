#ifndef PACKETSNIFFER_HPP
#define PACKETSNIFFER_HPP

#include <pcap.h>
#include <QThread>
#include "PacketInfo.hpp"

class PacketSniffer : public QThread {
    Q_OBJECT
public :
    // --- CONSTRUCTORS ---
    // Dùng cho bắt gói tin trực tiếp (live capture) VỚI CAPTURE FILTER
    explicit PacketSniffer(const QString &interfaceName, const QString &captureFilter, QObject *parent = nullptr);

    // Dùng cho đọc gói tin từ file (offline mode)
    explicit PacketSniffer(const QString &filePath, bool isFile, QObject *parent = nullptr);
    ~PacketSniffer();

    // --- CÁC HÀM ĐIỀU KHIỂN ---
    void setPaused(bool paused);
    void stopCapture();
    bool startSavingToFile(const QString &filePath); // <--- HÀM NÀY CHƯA ĐƯỢC DÙNG NHƯNG VẪN GIỮ LẠI

signals:
    void packetCaptured(const PacketInfo &packet);
    void captureStopped();
    void errorOccurred(const QString &errorString);

protected:
    void run() override;

private:
    /**
     * @brief Hàm nội bộ để áp dụng filter BPF.
     */
    bool applyFilter();

    pcap_t *handle;
    pcap_dumper_t *pcapDumper;

    QString sourceName; // Lưu tên interface hoặc đường dẫn file
    bool isFileMode;    // Cờ xác định chế độ hoạt động

    // --- Biến lưu trữ Capture Filter ---
    QString m_captureFilter;

    bool running;
    bool paused;
};

#endif // PACKETSNIFFER_HPP
