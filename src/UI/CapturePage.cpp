#include "CapturePage.hpp"
#include "NetworkViewer.hpp"
#include "PacketSniffer.hpp"
#include "StatisticsDialog.hpp"
#include "StatisticsManager.hpp" // <-- Include Trình quản lý

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDateTime>
#include <pcap.h>
#include <QDir>
#include <QCoreApplication>
#include <QRegularExpression> // Để tách chuỗi filter

CapturePage::CapturePage(QWidget *parent)
    : QWidget(parent),
    scanner(nullptr),
    isLiveCapture(false),
    isPaused(false),
    m_captureFilter(""), // Bộ lọc BẮT GÓI (từ WelcomePage)
    m_displayFilter("") // Bộ lọc HIỂN THỊ (từ ô filter trên trang này)
{
    // --- 1. Khởi tạo Trình quản lý Thống kê ---
    m_statsManager = new StatisticsManager(this);

    // --- 2. Setup Giao diện (UI) ---
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 5, 0, 0);
    mainLayout->setSpacing(5);

    auto *controlLayout = new QHBoxLayout();
    sourceNameLabel = new QLabel("Source: Not selected");
    sourceNameLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    restartBtn = new QPushButton("Restart");
    stopBtn = new QPushButton("Stop");
    pauseBtn = new QPushButton("Pause");
    statisticsBtn = new QPushButton("Statistics");
    controlLayout->addWidget(sourceNameLabel);
    controlLayout->addStretch();
    controlLayout->addWidget(restartBtn);
    controlLayout->addWidget(stopBtn);
    controlLayout->addWidget(pauseBtn);
    controlLayout->addWidget(statisticsBtn);
    mainLayout->addLayout(controlLayout);

    auto *filterLayout = new QHBoxLayout();
    filterLayout->setContentsMargins(5, 0, 5, 0);
    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText("Apply a display filter... (e.g., 'icmp', '192.168.1.8')");
    applyFilterButton = new QPushButton("Apply", this);
    filterLayout->addWidget(filterLineEdit, 1);
    filterLayout->addWidget(applyFilterButton);
    mainLayout->addLayout(filterLayout);

    viewer = new NetworkViewer(this);
    mainLayout->addWidget(viewer, 1);

    // --- 3. Kết nối tín hiệu ---
    connect(restartBtn, &QPushButton::clicked, this, &CapturePage::onRestartCaptureClicked);
    connect(stopBtn, &QPushButton::clicked, this, &CapturePage::onStopCaptureClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &CapturePage::onPauseCaptureClicked);
    connect(statisticsBtn, &QPushButton::clicked, this, &CapturePage::onStatisticsClicked);
    connect(applyFilterButton, &QPushButton::clicked, this, &CapturePage::onApplyFilterClicked);
    connect(filterLineEdit, &QLineEdit::returnPressed, this, &CapturePage::onApplyFilterClicked);

    // --- Kết nối StatisticsManager với NetworkViewer ---
    // Khi manager tạo xong chuỗi, gửi nó đến slot của viewer.
    connect(m_statsManager, &StatisticsManager::statsStringUpdated,
            viewer, &NetworkViewer::updateStatsString);
}

// --- (ĐÃ SỬA) Slot cho nút Statistics ---
void CapturePage::onStatisticsClicked()
{
    if (capturedPackets.isEmpty()) {
        QMessageBox::information(this, "Statistics", "No packets captured to analyze.");
        return;
    }

    // --- SỬA LỖI: ---
    // Bây giờ chúng ta truyền Trình quản lý (Manager)
    // thay vì danh sách gói tin thô.
    StatisticsDialog dialog(m_statsManager, this);
    dialog.exec(); // Hiển thị dialog (dạng modal)
}


// --- LOGIC DISPLAY FILTER (Giữ nguyên) ---
void CapturePage::onApplyFilterClicked()
{
    m_displayFilter = filterLineEdit->text().trimmed().toLower();
    qDebug() << "Display filter applied:" << m_displayFilter;
    refreshPacketView();
}

void CapturePage::refreshPacketView()
{
    viewer->clearData();
    for (const PacketInfo &packet : capturedPackets) {
        if (packetMatchesFilter(packet, m_displayFilter)) {
            viewer->addPacket(packet);
        }
    }
}

bool CapturePage::packetMatchesFilter(const PacketInfo& packet, const QString& filter)
{
    if (filter.isEmpty()) {
        return true;
    }
    QStringList keywords = filter.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QString packetMegaString = QString("%1 %2 %3 %4")
                                   .arg(packet.protocol)
                                   .arg(packet.srcIP)
                                   .arg(packet.dstIP)
                                   .arg(packet.info)
                                   .toLower();
    for (const QString &keyword : keywords) {
        if (!packetMegaString.contains(keyword)) {
            return false;
        }
    }
    return true;
}


// --- CÁC HÀM ĐIỀU KHIỂN CÔNG KHAI (Giữ nguyên) ---
void CapturePage::startInitialCapture(const QString &interfaceName, const QString &captureFilter)
{
    isLiveCapture = true;
    currentCaptureSource = interfaceName;
    m_captureFilter = captureFilter;
    sourceNameLabel->setText(QString("Interface: %1").arg(interfaceName));
    restartBtn->setEnabled(true);
    m_displayFilter = "";
    filterLineEdit->setText("");
    startCaptureInternal();
}

void CapturePage::startCaptureFromFile(const QString &filePath)
{
    isLiveCapture = false;
    currentCaptureSource = filePath;
    m_captureFilter = "";
    sourceNameLabel->setText(QString("File: %1").arg(QFileInfo(filePath).fileName()));
    restartBtn->setEnabled(false);
    m_displayFilter = "";
    filterLineEdit->setText("");
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

    pcap_t *pcap_handle = pcap_open_dead(DLT_EN10MB, 65535);
    if (!pcap_handle) {
        QMessageBox::critical(this, "Save Error", "Could not create pcap handle for writing.");
        return;
    }

    std::string filePathStd = filePath.toStdString();

    pcap_dumper_t *dumper = pcap_dump_open(pcap_handle, filePathStd.c_str());

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


// --- CÁC SLOT VÀ HÀM NỘI BỘ (Giữ nguyên) ---
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

/**
 * @brief (ĐÃ CẬP NHẬT)
 * Hàm nội bộ để khởi tạo hoặc khởi động lại 'scanner'.
 */
void CapturePage::startCaptureInternal() {
    if (scanner) {
        if(scanner->isRunning()) { scanner->stopCapture(); scanner->wait(); }
        scanner->deleteLater();
    }
    capturedPackets.clear();
    viewer->clearData();

    // --- Reset Trình quản lý Thống kê ---
    m_statsManager->clear();

    if (currentCaptureSource.isEmpty()) return;
    isPaused = false;
    pauseBtn->setText("Pause");

    if (isLiveCapture) {
        scanner = new PacketSniffer(currentCaptureSource, m_captureFilter, this);
    } else {
        scanner = new PacketSniffer(currentCaptureSource, true, this);
    }

    // --- (ĐÃ CẬP NHẬT) Kết nối signal khi có gói tin mới ---
    connect(scanner, &PacketSniffer::packetCaptured, this, [this](const PacketInfo &packet){
        // 1. Lưu vào master list
        capturedPackets.append(packet);

        // 2. CHỈ hiển thị nếu nó khớp với m_displayFilter
        if (packetMatchesFilter(packet, m_displayFilter)) {
            viewer->addPacket(packet);
        }

        // 3. Gửi gói tin cho Trình quản lý Thống kê
        m_statsManager->processPacket(packet);
    });

    connect(scanner, &PacketSniffer::errorOccurred, this, [this](const QString &errorString){
        QMessageBox::critical(this, "Capture Error", errorString);
        // Gửi lỗi lên thanh trạng thái
        m_statsManager->statsStringUpdated(QString("Error: %1").arg(errorString));
    });

    connect(scanner, &QThread::finished, this, [this](){
        // Gửi trạng thái dừng lên thanh trạng thái
        m_statsManager->statsStringUpdated("Capture stopped.");
    });

    scanner->start();
}
