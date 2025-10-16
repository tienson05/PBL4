#include "CapturePage.hpp"
#include "NetworkViewer.hpp"
#include "NetworkScanner.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

CapturePage::CapturePage(QWidget *parent)
    : QWidget(parent), scanner(nullptr), isPaused(false)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *controlLayout = new QHBoxLayout();

    // THAY ĐỔI: Bỏ QComboBox, thay bằng một QLabel để hiển thị tên interface
    interfaceNameLabel = new QLabel("Interface: Not selected");
    interfaceNameLabel->setStyleSheet("font-weight: bold; padding: 5px;");

    startBtn = new QPushButton("Start");
    stopBtn = new QPushButton("Stop");
    pauseBtn = new QPushButton("Pause");

    // XÓA BỎ: Các dòng liên quan đến interfaceBox
    // controlLayout->addWidget(new QLabel("Interface:"));
    // controlLayout->addWidget(interfaceBox);

    // THÊM MỚI: Thêm QLabel hiển thị vào layout
    controlLayout->addWidget(interfaceNameLabel);
    controlLayout->addStretch(); // Đẩy các nút về bên phải
    controlLayout->addWidget(startBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(pauseBtn);

    mainLayout->addLayout(controlLayout);

    viewer = new NetworkViewer(this);
    mainLayout->addWidget(viewer);

    connect(startBtn, &QPushButton::clicked, this, &CapturePage::startCapture);
    connect(stopBtn, &QPushButton::clicked, this, &CapturePage::stopCapture);
    connect(pauseBtn, &QPushButton::clicked, this, &CapturePage::pauseCapture);
}

// CẬP NHẬT: Hàm này giờ sẽ lưu trữ tên interface và cập nhật giao diện
void CapturePage::startInitialCapture(const QString &interfaceName)
{
    // 1. Lưu tên interface được truyền vào
    this->currentInterfaceName = interfaceName;

    // 2. Cập nhật QLabel để hiển thị cho người dùng
    interfaceNameLabel->setText(QString("Interface: %1").arg(interfaceName));

    // 3. Tự động bắt đầu capture
    startCapture();
}


void CapturePage::startCapture() {
    if (scanner) {
        if(scanner->isRunning()) {
            scanner->stopCapture();
            scanner->wait();
        }
        scanner->deleteLater();
    }

    viewer->clearData();

    // THAY ĐỔI LỚN: Sử dụng tên interface đã được lưu trữ, thay vì đọc từ ComboBox
    QString iface = this->currentInterfaceName;
    if (iface.isEmpty()) {
        qDebug() << "CapturePage: No interface selected to start capture.";
        return;
    }

    isPaused = false;
    pauseBtn->setText("Pause");

    scanner = new NetworkScanner(this);
    connect(scanner, &NetworkScanner::packetCaptured, viewer, &NetworkViewer::addPacket);

    if (scanner->openInterface(iface)) {
        scanner->start();
        qDebug() << "CapturePage: Started capturing on" << iface;
    } else {
        qDebug() << "CapturePage: Failed to open interface, cleaning up scanner.";
        scanner->deleteLater();
        scanner = nullptr;
    }
}

void CapturePage::stopCapture() {
    if (scanner && scanner->isRunning()) {
        scanner->stopCapture();
        scanner->wait();
        qDebug() << "CapturePage: Stopped capture";
        isPaused = false;
        pauseBtn->setText("Pause");
    }
}

void CapturePage::pauseCapture() {
    if (scanner && scanner->isRunning()) {
        isPaused = !isPaused;
        scanner->setPaused(isPaused);
        pauseBtn->setText(isPaused ? "Resume" : "Pause");
        qDebug() << "CapturePage:" << (isPaused ? "Paused" : "Resumed");
    }
}
