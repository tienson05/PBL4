#pragma once
#include <QWidget>

class QListWidget;

class NetworkViewer : public QWidget {
    Q_OBJECT
public:
    explicit NetworkViewer(QWidget *parent = nullptr);
};
