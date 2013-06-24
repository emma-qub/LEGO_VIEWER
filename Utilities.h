#ifndef UTILITIES_H
#define UTILITIES_H

#include <osg/Camera>

class Utilities {

public:
    Utilities(void) {}

    static osg::Camera* createHUDCamera(double left, double right, double bottom, double top);
};

#endif // UTILITIES_H
