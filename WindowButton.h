#ifndef WINDOWBUTTON_H
#define WINDOWBUTTON_H

#include <QAbstractButton>
#include <QPixmap>

#include "WindowButton.h"

class WindowButton : public QAbstractButton {
    Q_OBJECT

public:
    enum ButtonType { BUTTON_CLOSE };

    WindowButton(ButtonType type, QWidget* parent = NULL);
    virtual ~WindowButton(void);

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);

    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    enum ButtonState { STATE_NORMAL, STATE_HOVERED, STATE_CLICKED };

    ButtonType _type;
    ButtonState _state;
    QPixmap* _normal;
    QPixmap* _hovered;
    QPixmap* _clicked;

    void initPixmaps(void);
    void initPixmap(QPixmap** pixmap);
    void initClose(void);
};

#endif // WINDOWBUTTON_H
