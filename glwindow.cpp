#include "glwindow.h"
#include <QtGui>
#include <QtOpenGL>
#include <QWidget>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glu.h>
#include <iostream>
#include <color.h>
#include <random>
#include <vector>
#include <set>

#define __space " "
GLWindow::GLWindow( QWidget * parent ) : QGLWidget(parent),
    _eye(glm::vec3(0,3,10))
{
    qApp->installEventFilter(this);

}

GLWindow::~GLWindow()
{

}

void GLWindow::initializeGL()
{

//        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    initializeGLFunctions();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    //    glEnable(GL_LIGHTING);
    //    glEnable(GL_LIGHT0);
    //    glEnable(GL_COLOR_MATERIAL);
    //    glEnable(GL_NORMALIZE);
    //    glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
    //    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
    //    glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,50.0);
    //    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
    generateVertexes();

}


void GLWindow::generateVertexes()
{
    const float INITIAL_LAMBDA = 380.0f;
    const float FINAL_LAMBDA =780.0;

    const float  INITIAL_LUMINANCE = 0;
    const float  FINAL_LUMINANCE =100;
    const float LAMBDA_STEP = 0.5;

    int NUM_SAMPLES =(int)(((FINAL_LAMBDA - INITIAL_LAMBDA)/LAMBDA_STEP) + 1);

    float *integral = new float[NUM_SAMPLES];

    integral[0]=0;
    for(int i=1;i<=NUM_SAMPLES;i++){
        float illuminant = corGetD65((float)(INITIAL_LAMBDA+i*LAMBDA_STEP));
        integral[i] = illuminant + integral[i-1];
    }


    std::set<std::pair<int,int>> mountain_pairs;
    for(int i=1;i<=NUM_SAMPLES;i++){
        for (int j=1; j<=NUM_SAMPLES; j++) {
            float sum = ((100.0f/integral[NUM_SAMPLES])*(integral[j]-integral[i-1]));
            if(i<j && sum >= INITIAL_LUMINANCE && sum <= FINAL_LUMINANCE){
                mountain_pairs.insert({i,j});
            }
        }
    }

    std::set<std::pair<int,int>> valley_pairs;
    for(int i=1;i<=NUM_SAMPLES;i++){
        for (int j=1; j<=NUM_SAMPLES; j++) {
            float sum1 = ((100.0f/integral[NUM_SAMPLES])*(integral[i]-integral[0]));
            float sum2 = ((100.0f/integral[NUM_SAMPLES])*(integral[NUM_SAMPLES]-integral[j-1]));

            float sum = sum1+sum2;
            if(i<j && sum >= INITIAL_LUMINANCE && sum <= FINAL_LUMINANCE){
                valley_pairs.insert({i,j});
            }
        }
    }


    _numberOfPoints = mountain_pairs.size() + valley_pairs.size();
    float * vertexes = new float[_numberOfPoints*3*sizeof(float)];
    int v=0;

    for(auto it : mountain_pairs){
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

    for(auto it : valley_pairs){
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


void GLWindow::drawScene (glm::mat4 modelMatrix)
{
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

    glPointSize(1);

    glPushMatrix();

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ARRAY_BUFFER,_vBuffer[0]);
    glVertexPointer(3,GL_FLOAT,0,NULL);

    glBindBuffer(GL_ARRAY_BUFFER,_vBuffer[0]);
    glColorPointer(3,GL_FLOAT,0,NULL);


    glDrawArrays(GL_POINTS,0,_numberOfPoints);
    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

}




void GLWindow::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int vp[4];
    glGetIntegerv(GL_VIEWPORT,vp);

    glm::mat4 projection = glm::perspective(50.0f, (float) vp[2]/vp[3], 1.0f, 100.f);

    glm::vec3 center(0,0,0);
    glm::vec3 up(0,1,0);
    glm::mat4 modelMatrix = glm::lookAt(_eye,center,up);

    modelMatrix = glm::rotate(modelMatrix,_rotation,glm::vec3(0,1,0));


    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(glm::value_ptr(projection));

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(glm::value_ptr(modelMatrix));

    drawScene(modelMatrix);
}

void GLWindow::resizeGL(int width, int height)
{
    glViewport(0,0,width,height);
}

bool GLWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseMove )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        //statusBar()->showMessage(QString("Mouse move (%1,%2)").arg(mouseEvent->pos().x()).arg(mouseEvent->pos().y()));
        //if(mouseEvent->buttons().)
        //std::cout << mouseEvent->buttons() << std::endl;
        if(mouseEvent->buttons() == Qt::LeftButton){
            _rotation = (mouseEvent->pos().x() - _lastPoint.y());
            repaint();
        }

    }else if(event->type() == QEvent::Wheel){
        QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
        _eye.z -= wheelEvent->angleDelta().y() *0.05;
        repaint();
    }else if(event->type() == QEvent::KeyPress){
        QKeyEvent * keyEvent = static_cast<QKeyEvent*>(event);
        std::cout << keyEvent->key();
        const float stepSize =0.5f;
        switch(keyEvent->key()){
        case Qt::UpArrow:
            _eye.y += stepSize;
            break;
        case Qt::LeftArrow:
            _eye.x -= stepSize;
            break;
        case Qt::DownArrow:
            _eye.z -= stepSize;
            break;
        case Qt::RightArrow:
            _eye.x += stepSize;
            break;
        case Qt::Key_R:
            _eye.x = 0;
            _eye.y = 2;
            _eye.z = 10;
            break;
        }
    }
    return false;

}

//void GLWindow::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton) {
//        std::cout << event->pos().x() <<  " " << event->pos().y() << std::endl;


//    }
//}
