#include "openglwidget.h"

OpenGLWidget::OpenGLWidget(QWidget *parent):QOpenGLWidget{parent}
{
}

OpenGLWidget::~OpenGLWidget()
{
    destroyVBOs();
    destroyShaders();
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0,0,0,1);

    qDebug("OpenGL Version: %s",glGetString(GL_VERSION));
    qDebug("GLSL Version: %s",glGetString(GL_SHADING_LANGUAGE_VERSION));

    connect(&timer, &QTimer::timeout, this, &OpenGLWidget::animate);
    timer.start(0);
    elapsedTimer.start();

    createShaders();
    createVBOs();
}

void OpenGLWidget::resizeGL(int w, int h)
{

}

void OpenGLWidget::drawRackets()
{
    makeCurrent();
    glUseProgram(shaderProgram);

    auto locColor{glGetUniformLocation(shaderProgram, "vColor")};
    GLint locScaling{glGetUniformLocation(shaderProgram, "scaling")};
    GLint locTranslation{glGetUniformLocation(shaderProgram, "translation")};

    glBindVertexArray(vaoRacket);

    // player one
    glUniform4f(locColor, 1, 1, 1, 1);
    glUniform1f(locScaling, 0.2f);
    glUniform4f(locTranslation, -0.9f, playerOnePosY, 0, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // player two
    glUniform4f(locColor, 1, 1, 1, 1);
    glUniform1f(locScaling, 0.2f);
    glUniform4f(locTranslation, 0.9f, playerTwoPosY, 0, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGLWidget::drawBall()
{
    makeCurrent();
    glUseProgram(shaderProgram);

    auto locColor{glGetUniformLocation(shaderProgram, "vColor")};
    GLint locTranslation{glGetUniformLocation(shaderProgram, "translation")};

    glBindVertexArray(vaoBall);

    glUniform4f(locColor, 1, 1, 1, 1);
    glUniform4f(locTranslation, ballPos[0], ballPos[1], 0, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);

    drawRackets();

    if (ballInGame) {
        drawBall();
    }

}

void OpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_W:
            playerOnePosYOffset = 2.0f;
            break;
        case Qt::Key_S:
            playerOnePosYOffset = -2.0f;
            break;
        case Qt::Key_Up:
            playerTwoPosYOffset = 2.0f;
            break;
        case Qt::Key_Down:
            playerTwoPosYOffset = -2.0f;
            break;
        case Qt::Key_Space:
            if (playerOnePoints < 10 && playerTwoPoints < 10 && !ballInGame) {
                float randX = (std::rand() % 300)/100.0f - 1.5f; // generate random x offset [-1.5, 1.5]
                float randY = (std::rand() % 300)/100.0f - 1.5f; // generate random y offset [-1.5, 1.5]

                ballPosOffset = { randX, randY };

                ballInGame = true;
            }
            break;
        case Qt::Key_R:
            ballInGame = false;
            emit UpdatePlayerWin(QString("")); // remove wins message

            playerOnePosY = 0;
            playerOnePoints = 0;
            emit updateScorePlayerOne(QString("%1").arg(playerOnePoints));

            playerTwoPosY = 0;
            playerTwoPoints = 0;
            emit updateScorePlayerTwo(QString("%1").arg(playerTwoPoints));
            break;
        case Qt::Key_Escape:
            QApplication::quit();
        break;
    }
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_W:
        case Qt::Key_S:
           playerOnePosYOffset = 0.0f;
           break;
        case Qt::Key_Up:
        case Qt::Key_Down:
            playerTwoPosYOffset = 0.0f;
            break;
    }
}

void OpenGLWidget::createShaders()
{
    makeCurrent();
    destroyShaders();

    QFile vs(":/shaders/vshader1.glsl");
    QFile fs(":/shaders/fshader1.glsl");

    if(!vs.open(QFile::ReadOnly | QFile::Text)) return;
    if(!fs.open(QFile::ReadOnly | QFile::Text)) return;

    auto byteArrayVs{vs.readAll()};
    auto byteArrayFs{fs.readAll()};
    const char *c_strVs{byteArrayVs};
    const char *c_strFs{byteArrayFs};

    vs.close();
    fs.close();

    GLuint vertexShader{glCreateShader(GL_VERTEX_SHADER)};
    glShaderSource(vertexShader, 1, &c_strVs, 0);
    glCompileShader(vertexShader);
    GLint isCompiled{0};

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength{0};
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);
        glDeleteShader(vertexShader);
        return;
    }

    GLuint fragmentShader{glCreateShader(GL_FRAGMENT_SHADER)};
    glShaderSource(fragmentShader, 1, &c_strFs, 0);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength{0};
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint isLinked{0};
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength{0};
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &maxLength);
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(shaderProgram, maxLength, &maxLength, &infoLog[0]);
        qDebug("%s", &infoLog[0]);
        glDeleteProgram(shaderProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void OpenGLWidget::destroyShaders()
{
    makeCurrent();
    glDeleteProgram(shaderProgram);
}

void OpenGLWidget::createVBOs()
{
    makeCurrent();
    destroyVBOs();

    // rackets vao
    std::vector<QVector4D> vertRackets;
    std::vector<GLuint> indRackets;

    vertRackets.resize(4);
    indRackets.resize(6);

    float offsetX{0.1f};
    float offsetY{0.4f};

    vertRackets[0] = QVector4D(0 - offsetX, 0 - offsetY, 0, 1);
    vertRackets[1] = QVector4D(0 + offsetX, 0 - offsetY, 0, 1);
    vertRackets[2] = QVector4D(0 + offsetX, 0 + offsetY, 0, 1);
    vertRackets[3] = QVector4D(0 - offsetX, 0 + offsetY, 0, 1);

    indRackets[0] = 0; indRackets[1] = 1; indRackets[2] = 2;
    indRackets[3] = 2; indRackets[4] = 3; indRackets[5] = 0;

    glGenVertexArrays(1, &vaoRacket);
    glBindVertexArray(vaoRacket);

    glGenBuffers(1, &vboVertRacket);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertRacket);
    glBufferData(GL_ARRAY_BUFFER, vertRackets.size()*sizeof(QVector4D), vertRackets.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &eboIndRacket);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndRacket);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indRackets.size()*sizeof(GLuint), indRackets.data(), GL_STATIC_DRAW);


    // ball vao
    std::vector<QVector4D> vertBall;
    std::vector<GLuint> indBall;

    vertBall.resize(4);
    indBall.resize(6);

    vertBall[0] = QVector4D(-0.1, -0.1, 0, 1);
    vertBall[1] = QVector4D( 0.1, -0.1, 0, 1);
    vertBall[2] = QVector4D( 0.1,  0.1, 0, 1);
    vertBall[3] = QVector4D(-0.1,  0.1, 0, 1);

    indBall[0] = 0; indBall[1] = 1; indBall[2] = 2;
    indBall[3] = 2; indBall[4] = 3; indBall[5] = 0;

    glGenVertexArrays(1, &vaoBall);
    glBindVertexArray(vaoBall);

    glGenBuffers(1, &vboVertBall);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertBall);
    glBufferData(GL_ARRAY_BUFFER, vertBall.size()*sizeof(QVector4D), vertBall.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &eboIndBall);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIndBall);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indBall.size()*sizeof(GLuint), indBall.data(), GL_STATIC_DRAW);
}

void OpenGLWidget::destroyVBOs()
{
    makeCurrent ();

    glDeleteBuffers(1, &vboVertRacket);
    glDeleteBuffers (1, &eboIndRacket);
    glDeleteVertexArrays (1, &vaoRacket);
    vboVertRacket=0;
    eboIndRacket=0;
    vaoRacket=0;

    glDeleteBuffers(1, &vboVertBall);
    glDeleteBuffers (1, &eboIndBall);
    glDeleteVertexArrays (1, &vaoBall);
    vboVertBall=0;
    eboIndBall=0;
    vaoBall=0;
}

void OpenGLWidget::animate()
{
    float time{elapsedTimer.restart()/1000.0f};
    float border{0.9f};
    float contactLimitBall{0.100f};

    // player one
    playerOnePosY += playerOnePosYOffset * time;
    if (playerOnePosY > border) playerOnePosY = border;
    if (playerOnePosY < -border) playerOnePosY = -border;

    // player two
    playerTwoPosY += playerTwoPosYOffset * time;
    if (playerTwoPosY > border) playerTwoPosY = border;
    if (playerTwoPosY < -border) playerTwoPosY = -border;

    // ball
    if (ballInGame) {
        ballPos[0] += ballPosOffset[0] * time;
        ballPos[1] += ballPosOffset[1] * time;


        // map limit
        if (ballPos[0] < -1.0f || ballPos[0] > 1.0f) {

            // score
            if (ballPos[0] < 0) {
                playerTwoPoints++;
                emit updateScorePlayerTwo(QString("%1").arg(playerTwoPoints));
            } else {
                playerOnePoints++;
                emit updateScorePlayerOne(QString("%1").arg(playerOnePoints));
            }

            ballPos = {0.0f, 0.0f};
            ballInGame = false;
        }

        // ceiling and floor reflection
        if (ballPos[1] < -1.0f || ballPos[1] > 1.0f) {
            ballPos[1] = ballPos[1] < 0 ? -1.0f : 1.0f;
            ballPosOffset[1] = -ballPosOffset[1];
        }


        // collision
        if (ballPos[0] < -(border - 0.05f) && ballPos[0] > -border) {

            float playerOneDiff = std::fabs(ballPos[1] - playerOnePosY);
            if( playerOneDiff < contactLimitBall)
            {
                int defOne = playerOneDiff < 0.5*contactLimitBall ? 1 : -1; // define the increase/decrease in ball offset

                ballPos[0] = -(border - 0.05f);
                ballPosOffset[0] = ballPosOffset[0] > -2 ? -(ballPosOffset[0] - defOne * (ballPosOffset[0] + 2) * 0.5) : -ballPosOffset[0]; // new ball offset (limited for -2)
            }
        }


        if (ballPos[0] > (border - 0.05f) && ballPos[0] < border) {

            float playerTwoDIff = std::fabs(ballPos[1] - playerTwoPosY);
            if( playerTwoDIff < contactLimitBall)
            {
                int defTwo = playerTwoDIff < 0.75 * contactLimitBall ? 1 : -1; // define the increase/decrease in ball offset

                ballPos[0] = (border - 0.05f);
                ballPosOffset[0] = ballPosOffset[0] < 2 ? -(ballPosOffset[0] + defTwo * (2 - ballPosOffset[0]) * 0.5) : -ballPosOffset[0]; // new ball offset (limited for 2)
            }
        }


    } else {
        if (playerOnePoints == 10) emit UpdatePlayerWin(QString("Player One Wins!"));
        if (playerTwoPoints == 10) emit UpdatePlayerWin(QString("Player Two Wins!"));
    }

    update();
}
