#include "glvideowidget.h"
#include <QApplication>
#include <QtCore>
#include <QtGui/QImage>
#define YUV_TEST 1
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if (a.arguments().contains("-angle"))
        QApplication::setAttribute(Qt::AA_UseOpenGLES);
    GLVideoWidget glw;
    if (!a.arguments().contains("-img")) {
        QFile f(":/1280x720.yuv");
        f.open(QIODevice::ReadOnly);
        QByteArray data(f.readAll());
        qDebug("data size: %d", data.size());
        const int w = 1280, h = 720;
        glw.setYUV420pParameters(w, h); //call once
        glw.setFrameData(data);
    } else {
        QImage img = QImage(":/test.png").convertToFormat(QImage::Format_RGB888);
        glw.setQImageParameters(img.format(), img.width(), img.height(), img.bytesPerLine()); //call once
        glw.setImage(img);
    }
    glw.show();
    return a.exec();
}
