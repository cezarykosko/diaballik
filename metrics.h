#ifndef METRICS_H
#define METRICS_H

// pakiet z obliczeniami metrycznymi i stałymi dot. planszy

// stałe dot. planszy
static const int BOARD_SIZE = 7;
static const int TILES_NUMBER = BOARD_SIZE * BOARD_SIZE;
static const int UP = -7;
static const int DOWN = 7;
static const int LEFT = -1;
static const int RIGHT = 1;
static const int DIRS[] = {UP, DOWN, LEFT, RIGHT};
static const int ALL_DIRS[] = {UP, UP + RIGHT, RIGHT, RIGHT + DOWN,
                               DOWN, DOWN + LEFT, LEFT, LEFT + UP};
static const int DIRS_NUM = 4;
static const int ALL_DIRS_NUM = 2 * DIRS_NUM;

/* funkcje tworzące bijekcję między zbiorem 0,1,...,48 a współrzędnymi
 * lewych górnych wierzchołków pól
 */
int numToCoord1(int num);
int numToCoord2(int num);
int coordsToNum(int coord1, int coord2);

// odległość między dwoma polami
int dist(int num1, int num2);

#endif // METRYKA_H
