#include "MainWindow.h"

#include <QDesktopWidget>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    _parser(),
    _currFile(),
    _currPart(),
    _bowtieColorWasChecked(false),
    _bowtieCorrecWasChecked(false) {

    // Remove frame, meaning title and close button
    //setWindowFlags(Qt::FramelessWindowHint);

    // Create viewer
    _viewer = new ViewerWidget;
    // Set central widget
    setCentralWidget(_viewer);

    //_viewer->changePiece(_parser.createNode("/home/shaolan/Documents/ldraw/parts/81599.dat"));

    // Fill favs list, according to recorded information within Favourites.ldr file
    fillFavsList();

    // Create tool bar
    createToolBar();
    // Create dock widget to browse parts
    createPartsDockWidget();
    // Create dock widget to handle corrections
    createCorrectDockWidget();

    // Set style
    //setStyle();

    // Maximize window
    setWindowState(Qt::WindowMaximized);

    // Set title
    setWindowTitle("LViewer: browse your bricks");

//    // Hide dock widget
//    _partsDockWidget->hide();

}

void MainWindow::createToolBar(void) {
    // Fav Action
    _favAction = new QAction(QIcon("../LViewer/icons/notFavIco.png"), "Add part to favs", this);
    // Add tool tip to explain
    _favAction->setToolTip("Add to fav in order to find it easier next time.");
    // Connect
    connect(_favAction, SIGNAL(triggered(void)), this, SLOT(addFav(void)));
    // Disable fav button, because there is no file loaded when starting the viewer
    _favAction->setEnabled(false);

    // Browse Action
    _browseAction = new QAction(QIcon("../LViewer/icons/browseIco.png"), "Browse parts", this);
    // Add tool tip to explain
    _browseAction->setToolTip("Browse parts.");
    // Connect
    connect(_browseAction, SIGNAL(triggered(void)), this, SLOT(browseParts(void)));

    // Create color combo box
    createColorComboBox();
    // Connect
    connect(_colorComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(changeColor(QString)));

    // Other On Left Action
    _otherOnLeftAction = new QAction(QIcon("../LViewer/icons/otherOnLeftIco.png"), "View 1", this);
    // Add tool tip to explain
    _otherOnLeftAction->setToolTip("Switch view mode: three views are on the left.");
    // Set button to checkable
    _otherOnLeftAction->setCheckable(true);
    // Check this view mode
    _otherOnLeftAction->setChecked(true);

    // Fav Action
    _quadrantAction = new QAction(QIcon("../LViewer/icons/quadrantIco.png"), "View 2", this);
    // Add tool tip to explain
    _quadrantAction->setToolTip("Switch view mode: four views are splitted equaly.");
    // Set button to checkable
    _quadrantAction->setCheckable(true);

    // Fav Action
    _soloAction = new QAction(QIcon("../LViewer/icons/soloIco.png"), "View 3", this);
    // Add tool tip to explain
    _soloAction->setToolTip("Switch view mode: only the main view is available.");
    // Set button to checkable
    _soloAction->setCheckable(true);

    // Connect
    connectToggle();

    // Create classic tool bar, with new, open, save...
    QToolBar* toolBar = addToolBar("ToolBar");
    toolBar->addAction(_favAction);
    toolBar->addAction(_browseAction);
    toolBar->addWidget(_colorComboBox);

    // Create spacer widget to separate these two tool bars
    QWidget* spacerWidget = new QWidget(this);
    spacerWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    spacerWidget->setVisible(true);
    toolBar->addWidget(spacerWidget);

    toolBar->addAction(_otherOnLeftAction);
    toolBar->addAction(_quadrantAction);
    toolBar->addAction(_soloAction);
    // Set tool bar object name to enable access when set style
    toolBar->setObjectName("ToolBar");
    // This tool bar cannot be moved, resized and so on
    toolBar->setMovable(false);
}

QStringList MainWindow::getPartsList(const QString& category) const {
    // Try to open category parts text file in read only mode
    QFile file("/home/shaolan/Documents/Qt/LViewer/config/"+category+".ldr");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening "+category+".ldr file within MainWindow::getPartsList";
    }

    // Stream text and encode in UTF-8
    QTextStream inFile(&file);
    inFile.setCodec("UTF-8");

    // Create parts list
    QStringList partsList;

    // Read line per line
    while (!inFile.atEnd()) {
        // Get next line and split it according to key words
        QString currLine = inFile.readLine();

        // If line is not empty
        if (!currLine.isEmpty()) {
            // Get cyrrent file name
            QString currFileName = currLine.split("0 !FILENAME").at(1).split("!DESCRIPTION").at(0).split(".dat").at(0).simplified();

            // Get current description
            QString currDescription = currLine.split("!DESCRIPTION").at(1).simplified();

            // Create blank
            QString blank = "";
            for (int k = 0; k < 20-currFileName.length(); k++)
                blank += " ";

            // Add curr part info to parts list
            partsList << currFileName + "\t" + currDescription;
        }
    }

    // Close file
    file.close();

    return partsList;
}

void MainWindow::createPartsDockWidget(void) {
    // Create model from categories
    _categoriesListModel = new QStringListModel(_parser.getCategoriesArray());

    // Create view
    _categoriesListView = new QListView;

    // Resize
    _categoriesListView->setFixedWidth(162);

    // Associate model to view
    _categoriesListView->setModel(_categoriesListModel);

    // Set current index to first one in list, to avoid seg fault when scrolling without choosing anything first
    _categoriesListView->setCurrentIndex(_categoriesListView->indexAt(QPoint(0, 0)));

    // Set alternating row colors within categories list
    _categoriesListView->setAlternatingRowColors(true);

    // Connect
    connect(_categoriesListView, SIGNAL(activated(QModelIndex)), this, SLOT(displayParts(QModelIndex)));
    connect(_categoriesListView, SIGNAL(clicked(QModelIndex)), this, SLOT(displayParts(QModelIndex)));
    connect(_categoriesListView, SIGNAL(entered(QModelIndex)), this, SLOT(displayParts(QModelIndex)));
    connect(_categoriesListView, SIGNAL(pressed(QModelIndex)), this, SLOT(displayParts(QModelIndex)));


    // Create model from categories
    _partsListModel = new QStringListModel;

    // Create view
    _partsListView = new QListView;

    // Resize
    _partsListView->setMinimumSize(238, 400);

    // Associate model to view
    _partsListView->setModel(_partsListModel);

    // Connect
    connect(_partsListView, SIGNAL(activated(QModelIndex)), this, SLOT(renderPart(QModelIndex)));
    connect(_partsListView, SIGNAL(clicked(QModelIndex)), this, SLOT(renderPart(QModelIndex)));
    connect(_partsListView, SIGNAL(entered(QModelIndex)), this, SLOT(renderPart(QModelIndex)));
    connect(_partsListView, SIGNAL(pressed(QModelIndex)), this, SLOT(renderPart(QModelIndex)));


    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(_categoriesListView);
    mainLayout->addWidget(_partsListView);
    mainLayout->setAlignment(Qt::AlignTop);

    _partsWidget = new QWidget(this);
    _partsWidget->setLayout(mainLayout);

    _partsDockWidget = new QDockWidget("Parts", this);
    addDockWidget(Qt::RightDockWidgetArea, _partsDockWidget);
    _partsDockWidget->setWidget(_partsWidget);
    _partsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    _partsDockWidget->setFloating(true);
    _partsDockWidget->move(1000, 200);
    _partsDockWidget->setWindowOpacity(0.80);

    //connect(_partsDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(bouh(bool)));

    _partsDockWidget->setFixedWidth(450);
}

void MainWindow::createCorrectDockWidget(void) {

    // Create bowtie quads group box
    _bowtieGroupBox = new QGroupBox("Bowtie quads");
    _bowtieGroupBox->setCheckable(true);
    _bowtieGroupBox->setChecked(false);

    _bowtieColorGroupBox = new QGroupBox("Color");
    _bowtieColorGroupBox->setCheckable(true);
    _bowtieColorGroupBox->setChecked(false);
    _bowtieColorLabel = new QLabel;
    QPixmap bowtiePixmap(50, 20);
    bowtiePixmap.fill(QColor("#FA6900"));
    _bowtieColorLabel->setPixmap(bowtiePixmap);
    _bowtieColorButton = new QPushButton("Change");

    _bowtieCorrecCheckBox = new QCheckBox;

    QHBoxLayout* bowtieColorLayout = new QHBoxLayout;
    bowtieColorLayout->addWidget(_bowtieColorLabel);
    bowtieColorLayout->addWidget(_bowtieColorButton);
    _bowtieColorGroupBox->setLayout(bowtieColorLayout);

    QFormLayout* bowtieCorrectionLayout = new QFormLayout;
    bowtieCorrectionLayout->addRow("Correction", _bowtieCorrecCheckBox);

    QHBoxLayout* bowtieMatchFoundLayout = new QHBoxLayout;
    _bowtieMatchFoundLabel = new QLabel(QString::number(_parser._nbBowtieQuads));
    QLabel* bowtieTextLabel = new QLabel("match(s) found.");
    bowtieMatchFoundLayout->addWidget(_bowtieMatchFoundLabel);
    bowtieMatchFoundLayout->addWidget(bowtieTextLabel);
    bowtieMatchFoundLayout->setAlignment(Qt::AlignLeft);

    QVBoxLayout* bowtieLayout = new QVBoxLayout;
    bowtieLayout->addWidget(_bowtieColorGroupBox);
    bowtieLayout->addLayout(bowtieCorrectionLayout);
    bowtieLayout->addLayout(bowtieMatchFoundLayout);

    _bowtieGroupBox->setLayout(bowtieLayout);

    connect(_bowtieCorrecCheckBox, SIGNAL(clicked()), this, SLOT(reloadPiece()));
    connect(_bowtieColorGroupBox, SIGNAL(clicked()), this, SLOT(reloadPiece()));
    connect(_bowtieGroupBox, SIGNAL(clicked()), this, SLOT(checkBowtie()));
    connect(this, SIGNAL(bowtieChecked()), this, SLOT(reloadPiece()));

    // Create non planar quads group box
    QGroupBox* nonPlanarGroupBox = new QGroupBox("Non planar quads");
    nonPlanarGroupBox->setCheckable(true);
    nonPlanarGroupBox->setChecked(false);

    _nonPlanarColorGroupBox = new QGroupBox("Color");
    _nonPlanarColorGroupBox->setCheckable(true);
    _nonPlanarColorGroupBox->setChecked(true);
    _nonPlanarColorLabel = new QLabel;
    QPixmap nonPlanarPixmap(50, 20);
    nonPlanarPixmap.fill(QColor("#26ADE4"));
    _nonPlanarColorLabel->setPixmap(nonPlanarPixmap);
    _nonPlanarColorButton = new QPushButton("Change");

    _nonPlanarCorrecCheckBox = new QCheckBox;

    QHBoxLayout* nonPlanarColorLayout = new QHBoxLayout;
    nonPlanarColorLayout->addWidget(_nonPlanarColorLabel);
    nonPlanarColorLayout->addWidget(_nonPlanarColorButton);
    _nonPlanarColorGroupBox->setLayout(nonPlanarColorLayout);

    QFormLayout* nonPlanarCorrectionLayout = new QFormLayout;
    nonPlanarCorrectionLayout->addRow("Correction", _nonPlanarCorrecCheckBox);

    QVBoxLayout* nonPlanarLayout = new QVBoxLayout;
    nonPlanarLayout->addWidget(_nonPlanarColorGroupBox);
    nonPlanarLayout->addLayout(nonPlanarCorrectionLayout);

    nonPlanarGroupBox->setLayout(nonPlanarLayout);

    // Main components
    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(_bowtieGroupBox);
    mainLayout->addWidget(nonPlanarGroupBox);


    _flawsWidget = new QWidget;
    _flawsWidget->setLayout(mainLayout);

    _flawsDockWidget = new QDockWidget("Flaws", this);
    addDockWidget(Qt::RightDockWidgetArea, _flawsDockWidget);
    _flawsDockWidget->setWidget(_flawsWidget);
    _flawsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea);
    _flawsDockWidget->setFloating(true);
    _flawsDockWidget->move(300, 200);
    _flawsDockWidget->setWindowOpacity(0.80);
}

//void MainWindow::bouh(bool) {

//    _partsWidget->setVisible(!_partsWidget->isVisible());
//    disconnect(_partsDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(bouh(bool)));
//    _partsDockWidget->setVisible(true);
//    connect(_partsDockWidget, SIGNAL(visibilityChanged(bool)), this, SLOT(bouh(bool)));
//}

void MainWindow::checkBowtie(void) {

//    disconnect(_bowtieColorGroupBox, SIGNAL(clicked()));
//    disconnect(_bowtieCorrecCheckBox, SIGNAL(clicked()));

    if (!_bowtieGroupBox->isChecked()) {
        _bowtieColorWasChecked = _bowtieColorGroupBox->isChecked();
        _bowtieCorrecWasChecked = _bowtieCorrecCheckBox->isChecked();

        _bowtieColorGroupBox->setChecked(false);
        _bowtieCorrecCheckBox->setChecked(false);
    } else {
        _bowtieColorGroupBox->setChecked(_bowtieColorWasChecked);
        _bowtieCorrecCheckBox->setChecked(_bowtieCorrecWasChecked);
    }

//    connect(_bowtieCorrecCheckBox, SIGNAL(clicked()), this, SLOT(reloadPiece()));
//    connect(_bowtieColorGroupBox, SIGNAL(clicked()), this, SLOT(reloadPiece()));

    emit bowtieChecked();
}

void MainWindow::reloadPiece(void) {
    _parser._isBowtieActivated = _bowtieColorGroupBox->isChecked();
    _parser._isBowtieCorrected = _bowtieCorrecCheckBox->isChecked();

    _parser._nbBowtieQuads = 0;
    _viewer->changePiece(_parser.createNode(_currFile), false);
}

void MainWindow::fillFavsList(void) {
    _favsList = getPartsList("Favourites");
}

void MainWindow::displayParts(QModelIndex) {
    // Get current index
    int currCategoryIndex = _categoriesListView->currentIndex().row();

    // Get category selected
    QString category = _categoriesListModel->stringList().at(currCategoryIndex);

    // Get parts list
    QStringList partsList = getPartsList(category);

    // Set model string list
    _partsListModel->setStringList(partsList);

    // Set alternating row colors within parts list
    _partsListView->setAlternatingRowColors(true);

    // Get first index
    QModelIndex currModelIndex = _partsListView->indexAt(QPoint(0, 0));

    // Set current index to first one in list, to avoid seg fault when scrolling without choosing anything first
    _partsListView->setCurrentIndex(currModelIndex);

    // Render selected index
    renderPart(currModelIndex);
}

void MainWindow::renderPart(QModelIndex) {
    // Enable fav button
    _favAction->setEnabled(true);

    // Get part index in list
    int currCategoryIndex = _partsListView->currentIndex().row();

    // Get line in list
    _currPart = _partsListModel->stringList().at(currCategoryIndex);

    // Create file name
    QString partName = _currPart.split(QRegExp("\\s+")).at(0)+".dat";

    // Create path name and store it
    _currFile = "/home/shaolan/Documents/ldraw/parts/"+partName;

    // Change viewer piece
    _parser._nbBowtieQuads = 0;
    _viewer->changePiece(_parser.createNode(_currFile));

    // Set fav button icon
    if (getIndexOfFav(_favsList, _currPart) == -1)
        _favAction->setIcon(QIcon("../LViewer/icons/notFavIco.png"));
    else
        _favAction->setIcon(QIcon("../LViewer/icons/favIco.png"));

    // Set nb of bowtie quads
    _bowtieMatchFoundLabel->setText(QString::number(_parser._nbBowtieQuads));
}

void MainWindow::createColorComboBox(void) {
    // Create combo box
    _colorComboBox = new QComboBox;

    // Get colors array
    QMap<int, LDrawParser::ColorParams> colorArray = _parser.getColorsArray();

    // Counters to know where the main color is within the list
    int index = 0, k = 0;

    // Browse colors to fill color combo box
    foreach (LDrawParser::ColorParams colorParam, colorArray.values()) {
        // Get current color name
        QString colorName = colorParam.colorName;

        // If we are parsing the edge color, we skip it
        if (colorName == "Edge Colour")
            continue;

        // If we are parsing the main color, this is the right index within combo box
        if (colorName == "Main Colour")
            index = k;

        // Get hexa code color
        QString surfValue = colorParam.surfValue;

        // Create a pixmap that be used in color combo box as an icon
        QPixmap colorPixmap(20, 15);
        // Fill with the current color
        colorPixmap.fill(QColor(surfValue));
        // Add color icon and name to color combo box
        _colorComboBox->addItem(QIcon(colorPixmap), colorName);

        // Increment counter
        k++;
    }

    // Set current index to the main color one
    _colorComboBox->setCurrentIndex(index);
}

void MainWindow::addFav(void) {
    // Open favs in read only mode
    QFile fileRead("/home/shaolan/Documents/Qt/LViewer/config/Favourites.ldr");
    if (!fileRead.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening in read only Favourites.ldr file within MainWindow::addFav";
    }

    // Get text stream
    QTextStream inFileRead(&fileRead);
    inFileRead.setCodec("UTF-8");

    // Create line file name and description
    QStringList argsList = _currPart.split(QRegExp("\\s"));
    QString fileName = argsList.takeAt(0) + ".dat";
    QString description = argsList.join(" ");
    QString line = "0 !FILENAME " + fileName + " !DESCRIPTION " + description;

    // Get all fav lines
    QStringList allFavsList = inFileRead.readAll().split("\n");

    // Close file, because we're gonna write on it, whether we add or delete fav
    fileRead.close();

    // Get index of fav
    int indexOfFav = getIndexOfFav(allFavsList, line);

    // If the piece is not in favs, we add it
    if (indexOfFav == -1) {
        // Open favs in append mode to add piece in favs
        QFile fileAppend("/home/shaolan/Documents/Qt/LViewer/config/Favourites.ldr");
        if (!fileAppend.open(QIODevice::Append | QIODevice::Text)) {
            qDebug() << "Error while opening in append Favourites.ldr file within MainWindow::addFav";
        }

        // Get text stream
        QTextStream inFileAppend(&fileAppend);
        inFileAppend.setCodec("UTF-8");

        // Add curr part to favourites
        inFileAppend << line << "\n";

        // Close file
        fileAppend.close();

        // Set fav button icon
        _favAction->setIcon(QIcon("../LViewer/icons/favIco.png"));
    // Else, we remove it from favs
    } else {
        // Remove fav from list
        allFavsList.removeAt(indexOfFav);

        // Remove blank line
        allFavsList.removeAll("");

        // Open favs in write mode to rewrite every piece from list
        QFile fileWrite("/home/shaolan/Documents/Qt/LViewer/config/Favourites.ldr");
        if (!fileWrite.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug() << "Error while opening in write only Favourites.ldr file within MainWindow::addFav";
        }

        // Get text stream
        QTextStream inFileWrite(&fileWrite);
        inFileWrite.setCodec("UTF-8");

        // Add every parts to favourites
        for (int k = 0; k < allFavsList.size(); k++)
        inFileWrite << allFavsList.at(k) << "\n";

        // Close file
        fileWrite.close();

        // Set fav button icon
        _favAction->setIcon(QIcon("../LViewer/icons/notFavIco.png"));
    }

    // Update favs list
    fillFavsList();

    // Update model if users are under favourites category
    if (_categoriesListView->currentIndex().row() == 0)
        _partsListModel->setStringList(_favsList);
}

int MainWindow::getIndexOfFav(const QStringList& allFavsList, const QString& line) const {
    return allFavsList.indexOf(QRegExp(line));
}

void MainWindow::browseParts(void) {
    // Show part dock widget
    _partsDockWidget->setVisible(true);
}

void MainWindow::changeColor(QString colorName) {
    // Get colors array
    QMap<int, LDrawParser::ColorParams> colorArray = _parser.getColorsArray();

    // Get actual color params and integer code
    for (QMap<int, LDrawParser::ColorParams>::iterator it = colorArray.begin(); it != colorArray.end(); it++) {
        // If it's the right color name we get params
        if (colorName == it.value().colorName) {
            // Get color code
            int currColorId = it.key();
            // Change default color in parser
            _parser.setCurrColorId(currColorId);
            // Color changed, so we have to render the piece again, with the new one
            _viewer->changePiece(_parser.createNode(_currFile), false);
        }
    }
}

void MainWindow::otherOnLeftView(bool) {
    // Change view
    _viewer->changeViews(ViewerWidget::OthersOnLeft);
    // Change check values, so in order to avoid infinite loop, disconnect signals
    disconnectToggle();
    _otherOnLeftAction->setChecked(true);
    _quadrantAction->setChecked(false);
    _soloAction->setChecked(false);
    // Connect signals again
    connectToggle();
}

void MainWindow::quadrantView(bool) {
    // Change view
    _viewer->changeViews(ViewerWidget::Quadrant);
    // Change check values, so in order to avoid infinite loop, disconnect signals
    disconnectToggle();
    _otherOnLeftAction->setChecked(false);
    _quadrantAction->setChecked(true);
    _soloAction->setChecked(false);
    // Connect signals again
    connectToggle();
}

void MainWindow::soloView(bool) {
    // Change view
    _viewer->changeViews(ViewerWidget::Solo);
    // Change check values, so in order to avoid infinite loop, disconnect signals
    disconnectToggle();
    _otherOnLeftAction->setChecked(false);
    _quadrantAction->setChecked(false);
    _soloAction->setChecked(true);
    // Connect signals again
    connectToggle();
}

void MainWindow::disconnectToggle(void) {
    // Disconnect view modes action
    disconnect(_otherOnLeftAction, SIGNAL(toggled(bool)), this, SLOT(otherOnLeftView(bool)));
    disconnect(_quadrantAction, SIGNAL(toggled(bool)), this, SLOT(quadrantView(bool)));
    disconnect(_soloAction, SIGNAL(toggled(bool)), this, SLOT(soloView(bool)));
}

void MainWindow::connectToggle(void) {
    // Connect view modes action
    connect(_otherOnLeftAction, SIGNAL(toggled(bool)), this, SLOT(otherOnLeftView(bool)));
    connect(_quadrantAction, SIGNAL(toggled(bool)), this, SLOT(quadrantView(bool)));
    connect(_soloAction, SIGNAL(toggled(bool)), this, SLOT(soloView(bool)));
}

void MainWindow::centerOnScreen(void) {
}

void MainWindow::showEvent(QShowEvent* event) {
    Q_UNUSED(event);

    centerOnScreen();
}

void MainWindow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);

    QPainter painter(this);

    QBrush background(QColor(23, 23, 24));

    painter.setBrush(background);
    painter.setPen(Qt::NoPen);

    painter.drawRect(0, 0, width(), height());
}

// ////////////////////////////////////
// STYLE SHEETS
// ////////////////////////////////////
void MainWindow::setStyle(void) {

    //"    background-color: #D6D1AF;";

//    QString dockWidgetStyle = "";
//    dockWidgetStyle += "QDockWidget { ";
//    dockWidgetStyle += "    font-size: 16pt;";
//    dockWidgetStyle += "    font-weight: bold;";
//    dockWidgetStyle += "    font-family: 'Visitor TT1 BRK';";
//    dockWidgetStyle += "    color: #FFFFFF;";
//    dockWidgetStyle += "    background-color: #757674;";
//    dockWidgetStyle += "    border: none;";
//    dockWidgetStyle += "}";
//    dockWidgetStyle += "QDockWidget::title {";
//    dockWidgetStyle += "    padding-left: 25px;";
//    dockWidgetStyle += "    background-color: #8A8780;";
//    dockWidgetStyle += "}";
//    dockWidgetStyle += "QDockWidget {";
//    dockWidgetStyle += "    titlebar-close-icon: url(\"\");";
//    dockWidgetStyle += "    titlebar-normal-icon: url(\"\");";
//    dockWidgetStyle += "}";
//    dockWidgetStyle += "QDockWidget::close-button, QDockWidget::float-button {";
//    dockWidgetStyle += "    padding: 0px;";
//    dockWidgetStyle += "}";
//    dockWidgetStyle += "QDockWidget::close-button:pressed, QDockWidget::float-button:pressed {";
//    dockWidgetStyle += "    padding: 1px -1px -1px 1px;";
//    dockWidgetStyle += "}";

//    QString listViewStyle = "";
//    listViewStyle += "QListView {";
//    listViewStyle += "      show-decoration-selected: 1;";
//    listViewStyle += "      background: #3B3B3B;";
//    listViewStyle += "      alternate-background-color: #2B2B2B;";
//    listViewStyle += "      color: #F2E3C6;";
//    listViewStyle += "      font-size: 14pt;";
//    listViewStyle += "      font-family: 'Visitor TT2 BRK';";
//    listViewStyle += "}";
//    listViewStyle += "QListView::item:selected {";
//    listViewStyle += "      border: 2px solid #F2E3C6;";
//    listViewStyle += "      color: #F2E3C6;";
//    listViewStyle += "}";

//    QString scrollBarStyle = "";
//    scrollBarStyle += "QScrollBar:horizontal {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     height: 20px;";
//    scrollBarStyle += "     margin: 0px 20px 0px 20px;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::handle:horizontal {";
//    scrollBarStyle += "     background: #F2E3C6;";
//    scrollBarStyle += "     min-width: 20px;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::add-line:horizontal {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     width: 20px;";
//    scrollBarStyle += "     subcontrol-position: right;";
//    scrollBarStyle += "     subcontrol-origin: margin;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::sub-line:horizontal {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     width: 20px;";
//    scrollBarStyle += "     subcontrol-position: left;";
//    scrollBarStyle += "     subcontrol-origin: margin;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     width: 3px;";
//    scrollBarStyle += "     height: 3px;";
//    scrollBarStyle += "     background: #F2E3C6;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {";
//    scrollBarStyle += "     background: none;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar:vertical {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     height: 20px;";
//    scrollBarStyle += "     margin: 0px 20px 0px 20px;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::handle:vertical {";
//    scrollBarStyle += "     background: #F2E3C6;";
//    scrollBarStyle += "     min-width: 20px;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::add-line:vertical {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     width: 20px;";
//    scrollBarStyle += "     subcontrol-position: right;";
//    scrollBarStyle += "     subcontrol-origin: margin;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::sub-line:vertical {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     background: #82837E;";
//    scrollBarStyle += "     width: 20px;";
//    scrollBarStyle += "     subcontrol-position: left;";
//    scrollBarStyle += "     subcontrol-origin: margin;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::left-arrow:vertical, QScrollBar::right-arrow:vertical {";
//    scrollBarStyle += "     border: 1p solid #F2E3C6;";
//    scrollBarStyle += "     width: 3px;";
//    scrollBarStyle += "     height: 3px;";
//    scrollBarStyle += "     background: #F2E3C6;";
//    scrollBarStyle += "}";
//    scrollBarStyle += "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {";
//    scrollBarStyle += "     background: none;";
//    scrollBarStyle += "}";

//    QString style = dockWidgetStyle
//                  + listViewStyle
                  /*+ scrollBarStyle*/;
    //setStyleSheet(style);
}
