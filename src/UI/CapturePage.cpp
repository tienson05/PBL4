#include "CapturePage.hpp"
#include "NetworkViewer.hpp"
#include "NetworkScanner.hpp" // THÊM DÒNG NÀY ĐỂ SỬA LỖI

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <pcap.h>
#include <QDir>
#include <QCoreApplication>   // THÊM MỚI

CapturePage::CapturePage(QWidget *parent)
    : QWidget(parent),
    scanner(nullptr),
    isLiveCapture(false),
    isPaused(false)
{
    auto *mainLayout = new QVBoxLayout(this);
    auto *controlLayout = new QHBoxLayout();
    sourceNameLabel = new QLabel("Source: Not selected");
    sourceNameLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    restartBtn = new QPushButton("Restart");
    stopBtn = new QPushButton("Stop");
    pauseBtn = new QPushButton("Pause");
    controlLayout->addWidget(sourceNameLabel);
    controlLayout->addStretch();
    controlLayout->addWidget(restartBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(pauseBtn);
    mainLayout->addLayout(controlLayout);
    viewer = new NetworkViewer(this);
    mainLayout->addWidget(viewer);
    connect(restartBtn, &QPushButton::clicked, this, &CapturePage::onRestartCaptureClicked);
    connect(stopBtn, &QPushButton::clicked, this, &CapturePage::onStopCaptureClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &CapturePage::onPauseCaptureClicked);
}

// --- CÁC HÀM ĐIỀU KHIỂN CÔNG KHAI ---
void CapturePage::startInitialCapture(const QString &interfaceName)
{
    isLiveCapture = true;
    currentCaptureSource = interfaceName;
    sourceNameLabel->setText(QString("Interface: %1").arg(interfaceName));
    restartBtn->setEnabled(true);
    startCaptureInternal();
}

void CapturePage::startCaptureFromFile(const QString &filePath)
{
    isLiveCapture = false;
    currentCaptureSource = filePath;
    sourceNameLabel->setText(QString("File: %1").arg(QFileInfo(filePath).fileName()));
    restartBtn->setEnabled(false);
    startCaptureInternal();
}

void CapturePage::saveCurrentCapture()
{
    if (capturedPackets.isEmpty()) {
        QMessageBox::warning(this, "Save Error", "There are no packets to save.");
        return;
    }

    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultDir = appDir + "/FileSave";
    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QFileDialog saveDialog(this, "Lưu File Dưới Dạng", defaultDir, "Pcap Files (*.pcap)");
    saveDialog.setAcceptMode(QFileDialog::AcceptSave);
    saveDialog.setDefaultSuffix("pcap");

    if (!saveDialog.exec()) {
        return; // Người dùng nhấn Cancel
    }

    const QStringList selectedFiles = saveDialog.selectedFiles();
    if (selectedFiles.isEmpty()) {
        return;
    }
    QString filePath = selectedFiles.first();

    // --- Phần code ghi file pcap ---
    pcap_t *pcap_handle = pcap_open_dead(DLT_EN10MB, 65535);
    if (!pcap_handle) {
        QMessageBox::critical(this, "Save Error", "Could not create pcap handle for writing.");
        return;
    }

    // ### SỬA LỖI WARNING CUỐI CÙNG ###
    // 1. Chuyển đổi QString sang std::string và lưu vào một biến để nó không bị hủy ngay lập tức.
    std::string filePathStd = filePath.toStdString();

    // 2. Sử dụng .c_str() trên biến đã được lưu trữ an toàn.
    pcap_dumper_t *dumper = pcap_dump_open(pcap_handle, filePathStd.c_str());
    // ### KẾT THÚC SỬA LỖI ###

    if (!dumper) {
        QMessageBox::critical(this, "Save Error", QString("Could not open file for writing: %1").arg(pcap_geterr(pcap_handle)));
        pcap_close(pcap_handle);
        return;
    }

    for (const PacketInfo &packet : capturedPackets) {
        pcap_pkthdr header;
        QDateTime dt = QDateTime::fromString(packet.timestamp, "hh:mm:ss.zzz");
        if (dt.isValid()) {
            header.ts.tv_sec = dt.toSecsSinceEpoch();
            header.ts.tv_usec = dt.time().msec() * 1000;
        } else {
            header.ts.tv_sec = 0;
            header.ts.tv_usec = 0;
        }
        header.caplen = packet.rawData.size();
        header.len = packet.rawData.size();
        pcap_dump(reinterpret_cast<u_char*>(dumper), &header, reinterpret_cast<const u_char*>(packet.rawData.constData()));
    }

    pcap_dump_close(dumper);
    pcap_close(pcap_handle);
    QMessageBox::information(this, "Save Successful", QString("Successfully saved %1 packets to file.").arg(capturedPackets.size()));
}
// --- CÁC SLOT VÀ HÀM NỘI BỘ ---
void CapturePage::onRestartCaptureClicked() { if (isLiveCapture) startCaptureInternal(); }

void CapturePage::onStopCaptureClicked() {
    if (scanner && scanner->isRunning()) {
        scanner->stopCapture();
        scanner->wait();
    }
}

void CapturePage::onPauseCaptureClicked() {
    if (scanner && scanner->isRunning()) {
        isPaused = !isPaused;
        scanner->setPaused(isPaused);
        pauseBtn->setText(isPaused ? "Resume" : "Pause");
    }
}

void CapturePage::startCaptureInternal() {
    if (scanner) {
        if(scanner->isRunning()) { scanner->stopCapture(); scanner->wait(); }
        scanner->deleteLater();
    }
    capturedPackets.clear();
    viewer->clearData();

    if (currentCaptureSource.isEmpty()) return;

    isPaused = false;
    pauseBtn->setText("Pause");

    if (isLiveCapture) {
        scanner = new NetworkScanner(currentCaptureSource, this);
    } else {
        scanner = new NetworkScanner(currentCaptureSource, true, this);
    }

    connect(scanner, &NetworkScanner::packetCaptured, this, [this](const PacketInfo &packet){
        viewer->addPacket(packet);
        capturedPackets.append(packet);
    });

    connect(scanner, &NetworkScanner::errorOccurred, this, [this](const QString &errorString){
        QMessageBox::critical(this, "Capture Error", errorString);
    });

    scanner->start();
}
