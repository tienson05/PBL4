#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QLabel>
// #include <QMap> // <- Không cần thiết ở đây nữa
#include "PacketInfo.hpp"

// (Khai báo trước giữ nguyên)
struct ether_header;
struct ether_arp;
struct ip;
struct tcphdr;
struct udphdr;
struct icmphdr;
struct dns_header;
struct dhcp_header;

class NetworkViewer : public QWidget {
    Q_OBJECT
public:
    explicit NetworkViewer(QWidget *parent = nullptr);

public slots:
    void addPacket(const PacketInfo &packet);
    void clearData(); // Slot để xóa dữ liệu khi bắt đầu phiên mới

    /**
     * @brief (THÊM MỚI) Slot câm, chỉ nhận chuỗi đã định dạng
     * từ StatisticsManager và hiển thị nó.
     */
    void updateStatsString(const QString &statsString);

private slots:
    void onPacketSelected();

private:
    // Hàm trợ giúp
    void displayPacketDetails(const QByteArray &rawData);
    void displayHexDump(const QByteArray &rawData);
    QString parseDnsName(const u_char* &reader, const u_char* start_of_packet);
    // void updateStats(); // <-- ĐÃ XÓA

    // Giao diện
    QSplitter *mainSplitter;
    QSplitter *bottomSplitter;
    QTableWidget *packetListTable;
    QTreeWidget *packetDetailsTree;
    QTextEdit *rawDataText;
    QLabel *statsLabel; // Vẫn giữ thanh trạng thái

    // --- CÁC BIẾN NÀY ĐÃ BỊ XÓA ---
    // int totalPacketCount;
    // QMap<QString, int> protocolCounts;
};
