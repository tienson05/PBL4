#pragma once

#include <QString>
#include <QPair>

class NetworkScanner {
public :
    static QList<QPair<QString, QString>> getDevices();
};
