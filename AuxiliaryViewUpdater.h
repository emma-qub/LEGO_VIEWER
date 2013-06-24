#ifndef AUXILIARYVIEWUPDATER_H
#define AUXILIARYVIEWUPDATER_H

#include <osgGA/GUIEventHandler>
//#include <osgGA/GUIEventAdapter>
#include <osgViewer/View>

class AuxiliaryViewUpdater : public osgGA::GUIEventHandler {

public:
    AuxiliaryViewUpdater(void);

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

protected:
    double _distance;
    float _offsetX, _offsetY;
    float _lastDragX, _lastDragY;
};

#endif // AUXILIARYVIEWUPDATER_H
