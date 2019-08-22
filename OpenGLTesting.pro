QT          += core gui widgets

TEMPLATE    = app
CONFIG      += c++14

TARGET      = OpenGLTesting

INCLUDEPATH +=  ./glm

SOURCES += main.cpp \
    glm/math.cpp \
    glm/matrix2x2.cpp \
    glm/matrix3x3.cpp \
    glm/matrix4x4.cpp \
    glm/vector2d.cpp \
    glm/vector3d.cpp \
    glm/vector4d.cpp \
    renderwindow.cpp \
    shader.cpp \
    mainwindow.cpp \
    triangle.cpp \
    texture.cpp \
    vertex.cpp \
    xyz.cpp \
    trianglesurface.cpp \
    input.cpp \
    visualobject.cpp \
    camera.cpp

HEADERS += \
    glm/math.h \
    glm/math_constants.h \
    glm/mathfwd.h \
    glm/matrix2x2.h \
    glm/matrix3x3.h \
    glm/matrix4x4.h \
    glm/vector2d.h \
    glm/vector3d.h \
    glm/vector4d.h \
    renderwindow.h \
    shader.h \
    mainwindow.h \
    triangle.h \
    texture.h \
    vertex.h \
    xyz.h \
    gltypes.h \
    trianglesurface.h \
    input.h \
    visualobject.h \
    camera.h

FORMS += \
    mainwindow.ui

DISTFILES += \
    plainfragment.frag \
    plainvertex.vert \
    texturefragmet.frag \
    texturevertex.vert
