#include "pawn.h"

typedef bool team;

Pawn::Pawn(team whichTeam, int pos, bool hasBall, QGraphicsView *gV, QWidget *port)
{
    teamCol = whichTeam;
    position = pos;
    coord1 = numToCoord1(pos);
    coord2 = numToCoord2(pos);
    ball = hasBall;
    graphicsView = gV;
    viewport = port;
}

// ustalenie pozycji pionka na planszy
void Pawn::setPos(int num, bool doPaint)
{
    position = num;
    if (doPaint) {
    prepareGeometryChange();
    coord1 = numToCoord1(num);
    prepareGeometryChange();
    coord2 = numToCoord2(num);
        graphicsView->repaint();
        viewport->update();
    }
}

// wyciągnięcie pozycji pionka na planszy
int Pawn::getPos()
{
    return position;
}

// sprawdzenie, czy pionek jest oznaczony kółkiem
bool Pawn::hasBall()
{
    return ball;
}

// sprawdzenie koloru pionka
team Pawn::getColour()
{
    return teamCol;
}

// element QGraphicsItem
QRectF Pawn::boundingRect() const
{
    return QRect(coord1,coord2,TILE_SIZE,TILE_SIZE);
}

// element QGraphicsItem - kolorowanie pionka
void Pawn::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    int boundsCoord1 = coord1 + DIST_FROM_BOUND;
    int boundsCoord2 = coord2 + DIST_FROM_BOUND;
    QRect bounds = QRect(boundsCoord1,boundsCoord2,PAWN_SIZE,PAWN_SIZE);

    QBrush background(Qt::gray);
    if(teamCol) {
        background.setColor(Qt::blue);
    } else {
        background.setColor(Qt::red);
    }

    painter->fillRect(bounds,background);

    if(ball)
        painter->drawEllipse(QRect(boundsCoord1+DIST_FROM_PAWNS_BOUND,
                                   boundsCoord2+DIST_FROM_PAWNS_BOUND,
                                   BALL_DIAM,BALL_DIAM));
    painter->drawRect(bounds);
}
