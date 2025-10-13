#include <QApplication>
#include "NetworkViewer.hpp"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    NetworkViewer viewer;
    viewer.setWindowTitle("Danh sách thiết bị mạng (libpcap)");
    viewer.resize(400, 300);
    viewer.show();

    return app.exec();
}
