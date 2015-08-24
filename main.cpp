#include "glvideowidget.h"
#include <QApplication>
#include <QtCore>
#include <QtGui/QImage>
#define YUV_TEST 1
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GLVideoWidget glw;

#if YUV_TEST
    QFile f("1280x720.yuv");
    f.open(QIODevice::ReadOnly);
    QByteArray data(f.readAll());
    qDebug("data size: %d", data.size());
    const int w = 1280, h = 720;
    glw.setYUV420pParameters(w, h); //call once
    glw.setFrameData(data);
#else
    QImage img = QImage("test.png").convertToFormat(QImage::Format_RGB888);
    glw.setQImageParameters(img.format(), img.width(), img.height(), img.bytesPerLine()); //call once
    glw.setImage(img);
#endif //YUV_TEST
    glw.show();
    return a.exec();
}
