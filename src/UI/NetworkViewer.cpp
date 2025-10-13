#include "NetworkViewer.hpp"
#include "NetworkScanner.hpp"

#include <QVBoxLayout>
#include <QListWidget>

NetworkViewer::NetworkViewer(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    auto *list = new QListWidget(this);

    QStringList devices = NetworkScanner::getDevices();
    list->addItems(devices);

    layout->addWidget(list);
    setLayout(layout);
}
