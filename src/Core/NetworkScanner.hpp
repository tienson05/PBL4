#pragma once
#include <pcap.h>
#include <QString>
#include <QThread>
#include <QVector>
#include <QByteArray>

struct PacketInfo {
    QString timestamp;
    QString srcIP;
    QString dstIP;
    QString protocol;
    int length;
    QString info;
    QByteArray rawData;
};

class NetworkScanner : public QThread {
    Q_OBJECT
public:
    explicit NetworkScanner(QObject *parent = nullptr);
    ~NetworkScanner();

    bool openInterface(const QString &interfaceName);
    void stopCapture();
    void setPaused(bool pause);
    void saveToPcap(const QString &filePath);
    bool openFromPcap(const QString &filePath);
static QList<QPair<QString, QString>> getDevices();
signals:
    void packetCaptured(const PacketInfo &packet);
    void captureStopped();

protected:
    void run() override;

private:
    pcap_t *handle;
    bool running;
    bool paused;
    QString currentInterface;
};
