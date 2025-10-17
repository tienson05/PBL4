#pragma once

#include <QString>
#include <QByteArray>
#include <QMetaType> // Cần thiết để đăng ký kiểu dữ liệu

/**
 * @brief Chứa thông tin đã được phân tích của một gói tin mạng.
 *
 * Cấu trúc này được dùng để truyền dữ liệu từ luồng NetworkScanner
 * sang luồng giao diện chính một cách an toàn.
 */
struct PacketInfo {
    QString timestamp;
    QString srcIP;
    QString dstIP;
    QString protocol;
    int length;
    QString info;
    QByteArray rawData;
};

// Đăng ký kiểu dữ liệu để có thể sử dụng trong cơ chế signal/slot của Qt
// Điều này rất quan trọng khi truyền dữ liệu giữa các luồng.
Q_DECLARE_METATYPE(PacketInfo)
