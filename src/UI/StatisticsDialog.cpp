#include "StatisticsDialog.hpp"
#include "StatisticsManager.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout> // <-- Thêm mới
#include <QTabWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <QLabel> // <-- Thêm mới

/**
 * @brief (ĐÃ THAY ĐỔI) Constructor này chỉ cài đặt UI
 * và kết nối tín hiệu (signal).
 */
StatisticsDialog::StatisticsDialog(StatisticsManager *statsManager, QWidget *parent)
    : QDialog(parent),
    m_statsManager(statsManager) // <-- Lưu con trỏ
{
    setWindowTitle("Packet Statistics");
    setMinimumSize(600, 400);

    // --- 1. Tạo các Label cho thông tin tổng quan ---
    m_totalPacketsLabel = new QLabel("Total Packets: ...");
    m_totalProtocolsLabel = new QLabel("Total Protocol Types: ...");

    // CSS đơn giản cho đẹp
    m_totalPacketsLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_totalProtocolsLabel->setStyleSheet("font-size: 14px;");

    auto *statsLayout = new QHBoxLayout();
    statsLayout->addWidget(m_totalPacketsLabel);
    statsLayout->addSpacing(20);
    statsLayout->addWidget(m_totalProtocolsLabel);
    statsLayout->addStretch();

    // --- 2. Tạo Giao diện Tab ---
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->addTab(createProtocolTab(), "Protocols");
    m_tabWidget->addTab(createIpTab("Source IPs"), "Sources");
    m_tabWidget->addTab(createIpTab("Destination IPs"), "Destinations");

    // --- 3. Cài đặt Layout chính ---
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(statsLayout); // Thêm các label tổng quan
    mainLayout->addWidget(m_tabWidget);
    setLayout(mainLayout);

    // --- 4. Kết nối tín hiệu "SỐNG" (LIVE) ---
    connect(m_statsManager, &StatisticsManager::statsDataUpdated,
            this, &StatisticsDialog::onStatsUpdated);

    // --- 5. Tải dữ liệu lần đầu tiên ---
    onStatsUpdated();
}

/**
 * @brief (THÊM MỚI) Slot này được gọi mỗi khi có gói tin mới.
 */
void StatisticsDialog::onStatsUpdated()
{
    // 1. Cập nhật các Label tổng quan
    m_totalPacketsLabel->setText(QString("Total Packets: %1").arg(m_statsManager->getTotalPacketCount()));
    m_totalProtocolsLabel->setText(QString("Total Protocol Types: %1").arg(m_statsManager->getProtocolTypeCount()));

    // 2. Lấy dữ liệu mới
    QMap<QString, qint64> protocolCounts = m_statsManager->getProtocolCounts();
    QMap<QString, qint64> sourceIpCounts = m_statsManager->getSourceIpCounts();
    QMap<QString, qint64> destIpCounts = m_statsManager->getDestinationIpCounts();

    // 3. Xóa và điền lại dữ liệu cho các cây
    populateTree(m_protocolTree, protocolCounts);
    populateTree(m_sourceTree, sourceIpCounts);
    populateTree(m_destTree, destIpCounts);
}

/**
 * @brief (ĐÃ THAY ĐỔI) Chỉ tạo UI
 */
QWidget* StatisticsDialog::createProtocolTab()
{
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    m_protocolTree = new QTreeWidget(); // <-- Gán cho biến thành viên
    m_protocolTree->setColumnCount(2);
    m_protocolTree->setHeaderLabels(QStringList() << "Protocol" << "Packet Count");
    m_protocolTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    layout->addWidget(m_protocolTree);
    return widget;
}

/**
 * @brief (ĐÃ THAY ĐỔI) Chỉ tạo UI
 */
QWidget* StatisticsDialog::createIpTab(const QString& title)
{
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    QTreeWidget* tree; // Biến tạm
    if (title.contains("Source")) {
        m_sourceTree = new QTreeWidget(); // <-- Gán cho biến thành viên
        tree = m_sourceTree;
    } else {
        m_destTree = new QTreeWidget(); // <-- Gán cho biến thành viên
        tree = m_destTree;
    }

    tree->setColumnCount(2);
    tree->setHeaderLabels(QStringList() << title << "Packet Count");
    tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);

    layout->addWidget(tree);
    return widget;
}

/**
 * @brief (ĐÃ THAY ĐỔI) Hàm này bây giờ sẽ xóa trước khi điền
 */
void StatisticsDialog::populateTree(QTreeWidget *tree, const QMap<QString, qint64> &counts)
{
    // Lưu lại cột đang sắp xếp
    int sortColumn = tree->sortColumn();
    Qt::SortOrder sortOrder = tree->header()->sortIndicatorOrder();

    tree->clear(); // <-- Xóa dữ liệu cũ

    for (auto it = counts.constBegin(); it != counts.constEnd(); ++it) {
        auto *item = new QTreeWidgetItem(tree);
        item->setText(0, it.key());

        // Đặt dữ liệu số vào cột 1 để sắp xếp đúng
        item->setData(1, Qt::DisplayRole, QVariant::fromValue(it.value()));
    }

    // Khôi phục lại sắp xếp
    tree->sortByColumn(sortColumn, sortOrder);
}
