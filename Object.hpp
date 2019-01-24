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
#include "Physics.hpp"
#include "Shapes.hpp"

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
    double mass_;
    bool soft_;
    // Calculated constants.
    std::vector<double> init_distances_;
    // Variables.
    std::vector<Vertex> vertices_;
    shapes::Cube shape;
    // Texture data.
    unsigned int texture_width, texture_height;
    unsigned char *texture_data = nullptr;
public:

    Object(double mass, bool soft = false) : mass_(mass), shape(shapes::Cube(shapes::bound())), soft_(soft) {}

    void draw(bool points = false) const {
        if (texture_data) { // Set texture.
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height,
                         0, GL_BGR, GL_UNSIGNED_BYTE, texture_data);
        }
        for (const fac &f : faces_) {
            glBegin(texture_data ? GL_TRIANGLES : GL_LINE_LOOP);
            for (uint i = 0; i < 3; i++) {
                const Vertex &v = vertices_.at(f.vertices[i]);
                if (texture_data) {
                    glTexCoord2d(f.texture_positions[i][0], f.texture_positions[i][1]);
                }
                glVertex3d(v.position[0], v.position[1], v.position[2]);
            }
            glEnd();
//            if (points) {
//                glPointSize(7);
//                glBegin(GL_POINTS);
//                for (uint index : f.vertices) {
//                    const Vector3d &v = vertices_.at(index).position;
//                    glVertex3d(v[0], v[1], v[2]);
//                }
//                glEnd();
//            }
        }
        if (texture_data) {
            glDisable(GL_TEXTURE_2D);
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

    void normalize() {
        for (Vertex &v:vertices_) {
            update_bounds(v.position);
        }
        Vector3d min = {shape.bounds.minx, shape.bounds.miny, shape.bounds.minz};
        Vector3d max = {shape.bounds.maxx, shape.bounds.maxy, shape.bounds.maxz};
        max = (max - min);
        for (Vertex &v:vertices_) {
            v.position = (v.position - min);
            v.position[0] /= max[0];
            v.position[1] /= max[1];
            v.position[2] /= max[2];
        }
    }

private:
    inline Vector3d getForceFor(uint v1, uint v2, double d0) const {
        Vector3d diff = vertices_[v2].position - vertices_[v1].position;
        double distance = diff.norm();
        double compression = distance - d0;
        diff.normalize();

        Vector3d velocity = vertices_[v2].velocity - vertices_[v1].velocity;

        return (compression * physics::spring_k + diff.dot(velocity) * physics::spring_kt) * diff;
    }

public:
    void calculate(double dt) {
        // Set initial acceleration by gravity.
        for (Vertex &v : vertices_) {
            v.acceleration = physics::g;
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
        shape.bounds = shapes::bound(); // Refresh rectangle bound.
        for (uint i = 0; i < vertices_.size(); i++) {
            if (!vertices_[i].movable) continue;

            Vector3d &position = vertices_[i].position;

            position += dt * vertices_[i].velocity;
            vertices_[i].velocity = vertices_[i].velocity + dt * vertices_[i].acceleration;

            // Force ground.
            if (position[2] < 0) {
                position[2] = 0;
                vertices_[i].velocity[2] *= -1 * physics::ground_rebound_speed_coef;
            }

            // Update rectangle bounds.
            update_bounds(position);
        }
    }

    bool isInside(const Vector3d &p, double &value, Vector3d &norm, Vector3d &v) {
        bool t = false;
        value = -DBL_MAX;
        for (const fac &f : faces_) {
            Vector3d v1 = vertices_[f[1]].position - vertices_[f[0]].position;
            Vector3d v2 = vertices_[f[2]].position - vertices_[f[0]].position;
            Vector3d n = v1.cross(v2);
            double val = n.dot(p) - n.dot(vertices_[f[0]].position);
            if (val < 0) {
                t = true;
                if (val > value) { // Closest face.
                    norm = n;
                    value = val;
                    v = vertices_[f[0]].position;
                }
            }
        }
        return t;
    }

    void collide(Object &o) {
        if (!shapes::isCollision(shape, o.shape)) return;  // Fast filter.

        // Invert speeds for colliding vertices.
        for (Vertex &v : o.vertices_) {
            if (shape.bounds.inside(v.position)) {  // Filter unwanted vertices.
                double val; // Amount inside (negative value)
                Vector3d n; // Normal (direction for correction).
                Vector3d v1;
                if (isInside(v.position, val, n, v1)) { // Check if truly inside the object.
                    std::cout << "Collision!" << std::endl;
                    v.position += std::abs(n.dot(v.position - v1)) * n / n.norm(); // Backtrack the distance.
                    v.velocity *= -physics::object_rebound_coef;
                }
            }
        }
    }

    const shapes::Shape &getShape() const {
        return shape;
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

    inline void update_bounds(Vector3d val) {
        if (val[0] < shape.bounds.minx) shape.bounds.minx = val[0];
        if (val[0] > shape.bounds.maxx) shape.bounds.maxx = val[0];
        if (val[1] < shape.bounds.miny) shape.bounds.miny = val[1];
        if (val[1] > shape.bounds.maxy) shape.bounds.maxy = val[1];
        if (val[2] < shape.bounds.minz) shape.bounds.minz = val[2];
        if (val[2] > shape.bounds.maxz) shape.bounds.maxz = val[2];
    }

    inline bool parse(const char *cmd, char *args, ulong line,
                      std::vector<Vector2d> &tmp_txtr, std::vector<Vector3d> &tmp_norm) {
        if (std::strcmp(cmd, "v") == 0) {  // Vertex.
            double x, y, z;
            if (!sscanf(args, "%lf %lf %lf", &x, &y, &z))
                throw std::runtime_error("Error reading vertex on line " + std::to_string(line));
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
    void load(const char *filepath) {
        FILE *f = fopen(filepath, "r");
        if (!f)
            throw std::runtime_error("Cannot open file: " + std::string(filepath));

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
            } else if (parse(cmd, args, line, tmp_texture_coordinates, tmp_normal)) {
                continue;
            }
        }
        fclose(f);
        initialize();
    }

    void load(std::string &filepath) {
        load(filepath.c_str());
    }

    int loadTexture(const char *texture_file) {
        FILE *file = fopen(texture_file, "rb");
        unsigned char header[54];
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
        texture_width = static_cast<unsigned int>(*(int *) &(header[0x12]));
        texture_height = static_cast<unsigned int>(*(int *) &(header[0x16]));
        unsigned int dataPos = static_cast<unsigned int>(*(int *) &(header[0x0A]));
        unsigned int imageSize = static_cast<unsigned int>(*(int *) &(header[0x22]));
        if (imageSize == 0) imageSize = texture_width * texture_height * 3;
        if (dataPos == 0) dataPos = 54;
        texture_data = new unsigned char[imageSize];
        fread(texture_data, imageSize, 1, file);
        fclose(file);
    }
};


#endif //PROJEKT_OBJECT_HPP
