#ifndef PLAYER_H
#define PLAYER_H
#include<pawn.h>
#include<metrics.h>
#include<tile.h>
#include<QVector>
#include<QPair>
#include<QSet>
#include<QMessageBox>

typedef bool team;

// klasa reprezentująca gracza
class Player
{
public:
    // stałe do obliczeń sztucznej inteligencji
    static const int TEAM_NUMBER = 7;
    static const int PAWNS_NUMBER = 2 * TEAM_NUMBER;
    static const int PAWN_W_BALL = 3;
    static const int TILE_SIZE = 49;

    Player(team col, bool ai, Tile *board[TILES_NUMBER], Pawn *pwns[PAWNS_NUMBER], QMainWindow *window);

    // własności gracza - czy jest komputerem i jaki ma kolor
    bool isArti();
    team getCol();

    void setOpponent(Player *opp);

    // funkcja zwracająca ruch wytworzony przez AI
    QVector<QPair<int, int> > makeMove();

    int alphabeta(int alpha, int beta, int depth, team currPlayer);

    bool checkUnfairLine(team player);

private:
    // dane pionka
    team colour;
    bool isAi;

    // wskaźniki do obecnego stanu gry
    Tile *tiles[TILES_NUMBER];
    Pawn *pawns[PAWNS_NUMBER];
    Player *opponent;

    // głębokość przeszukiwania drzewa gry
    const int ALPHABETA_DEPTH = 2;

    // wartość bezwzględna początkowych kresów przeszukiwania alfabety
    const int START_GRADE = 6000;

    // funkcja oceniająca
    const int NO_BALL_GRADE[TILES_NUMBER] = {
        3,   1,   1,   1,   1,   1,   3,
        4,   4,   2,   2,   2,   4,   4,
        5,   5,   5,   3,   5,   5,   5,
        6,   6,   6,   8,   6,   6,   6,
        7,   7,   9,   9,   9,   7,   7,
        8,  10,  10,  10,  10,  10,   8,
        20,  23,  26,  30,  26,  23,  20
    };
    const int BALL_GRADE[TILES_NUMBER] = {
        100,  50,  50,  50,  50,  50, 100,
        120, 120,  60,  60,  60, 120, 120,
        140, 200, 200,  70, 200, 200, 140,
        160, 160, 160, 240, 160, 160, 160,
        180, 180, 260, 260, 260, 180, 180,
        200, 300, 300, 300, 300, 300, 200,
        3000,3000,3000,3000,3000,3000,3000
    };

    // oznaczenia typów rezydentów pól - do funkcji hashującej
    const int EMPTY = 0;
    const int RED_W_BALL = 1;
    const int BLUE_W_BALL = 2;
    const int RED_WO_BALL = 3;
    const int BLUE_WO_BALL = 4;
    static const int RESIDENTS_NUMBER = 5;


    const int MAX_PASSES = 1;
    const int MAX_MOVES = 2;

    // funkcje pomocnicze wyznaczania ruchu - przetwarzanie przesunięć i podań
    void pushAvailwoBall(Pawn *pawn, QVector<QPair<int, int> > *results);
    void pushAvailwBall(Pawn *pawn, QVector<QPair<int, int> > *results);
    QVector<int> passDirs(team colour, int dir, int now);

    // funkcje kolejkujące możliwe ruchy i tury
    QVector<QPair<int, int> > getMoves(bool movesValid, bool passesValid);
    QVector<QVector<QPair<int, int> > > getChildren();

    // funkcja oceniająca stan planszy
    int grade(team currPlayer);

    // tymczasowe wprowadzenie ruchów na planszę
    void move(Tile *a, Tile *b, bool isForward);
    void parseTurn(QVector<QPair<int, int> > turn, bool isForward);

    // hash obecnego stanu, funkcja i tablica hashująca
    unsigned long currentHash;
    unsigned long hash();
    unsigned long hashTable[TILES_NUMBER][RESIDENTS_NUMBER];

    // funkcja zwracająca typ rezydenta pola
    int analyzePawn(Pawn *pawn);

    // liczniki ruchów w turze
    int movesCounter;
    int passesCounter;

    // bool sprawdzający, czy AI ma przeszukiwać drzewo gry
    bool doProcess;

    // wskaźnik do okna głównego gry
    QMainWindow *ui;

public slots:
    // slot wywoływany, gdy AI ma przestać wykonywać obliczenia
    void interrupted();
};

#endif // PLAYER_H
