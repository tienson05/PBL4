#include "MainWindow.hpp"
#include "HeaderWidget.hpp"
#include "WelcomePage.hpp"
#include "CapturePage.hpp"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QFileDialog>
#include <QDir>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // ... (Phần setup UI giữ nguyên) ...
    QWidget *central = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    header = new HeaderWidget(this);
    mainLayout->addWidget(header);
    stackedWidget = new QStackedWidget(this);
    mainLayout->addWidget(stackedWidget, 1);
    welcomePage = new WelcomePage(this);
    capturePage = new CapturePage(this);
    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(capturePage);
    setCentralWidget(central);
    setWindowTitle("WiresharkMini");
    resize(1280, 800);

    // --- KẾT NỐI TÍN HIỆU ---
    // 1. (KHÔNG THAY ĐỔI) Dòng connect này sẽ tự động
    //    khớp với signal/slot mới vì tên hàm giống nhau.
    connect(welcomePage, &WelcomePage::interfaceSelected, this, &MainWindow::showCapturePage);

    // 2. (Giữ nguyên)
    connect(welcomePage, &WelcomePage::openFileRequested, this, &MainWindow::handleOpenFileRequest);
    // 3. (Giữ nguyên)
    connect(header, &HeaderWidget::openFileRequested, this, &MainWindow::handleOpenFileRequest);
    connect(header, &HeaderWidget::saveFileRequested, this, &MainWindow::handleSaveFileRequest);
}

// --- THAY ĐỔI: Thêm tham số captureFilter ---
void MainWindow::showCapturePage(const QString &interfaceName, const QString &captureFilter)
{
    stackedWidget->setCurrentWidget(capturePage);

    // --- THAY ĐỔI: Truyền filter xuống CapturePage ---
    capturePage->startInitialCapture(interfaceName, captureFilter);
}

void MainWindow::handleOpenFileRequest()
{
    // ... (Hàm này giữ nguyên) ...
    QString appDir = QCoreApplication::applicationDirPath();
    QString defaultDir = appDir + "/FileSave";
    QDir dir(defaultDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Mở File Pcap"),
        defaultDir,
        tr("Packet Capture Files (*.pcap *.pcapng);;All Files (*)")
        );
    if (!filePath.isEmpty()) {
        stackedWidget->setCurrentWidget(capturePage);
        capturePage->startCaptureFromFile(filePath);
    }
}

void MainWindow::handleSaveFileRequest()
{
    // ... (Hàm này giữ nguyên) ...
    if (stackedWidget->currentWidget() == capturePage) {
        capturePage->saveCurrentCapture();
    }
}
