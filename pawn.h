#ifndef PAWN_H
#define PAWN_H
#include<QtGui>
#include<QtCore>
#include<QGraphicsItem>
#include<QGraphicsView>
#include<QPainter>
#include<metrics.h>

// klasa reprezentująca pionka
class Pawn : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    // stałe reprezentujące kolory drużyn
    typedef bool team;
    static const team RED = false;
    static const team BLUE = true;

    Pawn(team whichTeam, int pos, bool hasBall, QGraphicsView *gV, QWidget *port);

    // operacje na pozycji pionka na planszy
    void setPos(int num);
    int getPos();

    // akcesory danych pionka
    bool hasBall();
    team getColour();

    // elementy QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    // stałe potrzebne do rysowania pionka
    static const int PAWN_SIZE = 35;
    static const int TILE_SIZE = 49;
    static const int DIST_FROM_BOUND = (TILE_SIZE - PAWN_SIZE) / 2;
    const float BALL_RADIUS = 10;
    const int BALL_DIAM = 2 * BALL_RADIUS;
    const int DIST_FROM_PAWNS_BOUND = (PAWN_SIZE - BALL_DIAM) / 2;

    // dane pionka - pozycja, kolor, bycie oznaczonym kółkiem
    int position;
    int coord1;
    int coord2;
    team teamCol;
    bool ball;

    // wskaźniki do obsługi wyświetlania pionka
    QGraphicsView *graphicsView;
    QWidget *viewport;

};

#endif // PAWN_H
