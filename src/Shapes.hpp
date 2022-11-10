#ifndef PROJEKT_SHAPES_HPP
#define PROJEKT_SHAPES_HPP


#include <algorithm>
#include <cfloat>
#include <Eigen/Dense>

namespace shapes {
//    enum CollisionType {
//        NONE, xyz, xyZ, xYz, xYZ, Xyz, XyZ, XYz, XYZ
//    };

    struct bound {
        double minx, maxx, miny, maxy, minz, maxz;

        bound() {
            minx = miny = minz = DBL_MAX;
            maxx = maxy = maxz = -DBL_MAX;
        }

        bool inside(double x, double y, double z) const {
            return x >= minx && x <= maxx && y >= miny && y <= maxy && z >= minz && z <= maxz;
        }

        bool inside(const Eigen::Vector3d &v) const {
            return v[0] >= minx && v[0] <= maxx && v[1] >= miny && v[1] <= maxy && v[2] >= minz && v[2] <= maxz;
        }

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

//    struct Orb : public Shape {
//        double radius;
//        Eigen::Vector3d center;
//
//        Orb(double radius, Eigen::Vector3d center) : radius(radius), center(center) {}
//    };

    static bool isCollision(Cube &c1, Cube &c2) {
        return c1.bounds.inside(c2.bounds);
    }

//    static bool isCollision(Orb &o1, Orb &o2) {
//        return (o1.center - o2.center).norm() < std::min(o1.radius, o2.radius);
//    }
//
//    static bool isCollision(Orb &o, Cube &c) {
//        const bound &b = c.bounds;
//        Eigen::Vector3d v;
//        v = {b.minx, b.miny, b.minz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.minx, b.miny, b.maxz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.minx, b.maxy, b.minz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.minx, b.maxy, b.maxz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.maxx, b.miny, b.minz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.maxx, b.miny, b.maxz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.maxx, b.maxy, b.minz};
//        if ((o.center - v).norm() < o.radius) return true;
//        v = {b.maxx, b.maxy, b.maxz};
//        if ((o.center - v).norm() < o.radius) return true;
//        return true;
//    }
//
//    static bool isCollision(Cube &c, Orb &o) {
//        return isCollision(o, c);
//    }
};


#endif //PROJEKT_SHAPES_HPP
