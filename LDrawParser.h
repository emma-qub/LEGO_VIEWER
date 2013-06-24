#ifndef LDRAWPARSER_H
#define LDRAWPARSER_H

#include <QString>
#include <QColor>
#include <QMap>

#include <osg/Node>

class LDrawParser {

public:
    enum Winding { ccw, cw };
    enum Certified { yes, no, unknown };

public:
    static int tab;
    static QStringList _faultyParts;
    static int _nbBowtieQuads;

public:
    class OpenFailed : std::exception {
    public:
        const virtual char *what() const throw () {
            return "Cannot open file";
        }
    };

public:
    struct ColorParams {
        QString colorName;
        QString surfValue;
        QString edgeValue;
        double alphaValue;
    };

public:
    LDrawParser(void);
    LDrawParser(const LDrawParser& lDrawParser);
    virtual ~LDrawParser(void);

    static osg::Vec3 calculateNormal(const osg::Vec3& a, const osg::Vec3& b, const osg::Vec3& c);
    static osg::Vec3 multMatVec(const osg::Vec3& vec, const osg::Matrix& mat);

    osg::Group* createNode(const QString &fileName);
    void fillArrays(QString fileName, bool accumCull, bool accumInvert, osg::Matrix accumTransformMatrix,
                    osg::Vec3Array* lineVerticesArray, osg::Vec3Array* triangleVerticesArray, osg::Vec3Array* quadVerticesArray,
                    osg::Vec4Array* lineColorsArray, osg::Vec4Array* triangleColorsArray, osg::Vec4Array* quadColorsArray, int currColor = 16);

    QMap<int, ColorParams> getColorsArray(void) const { return _colorsArray; }
    QStringList getCategoriesArray(void) const { return _categoriesArray; }

    void setCurrColorId(int currColorId) { _currColorId = currColorId; }

private:
    void fillColorsArray(void);
    void fillCategoriesArray(void);
    QString getSurfQString(int colorId);
    QString getEdgeQString(int colorId);
    osg::Vec4 getSurfOrEdgeColor(int colorId, bool isSurf = true);
    double getAlphaValue(int colorId);
    void detectCrossing(const osg::Vec3 A, const osg::Vec3 B, const osg::Vec3 C, const osg::Vec3 D, bool& cross1, bool& cross2) const;
    void detectConcav(const osg::Vec3& A, const osg::Vec3& B, const osg::Vec3& C, const osg::Vec3& D, bool& concav, int nbLoop) const;
//    double determinant(const osg::Vec2 u, const osg::Vec2 v) const;
//    double mixteProduct(const osg::Vec3 u, const osg::Vec3 v, const osg::Vec3 w) const;
    osg::Vec3 crossProduct(const osg::Vec3 u, const osg::Vec3 v) const;
    double dotProduct(const osg::Vec3 u, const osg::Vec3 v) const;
    void writeFaultyParts(QString fileName) const;

private:
    int _currColorId;

public:
    bool _isBowtieActivated;
    bool _isBowtieCorrected;
    bool _isNonPlanarActivate;
    bool _isNonPlanarCorected;

private:
    QMap<int, ColorParams> _colorsArray;
    QStringList _categoriesArray;
};

#endif // LDRAWPARSER_H
