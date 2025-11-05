#include "StatisticsManager.hpp"
#include <QStringList>
#include <algorithm> // Cần cho std::sort

StatisticsManager::StatisticsManager(QObject *parent)
    : QObject(parent)
{
    clear();
}

void StatisticsManager::clear()
{
    m_totalPacketCount = 0;
    m_protocolCounts.clear();
    m_sourceIpCounts.clear();
    m_destIpCounts.clear();
    emit statsStringUpdated("Ready to capture.");
}

void StatisticsManager::processPacket(const PacketInfo &packet)
{
    // 1. Cập nhật tất cả các bộ đếm
    m_totalPacketCount++;
    m_protocolCounts[packet.protocol]++;

    if (!packet.srcIP.isEmpty()) {
        m_sourceIpCounts[packet.srcIP]++;
    }
    if (!packet.dstIP.isEmpty()) {
        m_destIpCounts[packet.dstIP]++;
    }

    // 2. Tạo chuỗi và phát tín hiệu cho status bar
    generateAndEmitStatsString();

    // 3. THÊM MỚI: Phát tín hiệu cho dialog
    emit statsDataUpdated();
}

void StatisticsManager::generateAndEmitStatsString()
{
    // ... (Hàm này giữ nguyên như cũ) ...
    if (m_totalPacketCount == 0) {
        emit statsStringUpdated("Ready to capture.");
        return;
    }
    QString statsText = QString("Total Packets: %1 | ").arg(m_totalPacketCount);
    QList<QPair<qint64, QString>> sortedProtocols;
    for (auto it = m_protocolCounts.constBegin(); it != m_protocolCounts.constEnd(); ++it) {
        sortedProtocols.append(qMakePair(it.value(), it.key()));
    }
    std::sort(sortedProtocols.rbegin(), sortedProtocols.rend());
    QStringList protocolStrings;
    for (const auto &pair : sortedProtocols) {
        protocolStrings << QString("%1: %2").arg(pair.second).arg(pair.first);
    }
    statsText += protocolStrings.join(" | ");
    emit statsStringUpdated(statsText);
}

// --- CÁC HÀM GETTER (CŨ VÀ MỚI) ---
QMap<QString, qint64> StatisticsManager::getProtocolCounts() const
{
    return m_protocolCounts;
}
QMap<QString, qint64> StatisticsManager::getSourceIpCounts() const
{
    return m_sourceIpCounts;
}
QMap<QString, qint64> StatisticsManager::getDestinationIpCounts() const
{
    return m_destIpCounts;
}
qint64 StatisticsManager::getTotalPacketCount() const
{
    return m_totalPacketCount; // <-- THÊM MỚI
}
int StatisticsManager::getProtocolTypeCount() const
{
    return m_protocolCounts.size(); // <-- THÊM MỚI
}
