#define SORT 0
#define LISTFAULTPARTS 0
#define MAIN 1

#if SORT

#include "LDrawParser.h"
#include <QStringList>
#include <QDir>
#include <QDebug>

int main(int argc, char** argv) {
    QString file = "/home/shaolan/Documents/Qt/LViewer/config/FaultPartsFileName.txt";

    QFile fileRO(file);
    if (!fileRO.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening in read only FaultPartsFileName.txt file within main";
    }

    QTextStream inFileRO(&fileRO);
    inFileRO.setCodec("UTF-8");

    QStringList allFiles = inFileRO.readAll().split("\n");
    allFiles.sort();

    fileRO.close();

    QFile fileWO(file);
    if (!fileWO.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Error while opening in write only FaultPartsFileName.txt  file within main";
    }

    QTextStream inFileWO(&fileWO);
    inFileWO.setCodec("UTF-8");

    for (int k = 0; k < allFiles.size(); k++) {
        inFileWO << allFiles.at(k) << " ";
    }

    fileWO.close();
}

#endif

#if LISTFAULTPARTS

#include "LDrawParser.h"
#include <QStringList>
#include <QDir>
#include <QDebug>

int main(int argc, char** argv) {
    QStringList allParts;

    QString path1 = "/home/shaolan/Documents/ldraw/parts/";
    QDir dir = QDir(path1);

    allParts = dir.entryList(QDir::Files | QDir::NoDotAndDotDot);

    LDrawParser parser;

    for (int k = 4848; k < allParts.size(); k++) {
        qDebug() << "Parsing" << allParts.at(k);

        QString fileName = "/home/shaolan/Documents/ldraw/parts/" + allParts.at(k);

        parser.createNode(fileName);
    }
}

#endif

#if MAIN

#include <QApplication>
#include <QtGui>
//#include <X11/Xlib.h>
#include "MainWindow.h"

int main(int argc, char** argv) {

//    // Init X11 threads, because I really wanted to use another thread to render OSG components but it does not work either
//    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    // Call QApplication
    QApplication app(argc, argv);

    // Create main window and display it
    MainWindow window;
    window.show();

    // Return application exec
    return app.exec();
}

#endif
