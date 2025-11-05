#ifndef PACKETPARSER_H
#define PACKETPARSER_H

#include "PacketInfo.hpp" // Cần file định nghĩa PacketInfo
#include <pcap.h>         // Cần cho pcap_pkthdr

/**
 * @brief Lớp tiện ích (utility class) chỉ chứa các hàm static
 * để phân tích dữ liệu gói tin thô.
 */
class PacketParser
{
public:
    /**
     * @brief Phân tích dữ liệu gói tin thô và điền vào cấu trúc PacketInfo.
     * @param header Con trỏ đến pcap_pkthdr (chứa timestamp, length).
     * @param data Con trỏ đến dữ liệu thô (raw data) của gói tin.
     * @return Một đối tượng PacketInfo đã được phân tích.
     */
    static PacketInfo parse(const struct pcap_pkthdr *header, const u_char *data);
};

#endif // PACKETPARSER_H
