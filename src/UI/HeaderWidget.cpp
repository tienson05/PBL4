#include "HeaderWidget.hpp"
#include <QMessageBox>

/* -------------------- Constructor -------------------- */
HeaderWidget::HeaderWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupTitleBar(this, mainLayout);
    setupMenuBar(this, mainLayout);
}

/* -------------------- Custom Title Bar -------------------- */
void HeaderWidget::setupTitleBar(QWidget *parent, QVBoxLayout *mainLayout)
{
    QFrame *titleBar = new QFrame(parent);
    titleBar->setFixedHeight(40);
    titleBar->setStyleSheet(
        "background-color: #2C3E50; color: white;"
        "border-top-left-radius: 8px; border-top-right-radius: 8px;"
        );

    QLabel *titleLabel = new QLabel("WiresharkMini", titleBar);
    titleLabel->setStyleSheet("font-weight: bold; margin-left: 15px; font-size: 14px; font-family: 'Segoe UI';");
    titleLabel->setFont(QFont("Segoe UI", 14, QFont::Bold));

    QPushButton *minBtn = new QPushButton("-", titleBar);
    QPushButton *maxBtn = new QPushButton("□", titleBar);
    QPushButton *closeBtn = new QPushButton("x", titleBar);

    QString btnStyle = R"(
        QPushButton {
            background: transparent;
            color: white;
            border: none;
            font-size: 18px;
            width: 40px;
            height: 40px;
            border-radius: 20px;
        }
        QPushButton:hover { background-color: rgba(255, 255, 255, 0.1); }
        QPushButton:pressed { background-color: rgba(255, 255, 255, 0.2); }
    )";

    minBtn->setStyleSheet(btnStyle);
    maxBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(btnStyle + "QPushButton:hover { background-color: #E74C3C; }");

    QHBoxLayout *layout = new QHBoxLayout(titleBar);
    layout->setContentsMargins(10, 0, 5, 0);
    layout->setSpacing(0);
    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(minBtn);
    layout->addWidget(maxBtn);
    layout->addWidget(closeBtn);

    // Gắn tín hiệu nút
    connect(minBtn, &QPushButton::clicked, this, &HeaderWidget::minimizeRequested);
    connect(maxBtn, &QPushButton::clicked, this, &HeaderWidget::maximizeRequested);
    connect(closeBtn, &QPushButton::clicked, this, &HeaderWidget::closeRequested);

    mainLayout->addWidget(titleBar);
}

/* -------------------- Custom Menu Bar -------------------- */
void HeaderWidget::setupMenuBar(QWidget *parent, QVBoxLayout *mainLayout)
{
    QFrame *menuBar = new QFrame(parent);
    menuBar->setFixedHeight(35);
    menuBar->setStyleSheet("background-color: #ECF0F1; border-bottom: 1px solid #D5D8DC;");

    QHBoxLayout *menuLayout = new QHBoxLayout(menuBar);
    menuLayout->setContentsMargins(10, 0, 0, 0);
    menuLayout->setSpacing(25);

    QFont menuFont("Segoe UI", 10, QFont::Medium);

    // --- File Menu ---
    QPushButton *fileBtn = new QPushButton("File");
    fileBtn->setStyleSheet("background: transparent; color: #2C3E50; border: none; font-weight: medium; padding: 0px 10px;");
    fileBtn->setFont(menuFont);
    QMenu *fileMenu = new QMenu(fileBtn);
    fileMenu->setStyleSheet(
        "QMenu { background-color: white; border: 1px solid #D5D8DC; border-radius: 4px; }"
        "QMenu::item { padding: 8px 20px; }"
        "QMenu::item:selected { background-color: #4A90E2; color: white; }"
        );
    QAction *newAct = fileMenu->addAction("New");
    QAction *openAct = fileMenu->addAction("Open");
    fileMenu->addSeparator();
    QAction *exitAct = fileMenu->addAction("Exit");
    fileBtn->setMenu(fileMenu);

    connect(newAct, &QAction::triggered, this, &HeaderWidget::onFileNew);
    connect(openAct, &QAction::triggered, this, &HeaderWidget::onFileOpen);
    connect(exitAct, &QAction::triggered, this, &HeaderWidget::closeRequested);

    // --- Edit Menu ---
    QPushButton *editBtn = new QPushButton("Edit");
    editBtn->setStyleSheet(fileBtn->styleSheet());
    editBtn->setFont(menuFont);
    QMenu *editMenu = new QMenu(editBtn);
    editMenu->setStyleSheet(fileMenu->styleSheet());
    QAction *cutAct = editMenu->addAction("Cut");
    QAction *copyAct = editMenu->addAction("Copy");
    editBtn->setMenu(editMenu);

    connect(cutAct, &QAction::triggered, this, &HeaderWidget::onEditCut);
    connect(copyAct, &QAction::triggered, this, &HeaderWidget::onEditCopy);

    // --- Capture Menu ---
    QPushButton *captureBtn = new QPushButton("Capture");
    captureBtn->setStyleSheet(fileBtn->styleSheet());
    captureBtn->setFont(menuFont);
    QMenu *captureMenu = new QMenu(captureBtn);
    captureMenu->setStyleSheet(fileMenu->styleSheet());
    QAction *startAct = captureMenu->addAction("Start");
    connect(startAct, &QAction::triggered, [=]() {
        QMessageBox::information(this, "Capture", "Start clicked");
    });
    captureBtn->setMenu(captureMenu);

    // Thêm vào layout
    menuLayout->addWidget(fileBtn);
    menuLayout->addWidget(editBtn);
    menuLayout->addWidget(captureBtn);
    menuLayout->addStretch();

    // Hover effect
    QString hoverStyle = "QPushButton:hover { color: #4A90E2; }";
    fileBtn->setStyleSheet(fileBtn->styleSheet() + hoverStyle);
    editBtn->setStyleSheet(editBtn->styleSheet() + hoverStyle);
    captureBtn->setStyleSheet(captureBtn->styleSheet() + hoverStyle);

    mainLayout->addWidget(menuBar);
}

/* -------------------- Slots -------------------- */
void HeaderWidget::minimizeRequested() { window()->showMinimized(); }
void HeaderWidget::maximizeRequested()
{
    if (window()->isMaximized())
        window()->showNormal();
    else
        window()->showMaximized();
}
void HeaderWidget::closeRequested() { window()->close(); }

void HeaderWidget::onFileNew() { QMessageBox::information(this, "File", "New file created."); }
void HeaderWidget::onFileOpen() { QMessageBox::information(this, "File", "Open file dialog."); }
void HeaderWidget::onEditCut() { QMessageBox::information(this, "Edit", "Cut action performed."); }
void HeaderWidget::onEditCopy() { QMessageBox::information(this, "Edit", "Copy action performed."); }
