#include "ViewerWidget.h"
#include "AuxiliaryViewUpdater.h"
#include "Utilities.h"

#include <QDebug>

ViewerWidget::ViewerWidget(osgViewer::ViewerBase::ThreadingModel threadingModel) :
    QWidget() {

    // Create main layout
    //_mainLayout = new QVBoxLayout;
    // No space between window's element and the border
    //_mainLayout->setMargin(0);
    // No space between window's element


    // Create window title bar
    //_windowTitleBar = new WindowTitleBar;
    // Add window title bar to main layout
    //_mainLayout->addWidget(_windowTitleBar);
    // Put the title bar at the top of the window


    _scene = new osg::Group;
    _viewTopLookDir = osg::Y_AXIS;
    _viewTopUp = osg::Z_AXIS;

    _viewLeftLookDir = osg::Z_AXIS;
    _viewLeftUp = -osg::Y_AXIS;

    _viewFrontLookDir = osg::X_AXIS;
    _viewFrontUp = -osg::Y_AXIS;

    _viewMainLookDir = osg::Y_AXIS;
    _viewMainUp = osg::X_AXIS;

    setThreadingModel(threadingModel);

    _widgetTop   = addViewWidget(createCamera(0,0,10,10), Top  );
    _widgetLeft  = addViewWidget(createCamera(0,0,10,10), Left );
    _widgetFront = addViewWidget(createCamera(0,0,10,10), Front);
    _widgetMain  = addViewWidget(createCamera(0,0,10,10), Main );

    _leftVBoxLayout = new QVBoxLayout;
    _rightVBoxLayout = new QVBoxLayout;

    _mainHBoxLayout = new QHBoxLayout;
    _mainHBoxLayout->addLayout(_leftVBoxLayout);
    _mainHBoxLayout->addLayout(_rightVBoxLayout);

    //_mainLayout->addLayout(_mainHBoxLayout);

    changeViews();

    //setLayout(_mainLayout);
    setLayout(_mainHBoxLayout);

    connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    _timer.start(10);
}

void ViewerWidget::changeViews(ViewFormat viewFormat) {

    QLayoutItem* child;
    while ((child = _leftVBoxLayout->takeAt(0)) != NULL)
        delete child;

    while ((child = _rightVBoxLayout->takeAt(0)) != NULL)
        delete child;

    int w = 300;

    switch (viewFormat) {
    case OthersOnLeft:
        _widgetTop->setFixedWidth(w);
        _widgetFront->setFixedWidth(w);
        _widgetLeft->setFixedWidth(w);
        _widgetMain->setFixedWidth(1100);

        _leftVBoxLayout->addWidget(_widgetTop);
        _leftVBoxLayout->addWidget(_widgetFront);
        _leftVBoxLayout->addWidget(_widgetLeft);
        _rightVBoxLayout->addWidget(_widgetMain);
        break;
    case Quadrant:
        _widgetTop->setFixedWidth(700);
        _widgetFront->setFixedWidth(700);
        _widgetLeft->setFixedWidth(700);
        _widgetMain->setFixedWidth(700);

        _leftVBoxLayout->addWidget(_widgetTop);
        _leftVBoxLayout->addWidget(_widgetFront);
        _rightVBoxLayout->addWidget(_widgetLeft);
        _rightVBoxLayout->addWidget(_widgetMain);
        break;
    case Solo:
        _widgetMain->setFixedWidth(1400);

        _leftVBoxLayout->addWidget(_widgetMain);
        break;
    }
}

QWidget* ViewerWidget::addViewWidget(osg::Camera* camera, ViewType viewType) {

    osgViewer::View* view = new osgViewer::View;
    view->setCamera(camera);
    addView(view);

    view->setSceneData(_scene);

    osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>(camera->getGraphicsContext());

//    osg::Camera* HUDCamera = Utilities::createHUDCamera(0, 1440, 0, 900);
//    HUDCamera->setGraphicsContext(gw);
//    HUDCamera->setViewport(0, 0, gw->getTraits()->width, gw->getTraits()->height);
//    HUDCamera->setClearColor(osg::Vec4(0.1, 0.7, 0.5, 1.0));

//    view->addSlave(HUDCamera);

    switch (viewType) {
    case Top:
        view->addEventHandler(new AuxiliaryViewUpdater);
        _viewTop = view;
        break;
    case Left:
        view->addEventHandler(new AuxiliaryViewUpdater);
        _viewLeft = view;
        break;
    case Front:
        view->addEventHandler(new AuxiliaryViewUpdater);
        _viewFront = view;
        break;
    case Main:
        view->addEventHandler(new osgViewer::StatsHandler);
        _viewMain = view;
        break;
    }

    return gw ? gw->getGLWidget() : NULL;
}

osg::Camera* ViewerWidget::createCamera(int x, int y, int w, int h, const std::string& name, bool windowDecoration) {
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->windowName = name;
    traits->windowDecoration = windowDecoration;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );

    //camera->setClearColor(osg::Vec4(252.0/255.0, 254.0/255.0 ,234.0/255.0, 1.0));
    camera->setClearColor(osg::Vec4(50.0/255.0, 50.0/255.0 ,50.0/255.0, 1.0));
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height));
    camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f);
    return camera.release();
}

void ViewerWidget::changePiece(osg::Group* piece, bool reinitCamera) {
    _scene->removeChildren(0, _scene->getNumChildren());
    _scene->addChild(piece);

    if (reinitCamera) {
        osg::Vec3 center = piece->getBound().center();
        double radius = piece->getBound().radius();

        osg::Vec3 eyeTop   = center - _viewTopLookDir  *(radius*2.0);
        osg::Vec3 eyeLeft  = center - _viewLeftLookDir *(radius*2.0);
        osg::Vec3 eyeFront = center - _viewFrontLookDir*(radius*2.0);
        osg::Vec3 eyeMain  = center - _viewMainLookDir *(radius*4.0);

        _viewMain->setCameraManipulator(new osgGA::TrackballManipulator);

        _viewTop->getCamera()->setViewMatrixAsLookAt(  eyeTop,   center, _viewTopUp  );
        _viewLeft->getCamera()->setViewMatrixAsLookAt( eyeLeft,  center, _viewLeftUp );
        _viewFront->getCamera()->setViewMatrixAsLookAt(eyeFront, center, _viewFrontUp);
        _viewMain->getCamera()->setViewMatrixAsLookAt( eyeMain,  center, _viewMainUp );
    }
}
