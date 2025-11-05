#pragma once
#include <QWidget>

class QLineEdit; // <-- THÊM MỚI: Khai báo trước cho QLineEdit

class WelcomePage : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);
signals:
    /**
     * @brief Tín hiệu này đã được THAY ĐỔI.
     * Nó được phát ra khi người dùng chọn 1 interface,
     * mang theo tên interface VÀ chuỗi filter (có thể rỗng).
     * @param interfaceName Tên hệ thống của interface (ví dụ: 'wlo1')
     * @param captureFilter Chuỗi filter BPF (ví dụ: 'tcp port 80')
     */
    void interfaceSelected(const QString &interfaceName, const QString &captureFilter); // <-- THAY ĐỔI

    void openFileRequested();

private:
    void setupUI();

    // --- THÊM MỚI: Biến thành viên ---
    QLineEdit *filterEdit; // Biến này sẽ lưu con trỏ đến ô filter
};
