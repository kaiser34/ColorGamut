
#include "glwindow.h"
#include <QtGui>
#include <QtOpenGL>
#include <QWidget>
#include <iostream>
#include <color.h>
#include <random>
#include <vector>
#include <set>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <QOpenGLShader>

#define __space " "

static const float DEFAULT_EYE_X  = 0;
static const float DEFAULT_EYE_Y  = 1;
static const float DEFAULT_EYE_Z  = 5;


const glm::mat4 CIEXYZ_TO_CIERGB_MAT  ({  2.36440,  -0.89580, -0.46770,     0,
                                         -0.51483,   1.42523,  0.08817,     0,
                                          0.00520,  -0.01440,  1.00921,     0,
                                                0,         0,        0,     1});

const glm::mat4 CIERGB_TO_CIEXYZ_MAT  ({ 0.490,  0.310,  0.200,     0,
                                         0.177,  0.813,  0.011,     0,
                                         0.000,  0.010,  0.990,     0,
                                             0,      0,      0,     1});

GLWindow::GLWindow( QWidget * parent ) : QGLWidget(parent),
    _eye(glm::vec3(DEFAULT_EYE_X,DEFAULT_EYE_Y,DEFAULT_EYE_Z)),
    _modeType(ModeType::CIEXYZ)
{
    qApp->installEventFilter(this);
    setWindowTitle("CIE XYZ");
}

GLWindow::~GLWindow()
{

}

void GLWindow::initializeGL()
{

    initializeGLFunctions();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    generateGamutData();
    _shaderProgram = glCreateProgram();

    glEnable(GL_DEPTH_TEST);

    if(!_shaderManager.addShaderFromSourceFile( QOpenGLShader::Vertex, "vertexShader.vsh" ))
    {
        std::cout << "Error loading Vertex Shader! " << std::endl;
        QApplication::exit(1);
    }

    if(!_shaderManager.addShaderFromSourceFile( QOpenGLShader::Fragment, "fragmentShader.fsh" ))
    {
        std::cout << "Error loading Fragment Shader! " << std::endl;
        QApplication::exit(1);
    }

    if(!_shaderManager.link())
    {
        std::cout << "Could not link Shader program! " << std::endl;
        QApplication::exit(1);
    }

    if(!_shaderManager.bind())
    {
        std::cout << "Could not bind Shader program! " << std::endl;
        QApplication::exit(1);
    }

    _shaderManager.setAttributeBuffer( "vertexPosition",GL_FLOAT,0,3);
    _shaderManager.enableAttributeArray( "vertexPosition" );

}


void GLWindow::generateGamutData()
{

    const float INITIAL_LAMBDA = 380.0f;
    const float FINAL_LAMBDA =780.0;

    const float  INITIAL_LUMINANCE = 0;
    const float  FINAL_LUMINANCE =100;
    const float LAMBDA_STEP = 0.5;

    int NUM_SAMPLES =(int)(((FINAL_LAMBDA - INITIAL_LAMBDA)/LAMBDA_STEP) + 1);

    float *integral = new float[NUM_SAMPLES];


    integral[0]=0;
//    float constant = 0;
    for(int i=1;i<=NUM_SAMPLES;i++)
    {
        float x_bar,y_bar,z_bar;
        float illuminant = corGetD65((float)(INITIAL_LAMBDA+i*LAMBDA_STEP));
        corGetCIExyz(INITIAL_LAMBDA+i*LAMBDA_STEP,&x_bar,&y_bar,&z_bar);
        integral[i] = illuminant*y_bar + integral[i-1];
    }
    float constant = 100.0f/integral[NUM_SAMPLES];



    std::set<std::pair<int,int>> mountain_pairs;
    for(int i=1;i<=NUM_SAMPLES;i++)
    {
        for (int j=1; j<=NUM_SAMPLES; j++)
        {
            float sum = (constant*(integral[j]-integral[i-1]));
            if(i<j && sum >= INITIAL_LUMINANCE && sum <= FINAL_LUMINANCE)
            {
                mountain_pairs.insert({i,j});
            }
        }
    }

    std::set<std::pair<int,int>> valley_pairs;
    for(int i=1;i<=NUM_SAMPLES;i++)
    {
        for (int j=1; j<=NUM_SAMPLES; j++)
        {
            float sum1 = (constant*(integral[i]-integral[0]));
            float sum2 = (constant*(integral[NUM_SAMPLES]-integral[j-1]));
            float sum = sum1+sum2;
            if(i<j && sum >= INITIAL_LUMINANCE && sum <= FINAL_LUMINANCE)
            {
                valley_pairs.insert({i,j});
            }
        }
    }

    _numberOfPoints = mountain_pairs.size() + valley_pairs.size();
    float * vertexes = new float[_numberOfPoints*3*sizeof(float)];
    int v=0;

    for(auto it : mountain_pairs)
    {
        std::pair<int,int> aPair = it;
        int lambda1 =aPair.first;
        int lambda2 =aPair.second;

        float reflectance[400];
        for(int i=0;i<400;i++)
        {
            reflectance[i] = 380.0f+i > lambda1 && 380.0f+i < lambda2 ? 1:0;
        }

        float X,Y,Z;

        corCIEXYZfromSurfaceReflectance(380.01f,400,1,reflectance,&X,&Y,&Z,D65);

        vertexes[v++]=X;
        vertexes[v++]=Y;
        vertexes[v++]=Z;
    }

    for(auto it : valley_pairs)
    {
        std::pair<int,int> aPair = it;
        int lambda1 =aPair.first;
        int lambda2 =aPair.second;

        float reflectance[400];
        for(int i=0;i<400;i++)
        {
            reflectance[i] = 380.0f+i > lambda1 && 380.0f+i < lambda2 ? 0:1;
        }

        float X,Y,Z;

        corCIEXYZfromSurfaceReflectance(380.01f,400,1,reflectance,&X,&Y,&Z,D65);

        vertexes[v++]=X;
        vertexes[v++]=Y;
        vertexes[v++]=Z;
    }

    glGenBuffers(1,_vBuffer);

    glBindBuffer(GL_ARRAY_BUFFER,_vBuffer[0]);
    glBufferData(GL_ARRAY_BUFFER,_numberOfPoints*3*sizeof(float),vertexes,GL_STATIC_DRAW);


    delete [] vertexes;
    delete [] integral;
}

void GLWindow::drawNewBase()
{

}

void GLWindow::drawDefaultAxis()
{
    glUseProgram(0);
    // Draw X Axis
    glColor3f(1,0,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(1000,0,0);
    glEnd();

    // Draw Y Axis
    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,1000,0);
    glEnd();

    // Draw Z Axis
    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,0,10000);
    glEnd();
}

void GLWindow::drawCieRGB2CieXYZAxis(glm::mat4 modelViewMatrix, glm::mat4 projection)
{

}

void GLWindow::drawSecondaryAxis()
{

    glUseProgram(0);

    glColor3f(1,0,0);

    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);
    glEnd();

    glColor3f(0,1,0);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);
    glEnd();

    glColor3f(0,0,1);
    glBegin(GL_LINES);
    glVertex3f(0,0,0);
    glVertex3f(0,0,1);
    glEnd();

    glColor3f(1,1,1);
    glBegin(GL_LINES);
    glVertex3f(1,0,0);
    glVertex3f(0,1,0);
    glVertex3f(0,1,0);
    glVertex3f(0,0,1);
    glVertex3f(0,0,1);
    glVertex3f(1,0,0);
    glEnd();

}

void GLWindow::drawScene (glm::mat4 viewMatrix, glm::mat4 projection)
{

    // Draw principal axis
    drawDefaultAxis();

    // Draw secondary axis if needed
    glm::mat4 modelViewMatrix;

    if(_modeType == ModeType::CIEXYZ2RGB)
    {
        modelViewMatrix = glm::transpose(CIEXYZ_TO_CIERGB_MAT);
        modelViewMatrix = viewMatrix * modelViewMatrix ;
        glLoadMatrixf(glm::value_ptr(modelViewMatrix));
        drawSecondaryAxis();
    }else{
        modelViewMatrix = viewMatrix;
    }

    // Draw points
    _shaderManager.bind();

    glm::mat4 mvp = projection * modelViewMatrix;
    QMatrix4x4 matrix(glm::value_ptr(glm::transpose(mvp)));

    _shaderManager.setUniformValue("MVP",matrix);

    glBindBuffer(GL_ARRAY_BUFFER,_vBuffer[0]);

    glDrawArrays(GL_POINTS,0,_numberOfPoints);

    _shaderManager.release();

}

void GLWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int vp[4];
    glGetIntegerv(GL_VIEWPORT,vp);

    glm::mat4 projection = glm::perspective(50.0f, (float) vp[2]/vp[3], 0.1f, 1000.f);

    glm::vec3 center(0,0,0);
    glm::vec3 up(0,1,0);
    glm::mat4 viewMatrix = glm::lookAt(_eye,center,up);

    viewMatrix = glm::rotate(viewMatrix,_rotation,glm::vec3(0,1,0));

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(viewMatrix));

    drawScene(viewMatrix,projection);
}

void GLWindow::resizeGL(int width, int height)
{
    glViewport(0,0,width,height);
}

bool GLWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->buttons() == Qt::LeftButton){
            _lastPoint = mouseEvent->pos();
        }
    }
    else if (event->type() == QEvent::MouseMove )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if(mouseEvent->buttons() == Qt::LeftButton){
            _rotation = (mouseEvent->pos().x() - _lastPoint.y());
            repaint();
        }

    }else if(event->type() == QEvent::Wheel){
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
#if _WIN32
        _eye.z -= wheelEvent->angleDelta().y() *0.005;
#elif __APPLE__
        _eye.z -= wheelEvent->angleDelta().y() *0.05;
#endif

        repaint();
    }else if(event->type() == QEvent::KeyPress){
        QKeyEvent * keyEvent = static_cast<QKeyEvent*>(event);
        std::cout << keyEvent->key();
        switch(keyEvent->key()){
        case Qt::Key_R:
            _eye.x = DEFAULT_EYE_X;
            _eye.y = DEFAULT_EYE_Y;
            _eye.z = DEFAULT_EYE_Z;
            break;
        case Qt::Key_1:
            _modeType = ModeType::CIEXYZ;
            setWindowTitle("CIE XYZ");
            break;
        case Qt::Key_2:
            _modeType = ModeType::CIEXYZ2RGB;
            setWindowTitle("CIE RGB in base CIE XYZ");
            break;

        }
        repaint();
    }
    return false;

}

