#include "CapturePage.hpp"
#include "NetworkViewer.hpp"
#include "PacketSniffer.hpp"

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
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 5, 0, 0);
    mainLayout->setSpacing(5);

    // --- 1. Thanh điều khiển (Control Layout) ---
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

    // --- 2. THANH FILTER (DISPLAY FILTER) ---
    auto *filterLayout = new QHBoxLayout();
    filterLayout->setContentsMargins(5, 0, 5, 0);
    filterLineEdit = new QLineEdit(this);
    filterLineEdit->setPlaceholderText("Apply a display filter... (e.g., 'icmp', '192.168.1.8')");
    applyFilterButton = new QPushButton("Apply", this);
    filterLayout->addWidget(filterLineEdit, 1);
    filterLayout->addWidget(applyFilterButton);
    mainLayout->addLayout(filterLayout);

    // --- 3. Bảng NetworkViewer ---
    viewer = new NetworkViewer(this);
    mainLayout->addWidget(viewer, 1);

    // --- 4. Kết nối tín hiệu ---
    connect(restartBtn, &QPushButton::clicked, this, &CapturePage::onRestartCaptureClicked);
    connect(stopBtn, &QPushButton::clicked, this, &CapturePage::onStopCaptureClicked);
    connect(pauseBtn, &QPushButton::clicked, this, &CapturePage::onPauseCaptureClicked);

    // Kết nối nút Apply và phím Enter
    connect(applyFilterButton, &QPushButton::clicked, this, &CapturePage::onApplyFilterClicked);
    connect(filterLineEdit, &QLineEdit::returnPressed, this, &CapturePage::onApplyFilterClicked);
}

// --- LOGIC DISPLAY FILTER ---
void CapturePage::onApplyFilterClicked()
{
    // 1. Dùng m_displayFilter
    m_displayFilter = filterLineEdit->text().trimmed().toLower();
    qDebug() << "Display filter applied:" << m_displayFilter;

    // 2. Làm mới toàn bộ bảng hiển thị bằng cách lọc lại master list
    refreshPacketView();
}

/**
 * @brief Lọc lại toàn bộ 'capturedPackets' và hiển thị trong 'viewer'.
 */
void CapturePage::refreshPacketView()
{
    // 1. Xóa sạch bảng hiển thị (nhưng không xóa 'capturedPackets')
    viewer->clearData();

    // 2. Lặp lại master list và thêm lại những cái phù hợp
    for (const PacketInfo &packet : capturedPackets) {
        // 3. Dùng m_displayFilter để kiểm tra
        if (packetMatchesFilter(packet, m_displayFilter)) {
            viewer->addPacket(packet);
        }
    }
}

/**
 * @brief (ĐÃ CẬP NHẬT)
 * Logic lọc mới, thông minh hơn. Tách filter thành các từ khóa.
 */
bool CapturePage::packetMatchesFilter(const PacketInfo& packet, const QString& filter)
{
    // Nếu filter rỗng, luôn luôn hiển thị (true)
    if (filter.isEmpty()) {
        return true;
    }

    // Tách filter thành các từ khóa (ví dụ: "tcp 80" -> ["tcp", "80"])
    QStringList keywords = filter.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    // Tạo một "siêu chuỗi" chứa toàn bộ thông tin gói tin (đã toLower)
    QString packetMegaString = QString("%1 %2 %3 %4")
                                   .arg(packet.protocol)
                                   .arg(packet.srcIP)
                                   .arg(packet.dstIP)
                                   .arg(packet.info)
                                   .toLower();

    // Kiểm tra xem TẤT CẢ các từ khóa có trong siêu chuỗi không
    for (const QString &keyword : keywords) {
        if (!packetMegaString.contains(keyword)) {
            // Chỉ cần MỘT từ khóa không khớp là false
            return false;
        }
    }

    // Nếu tất cả từ khóa đều khớp
    return true;
}


// --- CÁC HÀM ĐIỀU KHIỂN CÔNG KHAI ---

/**
 * @brief Bắt đầu live capture với một capture filter.
 */
void CapturePage::startInitialCapture(const QString &interfaceName, const QString &captureFilter)
{
    isLiveCapture = true;
    currentCaptureSource = interfaceName;
    m_captureFilter = captureFilter; // <-- LƯU BỘ LỌC BẮT GÓI

    sourceNameLabel->setText(QString("Interface: %1").arg(interfaceName));
    restartBtn->setEnabled(true);

    // Reset bộ lọc hiển thị khi bắt đầu session mới
    m_displayFilter = "";
    filterLineEdit->setText("");

    startCaptureInternal();
}

/**
 * @brief Bắt đầu capture từ file (không cần capture filter).
 */
void CapturePage::startCaptureFromFile(const QString &filePath)
{
    isLiveCapture = false;
    currentCaptureSource = filePath;
    m_captureFilter = ""; // Không áp dụng capture filter khi đọc file
    sourceNameLabel->setText(QString("File: %1").arg(QFileInfo(filePath).fileName()));
    restartBtn->setEnabled(false);

    // Reset bộ lọc hiển thị
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

    std::string filePathStd = filePath.toStdString(); // Đã sửa lỗi

    pcap_dumper_t *dumper = pcap_dump_open(pcap_handle, filePathStd.c_str());

    if (!dumper) {
        // --- DÒNG ĐÃ SỬA LỖI ---
        // Đã đổi 'handle' thành 'pcap_handle'
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

/**
 * @brief Hàm nội bộ để khởi tạo hoặc khởi động lại 'scanner'.
 */
void CapturePage::startCaptureInternal() {
    if (scanner) {
        if(scanner->isRunning()) { scanner->stopCapture(); scanner->wait(); }
        scanner->deleteLater();
    }

    // Xóa cả master list VÀ bảng hiển thị
    capturedPackets.clear();
    viewer->clearData();

    if (currentCaptureSource.isEmpty()) return;

    isPaused = false;
    pauseBtn->setText("Pause");

    // Khởi tạo PacketSniffer, truyền Capture Filter (nếu có)
    if (isLiveCapture) {
        scanner = new PacketSniffer(currentCaptureSource, m_captureFilter, this);
    } else {
        // ĐỌC FILE
        scanner = new PacketSniffer(currentCaptureSource, true, this);
    }

    // Kết nối signal khi có gói tin mới
    connect(scanner, &PacketSniffer::packetCaptured, this, [this](const PacketInfo &packet){
        // 1. LUÔN LUÔN lưu vào master list
        capturedPackets.append(packet);

        // 2. CHỈ hiển thị nếu nó khớp với m_displayFilter
        if (packetMatchesFilter(packet, m_displayFilter)) {
            viewer->addPacket(packet);
        }
    });

    connect(scanner, &PacketSniffer::errorOccurred, this, [this](const QString &errorString){
        QMessageBox::critical(this, "Capture Error", errorString);
    });

    scanner->start();
}
