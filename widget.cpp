#include "widget.h"

char const *const* attributeNames()
{
    static const char *names[] = {
        "a_Position",
        "a_TexCoords",
        0
    };
    return names;
}

Widget::Widget(QWidget *parent)
    : QGLWidget(parent)
    , update_res(true)
    , m_program(0)
{
    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);
    /* To rapidly update custom widgets that constantly paint over their entire areas with
     * opaque content, e.g., video streaming widgets, it is better to set the widget's
     * Qt::WA_OpaquePaintEvent, avoiding any unnecessary overhead associated with repainting the
     * widget's background
     */
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    //default: swap in qpainter dtor. we should swap before QPainter.endNativePainting()
    setAutoBufferSwap(false);
    setAutoFillBackground(false);

    memset(tex, 0, 3);
}

void Widget::setFrameData(const QByteArray &data, int *strides)
{
    //QMutexLocker lock(&m_mutex);
    m_data = data;
    pitch[0] = (char*)m_data.constData();
    if (strides) {
        pitch[1] = pitch[0] + strides[0]*height;
        pitch[2] = pitch[1] + strides[1]*height/2;
    } else {
        pitch[1] = pitch[0] + width*height;
        pitch[2] = pitch[1] + width/2*height/2;
    }
    qDebug() << QByteArray(pitch[0], 20);
    qDebug() << QByteArray(pitch[1], 20);
    qDebug() << QByteArray(pitch[2], 20);
    qDebug() << "data set";
}

void Widget::bind()
{
    for (int i = 0; i < 3; ++i) {
        bindPlane((i + 1) % 3);
    }
}

void Widget::bindPlane(int p)
{
    glActiveTexture(GL_TEXTURE0 + p);
    glBindTexture(GL_TEXTURE_2D, tex[p]);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // This is necessary for non-power-of-two textures
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    qDebug("bind plane %d, %p", p, pitch[p]);
    qDebug() << QByteArray(pitch[p], 20);
    //qDebug() << tex_upload_size[p];
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex_upload_size[p].width(), tex_upload_size[p].height(), GL_LUMINANCE, GL_UNSIGNED_BYTE, pitch[p]);
}

void Widget::initTextures()
{
    for (int i = 0; i < 3; ++i) {
        tex_size[i].setWidth(tex_size[i].width());
        tex_upload_size[i].setWidth(tex_upload_size[i].width());
    }

    glDeleteTextures(3, tex);
    memset(tex, 0, 3);
    glGenTextures(3, tex);
    qDebug("init textures...");
    for (int i = 0; i < 3; ++i) {
        qDebug("tex[%d]: %u", i, tex[i]);
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // This is necessary for non-power-of-two textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, tex_size[i].width(), tex_size[i].height(), 0/*border, ES not support*/, GL_LUMINANCE, GL_UNSIGNED_BYTE, NULL);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Widget::setFrameSize(int w, int h)
{
    width = w;
    height = h;
    tex_size[0].setWidth(w); //use stride
    tex_upload_size[0].setWidth(w);
    tex_size[0].setHeight(h);
    tex_upload_size[0].setHeight(h);
    for (int i = 1; i < 3; ++i) {
        tex_size[i].setWidth(w/2);
        tex_upload_size[i].setWidth(w/2);
        tex_size[i].setHeight(h/2);
        tex_upload_size[i].setHeight(h/2);
    }
}

void Widget::setYUV420pParameters(int w, int h, int *strides)
{
    update_res = true;
    plane.resize(3);
    Plane &p = plane[0];
    p.stride = stride && stride[0] ? stride[0] : w;
    p.tex_size.setWidth(p.stride);
    p.upload_size.setWidth(p.stride);
    p.tex_size.setHeight(h);
    p.upload_size.setHeight(h);


    tex_size[0].setWidth(w); //use stride
    tex_upload_size[0].setWidth(w);
    tex_size[0].setHeight(h);
    tex_upload_size[0].setHeight(h);
    for (int i = 1; i < 3; ++i) {
        tex_size[i].setWidth(w/2);
        tex_upload_size[i].setWidth(w/2);
        tex_size[i].setHeight(h/2);
        tex_upload_size[i].setHeight(h/2);
    }
}

static const QMatrix4x4 yuv2rgb_bt601 =
           QMatrix4x4(
                1.0f,  0.000f,  1.402f, 0.0f,
                1.0f, -0.344f, -0.714f, 0.0f,
                1.0f,  1.772f,  0.000f, 0.0f,
                0.0f,  0.000f,  0.000f, 1.0f)
            *
            QMatrix4x4(
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, -0.5f,
                0.0f, 0.0f, 1.0f, -0.5f,
                0.0f, 0.0f, 0.0f, 1.0f);

void Widget::paintGL()
{
    if (update_res)
        return;
    if (!m_program)
        return;
    //QMutexLocker lock(&m_mutex);
    if (!tex[0]) {
        initTextures();
    }
    bind();
    m_program->bind();
    for (int i = 0; i < 3; ++i) {
        m_program->setUniformValue(u_Texture[i], (GLint)i);
    }
    m_program->setUniformValue(u_colorMatrix, yuv2rgb_bt601);
    m_program->setUniformValue(u_MVP_matrix, m_mat);
    // uniform end. attribute begin
    const GLfloat kVertices[] = {
        -1, 1,
        1, 1,
        1, -1,
        -1, -1,
    };
    const GLfloat kTexCoords[] = {
        0, 0,
        1, 0,
        1, 1,
        0, 1,
    };
    // normalize?
    m_program->setAttributeArray(0, GL_FLOAT, kVertices, 2);
    m_program->setAttributeArray(1, GL_FLOAT, kTexCoords, 2);
    char const *const *attr = attributeNames();
    for (int i = 0; attr[i]; ++i) {
        m_program->enableAttributeArray(i); //TODO: in setActiveShader
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    // d.m_program->release(); //glUseProgram(0)
    for (int i = 0; attr[i]; ++i) {
        m_program->disableAttributeArray(i); //TODO: in setActiveShader
    }
    swapBuffers();
}

void Widget::initializeGL()
{
    qDebug("init gl");
    initializeGLFunctions();
    initializeShader();
}

void Widget::resizeGL(int w, int h)
{
    qDebug("resizeGL %dx%d", w, h);
    glViewport(0, 0, w, h);
    m_mat.setToIdentity();
    //m_mat.ortho(QRectF(0, 0, w, h));
}

void Widget::initializeShader()
{
    m_program = new QGLShaderProgram(this);
#define glsl(x) #x
    static const char kVertexShader[] = glsl(
        attribute vec4 a_Position;
        attribute vec2 a_TexCoords;
        uniform mat4 u_MVP_matrix;
        varying vec2 v_TexCoords;
        void main() {
          gl_Position = u_MVP_matrix * a_Position;
          v_TexCoords = a_TexCoords;
        });
    static const char kFragmentShader[] = glsl(
            uniform sampler2D u_Texture0;
            uniform sampler2D u_Texture1;
            uniform sampler2D u_Texture2;
            varying lowp vec2 v_TexCoords;
            uniform mat4 u_colorMatrix;
            void main()
            {
                 gl_FragColor = clamp(u_colorMatrix
                                     * vec4(
                                         texture2D(u_Texture0, v_TexCoords).r,
                                         texture2D(u_Texture1, v_TexCoords).r,
                                         texture2D(u_Texture2, v_TexCoords).r,
                                         1)
                                     , 0.0, 1.0);
            });
#undef glsl
    Q_ASSERT_X(!m_program->isLinked(), "Widget::compile()", "Compile called multiple times!");
    m_program->addShaderFromSourceCode(QGLShader::Vertex, kVertexShader);
    m_program->addShaderFromSourceCode(QGLShader::Fragment, kFragmentShader);
    char const *const *attr = attributeNames();
    for (int i = 0; attr[i]; ++i) {
        qDebug("attributes: %s", attr[i]);
        m_program->bindAttributeLocation(attr[i], i);
    }
    if (!m_program->link()) {
        qWarning("QSGMaterialShader: Shader compilation failed:");
        qWarning() << m_program->log();
    }

    u_MVP_matrix = m_program->uniformLocation("u_MVP_matrix");
    // fragment shader
    u_colorMatrix = m_program->uniformLocation("u_colorMatrix");
    for (int i = 0; i < 3; ++i) {
        QString tex_var = QString("u_Texture%1").arg(i);
        u_Texture[i] = m_program->uniformLocation(tex_var);
        qDebug("glGetUniformLocation(\"%s\") = %d", tex_var.toUtf8().constData(), u_Texture[i]);
    }
    qDebug("glGetUniformLocation(\"u_MVP_matrix\") = %d", u_MVP_matrix);
    qDebug("glGetUniformLocation(\"u_colorMatrix\") = %d", u_colorMatrix);
}
