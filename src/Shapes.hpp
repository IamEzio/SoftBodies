#ifndef PROJEKT_SHAPES_HPP
#define PROJEKT_SHAPES_HPP


#include <algorithm>
#include <cfloat>
#include <Eigen/Dense>

namespace shapes {

    // bound contains the bounding cube of the object
    // functions to check if something lies in the bounding cube of the object
    struct bound {
        double minx, maxx, miny, maxy, minz, maxz;

        bound() {
            minx = miny = minz = DBL_MAX;
            maxx = maxy = maxz = -DBL_MAX;
        }

        // returns true if point (x,y,z) lies inside bounding cube
        bool inside(double x, double y, double z) const {
            return x >= minx && x <= maxx && y >= miny && y <= maxy && z >= minz && z <= maxz;
        }

        // returns true if Eigen::Vector3d lies inside bounding cube
        bool inside(const Eigen::Vector3d &v) const {
            return v[0] >= minx && v[0] <= maxx && v[1] >= miny && v[1] <= maxy && v[2] >= minz && v[2] <= maxz;
        }

        // returns true if any corner of bounding cube b lies inside this bounding cube
        bool inside(const bound &b) {
            return inside(b.minx, b.miny, b.minz)
                   || inside(b.minx, b.miny, b.maxz)
                   || inside(b.minx, b.maxy, b.minz)
                   || inside(b.minx, b.maxy, b.maxz)
                   || inside(b.maxx, b.miny, b.minz)
                   || inside(b.maxx, b.miny, b.maxz)
                   || inside(b.maxx, b.maxy, b.minz)
                   || inside(b.maxx, b.maxy, b.maxz);

        }
    };

    struct Shape {
    };

    struct Cube : public Shape {
        bound bounds;

        Cube(bound b) : bounds(b) {}
    };


    static bool isCollision(Cube &c1, Cube &c2) {
        return c1.bounds.inside(c2.bounds);
    }

};


#endif //PROJEKT_SHAPES_HPP
