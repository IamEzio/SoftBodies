//
// Created by lirfu on 04.10.18..
//

#ifndef LAB_RACGRA_DRAWABLE_H
#define LAB_RACGRA_DRAWABLE_H


#include <vector>
#include <sstream>
#include "Utils.h"
#include <eigen3/Dense>
#include <cfloat>

class Drawable {
public:
    Eigen::Vector3f normal_;
    Eigen::Vector3f lookup_;
    std::vector<Eigen::Vector3f> vertices_;

    Drawable() : normal_({1, 0, 0}), lookup_({0, 0, 1}) {}

    Eigen::Vector3f get_normal() const {
        return normal_;
    }

    void set_normal(const Eigen::Vector3f &v) {
        normal_ = v;
    }

    void set_lookup(const Eigen::Vector3f &v) {
        lookup_ = v;
    }

    void center() {
        Eigen::Vector3f center = {0, 0, 0};
        for (Eigen::Vector3f &p:vertices_)
            center += p;
        center *= 1. / vertices_.size();
        for (Eigen::Vector3f &p:vertices_)
            p -= center;
    }

    void normalize() { //TODO Optional: constraint object to a [-1,1] box.
        Eigen::Vector3f min = {FLT_MAX, FLT_MAX, FLT_MAX};
        Eigen::Vector3f max = {FLT_MIN, FLT_MIN, FLT_MIN};
        for (const Eigen::Vector3f &p:vertices_) {
            for (int i = 0; i < 3; i++) {
                if (p[i] < min[i]) min[i] = p[i];
                if (p[i] > max[i]) max[i] = p[i];
            }
        }
        for (Eigen::Vector3f &p:vertices_) {
            for (int i = 0; i < 3; i++) {
                p[i] = (p[i] - min[i]) / (max[i] - min[i]);
            }
        }
    }

    std::string to_obj() {
        std::stringstream str;
        return str.str();
    }

    static double calculate_axis_rotation(Eigen::Vector3f start, Eigen::Vector3f target, Eigen::Vector3f &axis) {
        start.normalize();
        target.normalize();
        axis = start.cross(target);
        return acos(start.dot(target)) * 180 / M_PI;
    }

    static Eigen::Matrix<double, 4, 4> calulate_DCM_rotation(const Eigen::Vector3f &obj_normal,
                                                             const Eigen::Vector3f &obj_lookup) {
        Eigen::Vector3f y = obj_lookup.cross(obj_normal);
        Eigen::Matrix<double, 4, 4, Eigen::RowMajor> R;
        R << obj_normal[0], y[0], obj_lookup[0], 0,
                obj_normal[1], y[1], obj_lookup[1], 0,
                obj_normal[2], y[2], obj_lookup[2], 0,
                0, 0, 0, 1;
        R = R.inverse().eval();
        return R;
    }
};


#endif //LAB_RACGRA_DRAWABLE_H