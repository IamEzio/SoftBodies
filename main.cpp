#include <iostream>
#include <GL/glut.h>
#include "Utils.hpp"
#include "Object.hpp"

#define REFRESH_MS 100u

bool paused_ = true;
Object obj_;

unsigned int width, height;
unsigned char *texture_data;

void display() {
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    racgra::draw_origin();

    glPushMatrix();
    glLoadIdentity();
    glColor3i(0, 0, 0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_data);

    obj_.draw(false);

    glDisable(GL_TEXTURE_2D);

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

    std::string object("../res/objects/cube.obj");
    obj_ = Object::load(object);

    obj_.move({0, 0, 0.5});

    // Load texture.
    unsigned char header[54];
    char *texture_file = const_cast<char *>("../res/textures/cat.bmp");
    FILE *file = fopen(texture_file, "rb");
    if (!file) {
        std::cout << "Image could not be opened! " << texture_file << std::endl;
        return 1;
    } else if (fread(header, 1, 54, file) != 54) { // If not 54 bytes read : problem
        printf("Not a correct BMP file\n");
        return 1;
    } else if (header[0] != 'B' || header[1] != 'M') {
        printf("Not a correct BMP file\n");
        return 1;
    }
    unsigned int dataPos = *(int *) &(header[0x0A]);
    unsigned int imageSize = *(int *) &(header[0x22]);
    width = *(int *) &(header[0x12]);
    height = *(int *) &(header[0x16]);
    if (imageSize == 0) imageSize = width * height * 3;
    if (dataPos == 0) dataPos = 54;
    texture_data = new unsigned char[imageSize];
    fread(texture_data, imageSize, 1, file);
    fclose(file);

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
