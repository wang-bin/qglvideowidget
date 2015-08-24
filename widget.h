#ifndef WIDGET_H
#define WIDGET_H

#include <QtOpenGL>
#include <QtCore>
#include <QtGui/QImage>

class Widget : public QGLWidget, public QGLFunctions
{
    Q_OBJECT

public:
    Widget(QWidget *parent = 0);
    /*!
     * \brief setYUV420pParameters
     * \param w frame width
     * \param h frame height
     * \param strides strides of each plane. If null, it's equals to {w, w/2, w/2}.
     */
    void setYUV420pParameters(int w, int h, int* strides = NULL);
    /*!
     * \brief setQImageParameters
     * \param fmt
     * \param w
     * \param h
     * \param stride QImage.bytesPerLine()
     */
    void setQImageParameters(QImage::Format fmt, int w, int h, int stride = 0);
    void setFrameSize(int w, int h);
    void setFrameData(const QByteArray& data, int* strides = 0);
    // TODO: only init(w,h,strides) init(QImage::Format, w, h, strides)
protected:
    void bind();
    void bindPlane(int p);
    void initializeShader();
    void initTextures();
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int w, int h);
private:
    bool update_res;
    int width;
    int height;
    int stride[3];
    char *pitch[3];
    QByteArray m_data;

    typedef struct {
        int stride;
        char* data;
        GLuint tex;
        int location;
        QSize tex_size;
        QSize upload_size;
    } Plane;
    QVector<Plane> plane;

    QSize tex_size[3], tex_upload_size[3];
    GLuint tex[3];
    int u_MVP_matrix, u_colorMatrix, u_Texture[3];
    QGLShaderProgram *m_program;
    QMutex m_mutex;
    QMatrix4x4 m_mat;
};

#endif // WIDGET_H
