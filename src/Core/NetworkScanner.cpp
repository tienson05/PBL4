#include "NetworkScanner.hpp"
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h> // Thêm header cho ICMP
#include <QDateTime>
#include <QDebug>
#include <QStringList>

// --- THÊM MỚI: Cấu trúc cơ bản cho MDNS Header ---
struct mdns_header {
    quint16 id;
    quint16 flags;
    quint16 num_questions;
    quint16 num_answers;
    quint16 num_authorities;
    quint16 num_additionals;
};

// Hàm trợ giúp để phân tích tên trong gói tin DNS/MDNS
QString parse_dns_name(const u_char* &reader, const u_char* start_of_packet) {
    QString name;
    while (*reader != 0) {
        if (!name.isEmpty()) {
            name.append('.');
        }
        int len = *reader;
        reader++;
        for (int i = 0; i < len; i++) {
            // SỬA LỖI: Chuyển đổi u_char thành QChar trước khi thêm vào QString
            name.append(QChar(*(reader++)));
        }
    }
    reader++; // Bỏ qua byte null cuối cùng
    return name;
}


NetworkScanner::NetworkScanner(QObject *parent)
    : QThread(parent), handle(nullptr), running(false), paused(false) {}

NetworkScanner::~NetworkScanner() {
    if (isRunning()) {
        stopCapture();
        wait();
    }
}

bool NetworkScanner::openInterface(const QString &interfaceName) {
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_live(interfaceName.toStdString().c_str(), BUFSIZ, 1, 500, errbuf);
    if (!handle) {
        qWarning() << "Error opening interface:" << interfaceName << ":" << errbuf;
        return false;
    }
    currentInterface = interfaceName;
    return true;
}

void NetworkScanner::run() {
    running = true;
    struct pcap_pkthdr *header;
    const u_char *data;

    while (running) {
        if (paused) {
            msleep(200);
            continue;
        }
        int res = pcap_next_ex(handle, &header, &data);
        if (res > 0) {
            PacketInfo packet;
            packet.timestamp = QDateTime::fromSecsSinceEpoch(header->ts.tv_sec).toString("hh:mm:ss.zzz");
            packet.length = header->len;
            packet.rawData = QByteArray(reinterpret_cast<const char*>(data), header->len);

            struct ether_header *eth = (struct ether_header *)data;
            if (ntohs(eth->ether_type) == ETHERTYPE_IP) {
                struct ip *iph = (struct ip *)(data + sizeof(struct ether_header));
                packet.srcIP = inet_ntoa(iph->ip_src);
                packet.dstIP = inet_ntoa(iph->ip_dst);

                if (iph->ip_p == IPPROTO_TCP) {
                    packet.protocol = "TCP";
                    if(header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(tcphdr)) {
                        struct tcphdr *tcph = (struct tcphdr *)(data + sizeof(ether_header) + iph->ip_hl * 4);
                        quint16 srcPort = ntohs(tcph->th_sport);
                        quint16 dstPort = ntohs(tcph->th_dport);

                        if (srcPort == 80 || dstPort == 80) packet.protocol = "HTTP";
                        else if (srcPort == 443 || dstPort == 443) packet.protocol = "TLS";
                        else if (srcPort == 53 || dstPort == 53) packet.protocol = "DNS";

                        QStringList flags;
                        if(tcph->th_flags & TH_SYN) flags << "SYN";
                        if(tcph->th_flags & TH_ACK) flags << "ACK";
                        if(tcph->th_flags & TH_FIN) flags << "FIN";
                        if(tcph->th_flags & TH_RST) flags << "RST";
                        if(tcph->th_flags & TH_PUSH) flags << "PSH";
                        if(tcph->th_flags & TH_URG) flags << "URG";
                        QString flagStr = flags.isEmpty() ? "" : QString("[%1] ").arg(flags.join(", "));

                        packet.info = QString("%1 → %2 %3Seq=%4 Ack=%5 Win=%6")
                                          .arg(srcPort).arg(dstPort).arg(flagStr)
                                          .arg(ntohl(tcph->th_seq)).arg(ntohl(tcph->th_ack)).arg(ntohs(tcph->th_win));
                    }
                } else if (iph->ip_p == IPPROTO_UDP) {
                    packet.protocol = "UDP";
                    if(header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(udphdr)) {
                        struct udphdr *udph = (struct udphdr *)(data + sizeof(ether_header) + iph->ip_hl * 4);
                        quint16 srcPort = ntohs(udph->uh_sport);
                        quint16 dstPort = ntohs(udph->uh_dport);

                        // --- NÂNG CẤP: PHÂN TÍCH GÓI TIN MDNS ---
                        if (dstPort == 5353 || srcPort == 5353) {
                            packet.protocol = "MDNS";
                            const u_char *payload = data + sizeof(ether_header) + iph->ip_hl * 4 + sizeof(udphdr);
                            struct mdns_header *mdnsh = (struct mdns_header *)payload;

                            // Kiểm tra đây có phải là một câu hỏi (query) không
                            if (ntohs(mdnsh->num_questions) > 0 && (ntohs(mdnsh->flags) & 0x8000) == 0) {
                                const u_char *qname_ptr = payload + sizeof(mdns_header);
                                QString question_name = parse_dns_name(qname_ptr, data);
                                packet.info = QString("Standard query 0x0000 PTR %1").arg(question_name);
                            } else {
                                packet.info = "MDNS Packet";
                            }
                        } else if (srcPort == 53 || dstPort == 53) {
                            packet.protocol = "DNS";
                            packet.info = QString("Standard query"); // Giả định
                        } else {
                            // SỬA LỖI: Sử dụng uh_ulen thay vì uh_len
                            packet.info = QString("%1 → %2 Len=%3").arg(srcPort).arg(dstPort).arg(ntohs(udph->uh_ulen) - sizeof(udphdr));
                        }
                    }
                } else if (iph->ip_p == IPPROTO_ICMP) {
                    packet.protocol = "ICMP";
                    if(header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(icmphdr)) {
                        struct icmphdr *icmph = (struct icmphdr *)(data + sizeof(ether_header) + iph->ip_hl * 4);
                        if(icmph->type == ICMP_ECHO) packet.info = "Echo (ping) request";
                        else if(icmph->type == ICMP_ECHOREPLY) packet.info = "Echo (ping) reply";
                        else packet.info = "ICMP Packet";
                    }
                } else {
                    packet.protocol = "Other";
                    packet.info = "IP Packet";
                }
            } else {
                // Thêm phân tích cho ARP
                if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
                    packet.protocol = "ARP";
                    packet.srcIP = "N/A";
                    packet.dstIP = "Broadcast";
                    packet.info = "Who has...? Tell...";
                } else {
                    packet.protocol = "Non-IP";
                    packet.info = "Ethernet Frame";
                }
            }
            emit packetCaptured(packet);
        } else if (res == 0) {
            continue;
        } else {
            break;
        }
    }

    if (handle) {
        pcap_close(handle);
        handle = nullptr;
    }
    emit captureStopped();
}

void NetworkScanner::stopCapture() {
    if (running) {
        running = false;
        if (handle) {
            pcap_breakloop(handle);
        }
    }
}

void NetworkScanner::setPaused(bool paused) {
    this->paused = paused;
}


// Các hàm saveToPcap và openFromPcap giữ nguyên
void NetworkScanner::saveToPcap(const QString &filePath) {
    if (!handle) return;
    pcap_dumper_t *dumper = pcap_dump_open(handle, filePath.toStdString().c_str());
    if (!dumper) return;
    // Lưu lại các gói tin bắt được (có thể mở rộng sau)
    pcap_dump_close(dumper);
}

bool NetworkScanner::openFromPcap(const QString &filePath) {
    char errbuf[PCAP_ERRBUF_SIZE];
    handle = pcap_open_offline(filePath.toStdString().c_str(), errbuf);
    return handle != nullptr;
}
QList<QPair<QString, QString>> NetworkScanner::getDevices()
{
    QList<QPair<QString, QString>> deviceList;
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Tìm tất cả các thiết bị
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        qWarning() << "Error in pcap_findalldevs:" << errbuf;
        return deviceList; // Trả về danh sách rỗng nếu có lỗi
    }

    // Duyệt qua danh sách và thêm vào deviceList
    for (pcap_if_t *d = alldevs; d != nullptr; d = d->next) {
        // Lấy tên hệ thống
        QString systemName = QString::fromLatin1(d->name);

        // Lấy mô tả, cung cấp giá trị mặc định nếu không có
        QString description = d->description
                                  ? QString::fromLatin1(d->description)
                                  : "No description available";

        // Thêm cặp dữ liệu (tên, mô tả) vào danh sách
        deviceList.append({systemName, description});
    }

    // Giải phóng bộ nhớ đã cấp phát
    pcap_freealldevs(alldevs);

    return deviceList;
}
