#include "WelcomePage.hpp"
#include "NetworkScanner.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPair>

WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void WelcomePage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    auto *pageContent = new QWidget(this);
    pageContent->setStyleSheet("background-color: white;");
    auto *pageLayout = new QVBoxLayout(pageContent);
    pageLayout->setContentsMargins(400, 80, 400, 50);
    pageLayout->setSpacing(10);

    // --- Welcome Banner (Lấy từ giao diện chi tiết của file 1) ---
    auto *welcome = new QLabel("Welcome to WiresharkMini");
    welcome->setAlignment(Qt::AlignCenter);
    welcome->setStyleSheet(R"(
        QLabel {
            color: white; background-color: #4A90E2; font-size: 24px;
            font-weight: bold; font-family: 'Segoe UI', Arial, sans-serif;
            padding: 12px 24px; border-radius: 25px;
            border: 2px solid rgba(255, 255, 255, 0.2); margin: 0px;
        }
    )");
    welcome->setFont(QFont("Segoe UI", 24, QFont::Bold));
    auto *shadow = new QGraphicsDropShadowEffect(welcome);
    shadow->setBlurRadius(25);
    shadow->setOffset(3, 3);
    shadow->setColor(QColor(0, 0, 0, 120));
    welcome->setGraphicsEffect(shadow);
    welcome->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    welcome->adjustSize();
    pageLayout->addWidget(welcome, 0, Qt::AlignHCenter); // Căn giữa cho đẹp hơn

    // --- Capture & Filter Section (Lấy từ giao diện chi tiết của file 1) ---
    auto *captureLabel = new QLabel("Capture");
    captureLabel->setStyleSheet("margin: 20px 0px 0px 0px; padding: 10px 0px; color: #2C3E50; font-weight: bold; font-family: 'Segoe UI';");
    captureLabel->setFont(QFont("Segoe UI", 24, QFont::Bold));

    auto *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("margin: 0px; padding-right: 4px; color: #2C3E50; font-weight: 500; font-family: 'Segoe UI';");
    filterLabel->setFont(QFont("Segoe UI", 15, QFont::DemiBold));

    auto *filterEdit = new QLineEdit();
    filterEdit->setPlaceholderText("Enter a capture filter...");
    filterEdit->setStyleSheet(R"(
        QLineEdit { border: 1px solid #BDC3C7; border-radius: 8px; padding: 8px; background-color: white; }
        QLineEdit:focus { border: 2px solid #4A90E2; outline: none; }
    )");
    filterEdit->setFont(QFont("Segoe UI", 10));

    auto *filterLayout = new QHBoxLayout();
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterEdit);

    auto *captureSectionLayout = new QVBoxLayout();
    captureSectionLayout->setSpacing(6);
    captureSectionLayout->addWidget(captureLabel);
    captureSectionLayout->addLayout(filterLayout);
    pageLayout->addLayout(captureSectionLayout);

    // --- Interface List (Lấy logic đúng từ file 2) ---
    auto *list = new QListWidget(pageContent);
    const auto devices = NetworkScanner::getDevices();
    for (const auto &devicePair : devices) {
        QString systemName = devicePair.first;
        QString description = QString("%1 (%2)").arg(devicePair.second).arg(systemName);
        QListWidgetItem *item = new QListWidgetItem(description);
        item->setData(Qt::UserRole, systemName); // Lưu tên hệ thống để sử dụng sau
        list->addItem(item);
    }
    list->setStyleSheet(R"(
        QListWidget { border: 1px solid #BDC3C7; border-radius: 8px; background-color: #FAFAFA; font-size: 12pt; padding: 6px; }
        QListWidget::item { padding: 6px 10px; }
        QListWidget::item:selected { background-color: #4A90E2; color: white; }
    )");
    pageLayout->addWidget(list);

    // --- Nút "Mở File Pcap" (Lấy từ file 2) ---
    auto *openFileBtn = new QPushButton("Mở File Pcap...", this);
    openFileBtn->setMinimumHeight(40);
    openFileBtn->setCursor(Qt::PointingHandCursor);
    openFileBtn->setStyleSheet(R"(
        QPushButton { font-size: 14px; font-weight: bold; color: #34495e; padding: 10px; margin-top: 15px; background-color: #ecf0f1; border: 1px solid #bdc3c7; border-radius: 5px; }
        QPushButton:hover { background-color: #dbe0e2; border-color: #a1a6a9; }
    )");
    pageLayout->addWidget(openFileBtn);
    pageLayout->setAlignment(openFileBtn, Qt::AlignCenter);

    mainLayout->addWidget(pageContent);

    // --- KẾT NỐI TÍN HIỆU (Lấy logic đúng từ file 2) ---
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        QString interfaceName = item->data(Qt::UserRole).toString();
        if (!interfaceName.isEmpty()) {
            emit interfaceSelected(interfaceName);
        }
    });

    connect(openFileBtn, &QPushButton::clicked, this, &WelcomePage::openFileRequested);
}
