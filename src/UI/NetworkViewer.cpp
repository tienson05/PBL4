#include "NetworkViewer.hpp"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidgetItem>
#include <QDebug>
#include <QScrollBar>
#include <QStringList>
#include <net/ethernet.h>
#include <netinet/ether.h>
#include <algorithm> // Cần cho std::sort

#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

// Các cấu trúc header chưa được định nghĩa trong các file hệ thống chuẩn
struct dns_header {
    quint16 id; quint16 flags;
    quint16 num_questions; quint16 num_answers;
    quint16 num_authorities; quint16 num_additionals;
};
struct dhcp_header {
    quint8 op; quint8 htype; quint8 hlen; quint8 hops;
    quint32 xid;
    quint16 secs; quint16 flags;
    quint32 ciaddr; quint32 yiaddr;
    quint32 siaddr; quint32 giaddr;
    quint8 chaddr[16];
    quint8 sname[64];
    quint8 file[128];
    quint32 magic_cookie;
};


NetworkViewer::NetworkViewer(QWidget *parent) : QWidget(parent) {
    totalPacketCount = 0;

    // --- Khởi tạo các thành phần giao diện ---
    packetListTable = new QTableWidget(0, 7, this);
    packetListTable->setHorizontalHeaderLabels({"No.", "Time", "Source", "Destination", "Protocol", "Length", "Info"});
    packetListTable->horizontalHeader()->setStretchLastSection(true);
    packetListTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    packetListTable->setSelectionMode(QAbstractItemView::SingleSelection);
    packetListTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    packetListTable->verticalHeader()->setVisible(false);
    packetListTable->setColumnWidth(0, 60);

    packetDetailsTree = new QTreeWidget(this);
    packetDetailsTree->setHeaderLabels({"Field", "Value"});
    packetDetailsTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);

    rawDataText = new QTextEdit(this);
    rawDataText->setReadOnly(true);
    rawDataText->setFontFamily("Monospace");

    // --- THÊM MỚI: Khởi tạo thanh trạng thái ---
    statsLabel = new QLabel("Ready to capture.", this);
    statsLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statsLabel->setMinimumHeight(24);
    statsLabel->setIndent(5);

    // --- Sắp xếp giao diện ---
    bottomSplitter = new QSplitter(Qt::Horizontal);
    bottomSplitter->addWidget(packetDetailsTree);
    bottomSplitter->addWidget(rawDataText);
    bottomSplitter->setSizes({450, 250});

    mainSplitter = new QSplitter(Qt::Vertical, this);
    mainSplitter->addWidget(packetListTable);
    mainSplitter->addWidget(bottomSplitter);
    mainSplitter->setSizes({400, 300});

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(mainSplitter);
    layout->addWidget(statsLabel); // Thêm thanh trạng thái vào cuối layout
    setLayout(layout);

    connect(packetListTable, &QTableWidget::itemSelectionChanged, this, &NetworkViewer::onPacketSelected);
}

void NetworkViewer::addPacket(const PacketInfo &packet) {
    QScrollBar *vbar = packetListTable->verticalScrollBar();
    bool atBottom = (vbar->value() == vbar->maximum());

    totalPacketCount++;
    protocolCounts[packet.protocol]++;
    updateStats();

    int row = packetListTable->rowCount();
    packetListTable->insertRow(row);

    QTableWidgetItem *noItem = new QTableWidgetItem(QString::number(row + 1));
    noItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    noItem->setData(Qt::UserRole, packet.rawData);

    QList<QTableWidgetItem*> items = {
        noItem,
        new QTableWidgetItem(packet.timestamp),
        new QTableWidgetItem(packet.srcIP),
        new QTableWidgetItem(packet.dstIP),
        new QTableWidgetItem(packet.protocol),
        new QTableWidgetItem(QString::number(packet.length)),
        new QTableWidgetItem(packet.info)
    };

    QColor color;
    if (packet.protocol == "TCP" || packet.protocol == "HTTP" || packet.protocol == "TLS") color.setRgb(230, 245, 225);
    else if (packet.protocol == "UDP" || packet.protocol == "DNS" || packet.protocol == "MDNS" || packet.protocol == "DHCP") color.setRgb(225, 240, 255);
    else if (packet.protocol == "ICMP") color.setRgb(255, 240, 220);
    else if (packet.protocol == "ARP") color.setRgb(250, 240, 250);
    else color = Qt::white;

    for(int i = 0; i < items.size(); ++i) {
        items[i]->setBackground(color);
        packetListTable->setItem(row, i, items[i]);
    }
    if (atBottom) {
        packetListTable->scrollToBottom();
    }
}

// THÊM MỚI: Hàm để xóa dữ liệu cũ khi bắt đầu phiên bắt mới
void NetworkViewer::clearData() {
    packetListTable->setRowCount(0);
    packetDetailsTree->clear();
    rawDataText->clear();
    totalPacketCount = 0;
    protocolCounts.clear();
    updateStats(); // Cập nhật lại thanh trạng thái về giá trị ban đầu
}

// THÊM MỚI: Hàm để cập nhật nội dung thanh trạng thái
void NetworkViewer::updateStats() {
    if (totalPacketCount == 0) {
        statsLabel->setText("Ready to capture.");
        return;
    }

    QString statsText = QString("Total Packets: %1 | ").arg(totalPacketCount);
    QList<QPair<int, QString>> sortedProtocols;
    for (auto it = protocolCounts.constBegin(); it != protocolCounts.constEnd(); ++it) {
        sortedProtocols.append(qMakePair(it.value(), it.key()));
    }
    // Sắp xếp theo số lượng giảm dần
    std::sort(sortedProtocols.rbegin(), sortedProtocols.rend());

    QStringList protocolStrings;
    for (const auto &pair : sortedProtocols) {
        protocolStrings << QString("%1: %2").arg(pair.second).arg(pair.first);
    }

    statsText += protocolStrings.join(" | ");
    statsLabel->setText(statsText);
}


void NetworkViewer::onPacketSelected() {
    auto selectedItems = packetListTable->selectedItems();
    if (selectedItems.isEmpty()) {
        packetDetailsTree->clear();
        rawDataText->clear();
        return;
    }
    QByteArray rawData = selectedItems.first()->data(Qt::UserRole).toByteArray();
    displayPacketDetails(rawData);
    displayHexDump(rawData);
}

QString NetworkViewer::parseDnsName(const u_char* &reader, const u_char* start_of_packet) {
    QString name;
    while (*reader != 0) {
        if ((*reader & 0xC0) == 0xC0) {
            quint16 offset = ((*reader & 0x3F) << 8) | *(reader + 1);
            reader += 2;
            const u_char *new_reader = start_of_packet + offset;
            name += parseDnsName(new_reader, start_of_packet);
            return name;
        }
        if (!name.isEmpty()) name.append('.');
        int len = *reader;
        reader++;
        for (int i = 0; i < len; i++) name.append(QChar(*(reader++)));
    }
    if (*reader == 0) reader++;
    return name;
}

void NetworkViewer::displayPacketDetails(const QByteArray &rawData) {
    packetDetailsTree->clear();
    const u_char *data = (const u_char *)rawData.constData();
    int size = rawData.size();

    if (size < sizeof(ether_header)) return;

    // --- LỚP ETHERNET ---
    struct ether_header *eth = (struct ether_header *)data;
    QTreeWidgetItem *ethRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Ethernet II");
    ethRoot->addChild(new QTreeWidgetItem(QStringList() << "Destination" << QString(ether_ntoa((struct ether_addr*)eth->ether_dhost))));
    ethRoot->addChild(new QTreeWidgetItem(QStringList() << "Source" << QString(ether_ntoa((struct ether_addr*)eth->ether_shost))));
    quint16 etherType = ntohs(eth->ether_type);
    ethRoot->addChild(new QTreeWidgetItem(QStringList() << "Type" << QString("0x%1").arg(etherType, 4, 16, QChar('0'))));
    const u_char *payload = data + sizeof(ether_header);

    // --- LỚP ARP ---
    if (etherType == ETHERTYPE_ARP) {
        struct ether_arp *arp_packet = (struct ether_arp *)payload;
        QTreeWidgetItem *arpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Address Resolution Protocol");
        arpRoot->addChild(new QTreeWidgetItem(QStringList() << "Opcode" << (ntohs(arp_packet->ea_hdr.ar_op) == ARPOP_REQUEST ? "Request (1)" : "Reply (2)")));
        arpRoot->addChild(new QTreeWidgetItem(QStringList() << "Sender MAC" << QString(ether_ntoa((struct ether_addr*)arp_packet->arp_sha))));
        arpRoot->addChild(new QTreeWidgetItem(QStringList() << "Sender IP" << inet_ntoa(*(struct in_addr*)arp_packet->arp_spa)));
        arpRoot->addChild(new QTreeWidgetItem(QStringList() << "Target MAC" << QString(ether_ntoa((struct ether_addr*)arp_packet->arp_tha))));
        arpRoot->addChild(new QTreeWidgetItem(QStringList() << "Target IP" << inet_ntoa(*(struct in_addr*)arp_packet->arp_tpa)));
    }
    // --- LỚP IPV4 ---
    else if (etherType == ETHERTYPE_IP) {
        struct ip *iph = (struct ip *)payload;
        QTreeWidgetItem *ipRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Internet Protocol Version 4");
        ipRoot->addChild(new QTreeWidgetItem(QStringList() << "Source" << inet_ntoa(iph->ip_src)));
        ipRoot->addChild(new QTreeWidgetItem(QStringList() << "Destination" << inet_ntoa(iph->ip_dst)));
        const u_char* transport_payload = payload + iph->ip_hl * 4;

        // --- LỚP TCP ---
        if (iph->ip_p == IPPROTO_TCP) {
            struct tcphdr *tcph = (struct tcphdr *)transport_payload;
            QTreeWidgetItem *tcpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Transmission Control Protocol");
            quint16 srcPort = ntohs(tcph->th_sport);
            quint16 dstPort = ntohs(tcph->th_dport);
            tcpRoot->addChild(new QTreeWidgetItem(QStringList() << "Source Port" << QString::number(srcPort)));
            tcpRoot->addChild(new QTreeWidgetItem(QStringList() << "Destination Port" << QString::number(dstPort)));

            // --- LỚP HTTP ---
            if (srcPort == 80 || dstPort == 80) {
                const char *http_payload_data = (const char*)(transport_payload + tcph->th_off * 4);
                QString httpPayloadStr(http_payload_data);
                if (!httpPayloadStr.isEmpty()) {
                    QTreeWidgetItem *httpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Hypertext Transfer Protocol");
                    httpRoot->addChild(new QTreeWidgetItem(QStringList() << httpPayloadStr.section("\r\n", 0, 0)));
                }
            }
        }
        // --- LỚP UDP ---
        else if (iph->ip_p == IPPROTO_UDP) {
            struct udphdr *udph = (struct udphdr *)transport_payload;
            QTreeWidgetItem *udpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "User Datagram Protocol");
            quint16 srcPort = ntohs(udph->uh_sport);
            quint16 dstPort = ntohs(udph->uh_dport);
            udpRoot->addChild(new QTreeWidgetItem(QStringList() << "Source Port" << QString::number(srcPort)));
            udpRoot->addChild(new QTreeWidgetItem(QStringList() << "Destination Port" << QString::number(dstPort)));
            const u_char *app_payload = transport_payload + sizeof(udphdr);

            // --- LỚP DNS / MDNS ---
            if (srcPort == 53 || dstPort == 53 || srcPort == 5353 || dstPort == 5353) {
                QTreeWidgetItem *dnsRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Domain Name System");
                struct dns_header *dnsh = (struct dns_header *)app_payload;
                dnsRoot->addChild(new QTreeWidgetItem(QStringList() << "Transaction ID" << QString("0x%1").arg(ntohs(dnsh->id), 4, 16, QChar('0'))));
                dnsRoot->addChild(new QTreeWidgetItem(QStringList() << "Questions" << QString::number(ntohs(dnsh->num_questions))));
                if (ntohs(dnsh->num_questions) > 0) {
                    const u_char *qname_ptr = app_payload + sizeof(dns_header);
                    dnsRoot->addChild(new QTreeWidgetItem(QStringList() << "Query" << parseDnsName(qname_ptr, app_payload)));
                }
            }
            // --- LỚP DHCP ---
            else if ((srcPort == 67 && dstPort == 68) || (srcPort == 68 && dstPort == 67)) {
                QTreeWidgetItem *dhcpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Dynamic Host Configuration Protocol");
                struct dhcp_header *dhcph = (struct dhcp_header *)app_payload;
                const u_char* options = app_payload + sizeof(dhcp_header);
                QString messageType = "Unknown";
                int i = 0;
                while(options[i] != 255 && i < 312-4) {
                    if(options[i] == 53) {
                        int msg_type = options[i+2];
                        if(msg_type == 1) messageType = "Discover";
                        else if(msg_type == 2) messageType = "Offer";
                        else if(msg_type == 3) messageType = "Request";
                        else if(msg_type == 5) messageType = "ACK";
                        break;
                    }
                    i += options[i+1] + 2;
                }
                dhcpRoot->addChild(new QTreeWidgetItem(QStringList() << "Message Type" << messageType));
            }
        }
        // --- LỚP ICMP ---
        else if (iph->ip_p == IPPROTO_ICMP) {
            struct icmphdr *icmph = (struct icmphdr *)transport_payload;
            QTreeWidgetItem *icmpRoot = new QTreeWidgetItem(packetDetailsTree, QStringList() << "Internet Control Message Protocol");
            icmpRoot->addChild(new QTreeWidgetItem(QStringList() << "Type" << QString::number(icmph->type)));
            icmpRoot->addChild(new QTreeWidgetItem(QStringList() << "Code" << QString::number(icmph->code)));
        }
    }
    packetDetailsTree->expandAll();
}


void NetworkViewer::displayHexDump(const QByteArray &rawData) {
    // --- Hiển thị dữ liệu thô ---
    QString hexView;
    QString asciiView;
    for (int i = 0; i < rawData.size(); ++i) {
        if (i % 16 == 0) {
            if (i > 0) {
                hexView += " " + asciiView + "\n";
            }
            hexView += QString("%1: ").arg(i, 4, 16, QChar('0')).toUpper();
            asciiView.clear();
        }
        hexView += QString("%1 ").arg((uchar)rawData[i], 2, 16, QChar('0')).toUpper();
        char c = rawData[i];
        asciiView += (c >= 32 && c < 127) ? c : '.';
    }
    hexView += QString((16 - (rawData.size() % 16)) % 16 * 3 + 1, ' '); // Căn lề
    hexView += " " + asciiView;
    rawDataText->setText(hexView);
}

