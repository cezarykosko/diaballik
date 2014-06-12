#include "tile.h"

Tile::Tile(int id)
{
    position = id;
    coord1 = numToCoord1(id);
    coord2 = numToCoord2(id);
    isAvail = false;
    isClicked = false;
    resident = NULL;
}

// pozycja pola na planszy
int Tile::getPosition()
{
    return position;
}

// sprawdzenie czy pole zostało oznakowane
bool Tile::isAvailable()
{
    return isAvail;
}

// potwierdzenie kliknięcia na pole
void Tile::confirmClick()
{
    isClicked = !isClicked;
    update();
}

// oznakowanie pola
void Tile::makeAvailable()
{
    isAvail = !isAvail;
    update();
}

// ustalenie pionka na polu
void Tile::setResident(Pawn *pawn)
{
    if(pawn != NULL){
        pawn->setPos(position);}
    resident=pawn;
}

// wskaźnik do pionka, który zajmuje pole
Pawn* Tile::getResident()
{
    return resident;
}

// element QGraphicsItem
QRectF Tile::boundingRect() const
{
    return QRect(coord1,coord2,TILE_SIZE,TILE_SIZE);
}

// element QGraphicsItem - malowanie pola
void Tile::paint(QPainter *painter,
                 const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QRectF bounds = boundingRect();
    QBrush background(Qt::gray);
    if(isClicked) {
        background.setColor(Qt::yellow);
    } else if(isAvail) {
        background.setColor(Qt::green);
    }

    painter->fillRect(bounds,background);
    painter->drawRect(bounds);
}

// obsługa kliknięcia myszą
void Tile::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    emit onClick();
    QGraphicsItem::mousePressEvent(event);
}
