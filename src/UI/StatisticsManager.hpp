#ifndef STATISTICSMANAGER_HPP
#define STATISTICSMANAGER_HPP

#include <QObject>
#include <QMap>
#include <QString>
#include "PacketInfo.hpp"

class StatisticsManager : public QObject
{
    Q_OBJECT
public:
    explicit StatisticsManager(QObject *parent = nullptr);

    // --- CÁC HÀM GETTER MỚI VÀ CŨ ---
    QMap<QString, qint64> getProtocolCounts() const;
    QMap<QString, qint64> getSourceIpCounts() const;
    QMap<QString, qint64> getDestinationIpCounts() const;
    qint64 getTotalPacketCount() const; // <-- THÊM MỚI
    int getProtocolTypeCount() const;    // <-- THÊM MỚI

public slots:
    void processPacket(const PacketInfo &packet);
    void clear();

signals:
    // Tín hiệu cho thanh trạng thái (status bar)
    void statsStringUpdated(const QString &statsString);

    // --- THÊM MỚI: Tín hiệu cho Dialog ---
    // Thông báo rằng dữ liệu đã thay đổi và dialog cần vẽ lại
    void statsDataUpdated();

private:
    void generateAndEmitStatsString();

    qint64 m_totalPacketCount;
    QMap<QString, qint64> m_protocolCounts;
    QMap<QString, qint64> m_sourceIpCounts;
    QMap<QString, qint64> m_destIpCounts;
};

#endif // STATISTICSMANAGER_HPP
