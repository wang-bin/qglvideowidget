#include "widget.h"
#include <QApplication>
#include <QtCore>
#include <QtGui/QImage>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget glw;
    QFile f("1280x720.yuv");
    f.open(QIODevice::ReadOnly);
    QByteArray data(f.readAll());
    qDebug("data size: %d", data.size());
    const int w = 1280, h = 720;
    glw.setYUV420pParameters(w, h); //call once

    glw.setFrameData(data);

    QFile ff("test.png");
    ff.open(QIODevice::ReadOnly);

    QImage img = QImage("test.png").convertToFormat(QImage::Format_RGB888);
    qDebug() << img;
    glw.setQImageParameters(img.format(), img.width(), img.height(), img.bytesPerLine()); //call once
    glw.setImage(img);

    glw.show();
    return a.exec();
}
