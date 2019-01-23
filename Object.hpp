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
        Vector3d normal;

        double mass;
        bool movable;

        Vertex(Vector3d pos) : position(std::move(pos)) {
            velocity = {0, 0, 0};
            acceleration = {0, 0, 0};
            normal = {0, 0, 0};
            mass = 0;
            movable = true;
        }
    };

    struct fac {
        std::vector<uint> vertices;
        std::vector<Vector2d> texture_positions;

        fac(uint v1, uint v2, uint v3) {
            vertices.emplace_back(v1);
            vertices.emplace_back(v2);
            vertices.emplace_back(v3);
        }

        uint operator[](uint index) const {
            return vertices[index];
        }
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

    void draw(bool points = false) const {
        for (const fac &f : faces_) {
            glBegin(GL_TRIANGLES);
            for (uint i = 0; i < 3; i++) {
                const Vertex &v = vertices_.at(f.vertices[i]);
                glTexCoord2d(f.texture_positions[i][0], f.texture_positions[i][1]);
                glVertex3d(v.position[0], v.position[1], v.position[2]);
            }
            glEnd();
            if (points) {
                glPointSize(7);
                glBegin(GL_POINTS);
                for (uint index : f.vertices) {
                    const Vector3d &v = vertices_.at(index).position;
                    glVertex3d(v[0], v[1], v[2]);
                }
                glEnd();
            }
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

    inline bool parse(const char *cmd, char *args, ulong line,
                      std::vector<Vector2d> &tmp_txtr, std::vector<Vector3d> &tmp_norm) {
        if (std::strcmp(cmd, "v") == 0) {  // Vertex.
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
            vertices_.emplace_back(Vector3d(x, y, z));
            return true;

        } else if (strcmp(cmd, "vt") == 0) {  // Texture coordinate.
            double u, v;
            if (!sscanf(args, "%lf %lf", &u, &v))
                throw std::runtime_error("Error reading vertex texture coordinate on line " + std::to_string(line));
            tmp_txtr.emplace_back(u, v);
            return true;

        } else if (strcmp(cmd, "vn") == 0) {  // Vertex normal.
            double x, y, z;
            if (!sscanf(args, "%lf %lf %lf", &x, &y, &z))
                throw std::runtime_error("Error reading vertex normal coordinate on line " + std::to_string(line));
            tmp_norm.emplace_back(x, y, z);
            return true;

        } else if (std::strcmp(cmd, "f") == 0) {  // Face.
            uint v1, v2, v3; // Vertex indexes.
            uint t1, t2, t3; // Texture indexes.
            uint n1, n2, n3; // Normal indexes.

            if (sscanf(args, "%d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3) == 9) {
                fac f(v1 - 1, v2 - 1, v3 - 1);
                f.texture_positions.push_back(tmp_txtr[t1 - 1]);
                f.texture_positions.push_back(tmp_txtr[t2 - 1]);
                f.texture_positions.push_back(tmp_txtr[t3 - 1]);
                faces_.push_back(f);

                vertices_[v1 - 1].normal = tmp_norm[n1 - 1];
                vertices_[v2 - 1].normal = tmp_norm[n2 - 1];
                vertices_[v3 - 1].normal = tmp_norm[n3 - 1];
                return true;

            } else if (sscanf(args, "%d/%d %d/%d %d/%d", &v1, &t1, &v2, &t2, &v3, &t3) == 6) {
                if (!tmp_norm.empty()) {
                    faces_.emplace_back(v1 - 1, v2 - 1, v3 - 1);
                    vertices_[v1 - 1].normal = tmp_norm[t1 - 1];
                    vertices_[v2 - 1].normal = tmp_norm[t2 - 1];
                    vertices_[v3 - 1].normal = tmp_norm[t3 - 1];
                } else {
                    fac f(v1 - 1, v2 - 1, v3 - 1);
                    f.texture_positions.push_back(tmp_txtr[t1 - 1]);
                    f.texture_positions.push_back(tmp_txtr[t2 - 1]);
                    f.texture_positions.push_back(tmp_txtr[t3 - 1]);
                    faces_.push_back(f);
                }
                return true;

            } else if (sscanf(args, "%u %u %u", &v1, &v2, &v3) == 3) {// Correct indexes.
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
                faces_.emplace_back(v1 - 1, v2 - 1, v3 - 1);
                return true;

            } else {
                throw std::runtime_error("Error reading face on line " + std::to_string(line));
            }
        }
        return false;
    }

public:
    static Object load(std::string &filepath) {
        FILE *f = fopen(filepath.data(), "r");
        if (!f)
            throw std::runtime_error("Cannot open file: " + filepath);
        Object obj;
        std::vector<Vector2d> tmp_texture_coordinates;
        std::vector<Vector3d> tmp_normal;
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
            } else if (obj.parse(cmd, args, line, tmp_texture_coordinates, tmp_normal)) {
                continue;
            }
        }
        fclose(f);
        obj.initialize();
        return obj;
    }
};


#endif //PROJEKT_OBJECT_HPP
