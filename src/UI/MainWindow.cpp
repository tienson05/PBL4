#include "MainWindow.hpp"
#include "HeaderWidget.hpp"
#include "WelcomePage.hpp"
#include "CapturePage.hpp"

#include <QVBoxLayout>
#include <QStackedWidget>

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
    mainLayout->addWidget(stackedWidget, 1);  // chiếm toàn bộ phần còn lại

    // --- Tạo các trang ---
    welcomePage = new WelcomePage(this);
    capturePage = new CapturePage(this);

    // Thêm các trang vào stacked widget
    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(capturePage);

    // Gán central widget
    setCentralWidget(central);

    // Khi double-click interface trong WelcomePage, chuyển sang CapturePage
    connect(welcomePage, &WelcomePage::interfaceSelected, this, &MainWindow::showCapturePage);
}

void MainWindow::showCapturePage()
{
    stackedWidget->setCurrentWidget(capturePage);
}
