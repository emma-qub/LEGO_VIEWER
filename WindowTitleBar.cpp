#include "WindowTitleBar.h"

#include <QPainter>
#include <QDebug>

WindowTitleBar::WindowTitleBar(QWidget* parent):
    QWidget(parent) {

    _title = new QLabel("LViewer: browse your bricks", this);
    _title->setStyleSheet("color: white; font-family: 'Visitor TT1 BRK'; font-weight: bold; font-size: 14px");
    _title->move  (width() - 35,  4);
    _title->resize(width() - 116, 29);

    qDebug() << "WindowTitleBar::width:" << width();

    // Init cache
    _cache = NULL;

    // Init close button
    _closeButton = new WindowButton(WindowButton::BUTTON_CLOSE, this);

    // Connect
    connect(_closeButton, SIGNAL(clicked()), this, SLOT(quit()));

    // Set fixed height
    setFixedHeight(70);
}

WindowTitleBar::~WindowTitleBar(void) {
}

void WindowTitleBar::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    // Remove old cache
    delete _cache;

    // Create a cache with same size as the widget
    _cache = new QPixmap(size());

    // Create a transparent background
    _cache->fill(Qt::transparent);

    // Start painting the cache
    QPainter painter(_cache);

    QColor lightBlue    (177, 177, 203, 255);
    QColor gradientStart(  0,   0,   0,   0);
    QColor gradientEnd  (  0,   0,   0, 220);

    QLinearGradient linearGrad(QPointF(0, 0), QPointF(0, height()));
    linearGrad.setColorAt(0, gradientStart);
    linearGrad.setColorAt(1, gradientEnd);

    QLinearGradient leftGrad(QPointF(width()-75, 0), QPointF(width()-5, 0));
    leftGrad.setColorAt(   0, gradientStart);
    leftGrad.setColorAt(0.66,     Qt::white);
    leftGrad.setColorAt(   1,     Qt::white);

/*********************
 * Title bar's frame *
 *********************/
    QPolygon frame;

    frame << QPoint(           5,  5)
          << QPoint(width() - 26,  5)
          << QPoint(width() - 26, 12)
          << QPoint(width() - 19, 12)
          << QPoint(width() - 19, 19)
          << QPoint(width() - 12, 19)
          << QPoint(width() - 12, 26)
          << QPoint(width() -  5, 26)
          << QPoint(width() -  5, 33)
          << QPoint(           5, 33);

    painter.setPen  (QPen  (lightBlue ));
    painter.setBrush(QBrush(linearGrad));

    painter.drawPolygon(frame);

/******************************
 * Title bar's little squares *
 ******************************/
    int x0 = width() - 5, x1 = width() - 12, x2 = width() - 19;
    int y0 =  5, y1 = 12, y2 = 19;

    QPolygon square1;
    square1 << QPoint(x0, y0)
            << QPoint(x0, y0+1)
            << QPoint(x0-1, y0+1)
            << QPoint(x0-1, y0);

    QPolygon square2;
    square2 << QPoint(x0, y1)
            << QPoint(x0, y1+3)
            << QPoint(x0-3, y1+3)
            << QPoint(x0-3, y1);

    QPolygon square3;
    square3 << QPoint(x1, y0)
            << QPoint(x1, y0+3)
            << QPoint(x1-3, y0+3)
            << QPoint(x1-3, y0);

    QPolygon square4;
    square4 << QPoint(x0, y2)
            << QPoint(x0, y2+5)
            << QPoint(x0-5, y2+5)
            << QPoint(x0-5, y2);

    QPolygon square5;
    square5 << QPoint(x1, y1)
            << QPoint(x1, y1+5)
            << QPoint(x1-5, y1+5)
            << QPoint(x1-5, y1);

    QPolygon square6;
    square6 << QPoint(x2, y0)
            << QPoint(x2, y0+5)
            << QPoint(x2-5, y0+5)
            << QPoint(x2-5, y0);

    painter.setPen  (QPen  (lightBlue));
    painter.setBrush(QBrush(Qt::white));

    painter.drawPolygon(square1);
    painter.drawPolygon(square2);
    painter.drawPolygon(square3);
    painter.drawPolygon(square4);
    painter.drawPolygon(square5);
    painter.drawPolygon(square6);

///*****************************
// * Title bar's left gradient *
// *****************************/
    QPolygon left;

    left << QPoint(width() - 75,  5)
         << QPoint(width() - 26,  5)
         << QPoint(width() - 26, 12)
         << QPoint(width() - 19, 12)
         << QPoint(width() - 19, 19)
         << QPoint(width() - 12, 19)
         << QPoint(width() - 12, 26)
         << QPoint(width() -  5, 26)
         << QPoint(width() -  5, 33)
         << QPoint(width() - 75, 33);

    painter.setPen  (QPen  (gradientStart));
    painter.setBrush(QBrush(leftGrad     ));

    painter.drawPolygon(left);
}

void WindowTitleBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    if (_cache) {
        QPainter painter(this);
        painter.drawPixmap(0, 0, *_cache);
    }
}

void WindowTitleBar::quit(void) {
    qApp->quit();
}
