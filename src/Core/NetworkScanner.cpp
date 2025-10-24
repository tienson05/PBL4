#include "NetworkScanner.hpp"
#include <pcap.h>
#include <QDebug>

QList<QPair<QString, QString>> NetworkScanner::getDevices() {
    QList<QPair<QString, QString>> deviceList;
    pcap_if_t *alldevs;
    char errbuf[PCAP_ERRBUF_SIZE];
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        qWarning() << "Error in pcap_findalldevs:" << errbuf;
        return deviceList;
    }
    for (pcap_if_t *d = alldevs; d != nullptr; d = d->next) {
        QString systemName = QString::fromLatin1(d->name);
        QString description = d->description ? QString::fromLatin1(d->description) : "No description available";
        deviceList.append({systemName, description});
    }
    pcap_freealldevs(alldevs);
    return deviceList;
}
