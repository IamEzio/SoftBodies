#include <iostream>
#include <GL/glut.h>
#include "Utils.hpp"
#include "Object.hpp"

uint refreshms = 20;
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
    glColor3i(0,0,0);

    obj_.draw(racgra::wire_);

    glPopMatrix();

    glutSwapBuffers();
}

void keyboard(unsigned char key, int mousex, int mousey) {
    racgra::camera_control(key, 0.5);
    racgra::redisplay_all();
}

void timer(int ignore) {
    racgra::redisplay_all();
    glutTimerFunc(refreshms, timer, 0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); // Double buffer
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);

    std::string object("../res/cube.obj");
    obj_ = Object::load(object);

    bound b = obj_.get_bounds();
    racgra::camera_[0] = b.maxx + 10;
    racgra::camera_[1] = b.maxy + 10;
    racgra::camera_[2] = b.maxz + 10;
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