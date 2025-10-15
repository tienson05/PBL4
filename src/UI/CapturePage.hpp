#pragma once
#include <QWidget>

class QLabel;

class CapturePage : public QWidget
{
    Q_OBJECT

public:
    explicit CapturePage(QWidget *parent = nullptr);
    void startCapture(const QString &interfaceName);

signals:
    void backToWelcome();  // nếu sau này bạn có nút quay lại

private:
    QLabel *statusLabel;
};
