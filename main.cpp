#include <iostream>
#include <GL/glut.h>
#include "Utils.hpp"
#include "Object.hpp"

#define REFRESH_MS 100u

bool paused_ = true;
Object obj_;

void display() {
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    racgra::draw_origin();

//    // Draw ground.
//    glPushMatrix();
//    glLoadIdentity();
//    glColor3f(0.5, 0.5, 0.5);
//    glBegin(GL_QUADS);
//    glVertex3f(-500, -500, 0);
//    glVertex3f(-500, 500, 0);
//    glVertex3f(500, 500, 0);
//    glVertex3f(500, -500, 0);
//    glEnd();
//    glPopMatrix();

    glPushMatrix();
    glLoadIdentity();
    glColor3i(0, 0, 0);
    obj_.draw(racgra::wire_);
    glPopMatrix();

    glutSwapBuffers();
}

void timer(int ignore) {
    if (paused_)
        return;
    uint iterations = 60;
    for (uint i = 0; i < iterations; i++) {
        obj_.calculate(REFRESH_MS / 1000. / iterations);
    }
    racgra::redisplay_all();
    glutTimerFunc(REFRESH_MS, timer, 0);
}

void keyboard(unsigned char key, int mousex, int mousey) {
    racgra::camera_control(key, 0.5);
    switch (key) {
        case '+':
            obj_.gravity_acc_[2] -= 0.2;
            std::cout << "Gravity: " << obj_.gravity_acc_[2] << std::endl;
            break;
        case '-':
            obj_.gravity_acc_[2] += 0.2;
            if (obj_.gravity_acc_[2] > 0)
                obj_.gravity_acc_[2] = 0;
            std::cout << "Gravity: " << obj_.gravity_acc_[2] << std::endl;
            break;
        case ' ':
            paused_ = !paused_;
            if (!paused_) {
                timer(0);
            }
            break;
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // Double buffer
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    std::string object("../res/cube.obj");
    obj_ = Object::load(object);
//    obj_ = Object({2, 1, 0}, {2, 1, 1});

    obj_.move({0, 0, 5});

    racgra::camera_ = {4, 3, 3};
    racgra::near_ = 0.1;
    racgra::far_ = 100;
    racgra::fovy_ = 45;

    racgra::window_ = static_cast<GLuint>(glutCreateWindow("Gummy face"));
    glutReshapeFunc(racgra::reshape);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(racgra::mouse_wheel);


    racgra::redisplay_all();
    timer(0);
    glutMainLoop();
    return 0;
}