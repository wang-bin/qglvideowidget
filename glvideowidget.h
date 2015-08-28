#ifndef WIDGET_H
#define WIDGET_H

#include <QtOpenGL>
#include <QtGui/QImage>

class GLVideoWidget : public QGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    GLVideoWidget(QWidget *parent = 0);
    // YUV420P
    /*!
     * \brief setYUV420pParameters
     * call once before setFrameData() if parameters changed
     * \param w frame width
     * \param h frame height
     * \param strides strides of each plane. If null, it's equals to {w, w/2, w/2}.
     */
    void setYUV420pParameters(int w, int h, int* strides = NULL);
    void setFrameData(const QByteArray& data);
    // QImage
    /*!
     * \brief setQImageParameters
     * call once before setImage() if parameters changed
     * \param fmt only RGB888 is supported
     * \param stride QImage.bytesPerLine()
     */
    void setQImageParameters(QImage::Format fmt, int w, int h, int stride);
    void setImage(const QImage& img);
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
    bool upload_tex;
    int width;
    int height;
    //char *pitch[3];
    QByteArray m_data;
    QImage m_image;

    typedef struct {
        char* data;
        int stride;
        GLint internal_fmt;
        GLenum fmt;
        GLenum type;
        int bpp;
        QSize tex_size;
        QSize upload_size;
    } Plane;
    QVector<Plane> plane;

    //QSize tex_size[3], tex_upload_size[3];
    GLuint tex[3];
    int u_MVP_matrix, u_colorMatrix, u_Texture[3];
    QGLShaderProgram *m_program;
    QMutex m_mutex;
    QMatrix4x4 m_mat;
};

#endif // WIDGET_H
