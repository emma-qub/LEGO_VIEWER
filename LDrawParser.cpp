#define RECORD_FAULTY_PARTS 0

#include "LDrawParser.h"

#include <osg/BlendFunc>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgUtil/SmoothingVisitor>

#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QStringList>

#include <cmath>

// Static integer to handle tab shift when debugging
int LDrawParser::tab = 0;

// Static QStringList to record faulty parts
QStringList LDrawParser::_faultyParts = QStringList();

// Count how many bowtie quads the part contains
int LDrawParser::_nbBowtieQuads = 0;

LDrawParser::LDrawParser(void) :
    _currColorId(-1),
    _isBowtieActivated(false),
    _isBowtieCorrected(false),
    _isNonPlanarActivate(false),
    _isNonPlanarCorected(false) {

    fillColorsArray();
    fillCategoriesArray();
}

LDrawParser::LDrawParser(const LDrawParser& lDrawParser) {
    _currColorId = lDrawParser._currColorId;
    _isBowtieActivated = lDrawParser._isBowtieActivated;
    _isBowtieActivated = lDrawParser._isBowtieCorrected;
    _isNonPlanarActivate = lDrawParser._isNonPlanarActivate;
    _isNonPlanarActivate = lDrawParser._isNonPlanarCorected;
    _colorsArray = lDrawParser._colorsArray;
    _categoriesArray = lDrawParser._categoriesArray;
}

LDrawParser::~LDrawParser(void) {
}

void LDrawParser::fillColorsArray(void) {
    // Try to open colors specifications text file in read only mode
    QFile file("/home/shaolan/Documents/ldraw/LDConfig.ldr");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening LDConfig.ldr file within LDrawParser::fillColorsArray";
        // Throw exception
        throw OpenFailed();
    }

    // Stream text and encode in UTF-8
    QTextStream inFile(&file);
    inFile.setCodec("UTF-8");

    // Read line per line
    QString currLine;
    while (!inFile.atEnd()) {
        // Get next line
        currLine = inFile.readLine();

        // Remove white spaces before and after
        currLine = currLine.trimmed();

        // Split line according to spaces, whatever the number of blanc parts
        QStringList commandArgs = currLine.split(QRegExp("\\s+"));

        // If the line has almost 9 fields and whose second one is !COLOUR, we create the color specification
        if (commandArgs.size() > 8 && commandArgs.at(1) == "!COLOUR") {
            // Create color params
            ColorParams currColorParams;
            // Every color within the file contains underscore that we replace with blank
            currColorParams.colorName = commandArgs.at(2).split("_").join(" ");
            currColorParams.surfValue = commandArgs.at(6);
            currColorParams.edgeValue = commandArgs.at(8);

            if (commandArgs.size() > 10)
                currColorParams.alphaValue = commandArgs.at(10).toDouble();
            else
                currColorParams.alphaValue = 255.0;

            // Get color id
            int colorId = commandArgs.at(4).toInt();

            // Insert it among other colors
            _colorsArray.insert(colorId, currColorParams);
        }
    }
    // Close file
    file.close();
}

void LDrawParser::fillCategoriesArray(void) {
    // Try to open colors specifications text file in read only mode
    QFile file("/home/shaolan/Documents/Qt/LViewer/config/Categories.ldr");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening Categories.ldr file within LDrawParser::fillCategoriesArray";
        // Throw exception
        throw OpenFailed();
    }

    // Stream text and encode in UTF-8
    QTextStream inFile(&file);
    inFile.setCodec("UTF-8");

    // Read line per line
    while (!inFile.atEnd()) {
        // Get next line
        _categoriesArray << inFile.readLine().split("!CATEGORY").at(1).simplified();
    }
}

QString LDrawParser::getSurfQString(int colorId) {
    if (colorId == 16 && _currColorId != -1)
        colorId = _currColorId;
    return _colorsArray.value(colorId).surfValue;
}

QString LDrawParser::getEdgeQString(int colorId) {
    if (colorId == 24 && _currColorId != -1)
        colorId = _currColorId;
    return _colorsArray.value(colorId).edgeValue;
}

double LDrawParser::getAlphaValue(int colorId) {
    if (colorId == 16 && _currColorId != -1)
        colorId = _currColorId;
    return _colorsArray.value(colorId).alphaValue;
}

osg::Vec4 LDrawParser::getSurfOrEdgeColor(int colorId, bool isSurf) {
    QString colorName;
    if (isSurf)
        colorName = getSurfQString(colorId);
    else
        colorName = getEdgeQString(colorId);

    QColor color(colorName);

    return osg::Vec4(static_cast<double>(color.red())   / 255.0,
                     static_cast<double>(color.green()) / 255.0,
                     static_cast<double>(color.blue())  / 255.0,
                     getAlphaValue(colorId) / 255.0);
}

osg::Vec3 LDrawParser::calculateNormal(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c) {
    osg::Vec3 ab = b - a;
    osg::Vec3 ac = c - a;

    osg::Vec3 n = osg::Vec3(ab[1]*ac[2]-ab[2]*ac[1], ab[2]*ac[0]-ab[0]*ac[2], ab[0]*ac[1]-ab[1]*ac[0]);
    n.normalize();
    return n;
}

osg::Vec3 LDrawParser::multMatVec(const osg::Vec3& vec, const osg::Matrix& mat) {
    return osg::Vec3(mat(0, 0)*vec[0] + mat(1, 0)*vec[1] + mat(2, 0)*vec[2] + mat(3, 0),
                     mat(0, 1)*vec[0] + mat(1, 1)*vec[1] + mat(2, 1)*vec[2] + mat(3, 1),
                     mat(0, 2)*vec[0] + mat(1, 2)*vec[1] + mat(2, 2)*vec[2] + mat(3, 2));
}

osg::Group* LDrawParser::createNode(const QString& fileName) {
    osg::ref_ptr<osg::Vec3Array> lineVerticesArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> triangleVerticesArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec3Array> quadVerticesArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> lineColorsArray = new osg::Vec4Array;
    osg::ref_ptr<osg::Vec4Array> triangleColorsArray = new osg::Vec4Array;
    osg::ref_ptr<osg::Vec4Array> quadColorsArray = new osg::Vec4Array;

    osg::Matrix ident(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);

    fillArrays(fileName, true, false, ident,
               lineVerticesArray.get(), triangleVerticesArray.get(), quadVerticesArray.get(),
               lineColorsArray.get(), triangleColorsArray.get(), quadColorsArray.get());

    osg::ref_ptr<osg::Vec3Array> verticesArray = new osg::Vec3Array;
    osg::ref_ptr<osg::Vec4Array> colorsArray = new osg::Vec4Array;
    osg::ref_ptr<osg::Vec3Array> normalsArray = new osg::Vec3Array;


/******************************************
 *  Triangles and Quads
 ******************************************/

    // Add every vertices to global vertices array and create normals
    for (unsigned int k = 0; k < triangleVerticesArray->getNumElements(); k+=3) {
        normalsArray->push_back(calculateNormal(triangleVerticesArray->at(k), triangleVerticesArray->at(k+1), triangleVerticesArray->at(k+2)));
        for (int i = 0; i < 3; i++)
            verticesArray->push_back(triangleVerticesArray->at(k + i));
    }
    for (unsigned int k = 0; k < quadVerticesArray->getNumElements(); k+=4) {
        normalsArray->push_back(calculateNormal(quadVerticesArray->at(k), quadVerticesArray->at(k+1), quadVerticesArray->at(k+2)));
        for (int i = 0; i < 4; i++)
            verticesArray->push_back(quadVerticesArray->at(k + i));
    }

    // Add every colors to global colors array
    for (unsigned int k = 0; k < triangleColorsArray->getNumElements(); k++)
        colorsArray->push_back(triangleColorsArray->at(k));
    for (unsigned int k = 0; k < quadColorsArray->getNumElements(); k++)
        colorsArray->push_back(quadColorsArray->at(k));

    // Create geometry
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;

    // Match vertices
    geometry->setVertexArray(verticesArray);

    // Match colors
    geometry->setColorArray(colorsArray);
    geometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    // Match normals
    geometry->setNormalArray(normalsArray);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    // Add primitives
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, triangleVerticesArray->getNumElements()));
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, triangleVerticesArray->getNumElements(), quadVerticesArray->getNumElements()));


/******************************************
 *  Lines
 ******************************************/

    // Create line geometry
    osg::ref_ptr<osg::Geometry> lineGeometry = new osg::Geometry;

    // Match vertices
    lineGeometry->setVertexArray(lineVerticesArray);

    // Match colors
    lineGeometry->setColorArray(lineColorsArray);
    lineGeometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);

    // Add primitives
    lineGeometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, lineVerticesArray->getNumElements()));

    // Disable lighting
    lineGeometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);


    // Create geode
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    // Handle transparency
    geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    geode->getOrCreateStateSet()->setAttributeAndModes(new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA), osg::StateAttribute::ON);

    // Add geometries
    geode->addDrawable(geometry);
    geode->addDrawable(lineGeometry);

    // Create group
    osg::ref_ptr<osg::Group> group = new osg::Group;

    // Add geode to node
    group->addChild(geode.get());

    // Return the lego geometry
    return group.release();
}

void LDrawParser::fillArrays(QString fileName, bool accumCull, bool accumInvert, osg::Matrix accumTransformMatrix,
                             osg::Vec3Array* lineVerticesArray, osg::Vec3Array* triangleVerticesArray, osg::Vec3Array* quadVerticesArray,
                             osg::Vec4Array* lineColorsArray, osg::Vec4Array* triangleColorsArray, osg::Vec4Array* quadColorsArray, int currColor) {
    bool localCull = true;
    Winding winding = ccw;
    ///Certified certified = unknown;
    bool invertNext = false;
    QStringList commandArgs;

    // Try to open text file in read only mode
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error while opening " + fileName + " file within LDrawParser::createNode";
        // Throw exception
        throw OpenFailed();
    }

    // Stream text and encode in UTF-8
    QTextStream inFile(&file);
    inFile.setCodec("UTF-8");

    // Boolean to store if file contains invertnext
    bool containsInvertNext = false;

    // Read line per line
    QString currLine;
    while (!inFile.atEnd()) {
        // Get next line
        currLine = inFile.readLine();

        // Remove white spaces before and after
        currLine = currLine.trimmed();

        // Split line according to spaces, whatever the number of blanc parts
        commandArgs.clear();
        commandArgs = currLine.split(QRegExp("\\s+"));

        // Boolean value to know whether conversion from string to int worked
        bool ok;

        // If the line is not empty, we process it
        if (commandArgs.size() > 0) {
            // Get first value:
            // - 0 is comment or meta command
            // - 1 is file include
            // - 2 is line
            // - 3 is triangle
            // - 4 is quad
            // - 5 is conditional line
            int n = commandArgs.at(0).toInt(&ok);
            // If conversion from string to int worked, we switch according to first number
            if (ok) {
                // It's a command or a comment
                if (n == 0) {
                    if (commandArgs.contains("BFC")) {
                        if (commandArgs.contains("CLIP")) {
                            localCull = true;
                        } if (commandArgs.contains("NOCLIP")) {
                            localCull = false;
                        } if (commandArgs.contains("CCW")) {
                            if (accumInvert)
                                winding = cw;
                            else
                                winding = ccw;
                        } if (commandArgs.contains("CW")) {
                            if (accumInvert)
                                winding = ccw;
                            else
                                winding = cw;
                        } if (commandArgs.contains("INVERTNEXT")) {
                            containsInvertNext = true;
                            invertNext = true;
                        }
                    }
                // It's a file include
                } if (n == 1) {
                    // The last parameter is the file's name to include
                    QString fileName = commandArgs.at(14);

                    // Sometimes, included file are in upper case, whereas file's names ar in lower case
                    fileName = fileName.toLower();

                    // If file's name starts with s, or 48, that means the next char i '\'
                    // and under UNIX systems, directory are separated with '/' char
                    if (fileName.startsWith("s\\") || fileName.startsWith("48")) {
                        fileName.replace('\\', '/');
                    }

                    // Files included can be located under p/ or parts/ directory...
                    QString path = "";
                    QDir dir1 = QDir("/home/shaolan/Documents/ldraw/p/");
                    QDir dir2 = QDir("/home/shaolan/Documents/ldraw/parts/");

                    // ...so we check where the file is, and create the path string accordingly
                    if (dir1.exists(fileName))
                        path = "/home/shaolan/Documents/ldraw/p/";
                    else if (dir2.exists(fileName))
                        path = "/home/shaolan/Documents/ldraw/parts/";
                    else {
                        qDebug() << "Cannot find" << fileName << "under p/ nor parts/ directory within LDrawParser::createNode.";
                        throw OpenFailed();
                    }

                    // Create complete path name, according to directory and file's name
                    QString pathName = path + fileName;

                    // Get color code
                    int nextColor = commandArgs.at(1).toInt();
                    if (nextColor == 16)
                        nextColor = currColor;

                    // Get 4x4 matrix transformation values
                    double x = commandArgs.at(2).toDouble();
                    double y = commandArgs.at(3).toDouble();
                    double z = commandArgs.at(4).toDouble();
                    double a = commandArgs.at(5).toDouble();
                    double b = commandArgs.at(6).toDouble();
                    double c = commandArgs.at(7).toDouble();
                    double d = commandArgs.at(8).toDouble();
                    double e = commandArgs.at(9).toDouble();
                    double f = commandArgs.at(10).toDouble();
                    double g = commandArgs.at(11).toDouble();
                    double h = commandArgs.at(12).toDouble();
                    double i = commandArgs.at(13).toDouble();

                    osg::Matrix transformMatrix(a, d, g, 0.0, b, e, h, 0.0, c, f, i, 0.0, x, y, z, 1.0);

                    fillArrays(pathName, accumCull && localCull, ((accumInvert && !invertNext) || (!accumInvert && invertNext)), transformMatrix*accumTransformMatrix,
                               lineVerticesArray, triangleVerticesArray, quadVerticesArray, lineColorsArray, triangleColorsArray, quadColorsArray, nextColor);
                // If it's a line
                } else if (n == 2) {
                    // Fill colors array
                    int colorCode = commandArgs.at(1).toInt();
                    if (colorCode == 24)
                        lineColorsArray->push_back(getSurfOrEdgeColor(currColor, false));
                    else
                        lineColorsArray->push_back(getSurfOrEdgeColor(colorCode, false));

                    // Create vertices
                    osg::Vec3 v0(commandArgs.at(2).toDouble(), commandArgs.at(3).toDouble(), commandArgs.at(4).toDouble());
                    v0 = multMatVec(v0, accumTransformMatrix);

                    osg::Vec3 v1(commandArgs.at(5).toDouble(), commandArgs.at(6).toDouble(), commandArgs.at(7).toDouble());
                    v1 = multMatVec(v1, accumTransformMatrix);

                    // Fill vertices array
                    lineVerticesArray->push_back(v0);
                    lineVerticesArray->push_back(v1);

                // If it's a triangle
                } else if (n == 3) {
                    double detMatrix = (accumTransformMatrix(0, 0)*accumTransformMatrix(1, 1)*accumTransformMatrix(2, 2)
                                     +  accumTransformMatrix(1, 0)*accumTransformMatrix(2, 1)*accumTransformMatrix(0, 2)
                                     +  accumTransformMatrix(2, 0)*accumTransformMatrix(0, 1)*accumTransformMatrix(1, 2)
                                     -  accumTransformMatrix(2, 0)*accumTransformMatrix(1, 1)*accumTransformMatrix(0, 2)
                                     -  accumTransformMatrix(0, 0)*accumTransformMatrix(2, 1)*accumTransformMatrix(1, 2)
                                     -  accumTransformMatrix(1, 0)*accumTransformMatrix(0, 1)*accumTransformMatrix(2, 2));

                    bool isMatDirect = detMatrix >= 0;

                    bool rotation = ( isMatDirect && (winding == ccw))
                                 || (!isMatDirect && (winding == cw ));

                    // Fill colors array
                    int colorCode = commandArgs.at(1).toInt();
                    if (colorCode == 16)
                        triangleColorsArray->push_back(getSurfOrEdgeColor(currColor));
                    else
                        triangleColorsArray->push_back(getSurfOrEdgeColor(colorCode));

                    // Create vertices
                    osg::Vec3 v0(commandArgs.at(2).toDouble(), commandArgs.at(3).toDouble(), commandArgs.at(4).toDouble());
                    osg::Vec3 v1(commandArgs.at(5).toDouble(), commandArgs.at(6).toDouble(), commandArgs.at(7).toDouble());
                    osg::Vec3 v2(commandArgs.at(8).toDouble(), commandArgs.at(9).toDouble(), commandArgs.at(10).toDouble());

                    v0 = multMatVec(v0, accumTransformMatrix);
                    v1 = multMatVec(v1, accumTransformMatrix);
                    v2 = multMatVec(v2, accumTransformMatrix);

                    // Fill vertices array
                    if (rotation) {
                        triangleVerticesArray->push_back(v0);
                        triangleVerticesArray->push_back(v1);
                        triangleVerticesArray->push_back(v2);
                    } else {
                        //qDebug() << "!CCW";
                        triangleVerticesArray->push_back(v2);
                        triangleVerticesArray->push_back(v1);
                        triangleVerticesArray->push_back(v0);
                    }
                // If it's a quad
                } else if (n == 4) {
                    double detMatrix = (accumTransformMatrix(0, 0)*accumTransformMatrix(1, 1)*accumTransformMatrix(2, 2)
                                     +  accumTransformMatrix(1, 0)*accumTransformMatrix(2, 1)*accumTransformMatrix(0, 2)
                                     +  accumTransformMatrix(2, 0)*accumTransformMatrix(0, 1)*accumTransformMatrix(1, 2)
                                     -  accumTransformMatrix(2, 0)*accumTransformMatrix(1, 1)*accumTransformMatrix(0, 2)
                                     -  accumTransformMatrix(0, 0)*accumTransformMatrix(2, 1)*accumTransformMatrix(1, 2)
                                     -  accumTransformMatrix(1, 0)*accumTransformMatrix(0, 1)*accumTransformMatrix(2, 2));

                    bool isMatDirect = detMatrix >= 0;

                    bool rotation = ( isMatDirect && (winding == ccw))
                                 || (!isMatDirect && (winding == cw ));

                    // Fill colors array
                    int colorCode = commandArgs.at(1).toInt();
                    if (colorCode == 16)
                        quadColorsArray->push_back(getSurfOrEdgeColor(currColor));
                    else
                        quadColorsArray->push_back(getSurfOrEdgeColor(colorCode));

                    // Create vertices
                    osg::Vec3 v0(commandArgs.at(2).toDouble(), commandArgs.at(3).toDouble(), commandArgs.at(4).toDouble());
                    osg::Vec3 v1(commandArgs.at(5).toDouble(), commandArgs.at(6).toDouble(), commandArgs.at(7).toDouble());
                    osg::Vec3 v2(commandArgs.at(8).toDouble(), commandArgs.at(9).toDouble(), commandArgs.at(10).toDouble());
                    osg::Vec3 v3(commandArgs.at(11).toDouble(), commandArgs.at(12).toDouble(), commandArgs.at(13).toDouble());

                    v0 = multMatVec(v0, accumTransformMatrix);
                    v1 = multMatVec(v1, accumTransformMatrix);
                    v2 = multMatVec(v2, accumTransformMatrix);
                    v3 = multMatVec(v3, accumTransformMatrix);



                    // Detect crossed polygons and fix them if any
                    bool cross1 = false;
                    bool cross2 = false;
                    detectCrossing(v0, v1, v2, v3, cross1, cross2);

                    if (cross1) {
                        _nbBowtieQuads++;
                        if (_isBowtieActivated)
                            (*quadColorsArray)[quadColorsArray->getNumElements()-1] = osg::Vec4(250.0/255.0, 105.0/255.0, 0.0, 1.0);
                        if (_isBowtieCorrected) {
                            osg::Vec3 tmp(v2);
                            v2 = v3;
                            v3 = tmp;
                        }
//                        qDebug() << "Cross 1 detected within" << fileName;
#if RECORD_FAULTY_PARTS
                        if (!_faultyParts.contains(fileName)) {
                            _faultyParts << fileName;
                            writeFaultyParts(fileName);
                        }
#endif
                    } else if (cross2) {
                        _nbBowtieQuads++;
                        if (_isBowtieActivated)
                            (*quadColorsArray)[quadColorsArray->getNumElements()-1] = osg::Vec4(250.0/255.0, 105.0/255.0, 0.0, 1.0);
                        if (_isBowtieCorrected) {
                            osg::Vec3 tmp(v1);
                            v1 = v2;
                            v2 = tmp;
                        }
//                        qDebug() << "Cross 2 detected within" << fileName;
#if RECORD_FAULTY_PARTS
                        if (!_faultyParts.contains(fileName)) {
                            _faultyParts << fileName;
                            writeFaultyParts(fileName);
                        }
#endif
                    }



                    // Detect three vertices aligned and concave quads at the same time
                    bool concav = false;
                    detectConcav(v0, v1, v2, v3, concav, 0);

                    if (concav) {
                        //(*quadColorsArray)[quadColorsArray->getNumElements()-1] = osg::Vec4(4.0/255.0, 109.0/255.0, 139.0/255.0, 1.0);
                    }




                    // Fill vertices array
                    if (rotation) {
                        quadVerticesArray->push_back(v0);
                        quadVerticesArray->push_back(v1);
                        quadVerticesArray->push_back(v2);
                        quadVerticesArray->push_back(v3);
                    } else {

                        quadVerticesArray->push_back(v3);
                        quadVerticesArray->push_back(v2);
                        quadVerticesArray->push_back(v1);
                        quadVerticesArray->push_back(v0);
                    }
                }
                if (n != 0 || (n == 0 && !containsInvertNext))
                    invertNext = false;
            }
        }
    }
    // Close file
    file.close();
}

osg::Vec3 LDrawParser::crossProduct(const osg::Vec3 u, const osg::Vec3 v) const {
    return osg::Vec3(u[1]*v[2] - u[2]*v[1],
                     u[2]*v[0] - u[0]*v[2],
                     u[0]*v[1] - u[1]*v[0]);
}

double LDrawParser::dotProduct(const osg::Vec3 u, const osg::Vec3 v) const {
    return u[0]*v[0] + u[1]*v[1] + u[2]*v[2];
}

void LDrawParser::detectCrossing(const osg::Vec3 A, const osg::Vec3 B, const osg::Vec3 C, const osg::Vec3 D, bool& cross1, bool& cross2) const {

    osg::Vec3 AB(B-A);
    osg::Vec3 AC(C-A);
    osg::Vec3 AD(D-A);

    osg::Vec3 CD(D-C);
    osg::Vec3 CA(A-C);
    osg::Vec3 CB(B-C);

    // First crossing tests
    osg::Vec3 ADxAB = crossProduct(AD, AB);
    //ADxAB/=(ADxAB.normalize());
    osg::Vec3 ADxAC = crossProduct(AD, AC);
    //ADxAC/=(ADxAC.normalize());

    osg::Vec3 CBxCA = crossProduct(CB, CA);
    //CBxCA/=(CBxCA.normalize());
    osg::Vec3 CBxCD = crossProduct(CB, CD);
    //CBxCD/=(CBxCD.normalize());

    cross1 = (ADxAB*ADxAC < 0 && CBxCA*CBxCD < 0);

    // Second crossing tests
    osg::Vec3 ABxAC = crossProduct(AB, AC);
    ABxAC/=(ABxAC.normalize());
    osg::Vec3 ABxAD = crossProduct(AB, AD);
    ABxAD/=(ABxAD.normalize());

    osg::Vec3 CDxCA = crossProduct(CD, CA);
    CDxCA/=(CDxCA.normalize());
    osg::Vec3 CDxCB = crossProduct(CD, CB);
    CDxCB/=(CDxCB.normalize());

    cross2 = (ABxAC*ABxAD < 0 && CDxCA*CDxCB < 0);
}

void LDrawParser::detectConcav(const osg::Vec3& A, const osg::Vec3& B, const osg::Vec3& C, const osg::Vec3& D, bool& concav, int nbLoop) const {
    if (nbLoop == 4) {
        return;
    } else {
        osg::Vec3 AB(B-A);
        osg::Vec3 AC(C-A);
        osg::Vec3 AD(D-A);

        double BAC = std::atan2(crossProduct(AB, AC).length(), dotProduct(AB, AC));
        double CAD = std::atan2(crossProduct(AC, AD).length(), dotProduct(AC, AD));

        qDebug() << "Test" << nbLoop << (BAC + CAD);

        concav = concav || (std::abs(BAC + CAD) >= M_PI);

        detectConcav(B, C, D, A, concav, nbLoop+1);
    }
}

void LDrawParser::writeFaultyParts(QString fileName) const {
    QString fileFaulty = "/home/shaolan/Documents/Qt/LViewer/config/FaultParts.ldr";
    QFile file(fileFaulty);
    if (!file.open(QIODevice::Append | QIODevice::Text)) {
        qDebug() << "Error while opening in read only " + fileFaulty + " file within main";
    }

    // Get text stream
    QTextStream inFile(&file);
    inFile.setCodec("UTF-8");

    fileName = fileName.split("parts/").last();

    // Add section name to file
    inFile << "0 !CATEGORY " << fileName << " !DESCRIPTION none\n";
}
