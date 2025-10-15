#pragma once

#include <QMainWindow>

class WelcomePage : public QMainWindow
{
    Q_OBJECT

public:
    explicit WelcomePage(QWidget *parent = nullptr);

signals:
    void interfaceSelected(const QString &interfaceName);

private:
    void setupCentralWidget(QWidget *parent);
};
