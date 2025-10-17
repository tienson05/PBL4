#include "MainWindow.hpp"
#include "HeaderWidget.hpp"
#include "WelcomePage.hpp"
#include "CapturePage.hpp"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QFileDialog> // Đảm bảo có dòng này
#include <QDir>
#include <QCoreApplication>   // THÊM MỚI

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // --- Tạo widget trung tâm ---
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- Header cố định ---
    header = new HeaderWidget(this);
    mainLayout->addWidget(header);

    // --- Stacked Widget để chuyển trang ---
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget, 1);

    // --- Tạo các trang ---
    welcomePage = new WelcomePage(this);
    capturePage = new CapturePage(this);

    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(capturePage);

    setCentralWidget(central);
    setWindowTitle("WiresharkMini");
    resize(1280, 800);

    // --- KẾT NỐI TÍN HIỆU ---
    // 1. Từ WelcomePage -> MainWindow (để bắt đầu live capture)
    connect(welcomePage, &WelcomePage::interfaceSelected, this, &MainWindow::showCapturePage);

    // 2. THÊM MỚI: Từ WelcomePage -> MainWindow (để mở file từ nút bấm)
    connect(welcomePage, &WelcomePage::openFileRequested, this, &MainWindow::handleOpenFileRequest);

    // 3. Từ HeaderWidget -> MainWindow (để Mở/Lưu file từ menu)
    connect(header, &HeaderWidget::openFileRequested, this, &MainWindow::handleOpenFileRequest);
    connect(header, &HeaderWidget::saveFileRequested, this, &MainWindow::handleSaveFileRequest);
}

void MainWindow::showCapturePage(const QString &interfaceName)
{
    stackedWidget->setCurrentWidget(capturePage);
    capturePage->startInitialCapture(interfaceName);
}

void MainWindow::handleOpenFileRequest()
{
    // 1. Lấy đường dẫn thư mục chứa file thực thi (.exe hoặc App)
    QString appDir = QCoreApplication::applicationDirPath();
    // 2. Tạo đường dẫn đến thư mục 'FileSave'
    QString defaultDir = appDir + "/FileSave";
    // 3. Tạo thư mục nếu nó chưa tồn tại
    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Mở File Pcap"),
        defaultDir, // <-- THAY ĐỔI: Sử dụng đường dẫn mặc định mới
        tr("Packet Capture Files (*.pcap *.pcapng);;All Files (*)")
        );

    if (!filePath.isEmpty()) {
        stackedWidget->setCurrentWidget(capturePage);
        capturePage->startCaptureFromFile(filePath);
    }
}

void MainWindow::handleSaveFileRequest()
{
    if (stackedWidget->currentWidget() == capturePage) {
        capturePage->saveCurrentCapture();
    }
}
