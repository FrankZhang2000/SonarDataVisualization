#ifndef UTILS
#define UTILS

#include <cmath>
#include <GL/glut.h>

#define square(x) ((x) * (x))

template<class T>
class TriplePair {
public:
    T x, y, z;
    TriplePair(T x_, T y_, T z_): x(x_), y(y_), z(z_) {}
    TriplePair() {}
};

class ObjectPoint {
public:
    GLfloat x, y, z, refl, r;
    ObjectPoint(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat _refl, GLfloat _r): 
        x(x_), y(y_), z(z_), refl(_refl), r(_r) {}
    ObjectPoint() {}
};

class Point {
public:
    GLfloat x, y, z, color;
    GLfloat r;
    bool is_origin;
    int group_id;
    Point(GLfloat x_, GLfloat y_, GLfloat z_, GLfloat color_, GLfloat r_, bool _origin): 
            x(x_), y(y_), z(z_), color(color_), r(r_), is_origin(_origin) {
        group_id = 0;
    }
    bool operator<(const Point & p) const {
        return (p.color - color >= 1e-6);
    }
    bool operator==(const Point & p) const {
        return (fabs(p.color - color) < 1e-6);
    }
    operator ObjectPoint() {
        return ObjectPoint(x, y, z, color, r);
    }
};

#endif