#include "WindowButton.h"

#include <QPainter>
#include <QDebug>

WindowButton::WindowButton(ButtonType type, QWidget* parent) :
    QAbstractButton(parent),
    _type(type),
    _state(STATE_NORMAL),
    _normal(NULL),
    _hovered(NULL),
    _clicked(NULL) {

    setFixedWidth(30);
    qDebug() << "WindowButton::width:" << width();

}

WindowButton::~WindowButton(void) {
    delete _normal;
    delete _hovered;
    delete _clicked;
}

void WindowButton::resizeEvent(QResizeEvent* event) {
    Q_UNUSED(event);

    initPixmaps();
}

void WindowButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);

    if (isEnabled()) {
        switch (_state) {
        case STATE_NORMAL:
            if (_normal)
                painter.drawPixmap(0, 0, *_normal);
            break;
        case STATE_HOVERED:
            if (_hovered)
                painter.drawPixmap(0, 0, *_hovered);
            break;
        case STATE_CLICKED:
            if (_clicked)
                painter.drawPixmap(0, 0, *_clicked);
            break;
        }
    } else {
        if (_normal)
            painter.drawPixmap(0, 0, *_normal);
    }
}

void WindowButton::initPixmaps(void) {
    // Delete previous button
    initPixmap(&_normal);
    initPixmap(&_hovered);
    initPixmap(&_clicked);

    initClose();
}

void WindowButton::initPixmap(QPixmap** pixmap) {
    delete *pixmap;

    *pixmap = new QPixmap(size());

    (*pixmap)->fill(Qt::transparent);
}

void WindowButton::initClose(void) {
    // Border
    QPolygon border;
    border << QPoint(27, 26)
           << QPoint(12, 26)
           << QPoint(12, 11)
           << QPoint(27, 11);

    // Symbol
    QLine symbol1(QPoint(17, 16), QPoint(22, 21));
    QLine symbol2(QPoint(17, 21), QPoint(22, 16));

    // Create painter
    QPainter painter;

    // Normal
    painter.begin(_normal);

    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::black));

    painter.drawPolygon(border);

    painter.setPen(QPen(QBrush(Qt::white), 2.0));

    painter.drawLine(symbol1);
    painter.drawLine(symbol2);

    painter.end();

    // Hovered
    painter.begin(_hovered);

    painter.setPen(QPen(Qt::white));
    painter.setBrush(QBrush(Qt::darkGray));

    painter.drawPolygon(border);

    painter.setPen(QPen(QBrush(Qt::white), 2.0));

    painter.drawLine(symbol1);
    painter.drawLine(symbol2);

    painter.end();

    // Clicked
    painter.begin(_clicked);

    painter.setPen(QPen(Qt::black));
    painter.setBrush(QBrush(Qt::white));

    painter.drawPolygon(border);

    painter.setPen(QPen(QBrush(Qt::black), 2.0));

    painter.drawLine(symbol1);
    painter.drawLine(symbol2);

    painter.end();
}

void WindowButton::enterEvent(QEvent* event) {
    Q_UNUSED(event);

    _state = STATE_HOVERED;

    update();
}

void WindowButton::leaveEvent(QEvent* event) {
    Q_UNUSED(event);

    _state = STATE_NORMAL;

    update();
}

void WindowButton::mousePressEvent(QMouseEvent* event) {
    QAbstractButton::mousePressEvent(event);

    _state = STATE_CLICKED;

    update();
}

void WindowButton::mouseReleaseEvent(QMouseEvent* event) {
    QAbstractButton::mouseReleaseEvent(event);

    if (underMouse())
        _state = STATE_HOVERED;
    else
        _state = STATE_NORMAL;

    update();
}




