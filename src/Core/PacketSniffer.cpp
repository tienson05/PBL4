#include "PacketSniffer.hpp"
#include "PacketParser.hpp" // <-- THÊM MỚI

#include <QDebug> // Chỉ cần cho debug
// #include <QDateTime> // (Không cần nữa)
// (Toàn bộ các include mạng như arpa/inet, netinet/ip... đã được xóa)

/**
 * @brief Constructor cho live capture
 */
PacketSniffer::PacketSniffer(const QString &interfaceName, const QString &captureFilter, QObject *parent)
    : QThread(parent),
    handle(nullptr),
    pcapDumper(nullptr),
    sourceName(interfaceName),
    isFileMode(false),
    m_captureFilter(captureFilter),
    running(false),
    paused(false)
{}

/**
 * @brief Constructor cho đọc file
 */
PacketSniffer::PacketSniffer(const QString &filePath, bool isFile, QObject *parent)
    : QThread(parent),
    handle(nullptr),
    pcapDumper(nullptr),
    sourceName(filePath),
    isFileMode(true),
    m_captureFilter(""),
    running(false),
    paused(false)
{}

/**
 * @brief Destructor
 */
PacketSniffer::~PacketSniffer() {
    if (isRunning()) {
        stopCapture();
        wait();
    }
    if (pcapDumper) {
        pcap_dump_close(pcapDumper);
    }
}

/**
 * @brief Hàm chính của thread
 */
void PacketSniffer::run() {
    running = true;
    char errbuf[PCAP_ERRBUF_SIZE];

    // Mở handle pcap
    if (isFileMode) {
        handle = pcap_open_offline(sourceName.toStdString().c_str(), errbuf);
    } else {
        handle = pcap_open_live(sourceName.toStdString().c_str(), BUFSIZ, 1, 500, errbuf);
    }

    if (!handle) {
        emit errorOccurred(QString("Error opening source: %1").arg(errbuf));
        return;
    }

    // Áp dụng capture filter
    if (!isFileMode && !m_captureFilter.isEmpty()) {
        if (!applyFilter()) {
            pcap_close(handle);
            handle = nullptr;
            return;
            // Dừng thread nếu filter sai
        }
    }

    struct pcap_pkthdr *header;
    const u_char *data;

    // Vòng lặp chính bắt gói tin
    while (running) {
        if (paused) { msleep(100); continue; }

        int res = pcap_next_ex(handle, &header, &data);

        // Gói tin hợp lệ
        if (res > 0) {
            if (pcapDumper) {
                pcap_dump(reinterpret_cast<u_char*>(pcapDumper), header, data);
            }

            // --- ĐÂY LÀ THAY ĐỔI QUAN TRỌNG ---
            // 1. Phân tích gói tin bằng lớp Parser mới
            PacketInfo packet = PacketParser::parse(header, data);

            // 2. Gửi gói tin đã phân tích đi
            emit packetCaptured(packet);
            // --- TOÀN BỘ LOGIC IF/ELSE ĐÃ BỊ XÓA KHỎI ĐÂY ---

        } else if (res == 0) { // Timeout
            continue;
        } else if (res == -2) { // End of file
            break;
        } else { // Error
            if (running) emit errorOccurred(pcap_geterr(handle));
            break;
        }
    }

    // Dọn dẹp
    if (pcapDumper) {
        pcap_dump_flush(pcapDumper);
        pcap_dump_close(pcapDumper);
        pcapDumper = nullptr;
    }

    if (handle) {
        pcap_close(handle);
        handle = nullptr;
    }
    emit captureStopped();
}

/**
 * @brief Hàm áp dụng capture filter (BPF)
 */
bool PacketSniffer::applyFilter()
{
    if (!handle) {
        qDebug() << "Cannot apply filter: handle is null";
        return false;
    }
    struct bpf_program filterProgram;
    if (pcap_compile(handle, &filterProgram,
                     m_captureFilter.toStdString().c_str(),
                     1, PCAP_NETMASK_UNKNOWN) == -1) {
        QString error = QString("Capture Filter compile error: %1").arg(pcap_geterr(handle));
        qDebug() << error;
        emit errorOccurred(error);
        return false;
    }
    if (pcap_setfilter(handle, &filterProgram) == -1) {
        QString error = QString("Capture Filter apply error: %1").arg(pcap_geterr(handle));
        qDebug() << error;
        emit errorOccurred(error);
        pcap_freecode(&filterProgram);
        return false;
    }
    pcap_freecode(&filterProgram);
    qDebug() << "Capture filter applied successfully:" << m_captureFilter;
    return true;
}


// --- CÁC HÀM ĐIỀU KHIỂN ---
void PacketSniffer::stopCapture() {
    running = false;
    if (handle) {
        pcap_breakloop(handle);
    }
}

void PacketSniffer::setPaused(bool paused) {
    this->paused = paused;
}

// SỬA LỖI GÕ NHẦM (PacketSnKiffer -> PacketSniffer)
bool PacketSniffer::startSavingToFile(const QString &filePath) {
    if (pcapDumper) {
        pcap_dump_close(pcapDumper);
    }
    if (!handle) {
        emit errorOccurred("Capture is not running. Cannot save.");
        return false;
    }
    pcapDumper = pcap_dump_open(handle, filePath.toStdString().c_str());
    if (!pcapDumper) {
        emit errorOccurred(QString("Error opening file for saving: %1").arg(pcap_geterr(handle)));
        return false;
    }
    qDebug() << "Started saving packets to" << filePath;
    return true;
}
