#pragma once

#include <QWidget> // THAY ĐỔI: Kế thừa từ QWidget

// Khai báo trước để giảm thời gian biên dịch
class QListWidgetItem;

class WelcomePage : public QWidget // THAY ĐỔI: Lớp cơ sở là QWidget
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

signals:
    // Tín hiệu này đã đúng, không cần thay đổi
    void interfaceSelected(const QString &interfaceName);

private:
    // THAY ĐỔI: Thêm khai báo cho hàm setupUI
    void setupUI();
};
