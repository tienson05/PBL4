#pragma once
#include <QWidget>

class WelcomePage : public QWidget
{
    Q_OBJECT
public:
    explicit WelcomePage(QWidget *parent = nullptr);
signals:
    void interfaceSelected(const QString &interfaceName);
    void openFileRequested(); // TÍN HIỆU MỚI
private:
    void setupUI();
};
