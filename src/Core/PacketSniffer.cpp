#include "PacketSniffer.hpp"

#include <QDateTime>

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

// --- Triển khai các CONSTRUCTOR ---
PacketSniffer::PacketSniffer(const QString &interfaceName, QObject *parent)
    : QThread(parent),
    handle(nullptr),
    pcapDumper(nullptr),
    sourceName(interfaceName),
    isFileMode(false),
    running(false),
    paused(false)
{}

PacketSniffer::PacketSniffer(const QString &filePath, bool isFile, QObject *parent)
    : QThread(parent),
    handle(nullptr),
    pcapDumper(nullptr),
    sourceName(filePath),
    isFileMode(true),
    running(false),
    paused(false)
{}

PacketSniffer::~PacketSniffer() {
    if (isRunning()) {
        stopCapture();
        wait();
    }
    if (pcapDumper) {
        pcap_dump_close(pcapDumper);
    }
}

// --- RUN FUNCTION ---
void PacketSniffer::run() {
    running = true;
    char errbuf[PCAP_ERRBUF_SIZE];

    if (isFileMode) {
        handle = pcap_open_offline(sourceName.toStdString().c_str(), errbuf);
    } else {
        handle = pcap_open_live(sourceName.toStdString().c_str(), BUFSIZ, 1, 500, errbuf);
    }

    if (!handle) {
        emit errorOccurred(QString("Error opening source: %1").arg(errbuf));
        return;
    }
    struct pcap_pkthdr *header;
    const u_char *data;

    while (running) {
        if (paused) { msleep(100); continue; }

        int res = pcap_next_ex(handle, &header, &data);
        if (res > 0) {
            if (pcapDumper) {
                pcap_dump(reinterpret_cast<u_char*>(pcapDumper), header, data);
            }

            // ### LOGIC PHÂN TÍCH GÓI TIN CHI TIẾT ĐẦY ĐỦ ###
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
                    if (header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(tcphdr)) {
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
                        QString flagStr = flags.isEmpty() ? "" : QString("[%1] ").arg(flags.join(", "));

                        packet.info = QString("%1 → %2 %3Seq=%4 Ack=%5 Win=%6")
                                          .arg(srcPort).arg(dstPort).arg(flagStr)
                                          .arg(ntohl(tcph->th_seq)).arg(ntohl(tcph->th_ack)).arg(ntohs(tcph->th_win));
                    }
                } else if (iph->ip_p == IPPROTO_UDP) {
                    packet.protocol = "UDP";
                    if (header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(udphdr)) {
                        struct udphdr *udph = (struct udphdr *)(data + sizeof(ether_header) + iph->ip_hl * 4);
                        quint16 srcPort = ntohs(udph->uh_sport);
                        quint16 dstPort = ntohs(udph->uh_dport);

                        if (dstPort == 5353 || srcPort == 5353) {
                            packet.protocol = "MDNS";
                            packet.info = "Standard query";
                        } else if (srcPort == 53 || dstPort == 53) {
                            packet.protocol = "DNS";
                            packet.info = "Standard query";
                        } else {
                            packet.info = QString("%1 → %2 Len=%3").arg(srcPort).arg(dstPort).arg(ntohs(udph->uh_ulen) - sizeof(udphdr));
                        }
                    }
                } else if (iph->ip_p == IPPROTO_ICMP) {
                    packet.protocol = "ICMP";
                    if (header->len >= sizeof(ether_header) + iph->ip_hl * 4 + sizeof(icmphdr)) {
                        struct icmphdr *icmph = (struct icmphdr *)(data + sizeof(ether_header) + iph->ip_hl * 4);
                        if(icmph->type == ICMP_ECHO) packet.info = "Echo (ping) request";
                        else if(icmph->type == ICMP_ECHOREPLY) packet.info = "Echo (ping) reply";
                        else packet.info = "ICMP Packet";
                    }
                } else {
                    packet.protocol = "Other IP";
                    packet.info = QString("Protocol: %1").arg(iph->ip_p);
                }
            } else if (ntohs(eth->ether_type) == ETHERTYPE_ARP) {
                packet.protocol = "ARP";
                packet.info = "Address Resolution Protocol";
            } else {
                packet.protocol = "Non-IP";
                packet.info = QString("EtherType: 0x%1").arg(ntohs(eth->ether_type), 4, 16, QChar('0'));
            }

            emit packetCaptured(packet);

        } else if (res == 0) { // Timeout
            continue;
        } else if (res == -2) { // End of file
            break;
        } else { // Error
            if (running) emit errorOccurred(pcap_geterr(handle));
            break;
        }
    }

    // ### DỌN DẸP AN TOÀN TRƯỚC KHI KẾT THÚC ###
    if (pcapDumper) {
        pcap_dump_flush(pcapDumper); // Ép ghi dữ liệu từ bộ đệm xuống file
        pcap_dump_close(pcapDumper);
        pcapDumper = nullptr;
    }

    if (handle) {
        pcap_close(handle);
        handle = nullptr;
    }
    emit captureStopped();
}

// --- CÁC HÀM ĐIỀU KHIỂN VÀ TIỆN ÍCH ---
void PacketSniffer::stopCapture() {
    running = false;
    if (handle) {
        pcap_breakloop(handle);
    }
}

void PacketSniffer::setPaused(bool paused) {
    this->paused = paused;
}

bool PacketSniffer::startSavingToFile(const QString &filePath) {
    if (pcapDumper) {
        pcap_dump_close(pcapDumper);
    }
    if (!handle) {
        emit errorOccurred("Capture is not running. Cannot save.");
        return false;
    }
    pcapDumper = pcap_dump_open(handle, filePath.toStdString().c_str());
    if (!pcapDumper) {
        emit errorOccurred(QString("Error opening file for saving: %1").arg(pcap_geterr(handle)));
        return false;
    }
    qDebug() << "Started saving packets to" << filePath;
    return true;
}





