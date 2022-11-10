#ifndef PROJEKT_PHYSICS_HPP
#define PROJEKT_PHYSICS_HPP

#include <Eigen/Dense>
#include "Object.hpp"
// #include "Eigen/Dense"

using namespace Eigen;

namespace physics {
    static uint refresh_ms = 20u;
    static uint euler_iterations = 10;

    static Vector3d g = {0, 0, -1};

    static double spring_k = 40;
    static double spring_kt = 3;

    static double ground_rebound_speed_coef = 0.9;
    static double object_rebound_coef = 1e-6;
};


#endif //PROJEKT_PHYSICS_HPP
