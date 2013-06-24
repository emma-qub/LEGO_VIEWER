#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ViewerWidget.h"
#include "LDrawParser.h"

#include <QtGui>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = NULL);

    void createToolBar(void);
    void createPartsDockWidget(void);
    void createCorrectDockWidget(void);
    void createColorComboBox(void);

    void disconnectToggle(void);
    void connectToggle(void);

    void setStyle(void);

public slots:
    void addFav(void);
    void browseParts(void);
    void changeColor(QString colorName);
    void otherOnLeftView(bool);
    void quadrantView(bool);
    void soloView(bool);
    void displayParts(QModelIndex);
    void renderPart(QModelIndex);
    void reloadPiece(void);
    void checkBowtie(void);
    //void bouh(bool);

private:
    ViewerWidget* _viewer;
    LDrawParser _parser;

    QString _currFile;
    QString _currPart;

    QAction* _favAction;
    QAction* _browseAction;
    QAction* _otherOnLeftAction;
    QAction* _quadrantAction;
    QAction* _soloAction;

    QComboBox* _colorComboBox;

    QWidget* _partsWidget;
    QDockWidget* _partsDockWidget;

    QWidget* _flawsWidget;
    QDockWidget* _flawsDockWidget;

    QGroupBox* _bowtieGroupBox;
    QGroupBox* _bowtieColorGroupBox;
    QLabel* _bowtieColorLabel;
    QPushButton* _bowtieColorButton;
    QCheckBox* _bowtieCorrecCheckBox;
    bool _bowtieColorWasChecked;
    bool _bowtieCorrecWasChecked;
    QLabel* _bowtieMatchFoundLabel;

    QGroupBox* _nonPlanarColorGroupBox;
    QLabel* _nonPlanarColorLabel;
    QPushButton* _nonPlanarColorButton;
    QCheckBox* _nonPlanarCorrecCheckBox;

    QStringListModel* _categoriesListModel;
    QListView* _categoriesListView;

    QStringListModel* _partsListModel;
    QListView* _partsListView;

    QStringList _favsList;

private:
    void fillFavsList(void);
    QStringList getPartsList(const QString& category) const;
    int getIndexOfFav(const QStringList& allFavsList, const QString& line) const;
    void centerOnScreen(void);

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void paintEvent(QPaintEvent* event);

signals:
    void bowtieChecked(void);
};

#endif // MAINWINDOW_H
