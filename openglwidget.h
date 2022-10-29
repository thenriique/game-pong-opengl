#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_0_Core>
#include <QTimer>
#include <QElapsedTimer>
#include <QVector4D>
#include <QFile>
#include <QKeyEvent>
#include <QApplication>

class OpenGLWidget : public QOpenGLWidget, public QOpenGLFunctions_4_0_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void drawRackets();
    void drawBall();

    GLuint vboVertRacket{0};
    GLuint eboIndRacket{0};
    GLuint vaoRacket{0};

    GLuint vboVertBall{0};
    GLuint eboIndBall{0};
    GLuint vaoBall{0};

    GLuint shaderProgram{0};

    void createVBOs();
    void createShaders();

    void destroyVBOs();
    void destroyShaders();

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    float playerOnePosY{0};
    float playerOnePosYOffset{0};
    int playerOnePoints{0};

    float playerTwoPosY{0};
    float playerTwoPosYOffset{0};
    int playerTwoPoints{0};

    bool ballInGame{false};
    std::array<float, 2> ballPos{0.0f, 0.0f};
    std::array<float, 2> ballPosOffset{-0.5f, 1.0f};

    QTimer timer;
    QElapsedTimer elapsedTimer;

private:

signals:
    void updateScorePlayerOne(QString);
    void updateScorePlayerTwo(QString);
    void UpdatePlayerWin(QString);

protected:


public slots:
    void animate();
};

#endif // OPENGLWIDGET_H
