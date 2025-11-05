#ifndef STATISTICSDIALOG_HPP
#define STATISTICSDIALOG_HPP

#include <QDialog>
#include <QMap>     // <-- Thêm QMap
#include <QString>  // <-- Thêm QString

// Khai báo trước
class QTabWidget;
class QWidget;
class QTreeWidget;
class StatisticsManager;
class QLabel; // <-- Thêm QLabel

class StatisticsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StatisticsDialog(StatisticsManager *statsManager, QWidget *parent = nullptr);

private slots:
    /**
     * @brief (THÊM MỚI) Slot này sẽ được gọi mỗi khi có gói tin mới.
     * Nó sẽ vẽ lại toàn bộ dialog.
     */
    void onStatsUpdated();

private:
    // Hàm trợ giúp để TẠO các tab (chỉ chạy 1 lần)
    QWidget* createProtocolTab();
    QWidget* createIpTab(const QString& title);

    // Hàm trợ giúp để ĐIỀN DỮ LIỆU vào cây
    void populateTree(QTreeWidget *tree, const QMap<QString, qint64> &counts);

    // --- THÊM MỚI: Biến thành viên ---
    StatisticsManager *m_statsManager; // Con trỏ để lấy dữ liệu

    QLabel *m_totalPacketsLabel;       // Label cho tổng số gói
    QLabel *m_totalProtocolsLabel;     // Label cho tổng số loại protocol

    QTreeWidget *m_protocolTree;       // Con trỏ đến cây protocol
    QTreeWidget *m_sourceTree;         // Con trỏ đến cây IP nguồn
    QTreeWidget *m_destTree;           // Con trỏ đến cây IP đích

    QTabWidget *m_tabWidget;
};

#endif // STATISTICSDIALOG_HPP
