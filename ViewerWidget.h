#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QGridLayout>

#include <osgViewer/CompositeViewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osgDB/ReadFile>
#include <osgQt/GraphicsWindowQt>

#include "WindowTitleBar.h"

#include <iostream>

class ViewerWidget : public QWidget, public osgViewer::CompositeViewer {

public:
    enum ViewType { Top, Left, Front, Main };
    enum ViewFormat { OthersOnLeft, Quadrant, Solo };

public:
    ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel = osgViewer::CompositeViewer::SingleThreaded);

    QWidget* addViewWidget(osg::Camera* camera, ViewType viewType);
    osg::Camera* createCamera(int x, int y, int w, int h, const std::string& name = "", bool windowDecoration = false);

    void changeViews(ViewFormat viewFormat = OthersOnLeft);
    void changePiece(osg::Group* piece, bool reinitCamera = true);

    virtual void paintEvent(QPaintEvent*) { frame(); }

protected:
    QTimer _timer;
    osg::ref_ptr<osg::Group> _scene;

    osgViewer::View* _viewTop,* _viewLeft,* _viewFront,* _viewMain;

    osg::Vec3 _viewTopLookDir, _viewLeftLookDir, _viewFrontLookDir, _viewMainLookDir;
    osg::Vec3 _viewTopUp,      _viewLeftUp,      _viewFrontUp,      _viewMainUp;

    QWidget* _widgetTop,* _widgetLeft,* _widgetFront,* _widgetMain;

    QVBoxLayout* _leftVBoxLayout;
    QVBoxLayout* _rightVBoxLayout;
    QHBoxLayout* _mainHBoxLayout;

    QVBoxLayout* _mainLayout;
    WindowTitleBar* _windowTitleBar;
    WindowButton* _close;
};
