#include "HeaderWidget.hpp"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QFont>

/* -------------------- Constructor -------------------- */
HeaderWidget::HeaderWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTitleBar(this, mainLayout);
    setupMenuBar(this, mainLayout);
}

/* -------------------- Custom Title Bar -------------------- */
void HeaderWidget::setupTitleBar(QWidget *parent, QVBoxLayout *mainLayout)
{
    // Phần code này không thay đổi, giữ nguyên
    QFrame *titleBar = new QFrame(parent);
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet("background-color: #2C3E50; color: white; border-top-left-radius: 8px; border-top-right-radius: 8px;");
    QLabel *titleLabel = new QLabel("WiresharkMini", titleBar);
    titleLabel->setStyleSheet("font-weight: bold; margin-left: 15px; font-size: 14px; font-family: 'Segoe UI';");
    QPushButton *minBtn = new QPushButton("-", titleBar);
    QPushButton *maxBtn = new QPushButton("□", titleBar);
    QPushButton *closeBtn = new QPushButton("x", titleBar);
    QString btnStyle = R"(QPushButton { background: transparent; color: white; border: none; font-size: 18px; width: 40px; height: 40px; } QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); } QPushButton:pressed { background-color: rgba(255, 255, 255, 0.2); })";
    minBtn->setStyleSheet(btnStyle);
    maxBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(btnStyle + "QPushButton:hover { background-color: #E74C3C; }");
    QHBoxLayout *layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(minBtn);
    layout->addWidget(maxBtn);
    layout->addWidget(closeBtn);
    connect(minBtn, &QPushButton::clicked, this, &HeaderWidget::minimizeRequested);
    connect(maxBtn, &QPushButton::clicked, this, &HeaderWidget::maximizeRequested);
    connect(closeBtn, &QPushButton::clicked, this, &HeaderWidget::closeRequested);
    mainLayout->addWidget(titleBar);
}

/* -------------------- Custom Menu Bar (Đã cập nhật) -------------------- */
void HeaderWidget::setupMenuBar(QWidget *parent, QVBoxLayout *mainLayout)
{
    QFrame *menuBar = new QFrame(parent);
    menuBar->setFixedHeight(35);
    menuBar->setStyleSheet("background-color: #ECF0F1; border-bottom: 1px solid #D5D8DC;");
    QHBoxLayout *menuLayout = new QHBoxLayout(menuBar);
    menuLayout->setContentsMargins(10, 0, 0, 0);
    menuLayout->setSpacing(25);
    QFont menuFont("Segoe UI", 10);
    QString menuBtnStyle = "background: transparent; border: none; padding: 0px 10px;";

    // --- File Menu ---
    QPushButton *fileBtn = new QPushButton("File");
    fileBtn->setStyleSheet(menuBtnStyle);
    fileBtn->setFont(menuFont);
    QMenu *fileMenu = new QMenu(fileBtn);
    fileMenu->setStyleSheet("QMenu { background-color: white; border: 1px solid #D5D8DC; } QMenu::item:selected { background-color: #4A90E2; color: white; }");

    QAction *openAct = fileMenu->addAction("Open...");
    QAction *saveAsAct = fileMenu->addAction("Save As...");
    fileMenu->addSeparator();
    QAction *exitAct = fileMenu->addAction("Exit");
    fileBtn->setMenu(fileMenu);

    connect(openAct, &QAction::triggered, this, &HeaderWidget::openFileRequested);
    connect(saveAsAct, &QAction::triggered, this, &HeaderWidget::saveFileRequested);
    connect(exitAct, &QAction::triggered, this, &HeaderWidget::closeRequested);

    // --- Edit Menu ---
    QPushButton *editBtn = new QPushButton("Edit");
    editBtn->setStyleSheet(menuBtnStyle);
    editBtn->setFont(menuFont);
    QMenu *editMenu = new QMenu(editBtn);
    editMenu->setStyleSheet(fileMenu->styleSheet());
    QAction *cutAct = editMenu->addAction("Cut");
    QAction *copyAct = editMenu->addAction("Copy");
    editBtn->setMenu(editMenu);

    connect(cutAct, &QAction::triggered, this, &HeaderWidget::editCutRequested);
    connect(copyAct, &QAction::triggered, this, &HeaderWidget::editCopyRequested);

    // --- Capture Menu ---
    QPushButton *captureBtn = new QPushButton("Capture");
    captureBtn->setStyleSheet(menuBtnStyle);
    captureBtn->setFont(menuFont);
    QMenu *captureMenu = new QMenu(captureBtn);
    captureMenu->setStyleSheet(fileMenu->styleSheet());
    QAction *startAct = captureMenu->addAction("Start");
    captureBtn->setMenu(captureMenu);

    connect(startAct, &QAction::triggered, this, &HeaderWidget::captureStartRequested);

    // Thêm các nút menu vào layout
    menuLayout->addWidget(fileBtn);
    menuLayout->addWidget(editBtn);
    menuLayout->addWidget(captureBtn);
    menuLayout->addStretch();

    mainLayout->addWidget(menuBar);
}

/* -------------------- Slots -------------------- */
void HeaderWidget::minimizeRequested() { if(window()) window()->showMinimized(); }
void HeaderWidget::maximizeRequested() {
    if (window()) {
        if (window()->isMaximized()) window()->showNormal();
        else window()->showMaximized();
    }
}
void HeaderWidget::closeRequested() { if(window()) window()->close(); }
