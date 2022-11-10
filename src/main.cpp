#include <iostream>
#include <GL/glut.h>
#include <fstream>
#include "Utils.hpp"
#include "Object.hpp"

bool paused_ = true;
std::vector<Object> objects_;

void display() {
    glMatrixMode(GL_MODELVIEW);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    racgra::draw_origin();

    glPushMatrix();
    glLoadIdentity();
    glColor3i(0, 0, 0);

    for (const Object &o : objects_) {
        o.draw(false);
    }

    glPopMatrix();

    glutSwapBuffers();
}

void timer(int ignore) {
    if (paused_)
        return;

    // Euler integrate objects.
    for (uint i = 0; i < physics::euler_iterations; i++) {
        for (Object &o : objects_) {
            o.calculate(physics::refresh_ms / 1000. / physics::euler_iterations);
        }
        for (uint i = 0; i < objects_.size(); i++) {
            for (uint j = 0; j < objects_.size(); j++) {
                if (i == j) continue;
                objects_[i].collide(objects_[j]);
            }
        }
    }

    racgra::redisplay_all();
    glutTimerFunc(physics::refresh_ms, timer, 0);
}

void keyboard(unsigned char key, int mousex, int mousey) {
    racgra::camera_control(key, 0.5);
    switch (key) {
        case '+':
            physics::g[2] -= 0.2;
            std::cout << "Gravity: " << physics::g[2] << std::endl;
            break;
        case '-':
            physics::g[2] += 0.2;
            if (physics::g[2] > 0)
                physics::g[2] = 0;
            std::cout << "Gravity: " << physics::g[2] << std::endl;
            break;
        case ' ':
            paused_ = !paused_;
            if (!paused_) {
                timer(0);
            }
            break;
        default:;
    }
}

int main(int argc, char **argv) {
    // OPENING CONFIG FILE
    if (argc != 2) {
        std::cerr << "Please provide a config file!" << std::endl;
        std::cerr << "Usage: projekt <config_file>" << std::endl;
        std::cerr << "Config file structure:" << std::endl;
        std::cerr << "<mass>(double) <x>(double) <y>(double) <z>(double) <path_to_model.obj> "
                "<path_to_texture.bmp or for wire model just write '_'>" << std::endl;
    }

    std::fstream filestream;
    filestream.open(argv[1], std::fstream::in);
    if (!filestream) {
        std::cerr << "Cannot open file: " << argv[1] << std::endl;
        return false;
    }
    {
        FILE *f = fopen(argv[1], "r");
        if (!f)
            throw std::runtime_error("Cannot open file: " + std::string(argv[1]));
        uint line = 0;
        while (true) {
            ++line;
            char *cmd = new char[255];
            char *args = new char[255];
            int res = fscanf(f, "%s %[^\n]s", cmd, args);
            if (res == EOF)
                break;

            if (res < 2 || *cmd == '#') {
                continue;
            } else if (std::strcmp(cmd, "o") == 0) {
                double mass, x, y, z;
                char *model_file = new char[100];
                char *texture_file = new char[100];
                int read = sscanf(args, "%lf %lf %lf %lf %s %s", &mass, &x, &y, &z, model_file, texture_file);
                if (read < 5)
                    throw std::runtime_error("Error reading object on line " + std::to_string(line));
                Object o(mass);
                o.load(model_file);
                o.normalize();
                o.move({x, y, z});
                if (std::strcmp(texture_file, "_") != 0) {
                    o.loadTexture(texture_file);
                }
                objects_.push_back(o);
            } else if (std::strcmp(cmd, "rms") == 0) {
                physics::refresh_ms = static_cast<uint>(std::stoi(args));
            } else if (std::strcmp(cmd, "eul") == 0) {
                physics::euler_iterations = static_cast<uint>(std::stoi(args));
            } else if (std::strcmp(cmd, "g") == 0) {
                sscanf(args, "%lf %lf %lf", &physics::g[0], &physics::g[1], &physics::g[2]);
            } else if (std::strcmp(cmd, "k") == 0) {
                physics::spring_k = std::stod(args);
            } else if (std::strcmp(cmd, "kt") == 0) {
                physics::spring_kt = std::stod(args);
            } else if (std::strcmp(cmd, "grb") == 0) {
                physics::ground_rebound_speed_coef = std::stod(args);
            } else if (std::strcmp(cmd, "orb") == 0) {
                physics::object_rebound_coef = std::stod(args);
            }
        }
        fclose(f);
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); // Double buffer
    glutInitWindowSize(1024, 756);
    glutInitWindowPosition(100, 100);

    racgra::camera_ = {8, 8, 4};
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
