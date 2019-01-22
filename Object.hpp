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
#include <map>
#include <cfloat>
#include <cmath>
#include "Utils.hpp"

using namespace Eigen;

class Object {
private:
    float mass_ = 1;
public:
    // Loaded.
    std::vector<fac> faces_;
    Vector3f normal_;
    Vector3f lookup_;
    // Calculated constants.
    bound bounds_;
    std::vector<float> init_distances_;
    // Variables.
    std::vector<Vector3f> vertices_;
    std::vector<Vector3f> velocities_;

    Object() : normal_({1, 0, 0}), lookup_({0, 0, 1}) {};

    void draw(bool wire = false) {
        for (fac &f : faces_) {
            glBegin(wire ? GL_LINE_LOOP : GL_TRIANGLES);
            for (uint index : f.vertices) {
                Vector3f &v = vertices_[index];
                glVertex3f(v[0], v[1], v[2]);
            }
            glEnd();
        }
    }

private:
    Vector3f getForceFor(uint v1, uint v2, float d0, float k, float k_t) {
        Vector3f forc = vertices_[v2] - vertices_[v1];
        forc *= k * (1 - d0 / forc.norm());
        // Damping.
        forc -= k_t *
                (velocities_[v1] - velocities_[v2]).dot(vertices_[v1] - vertices_[v2])
                * (vertices_[v1] - vertices_[v2]);
        return forc;
    }

public:
    void calculate(float dt) {
        Vector3f gravity = {0, 0, 0};
        gravity *= mass_;
//        // Calculate centroid.
//        Vector3f center({0, 0, 0});
//        for (Vector3f &v:vertices_) {
//            center += v;
//        }
//        center /= vertices_.size();
//        uint i = 0;
//        for (Vector3f &v:vertices_) {
//            // Calculate centroid vector.
//            Vector3f force = center - v;
//            float distance = force.norm();
//            // Centroid force.
//            force *= ((distance - init_distances_[i]) / distance);
//            std::cout << force << std::endl;
//            // First vertex force.
//
//            // Total force.
//            force += gravity;
//            force *= 0.000001;
//            // Update vertex velocity
//            Vector3f v0 = velocities_[i];
//            velocities_[i] += dt * force;
//            // Update vertex position.
//            v += dt * v0 + 0.5 * dt * dt * force;
//            // Ground is z=0.
//            if (v[2] < 0) {
//                v[2] = 0;
//            }
//            i++;
//        }

        // Accumulate forces on vertices.
        float m = mass_ / vertices_.size();
        float k = 0.01;
        float k_t = 0.1;
        std::vector<Vector3f> forces;
        forces.reserve(vertices_.size());
        uint i = 0;
        for (const fac &f : faces_) {
            // First pair.
            Vector3f force = getForceFor(f[0], f[1], init_distances_[i++], k, k_t);
            forces[f[0]] += force;
            forces[f[1]] -= force;

            force = getForceFor(f[1], f[2], init_distances_[i++], k, k_t);
            forces[f[1]] += force;
            forces[f[2]] -= force;

            force = getForceFor(f[2], f[0], init_distances_[i++], k, k_t);
            forces[f[2]] += force;
            forces[f[0]] -= force;
        }

        // Apply forces to vertices.
        for (uint j = 0; j < vertices_.size(); j++) {
            vertices_[j] += dt * velocities_[j];
            velocities_[j] += dt * (gravity + forces[j]) / m;

            if (vertices_[j][2] < 0) {
                vertices_[j][2] = 0;
                velocities_[j][2] = 0;
            }
        }
    }

    bound get_bounds() {
        return bounds_;
    }

private:
    inline float dist(const Vector3f &v1, const Vector3f &v2) {
        return (v1 - v2).norm();
    }

    inline void initialize() {
//        // Calculate center.
//        Vector3f center;
//        for (const Vector3f &v: vertices_) {
//            center += v;
//        }
//        center /= vertices_.size();
//        // Calculate distances from center.
//        for (const Vector3f &v: vertices_) {
//            init_distances_.emplace_back((center - v).norm());
//        }
//        init_distances_.reserve(2 * vertices_.size()); // Optimistic that each vertex is in na simplex.
        init_distances_.reserve(faces_.size() * 3);
        uint i = 0;
        for (const fac &f : faces_) {
            init_distances_[i++] = dist(vertices_[f[0]], vertices_[f[1]]);
            init_distances_[i++] = dist(vertices_[f[1]], vertices_[f[2]]);
            init_distances_[i++] = dist(vertices_[f[2]], vertices_[f[0]]);
        }
        // Set variables.
        for (uint i = 0; i < vertices_.size(); i++) {
            velocities_.emplace_back(0, 0, 0);
        }
        vertices_[1][2] += 0.1;
    }

    inline void update_bound(double val, double &container, bool min) {
        if (min && val < container || !min && val > container)
            container = val;
    }

    bool parse(const char *cmd, char *args, ulong line) {
        if (!std::strcmp(cmd, "v")) {  // vertex
            float x, y, z;
            if (!sscanf(args, "%f %f %f", &x, &y, &z))
                throw std::runtime_error("Error reading vertex on line " + std::to_string(line));
            // Update bounds.
            update_bound(x, bounds_.minx, true);
            update_bound(x, bounds_.maxx, false);
            update_bound(y, bounds_.miny, true);
            update_bound(y, bounds_.maxy, false);
            update_bound(z, bounds_.minz, true);
            update_bound(z, bounds_.maxz, false);
            vertices_.emplace_back(x, y, z);
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
            fac face = {.vertices={v1 - 1, v2 - 1, v3 - 1}};
            faces_.push_back(face);
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
            } else if (!std::strcmp(cmd, "norm")) {
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
        obj.initialize();
        return obj;
    }
};


#endif //PROJEKT_OBJECT_HPP
