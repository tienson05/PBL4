#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QLabel> // Thêm header cho thanh trạng thái
#include <QMap>   // Thêm header để lưu trữ số lượng gói tin
#include "PacketInfo.hpp"

// Forward declarations for protocol headers to keep this file clean
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

private slots:
    void onPacketSelected();

private:
    // Hàm trợ giúp
    void displayPacketDetails(const QByteArray &rawData);
    void displayHexDump(const QByteArray &rawData);
    QString parseDnsName(const u_char* &reader, const u_char* start_of_packet);
    void updateStats(); // Hàm cập nhật thanh trạng thái

    // Giao diện
    QSplitter *mainSplitter;
    QSplitter *bottomSplitter;
    QTableWidget *packetListTable;
    QTreeWidget *packetDetailsTree;
    QTextEdit *rawDataText;
    QLabel *statsLabel; // Thanh trạng thái hiển thị thống kê

    // Dữ liệu thống kê
    int totalPacketCount;
    QMap<QString, int> protocolCounts;
};

