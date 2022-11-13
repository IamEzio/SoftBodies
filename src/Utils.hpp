#ifndef PROJEKT_UTILS_HPP
#define PROJEKT_UTILS_HPP

#include <GL/glut.h>
#include <cfloat>
#include <Eigen/Dense>

namespace racgra {
    GLuint window_;
    GLuint win_width_ = 600, win_height_ = 600, win_x_ = 100, win_y_ = 100;
    Eigen::Vector3d camera_(1.0, 0.0, 0.0);
    Eigen::Vector3d lookat_(0.0, 0.0, 0.0);
    Eigen::Vector3d lookup_(0.0, 0.0, 1.0);
    double angle_ = 0;
    double fovy_ = 1.0;
    GLdouble near_ = 0.5, far_ = 20.0;
    bool wire_ = false;
    double amount_ = 1;

    void reshape(int w, int h) {
        win_width_ = (uint) w;
        win_height_ = (uint) h;
        glViewport(0, 0, win_width_, win_height_);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glClearColor(0.89f, 0.87f, 0.86f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gluPerspective(fovy_, (float) win_width_ / win_height_, near_, far_);
        gluLookAt(camera_[0], camera_[1], camera_[2],
                  lookat_[0], lookat_[1], lookat_[2],
                  lookup_[0], lookup_[1], lookup_[2]);
        glRotated(angle_, lookup_[0], lookup_[1], lookup_[2]);
    }

    void redisplay_all(void) {
        glutSetWindow(window_);
        reshape(win_width_, win_height_);
        glutPostRedisplay();
    }

    void zoom(bool in, double amount) {
        Eigen::Vector3d dist = lookat_ - camera_;
        // Movement vector.
        Eigen::Vector3d v(dist);
        v.normalize();
        v *= amount;
        // End if movement is larger than distance.
        dist -= v;
        if (in && dist.norm() < 0.1) return;
        // Move.
        in ? camera_ += v : camera_ -= v;
    }

    // draws x, y and z axes in different colors and floor
    void draw_origin() {
        glPushMatrix();
        glLoadIdentity();
        // X
        glLineWidth(3);
        glBegin(GL_LINES);
        glColor3d(1, 0, 0);
        glVertex3d(0, 0, 0);
        glVertex3d(1, 0, 0);
        // Y
        glColor3d(0, 1, 0);
        glVertex3d(0, 0, 0);
        glVertex3d(0, 1, 0);
        // Z
        glColor3d(0, 0, 1);
        glVertex3d(0, 0, 0);
        glVertex3d(0, 0, 1);
        glEnd();
        glLineWidth(1);
        glPopMatrix();

        glBegin(GL_QUADS);
        glColor3d(0.7,0.7,0.7);
        glVertex3d(8,8,0);
        glVertex3d(8,-8,0);
        glVertex3d(-8,-8,0);
        glVertex3d(-8,8,0);
        glEnd();
    }

    void camera_control(char c, double amount) {
        switch (c) {
            case 'a':
                angle_ += 5;
                break;
            case 'd':
                angle_ -= 5;
                break;
            case 'w':
                camera_[2] += amount;
                break;
            case 's':
                camera_[2] -= amount;
                break;
            case 'z':
                zoom(false, amount_);
                break;
            case 'x':
                wire_ = !wire_;
                break;
            default:
                return;
        }
        redisplay_all();
    }

    void mouse_wheel(int button, int dir, int x, int y) {
        switch (button) {
            case GLUT_LEFT_BUTTON:
                angle_ -= amount_ * 2;
                break;
            case GLUT_RIGHT_BUTTON:
                angle_ += amount_ * 2;
                break;
            case 3:
                zoom(true, amount_);
                break;
            case 4:
                zoom(false, amount_);
                break;
        }
        redisplay_all();
    }
};


#endif //PROJEKT_UTILS_HPP
