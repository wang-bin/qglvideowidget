#include "widget.h"
#include <QApplication>
#include <QtCore>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget glw;
    QFile f("1280x720.yuv");
    f.open(QIODevice::ReadOnly);
    QByteArray data(f.readAll());
    qDebug("data size: %d", data.size());
    const int w = 1280, h = 720;
    glw.setFrameSize(w, h);

    glw.setFrameData(data);
    glw.show();
    return a.exec();
}
