#ifndef PLAYER_H
#define PLAYER_H
#include<pawn.h>
#include<metrics.h>
#include<tile.h>
#include<QVector>
#include<QPair>

typedef bool team;

// klasa reprezentująca gracza
class Player
{
public:
    // stałe do obliczeń sztucznej inteligencji
    static const int TEAM_NUMBER = 7;
    static const int PAWNS_NUMBER = 2 * TEAM_NUMBER;
    static const int PAWN_W_BALL = 3;

    Player(team col, bool ai, Tile *board[TILES_NUMBER], Pawn *pwns[PAWNS_NUMBER]);

    // własności gracza - czy jest komputerem i jaki ma kolor
    bool isArti();
    team getCol();

    // funkcja zwracająca ruch wytworzony przez AI
    QPair<int, int> makeMove(bool movesValid, bool passesValid);


private:
    // dane pionka
    team colour;
    bool isAi;

    // wskaźniki do obecnego stanu gry
    Tile *tiles[TILES_NUMBER];
    Pawn *pawns[PAWNS_NUMBER];

    // funkcja oceniająca
    const int START_GRADE = -300000;
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

    /* funkcje pomocnicze wyznaczania ruchu - przetwarzanie przesunięć,
     * podań i ocena stanu.
     * dalej zmienne trzymające najlepszy aktualnie stan możliwy do uzyskania.
     */
    void pushAvailwoBall(Pawn *pawn);
    void pushAvailwBall(Pawn *pawn);
    QVector<int> passDirs(team colour, int dir, int now);
    int gradePair(QPair<int, int> pair);
    int bestGrade;
    QPair<int, int> bestPair;
};

#endif // PLAYER_H
