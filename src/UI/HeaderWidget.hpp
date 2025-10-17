#pragma once

#include <QWidget>

class QVBoxLayout;

class HeaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit HeaderWidget(QWidget *parent = nullptr);

signals:
    // Tín hiệu cho menu File
    void openFileRequested(); // Giữ lại Open
    void saveFileRequested(); // Giữ lại Save As

    // Tín hiệu cho menu Edit
    void editCutRequested();
    void editCopyRequested();

    // Tín hiệu cho menu Capture
    void captureStartRequested();

private slots:
    // Các slot này chỉ để điều khiển cửa sổ
    void minimizeRequested();
    void maximizeRequested();
    void closeRequested();

private:
    void setupTitleBar(QWidget *parent, QVBoxLayout *mainLayout);
    void setupMenuBar(QWidget *parent, QVBoxLayout *mainLayout);
};
