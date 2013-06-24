#include "Utilities.h"

osg::Camera* Utilities::createHUDCamera(double left, double right, double bottom, double top) {
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);
    camera->setAllowEventFocus(false);
    camera->setProjectionMatrix(osg::Matrix::ortho2D(left, right, bottom, top));
    camera->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    return camera.release();
}
