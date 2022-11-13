#ifndef PROJECT_PHYSICS_HPP
#define PROJECT_PHYSICS_HPP

#include <Eigen/Dense>
#include "Object.hpp"
// #include "Eigen/Dense"

// HEADER FILE CONTAINS VARIABLES LOADED FROM CONFIG FILE

using namespace Eigen;

namespace physics {
    static uint refresh_ms = 20u; //refresh rate in ms
    static uint euler_iterations = 10; //no of euler integrations between draws

    static Vector3d g = {0, 0, -1}; //gravity vector

    static double spring_k = 40; //spring constant
    static double spring_kt = 3; //damping constant

    static double ground_rebound_speed_coef = 0.9; //for collisions with ground
    static double object_rebound_coef = 1e-6; //for inter object collisions
};


#endif //PROJECT_PHYSICS_HPP
