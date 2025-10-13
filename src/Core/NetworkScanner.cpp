#include "NetworkScanner.hpp"
#include <pcap.h>
#include <QString>
#include <QDebug>

QStringList NetworkScanner::getDevices() {
    QStringList devices;
    pcap_if_t *alldevs;
    pcap_if_t *d;
    char errbuf[PCAP_ERRBUF_SIZE + 1];

    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        devices << QString("Lỗi: %1").arg(errbuf);
        return devices;
    }

    for (d = alldevs; d; d = d->next) {
        QString name = QString::fromUtf8(d->name);
        QString desc = d->description ? QString::fromUtf8(d->description) : "(Không có mô tả)";
        devices << name + " - " + desc;
    }

    pcap_freealldevs(alldevs);
    return devices;
}
