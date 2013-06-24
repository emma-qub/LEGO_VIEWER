#ifndef WINDOWTITLEBAR_H
#define WINDOWTITLEBAR_H

#include <QWidget>
#include <QPixmap>
#include <QLabel>
#include <QtGui/QApplication>

#include "WindowButton.h"

class WindowTitleBar : public QWidget {
    Q_OBJECT

public:
    WindowTitleBar(QWidget* parent = NULL);
    virtual ~WindowTitleBar(void);

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);

public slots:
    void quit(void);

private:
    QPixmap* _cache;
    QLabel* _title;
    WindowButton* _closeButton;
};

#endif // WINDOWTITLEBAR_H
