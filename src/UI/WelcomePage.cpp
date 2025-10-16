#include "WelcomePage.hpp"
#include "NetworkScanner.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QGraphicsDropShadowEffect>
#include <QFont>
#include <QPair> // Thư viện cần thiết

WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent) // THAY ĐỔI: Khớp với lớp cơ sở trong file .hpp
{
    setupUI(); // Gọi hàm đã được khai báo
}

// THAY ĐỔI: Tên hàm khớp với khai báo trong file .hpp
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

    // --- Welcome Label ---
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
    pageLayout->addWidget(welcome, 0, Qt::AlignLeft | Qt::AlignTop);

    // --- Capture Section ---
    auto *captureLabel = new QLabel("Capture");
    captureLabel->setStyleSheet("margin: 0px; padding: 10px 0px; color: #2C3E50; font-weight: bold; font-family: 'Segoe UI', Arial, sans-serif;");
    captureLabel->setFont(QFont("Segoe UI", 24, QFont::Bold));
    captureLabel->setAlignment(Qt::AlignLeft);

    // --- Filter Section ---
    auto *filterLabel = new QLabel("Filter:");
    filterLabel->setStyleSheet("margin: 0px; padding-right: 4px; color: #2C3E50; font-weight: 500; font-family: 'Segoe UI', Arial, sans-serif;");
    filterLabel->setFont(QFont("Segoe UI", 15, QFont::DemiBold));
    auto *filterEdit = new QLineEdit();
    filterEdit->setFixedWidth(900);
    filterEdit->setPlaceholderText("Enter filter expression...");
    filterEdit->setStyleSheet(R"(
        QLineEdit { border: 1px solid #BDC3C7; border-radius: 8px; padding: 8px; background-color: white; }
        QLineEdit:focus { border: 2px solid #4A90E2; outline: none; }
    )");
    filterEdit->setFont(QFont("Segoe UI", 10));

    auto *filterLayout = new QHBoxLayout();
    filterLayout->setSpacing(10);
    filterLayout->setContentsMargins(0, 0, 0, 0);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterEdit);
    filterLayout->addStretch();

    auto *captureSectionLayout = new QVBoxLayout();
    captureSectionLayout->setSpacing(6);
    captureSectionLayout->addWidget(captureLabel);
    captureSectionLayout->addLayout(filterLayout);
    pageLayout->addLayout(captureSectionLayout);

    // --- Interface List ---
    auto *list = new QListWidget(pageContent);

    // Lấy danh sách thiết bị và điền vào list widget
    const auto devices = NetworkScanner::getDevices(); // Gọi hàm với ()
    for (const auto &devicePair : devices) {
        QString systemName = devicePair.first;
        QString description = QString("%1 (%2)").arg(devicePair.second).arg(systemName);
        QListWidgetItem *item = new QListWidgetItem(description);
        item->setData(Qt::UserRole, systemName);
        list->addItem(item);
    }

    list->setStyleSheet(R"(
        QListWidget { border: 1px solid #BDC3C7; border-radius: 8px; background-color: #FAFAFA; font-size: 12pt; padding: 6px; }
        QListWidget::item { padding: 6px 10px; }
        QListWidget::item:selected { background-color: #4A90E2; color: white; }
    )");
    pageLayout->addWidget(list);

    mainLayout->addWidget(pageContent);

    // Kết nối tín hiệu, giờ sẽ hoạt động chính xác
    connect(list, &QListWidget::itemDoubleClicked, this, [this](QListWidgetItem *item) {
        QString interfaceName = item->data(Qt::UserRole).toString();
        if (!interfaceName.isEmpty()) {
            emit interfaceSelected(interfaceName);
        }
    });
}
