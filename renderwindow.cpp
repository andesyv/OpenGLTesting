#include "renderwindow.h"
#include <QTimer>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLDebugLogger>
#include <QKeyEvent>
#include <QStatusBar>
#include <QDebug>

#include "shader.h"
#include "mainwindow.h"
#include "matrix4x4.h"
#include "gsl_math.h"

#include "xyz.h"
#include "trianglesurface.h"

const float quadVertices[] = {
    // xyz          // uvs
    1.f, 1.f,       1.f, 1.f,
    -1.f, 1.f,      0.f, 1.f,
    -1.f, -1.f,     0.f, 0.f,

    -1.f, -1.f,     0.f, 0.f,
    1.f, -1.f,      1.f, 0.f,
    1.f, 1.f,       1.f, 1.f
};

float skyboxVertices[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

RenderWindow::RenderWindow(const QSurfaceFormat &format, MainWindow *mainWindow)
    : mContext(nullptr), mInitialized(false), mMainWindow(mainWindow)
{
    //This is sent to QWindow:
    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);
    //Make the OpenGL context
    mContext = new QOpenGLContext(this);
    //Give the context the wanted OpenGL format (v4.1 Core)
    mContext->setFormat(requestedFormat());
    if (!mContext->create()) {
        delete mContext;
        mContext = nullptr;
        qDebug() << "Context could not be made - quitting this application";
    }

    //Make the gameloop timer:
    mRenderTimer = new QTimer(this);
}

RenderWindow::~RenderWindow()
{
}

/// Sets up the general OpenGL stuff and the buffers needed to render a triangle
void RenderWindow::init()
{
    //Connect the gameloop timer to the render function:
    connect(mRenderTimer, SIGNAL(timeout()), this, SLOT(render()));

    //********************** General OpenGL stuff **********************

    //The OpenGL context has to be set.
    //The context belongs to the instanse of this class!
    if (!mContext->makeCurrent(this)) {
        qDebug() << "makeCurrent() failed";
        return;
    }

    //just to make sure we don't init several times
    //used in exposeEvent()
    if (!mInitialized)
        mInitialized = true;

    //must call this to use OpenGL functions
    initializeOpenGLFunctions();

    //Start the Qt OpenGL debugger
    //Really helpfull when doing OpenGL
    //Supported on most Windows machines
    //reverts to plain glGetError() on Mac and other unsupported PCs
    // - can be deleted
    startOpenGLDebugger();

    //general OpenGL stuff:
    glEnable(GL_DEPTH_TEST);    //enables depth sorting - must use GL_DEPTH_BUFFER_BIT in glClear
    glEnable(GL_CULL_FACE);     //draws only front side of models - usually what you want -
    glClearColor(0.4f, 0.4f, 0.4f, 1.0f);    //color used in glClear GL_COLOR_BUFFER_BIT

    //Compile shaders:
    //NB: hardcoded path to files! You have to change this if you change directories for the project.
    //Qt makes a build-folder besides the project folder. That is why we go down one directory
    // (out of the build-folder) and then up into the project folder.
    mShaderProgram[0] = new Shader("../OpenGLTesting/plainvertex.vert", "../OpenGLTesting/plainfragment.frag");
    qDebug() << "Plain shader program id: " << mShaderProgram[0]->getProgram();
    mShaderProgram[1] = new Shader("../OpenGLTesting/texturevertex.vert", "../OpenGLTesting/texturefragmet.frag");
    qDebug() << "Texture shader program id: " << mShaderProgram[1]->getProgram();
    mShaderProgram[2] = new Shader("../OpenGLTesting/phong.vert", "../OpenGLTesting/phong.frag");
    qDebug() << "Phong shader program id: " << mShaderProgram[2]->getProgram();
    mShaderProgram[3] = new Shader("../OpenGLTesting/depth.vert", "../OpenGLTesting/depth.frag", "../OpenGLTesting/depth.geom");
    qDebug() << "Depth shader program id: " << mShaderProgram[3]->getProgram();
    mShaderProgram[4] = new Shader("../OpenGLTesting/postprocess.vert", "../OpenGLTesting/depthVisualize.frag");
    qDebug() << "Postprocess shader program id: " << mShaderProgram[4]->getProgram();
    mShaderProgram[5] = new Shader("../OpenGLTesting/skybox.vert", "../OpenGLTesting/skybox.frag");
    qDebug() << "Skybox shader program id: " << mShaderProgram[5]->getProgram();


    setupPlainShader(0);
    setupTextureShader(1);

    //**********************  Texture stuff: **********************
    mTexture[0] = new Texture();
    mTexture[1] = new Texture("../OpenGLTesting/Assets/hund.bmp");
    mTexture[2] = new Texture("../OpenGLTesting/Assets/wood.bmp");

    //Set the textures loaded to a texture unit
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexture[0]->id());
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mTexture[1]->id());
    glActiveTexture(GL_TEXTURE0);

    //********************** Making the objects to be drawn **********************
    VisualObject *temp = new XYZ();
    temp->init();
    mVisualObjects.push_back(temp);

    // Cube
    temp = new TriangleSurface("../OpenGLTesting/Assets/cube.txt");
    temp->init();
    temp->mMatrix.translate(-0.8f, 0.4f, 1.2f);
    mVisualObjects.push_back(temp);

    // Ground
    temp = new TriangleSurface();
    temp->init();
    temp->mMatrix.rotateX(90.f);
    temp->mMatrix.scale(50.f);
    temp->mMatrix.translate(-0.25f, -0.25f, 0.f);
    mVisualObjects.push_back(temp);

    // Donut
    temp = new TriangleSurface();
    static_cast<TriangleSurface*>(temp)->constructTorus();
    temp->init();
    temp->mMatrix.scale(0.2f);
    temp->mMatrix.translate(4.f, 1.5f, -2.f);
    mVisualObjects.push_back(temp);

    //********************** Set up camera **********************
    mCurrentCamera = new Camera();
    mCurrentCamera->setPosition(gsl::Vector3D(-1.f, -.5f, -2.f));

    // Make a plane that covers the screen
    glGenVertexArrays(1, &screenPlaneVAO);
    glBindVertexArray(screenPlaneVAO);

    GLuint screenPlaneVBO;
    glGenBuffers(1, &screenPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, screenPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)( 2 * sizeof(GLfloat)) );
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //********************** Set up shadows *********************
    // Make framebuffer for depthmap (from light perspective)
    glGenFramebuffers(1, &shadowFBO);

//    // Create a texture to save the depthmap
//    glGenTextures(1, &shadowMap);
//    glBindTexture(GL_TEXTURE_2D, shadowMap);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
//    float borderColor[] = {1.f, 1.f, 1.f, 1.f}; // Pure white
//    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
    for (unsigned int i = 0; i < 6; i++)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                     SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // Add texture to framebuffer and complete creation of framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Creation of shadow framebuffer failed!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //******************** Create a skybox *************************
    //Vertex Array Object - VAO
    glGenVertexArrays(1, &skyboxVAO);
    glBindVertexArray(skyboxVAO);

    //Vertex Buffer Object to hold vertices - VBO
    unsigned int skyboxVBO;
    glGenBuffers( 1, &skyboxVBO );
    glBindBuffer( GL_ARRAY_BUFFER, skyboxVBO );

    glBufferData( GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW );

    // 1st attribute buffer : vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT,GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

///Called each frame - doing the rendering
void RenderWindow::render()
{
    //input
    handleInput();

    mCurrentCamera->update();

    mTimeStart.restart(); //restart FPS clock
    mContext->makeCurrent(this); //must be called every frame (every time mContext->swapBuffers is called)

    gsl::Vector3D sun{-2.5f, 4.f, 4.f};
    glDepthFunc(GL_LESS);

    //******** This should be done with a loop!
    {
        // *****************  Render to depthmap for shadows ********************
        gsl::Matrix4x4 lightProjection;
        // Perspective projection matrix must use a FOV of 90 degrees to be the same as a side of a cube.
        lightProjection.perspective(90.f, mAspectratio, 0.1f, 100.f);
        mCurrentCamera->mProjectionMatrix = lightProjection;

        // Create different light-space matrises for each side of the cubemap.
        std::vector<gsl::Matrix4x4> lightViewProjMatrises;
        lightViewProjMatrises.reserve(6);
        gsl::Matrix4x4 temp; //= gsl::Matrix4x4::lookAtRotation(sun, gsl::Vector3D{0, 0, 0}, gsl::Vector3D{0, 1, 0});

        temp.lookAt(sun, sun + gsl::Vector3D{1, 0, 0}, gsl::Vector3D{0, -1, 0});
        lightViewProjMatrises.push_back(lightProjection * temp);
        temp.lookAt(sun, sun + gsl::Vector3D{-1, 0, 0}, gsl::Vector3D{0, -1, 0});
        lightViewProjMatrises.push_back(lightProjection * temp);
        temp.lookAt(sun, sun + gsl::Vector3D{0, 1, 0}, gsl::Vector3D{0, 0, 1});
        lightViewProjMatrises.push_back(lightProjection * temp);
        temp.lookAt(sun, sun + gsl::Vector3D{0, -1, 0}, gsl::Vector3D{0, 0, -1});
        lightViewProjMatrises.push_back(lightProjection * temp);
        temp.lookAt(sun, sun + gsl::Vector3D{0, 0, 1}, gsl::Vector3D{0, -1, 0});
        lightViewProjMatrises.push_back(lightProjection * temp);
        temp.lookAt(sun, sun + gsl::Vector3D{0, 0, -1}, gsl::Vector3D{0, -1, 0});/*mCurrentCamera->mViewMatrix = temp;*/
        lightViewProjMatrises.push_back(lightProjection * temp);
        // std::cout << "lightviewprojmatrix: " << lightViewProjMatrix << std::endl;

        mShaderProgram[3]->use();
        glBindTexture(GL_TEXTURE_2D, mTexture[1]->id());
        for (unsigned int i{0}; i < 6; ++i) {
            std::stringstream uniformName;
            uniformName << "lightViewProjMatrix[" << i << ']';
            glUniformMatrix4fv(glGetUniformLocation(mShaderProgram[3]->getProgram(), uniformName.str().c_str()), 1, GL_TRUE, lightViewProjMatrises.at(i).constData());
        }
        glUniform3fv(glGetUniformLocation(mShaderProgram[3]->getProgram(), "lightPos"), 1, sun.xP());
        glUniform1f(glGetUniformLocation(mShaderProgram[3]->getProgram(), "far_plane"), 100.f);

        glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        // glCullFace(GL_FRONT);

//        glUniformMatrix4fv(mShaderProgram[3]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[0]->mMatrix.constData());
//        mVisualObjects[0]->draw();
        glUniformMatrix4fv(mShaderProgram[3]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[1]->mMatrix.constData());
        mVisualObjects[1]->draw();
        glUniformMatrix4fv(mShaderProgram[3]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[2]->mMatrix.constData());
        mVisualObjects[2]->draw();
        glUniformMatrix4fv(mShaderProgram[3]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[3]->mMatrix.constData());
        mVisualObjects[3]->draw();


        // ******************** Render scene normally ************************
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, width(), height());
        //to clear the screen for each redraw
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glCullFace(GL_BACK);

        /// Vertex color shader:
        mShaderProgram[0]->use();
        glUniformMatrix4fv( mShaderProgram[0]->vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[0]->pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        // XYZ
        glUniformMatrix4fv( mShaderProgram[0]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[0]->mMatrix.constData());
        mVisualObjects[0]->draw();

        /// Phong shader:
        mShaderProgram[2]->use();
        glUniformMatrix4fv( mShaderProgram[2]->vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[2]->pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        // Adding a light (the sun) to the scene
        glUniform3fv(glGetUniformLocation(mShaderProgram[2]->getProgram(), "lightPos"), 1, sun.xP());
        // Sending viewPos
        auto cameraPos = mCurrentCamera->position();
        glUniform3fv(glGetUniformLocation(mShaderProgram[2]->getProgram(), "viewPos"), 1, cameraPos.xP());

        // Add the shadow map to the 7th texture slot
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
        glUniform1i(glGetUniformLocation(mShaderProgram[2]->getProgram(), "shadowMap"), 7);
        glActiveTexture(GL_TEXTURE0);

        // Donut
        glUniformMatrix4fv( mShaderProgram[2]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[3]->mMatrix.constData());
        mVisualObjects[3]->draw();

        // Box
        glUniformMatrix4fv( mShaderProgram[2]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[1]->mMatrix.constData());
        // glBindTexture(GL_TEXTURE_2D, shadowMap);
        mVisualObjects[1]->draw();

        // Ground plane
        glUniformMatrix4fv( mShaderProgram[2]->mMatrixUniform, 1, GL_TRUE, mVisualObjects[2]->mMatrix.constData());
        // glBindTexture(GL_TEXTURE_2D, mTexture[2]->id());
        mVisualObjects[2]->draw();

        /// Texture shader:
        glUseProgram(mShaderProgram[1]->getProgram());
        glUniformMatrix4fv( mShaderProgram[1]->vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[1]->pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());

//        // Draw a triangle over the screen
//        mShaderProgram[4]->use();
//        glBindTexture(GL_TEXTURE_2D, shadowMap);
//        glUniform1i(glGetUniformLocation(mShaderProgram[4]->getProgram(), "depthMap"), 0);
//        glBindVertexArray(screenPlaneVAO);
//        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Skybox
        glDepthFunc(GL_LEQUAL);
        glBindVertexArray(skyboxVAO);
        mShaderProgram[5]->use();
        glUniformMatrix4fv( mShaderProgram[5]->vMatrixUniform, 1, GL_TRUE, mCurrentCamera->mViewMatrix.constData());
        glUniformMatrix4fv( mShaderProgram[5]->pMatrixUniform, 1, GL_TRUE, mCurrentCamera->mProjectionMatrix.constData());
        glBindTexture(GL_TEXTURE_CUBE_MAP, shadowMap);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    //Calculate framerate before
    // checkForGLerrors() because that takes a long time
    // and before swapBuffers(), else it will show the vsync time
    calculateFramerate();

    //using our expanded OpenGL debugger to check if everything is OK.
    checkForGLerrors();

    //Qt require us to call this swapBuffers() -function.
    // swapInterval is 1 by default which means that swapBuffers() will (hopefully) block
    // and wait for vsync.
    mContext->swapBuffers(this);
}

void RenderWindow::setupPlainShader(int shaderIndex)
{
    mMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform0 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
}

void RenderWindow::setupTextureShader(int shaderIndex)
{
    mMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "mMatrix" );
    vMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "vMatrix" );
    pMatrixUniform1 = glGetUniformLocation( mShaderProgram[shaderIndex]->getProgram(), "pMatrix" );
    mTextureUniform = glGetUniformLocation(mShaderProgram[shaderIndex]->getProgram(), "textureSampler");
}

//This function is called from Qt when window is exposed (shown)
//and when it is resized
//exposeEvent is a overridden function from QWindow that we inherit from
void RenderWindow::exposeEvent(QExposeEvent *)
{
    if (!mInitialized)
        init();

    //This is just to support modern screens with "double" pixels
    const qreal retinaScale = devicePixelRatio();
    glViewport(0, 0, static_cast<GLint>(width() * retinaScale), static_cast<GLint>(height() * retinaScale));

    //If the window actually is exposed to the screen we start the main loop
    //isExposed() is a function in QWindow
    if (isExposed())
    {
        //This timer runs the actual MainLoop
        //16 means 16ms = 60 Frames pr second (should be 16.6666666 to be exact..)
        mRenderTimer->start(16);
        mTimeStart.start();
    }
    mAspectratio = static_cast<float>(width()) / height();
    //    qDebug() << mAspectratio;
    mCurrentCamera->mProjectionMatrix.perspective(45.f, mAspectratio, 0.1f, 100.f);
    //    qDebug() << mCamera.mProjectionMatrix;
}

//Simple way to turn on/off wireframe mode
//Not totally accurate, but draws the objects with
//lines instead of filled polygons
void RenderWindow::toggleWireframe()
{
    mWireframe = !mWireframe;
    if (mWireframe)
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);    //turn on wireframe mode
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    //turn off wireframe mode
        glEnable(GL_CULL_FACE);
    }
}

//The way this is set up is that we start the clock before doing the draw call,
//and check the time right after it is finished (done in the render function)
//This will approximate what framerate we COULD have.
//The actual frame rate on your monitor is limited by the vsync and is probably 60Hz
void RenderWindow::calculateFramerate()
{
    long long nsecElapsed = mTimeStart.nsecsElapsed();
    static int frameCount{0};                       //counting actual frames for a quick "timer" for the statusbar

    if (mMainWindow)    //if no mainWindow, something is really wrong...
    {
        ++frameCount;
        if (frameCount > 30) //once pr 30 frames = update the message twice pr second (on a 60Hz monitor)
        {
            //showing some statistics in status bar
            mMainWindow->statusBar()->showMessage(" Time pr FrameDraw: " +
                                                  QString::number(nsecElapsed/1000000., 'g', 4) + " ms  |  " +
                                                  "FPS (approximated): " + QString::number(1E9 / nsecElapsed, 'g', 7));
            frameCount = 0;     //reset to show a new message in 60 frames
        }
    }
}


/// Uses QOpenGLDebugLogger if this is present
/// Reverts to glGetError() if not
void RenderWindow::checkForGLerrors()
{
    if(mOpenGLDebugLogger)
    {
        const QList<QOpenGLDebugMessage> messages = mOpenGLDebugLogger->loggedMessages();
        for (const QOpenGLDebugMessage &message : messages)
            qDebug() << message;
    }
    else
    {
        GLenum err = GL_NO_ERROR;
        while((err = glGetError()) != GL_NO_ERROR)
        {
            qDebug() << "glGetError returns " << err;
        }
    }
}

/// Tries to start the extended OpenGL debugger that comes with Qt
void RenderWindow::startOpenGLDebugger()
{
    QOpenGLContext * temp = this->context();
    if (temp)
    {
        QSurfaceFormat format = temp->format();
        if (! format.testOption(QSurfaceFormat::DebugContext))
            qDebug() << "This system can not use QOpenGLDebugLogger, so we revert to glGetError()";

        if(temp->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
        {
            qDebug() << "System can log OpenGL errors!";
            mOpenGLDebugLogger = new QOpenGLDebugLogger(this);
            if (mOpenGLDebugLogger->initialize()) // initializes in the current context
                qDebug() << "Started OpenGL debug logger!";
        }

        if(mOpenGLDebugLogger)
            mOpenGLDebugLogger->disableMessages(QOpenGLDebugMessage::APISource, QOpenGLDebugMessage::OtherType, QOpenGLDebugMessage::NotificationSeverity);
    }
}

void RenderWindow::setCameraSpeed(float value)
{
    mCameraSpeed += value;

    //Keep within min and max values
    if(mCameraSpeed < 0.01f)
        mCameraSpeed = 0.01f;
    if (mCameraSpeed > 0.3f)
        mCameraSpeed = 0.3f;
}

void RenderWindow::handleInput()
{
    //Camera
    mCurrentCamera->setSpeed(0.f);  //cancel last frame movement
    if(mInput.RMB)
    {
        if(mInput.W)
            mCurrentCamera->setSpeed(mCameraSpeed);
        if(mInput.S)
            mCurrentCamera->setSpeed(-mCameraSpeed);
        if(mInput.D)
            mCurrentCamera->moveRight(mCameraSpeed);
        if(mInput.A)
            mCurrentCamera->moveRight(-mCameraSpeed);
        if(mInput.Q)
            mCurrentCamera->updateHeigth(mCameraSpeed);
        if(mInput.E)
            mCurrentCamera->updateHeigth(-mCameraSpeed);
    }
}

void RenderWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) //Shuts down whole program
    {
        mMainWindow->close();
    }

    //    You get the keyboard input like this
    if(event->key() == Qt::Key_W)
    {
        mInput.W = true;
    }
    if(event->key() == Qt::Key_S)
    {
        mInput.S = true;
    }
    if(event->key() == Qt::Key_D)
    {
        mInput.D = true;
    }
    if(event->key() == Qt::Key_A)
    {
        mInput.A = true;
    }
    if(event->key() == Qt::Key_Q)
    {
        mInput.Q = true;
    }
    if(event->key() == Qt::Key_E)
    {
        mInput.E = true;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
        mInput.UP = true;
    }
    if(event->key() == Qt::Key_Down)
    {
        mInput.DOWN = true;
    }
    if(event->key() == Qt::Key_Left)
    {
        mInput.LEFT = true;
    }
    if(event->key() == Qt::Key_Right)
    {
        mInput.RIGHT = true;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
}

void RenderWindow::keyReleaseEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_W)
    {
        mInput.W = false;
    }
    if(event->key() == Qt::Key_S)
    {
        mInput.S = false;
    }
    if(event->key() == Qt::Key_D)
    {
        mInput.D = false;
    }
    if(event->key() == Qt::Key_A)
    {
        mInput.A = false;
    }
    if(event->key() == Qt::Key_Q)
    {
        mInput.Q = false;
    }
    if(event->key() == Qt::Key_E)
    {
        mInput.E = false;
    }
    if(event->key() == Qt::Key_Z)
    {
    }
    if(event->key() == Qt::Key_X)
    {
    }
    if(event->key() == Qt::Key_Up)
    {
        mInput.UP = false;
    }
    if(event->key() == Qt::Key_Down)
    {
        mInput.DOWN = false;
    }
    if(event->key() == Qt::Key_Left)
    {
        mInput.LEFT = false;
    }
    if(event->key() == Qt::Key_Right)
    {
        mInput.RIGHT = false;
    }
    if(event->key() == Qt::Key_U)
    {
    }
    if(event->key() == Qt::Key_O)
    {
    }
}

void RenderWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        mInput.RMB = true;
    if (event->button() == Qt::LeftButton)
        mInput.LMB = true;
    if (event->button() == Qt::MiddleButton)
        mInput.MMB = true;
}

void RenderWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
        mInput.RMB = false;
    if (event->button() == Qt::LeftButton)
        mInput.LMB = false;
    if (event->button() == Qt::MiddleButton)
        mInput.MMB = false;
}

void RenderWindow::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;

    //if RMB, change the speed of the camera
    if (mInput.RMB)
    {
        if (numDegrees.y() < 1)
            setCameraSpeed(0.001f);
        if (numDegrees.y() > 1)
            setCameraSpeed(-0.001f);
    }
    event->accept();
}

void RenderWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (mInput.RMB)
    {
        //Using mMouseXYlast as deltaXY so we don't need extra variables
        mMouseXlast = event->pos().x() - mMouseXlast;
        mMouseYlast = event->pos().y() - mMouseYlast;

        if (mMouseXlast != 0)
            mCurrentCamera->yaw(mCameraRotateSpeed * mMouseXlast);
        if (mMouseYlast != 0)
            mCurrentCamera->pitch(mCameraRotateSpeed * mMouseYlast);
    }
    mMouseXlast = event->pos().x();
    mMouseYlast = event->pos().y();
}
