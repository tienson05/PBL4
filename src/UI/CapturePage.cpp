#include "CapturePage.hpp"
#include <QVBoxLayout>
#include <QLabel>

CapturePage::CapturePage(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    statusLabel = new QLabel("No capture started yet.", this);
    statusLabel->setStyleSheet("font-size: 16px; color: #2C3E50;");
    layout->addWidget(statusLabel);
    setLayout(layout);
}

void CapturePage::startCapture(const QString &interfaceName)
{
    // Sau này bạn sẽ gọi pcap_open_live() ở đây
    statusLabel->setText("Capturing on interface: " + interfaceName);
}
