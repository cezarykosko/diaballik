#ifndef TILE_H
#define TILE_H
#include<QtGui>
#include<QMainWindow>
#include<QtCore>
#include<QGraphicsItem>
#include<QGraphicsView>
#include<QPainter>
#include<metrics.h>
#include<pawn.h>

// klasa reprezentująca pole planszy
class Tile : public QObject, public QGraphicsItem
{
    Q_OBJECT

public:
    // rozmiar pola
    static const int TILE_SIZE = 49;

    Tile(int i);

    // akcesory pozycji i oznakowania
    int getPosition();
    bool isAvailable();

    // potwierdzenie kliknięcia i oznakowanie jako możliwy kierunek ruchu
    void confirmClick();
    void makeAvailable();

    // obsługa pionka na polu (lub jego braku)
    void setResident(Pawn *pawn, bool doPaint);
    Pawn* getResident();

    // elementy QGraphicsItem
    QRectF boundingRect() const;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option, QWidget *widget);



protected:
    // obsługa kliknięcia
    void mousePressEvent(QGraphicsSceneMouseEvent *event);

private:
    // pozycja pola w planszy
    int position;
    int coord1;
    int coord2;

    // oznakowanie pola
    bool isAvail;
    bool isClicked;

    // wskaźnik do pionka na polu
    Pawn *resident;

signals:
    // sygnał informujący o kliknięciu
    void onClick();
};

#endif // TILE_H
