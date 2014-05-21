#ifndef GLWINDOW_H
#define GLWINDOW_H
#include <QGLWidget>
#include <QGLFunctions>
#include <glm/glm.hpp>


class GLWindow : public QGLWidget, protected QGLFunctions
{
    Q_OBJECT

public:
    GLWindow( QWidget * parent = 0);
    ~GLWindow();
    bool eventFilter(QObject *obj, QEvent *event);

//    void mousePressEvent(QMouseEvent *event);
protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int width, int height);
private:
    void drawScene(glm::mat4 modelMatrix);
    int SIZE_OF_INDICES = 0;
    unsigned int * indices;
    void generateVertexes();
    QPoint _lastPoint;
    float _rotation =0;
    glm::vec3 _eye;
    glm::mat4 _modelMatrix;

    GLuint _vao;
    GLuint _vBuffer[2];
    int _numberOfPoints;

};

#endif // GLWINDOW_H
