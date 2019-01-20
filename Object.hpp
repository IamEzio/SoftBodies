//
// Created by lirfu on 20.01.19..
//

#ifndef PROJEKT_OBJECT_HPP
#define PROJEKT_OBJECT_HPP

#include <string>
#include <iostream>
#include <cstring>
#include <GL/glut.h>
#include <vector>
#include <cfloat>
#include <cmath>
#include "Utils.hpp"

class Object {
public:
    Eigen::Vector3f normal_;
    Eigen::Vector3f lookup_;
    std::vector<Eigen::Vector3f> vertices_;
    Eigen::Vector3f center_;
    std::vector<fac> faces_;
    bound bounds_;

    Object() : normal_({1, 0, 0}), lookup_({0, 0, 1}) {};

    void initialize() {}

    void draw(bool wire = false) {
        for (fac &f:faces_) {
            glBegin(wire ? GL_LINE_LOOP : GL_TRIANGLES);
            for (uint index:f.vertices) {
                Eigen::Vector3f &v = vertices_[index];
                glVertex3f(v[0], v[1], v[2]);
            }
            glEnd();
        }
    }

    bound get_bounds() {
        return bounds_;
    }

private:
    inline void update(double val, double &container, bool min) {
        if (min && val < container || !min && val > container)
            container = val;
    }

    void parse(const char *cmd, char *args, ulong line) {
        if (!std::strcmp(cmd, "v")) {  // vertex
            float x, y, z;
            if (!sscanf(args, "%f %f %f", &x, &y, &z))
                throw std::runtime_error("Error reading vertex on line " + std::to_string(line));
            // Update bounds.
            update(x, bounds_.minx, true);
            update(x, bounds_.maxx, false);
            update(y, bounds_.miny, true);
            update(y, bounds_.maxy, false);
            update(z, bounds_.minz, true);
            update(z, bounds_.maxz, false);
            vertices_.emplace_back(x, y, z);

        } else if (!std::strcmp(cmd, "f")) { // face
            uint v1, v2, v3; // Vertex indexes.
            if (!sscanf(args, "%u %u %u", &v1, &v2, &v3))
                throw std::runtime_error("Error reading face on line " + std::to_string(line));
            // Correct indexes.
            ulong size = vertices_.size();
            if (v1 > size)
                throw std::runtime_error("(" + std::to_string(line) +
                                         ") Unknown vertex index: " + std::to_string(v1));
            if (v2 > size)
                throw std::runtime_error("(" + std::to_string(line) +
                                         ") Unknown vertex index: " + std::to_string(v2));
            if (v3 > size)
                throw std::runtime_error("(" + std::to_string(line) +
                                         ") Unknown vertex index: " + std::to_string(v3));
            // Construct.
            std::vector<uint> sss;
            fac face = {.vertices={v1 - 1, v2 - 1, v3 - 1}};
            faces_.push_back(face);
        }
    }

public:
    static Object load(std::string &filepath) {
        FILE *f = fopen(filepath.data(), "r");
        if (!f)
            throw std::runtime_error("Cannot open file: " + filepath);
        Object obj;
        uint line = 0;
        while (true) {
            ++line;
            char *cmd = new char[255];
            char *args = new char[255];
            int res = fscanf(f, "%s %[^\n]s", cmd, args);
            if (res == EOF)
                break;

            if (res < 2 || *cmd == '#') continue;
            obj.parse(cmd, args, line);
            if (!std::strcmp(cmd, "norm")) {
                float x, y, z;
                if (!sscanf(args, "%f %f %f", &x, &y, &z))
                    throw std::runtime_error("Error reading norm on line " + std::to_string(line));
                obj.normal_ = {x, y, z};
            } else if (!std::strcmp(cmd, "lookup")) {
                float x, y, z;
                if (!sscanf(args, "%f %f %f", &x, &y, &z))
                    throw std::runtime_error("Error reading lookup on line " + std::to_string(line));
                obj.lookup_ = {x, y, z};
            }
        }
        fclose(f);
        return obj;
    }
};


#endif //PROJEKT_OBJECT_HPP
