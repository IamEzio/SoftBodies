//
// Created by lirfu on 20.01.19..
//

#ifndef PROJEKT_OBJECT_HPP
#define PROJEKT_OBJECT_HPP

#include <string>
#include <iostream>
#include <cstring>
#include <GL/glut.h>
#include <utility>
#include <vector>
#include <map>
#include <cmath>
#include "Utils.hpp"

using namespace Eigen;

class Object {
private:
    struct Vertex {
        Vector3d position;
        Vector3d velocity;
        Vector3d acceleration;
        double mass;
        bool movable;

        Vertex(Vector3d ver, Vector3d vel, Vector3d forc, double mass, bool movable) :
                position(ver), velocity(vel), acceleration(forc), mass(mass), movable(movable) {}
    };

    // Loaded.
    std::vector<fac> faces_;
    // Calculated constants.
    bound bounds_;
    std::vector<double> init_distances_;
    // Variables.
    std::vector<Vertex> vertices_;

public:
    Vector3d gravity_acc_ = {0, 0, -9.81};
    double mass_ = 10;
    double k = 200;
    double k_t = 1;

    Object() = default;

    Object(Vector3d p1, Vector3d p2) {
        vertices_.emplace_back(p1, Vector3d(0, 0, 0), Vector3d(0, 0, 0), 0.5, true);
        vertices_.emplace_back(p2, Vector3d(0, 0, 0), Vector3d(0, 0, 0), 0.5, true);
        faces_.emplace_back(0, 0, 1);
        initialize();
    }

    void draw(bool wire = false) const {
        for (const fac &f : faces_) {
            glPointSize(7);
            glBegin(wire ? GL_LINE_LOOP : GL_TRIANGLES);
            for (uint index : f.vertices) {
                const Vector3d &v = vertices_.at(index).position;
                glVertex3d(v[0], v[1], v[2]);
            }
            glEnd();
            glBegin(GL_POINTS);
            for (uint index : f.vertices) {
                const Vector3d &v = vertices_.at(index).position;
                glVertex3d(v[0], v[1], v[2]);
            }
            glEnd();
        }
    }

    Vector3d center() const {
        Vector3d center;
        for (const Vertex &v : vertices_)
            center += v.position;
        center /= vertices_.size();
        return center;
    }

    void move(const Vector3d &m) {
        for (Vertex &v : vertices_) {
            v.position += m;
        }
    }

private:
    inline Vector3d getForceFor(uint v1, uint v2, double d0) const {
        Vector3d diff = vertices_[v2].position - vertices_[v1].position;
        double distance = diff.norm();
        double compression = distance - d0;
        diff.normalize();

        Vector3d velocity = vertices_[v2].velocity - vertices_[v1].velocity;

        return (compression * k + diff.dot(velocity) * k_t) * diff;
    }

public:
    void calculate(double dt) {
        // Set initial acceleration by gravity.
        for (Vertex &v : vertices_) {
            v.acceleration = gravity_acc_;
        }

        // Accumulate structural forces on vertices.
        uint t = 0;
        for (uint i = 0; i < vertices_.size(); i++) {
            for (uint j = i + 1; j < vertices_.size(); j++) {
                Vector3d force = getForceFor(i, j, init_distances_[t++]);
                if (vertices_[i].movable)
                    vertices_[i].acceleration += force / vertices_[i].mass;
                if (vertices_[j].movable)
                    vertices_[j].acceleration -= force / vertices_[j].mass;
            }
        }

        // Apply forces to vertices.
        for (uint i = 0; i < vertices_.size(); i++) {
            if (!vertices_[i].movable) continue;

            vertices_[i].position += dt * vertices_[i].velocity;
            vertices_[i].velocity = 0.999 * vertices_[i].velocity + dt * vertices_[i].acceleration;

            // Force ground.
            if (vertices_[i].position[2] < 0) {
                vertices_[i].position[2] = 0;
                vertices_[i].velocity[2] = 0;
            }
        }
    }

    const bound &get_bounds() const {
        return bounds_;
    }

private:
    inline double dist(const Vector3d &v1, const Vector3d &v2) const {
        return (v2 - v1).norm();
    }

    inline void initialize() {
        init_distances_.reserve(vertices_.size() * (vertices_.size() - 1) / 2);
        for (uint i = 0; i < vertices_.size(); i++) {
            for (uint j = i + 1; j < vertices_.size(); j++) {
                init_distances_.emplace_back(dist(vertices_[i].position, vertices_[j].position));
            }
            vertices_[i].mass = mass_ / vertices_.size();
        }
    }

    inline void update_bound(double val, double &container, bool min) {
        if (min && val < container || !min && val > container)
            container = val;
    }

    inline bool parse(const char *cmd, char *args, ulong line) {
        if (!std::strcmp(cmd, "v")) {  // vertex
            double x, y, z;
            if (!sscanf(args, "%lf %lf %lf", &x, &y, &z))
                throw std::runtime_error("Error reading vertex on line " + std::to_string(line));
            // Update bounds.
            update_bound(x, bounds_.minx, true);
            update_bound(x, bounds_.maxx, false);
            update_bound(y, bounds_.miny, true);
            update_bound(y, bounds_.maxy, false);
            update_bound(z, bounds_.minz, true);
            update_bound(z, bounds_.maxz, false);
            vertices_.push_back(Vertex(Vector3d(x, y, z), Vector3d(0., 0., 0.), Vector3d(0., 0., 0.), 0., true));
            return true;

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
            faces_.emplace_back(v1 - 1, v2 - 1, v3 - 1);
            return true;
        }
        return false;
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

            if (res < 2 || *cmd == '#') {
                continue;
            } else if (obj.parse(cmd, args, line)) {
                continue;
            }
        }
        fclose(f);
        obj.initialize();
        return obj;
    }
};


#endif //PROJEKT_OBJECT_HPP
