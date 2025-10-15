#include "WelcomePage.hpp"
#include "NetworkScanner.hpp"
#include <QVBoxLayout>
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QFont>

/* -------------------- Constructor -------------------- */
WelcomePage::WelcomePage(QWidget *parent)
    : QMainWindow(parent)
{
    setupCentralWidget(this);
}

/* -------------------- Setup Central Widget -------------------- */
void WelcomePage::setupCentralWidget(QWidget *parent)
{
    QWidget *content = new QWidget(parent);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // --- Nội dung trang chính ---
    QWidget *pageContent = new QWidget(content);
    pageContent->setStyleSheet("background-color: white;");
    QVBoxLayout *pageLayout = new QVBoxLayout(pageContent);
    pageLayout->setContentsMargins(400, 80, 400, 50);
    pageLayout->setSpacing(10);

    // --- Welcome ---
    QLabel *welcome = new QLabel("Welcome to WiresharkMini");
    welcome->setAlignment(Qt::AlignCenter); // Căn giữa chữ trong ô

    welcome->setStyleSheet(R"(
        QLabel {
            color: white;
            background-color: #4A90E2;
            font-size: 24px;
            font-weight: bold;
            font-family: 'Segoe UI', Arial, sans-serif;
            padding: 12px 24px;
            border-radius: 25px;
            border: 2px solid rgba(255, 255, 255, 0.2);
            margin: 0px;
        }
    )");

    welcome->setFont(QFont("Segoe UI", 24, QFont::Bold));

    // Hiệu ứng shadow mềm mại, nổi khối
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(welcome);
    shadow->setBlurRadius(25); // Tăng blur để shadow lan tỏa tự nhiên hơn
    shadow->setOffset(3, 3); // Offset nhẹ để shadow không quá xa
    shadow->setColor(QColor(0, 0, 0, 120)); // Đen mờ hơn một chút cho độ sâu
    welcome->setGraphicsEffect(shadow);

    // Làm widget vừa khít nội dung (chỉ bao quanh chữ + padding)
    welcome->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    welcome->adjustSize(); // Tính toán kích thước chính xác sau style

    // Add vào layout: Căn giữa ngang (và trên nếu cần)
    pageLayout->addWidget(welcome, 0, Qt::AlignLeft | Qt::AlignTop); // Căn giữa trong pageLayout

    // --- Capture ---
    QLabel *captureLabel = new QLabel("Capture");
    captureLabel->setStyleSheet("margin: 0px; padding: 10px 0px; color: #2C3E50; font-weight: bold; font-family: 'Segoe UI', Arial, sans-serif;");
    captureLabel->setFont(QFont("Segoe UI", 24, QFont::Bold));
    captureLabel->setAlignment(Qt::AlignLeft);

    // --- Filter ---
    QLabel *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("margin: 0px; padding-right: 4px; color: #2C3E50; font-weight: 500; font-family: 'Segoe UI', Arial, sans-serif;");
    filterLabel->setFont(QFont("Segoe UI", 15, QFont::DemiBold));

    QLineEdit *filterEdit = new QLineEdit();
    filterEdit->setFixedWidth(900);
    filterEdit->setPlaceholderText("Enter filter expression...");
    filterEdit->setStyleSheet(R"(
        QLineEdit {
            border: 1px solid #BDC3C7;
            border-radius: 8px;
            padding: 8px;
            font-family: 'Segoe UI', Arial, sans-serif;
            background-color: white;
        }
        QLineEdit:focus {
            border: 2px solid #4A90E2;
            outline: none;
        }
    )");
    filterEdit->setFont(QFont("Segoe UI", 10));

    // --- Layout ---
    QHBoxLayout *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterEdit);
    filterLayout->addStretch();

    // --- Gộp Capture + Filter ---
    QVBoxLayout *captureSectionLayout = new QVBoxLayout();
    captureSectionLayout->setSpacing(6);  // khoảng cách nhỏ giữa Capture và Filter
    captureSectionLayout->setContentsMargins(0, 0, 0, 0);
    captureSectionLayout->addWidget(captureLabel);
    captureSectionLayout->addLayout(filterLayout);

    // Thêm vào layout chính
    pageLayout->addLayout(captureSectionLayout);

    QListWidget *list = new QListWidget(pageContent);
    QStringList devices = NetworkScanner::getDevices();
    list->addItems(devices);

    list->setStyleSheet(R"(
        QListWidget {
            border: 1px solid #BDC3C7;
            border-radius: 8px;
            background-color: #FAFAFA;
            font-family: 'Segoe UI', Arial, sans-serif;
            font-size: 12pt;
            padding: 6px;
        }
        QListWidget::item {
            padding: 6px 10px;
        }
        QListWidget::item:selected {
            background-color: #4A90E2;
            color: white;
        }
    )");
    pageLayout->addWidget(list);

    /* -------------------- FINALIZE -------------------- */
    layout->addWidget(pageContent);
    setCentralWidget(content);

    // 🔹 Kết nối double-click
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        emit interfaceSelected(item->text());
    });
}
