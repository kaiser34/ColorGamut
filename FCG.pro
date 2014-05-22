#-------------------------------------------------
#
# Project created by QtCreator 2014-03-30T11:44:02
#
#-------------------------------------------------

QT       += core opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11
INCLUDEPATH += glm
TARGET = HelloQt
TEMPLATE = app
SOURCES += *.cpp
HEADERS  += *.h

OTHER_FILES += \
    vertexShader.vsh \
    fragmentShader.fsh
