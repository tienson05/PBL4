#include "PacketParser.hpp"

// Toàn bộ includes cần thiết cho phân tích
#include <QDateTime>
#include <QStringList> // Cần cho TCP Flags

#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>      // Cho IPv6
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/icmp6.h>    // Cho ICMPv6

/**
 * @brief Hàm static chính, nhận dữ liệu thô và trả về PacketInfo.
 */
PacketInfo PacketParser::parse(const struct pcap_pkthdr *header, const u_char *data)
{
    PacketInfo packet;
    packet.timestamp = QDateTime::fromSecsSinceEpoch(header->ts.tv_sec).toString("hh:mm:ss.zzz");
    packet.length = header->len;
    packet.rawData = QByteArray(reinterpret_cast<const char*>(data), header->len);

    struct ether_header *eth = (struct ether_header *)data;
    const int eth_header_len = sizeof(struct ether_header);

    // --- TRƯỜNG HỢP 1: IPv4 ---
    if (ntohs(eth->ether_type) == ETHERTYPE_IP) { // 0x0800
        struct ip *iph = (struct ip *)(data + eth_header_len);
        int ip_header_len = iph->ip_hl * 4;
        packet.srcIP = inet_ntoa(iph->ip_src);
        packet.dstIP = inet_ntoa(iph->ip_dst);

        if (iph->ip_p == IPPROTO_TCP) {
            packet.protocol = "TCP";
            if (header->len >= eth_header_len + ip_header_len + sizeof(tcphdr)) {
                struct tcphdr *tcph = (struct tcphdr *)(data + eth_header_len + ip_header_len);
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
            if (header->len >= eth_header_len + ip_header_len + sizeof(udphdr)) {
                struct udphdr *udph = (struct udphdr *)(data + eth_header_len + ip_header_len);
                quint16 srcPort = ntohs(udph->uh_sport);
                quint16 dstPort = ntohs(udph->uh_dport);
                if (dstPort == 5353 || srcPort == 5353) packet.protocol = "MDNS";
                else if (srcPort == 53 || dstPort == 53) packet.protocol = "DNS";
                packet.info = QString("%1 → %2 Len=%3").arg(srcPort).arg(dstPort).arg(ntohs(udph->uh_ulen) - sizeof(udphdr));
            }
        } else if (iph->ip_p == IPPROTO_ICMP) {
            packet.protocol = "ICMP";
            if (header->len >= eth_header_len + ip_header_len + sizeof(icmphdr)) {
                struct icmphdr *icmph = (struct icmphdr *)(data + eth_header_len + ip_header_len);
                if(icmph->type == ICMP_ECHO) packet.info = "Echo (ping) request";
                else if(icmph->type == ICMP_ECHOREPLY) packet.info = "Echo (ping) reply";
                else packet.info = "ICMP Packet";
            }
        } else {
            packet.protocol = "Other IP";
            packet.info = QString("Protocol: %1").arg(iph->ip_p);
        }

        // --- TRƯỜNG HỢP 2: IPv6 ---
    } else if (ntohs(eth->ether_type) == ETHERTYPE_IPV6) { // 0x86dd
        packet.protocol = "IPv6";

        struct ip6_hdr *ip6h = (struct ip6_hdr *)(data + eth_header_len);
        char src_ip_str[INET6_ADDRSTRLEN];
        char dst_ip_str[INET6_ADDRSTRLEN];

        inet_ntop(AF_INET6, &(ip6h->ip6_src), src_ip_str, INET6_ADDRSTRLEN);
        inet_ntop(AF_INET6, &(ip6h->ip6_dst), dst_ip_str, INET6_ADDRSTRLEN);

        packet.srcIP = QString(src_ip_str);
        packet.dstIP = QString(dst_ip_str);

        uint8_t next_header_type = ip6h->ip6_nxt;
        int transport_header_offset = eth_header_len + sizeof(struct ip6_hdr);

        if (next_header_type == IPPROTO_TCP) {
            packet.protocol = "TCP";
            if (header->len >= transport_header_offset + sizeof(tcphdr)) {
                struct tcphdr *tcph = (struct tcphdr *)(data + transport_header_offset);
                quint16 srcPort = ntohs(tcph->th_sport);
                quint16 dstPort = ntohs(tcph->th_dport);
                if (srcPort == 443 || dstPort == 443) packet.protocol = "TLS";
                packet.info = QString("%1 → %2 [TCPv6]").arg(srcPort).arg(dstPort);
            }
        } else if (next_header_type == IPPROTO_UDP) {
            packet.protocol = "UDP";
            if (header->len >= transport_header_offset + sizeof(udphdr)) {
                struct udphdr *udph = (struct udphdr *)(data + transport_header_offset);
                quint16 srcPort = ntohs(udph->uh_sport);
                quint16 dstPort = ntohs(udph->uh_dport);
                if (srcPort == 53 || dstPort == 53) packet.protocol = "DNS";
                packet.info = QString("%1 → %2 [UDPv6]").arg(srcPort).arg(dstPort);
            }
        } else if (next_header_type == IPPROTO_ICMPV6) {
            packet.protocol = "ICMPv6";
            packet.info = "ICMPv6 Packet";
        } else {
            packet.protocol = "IPv6";
            packet.info = QString("Next Header: %1").arg(next_header_type);
        }

        // --- TRƯỜNG HỢP 3: ARP ---
    } else if (ntohs(eth->ether_type) == ETHERTYPE_ARP) { // 0x0806
        packet.protocol = "ARP";
        packet.info = "Address Resolution Protocol";

        // --- TRƯỜNG HỢP 4: Khác ---
    } else {
        packet.protocol = "Non-IP";
        packet.info = QString("EtherType: 0x%1").arg(ntohs(eth->ether_type), 4, 16, QChar('0'));
    }

    return packet;
}
