#include<QtCore>

// stałe opisujące wymiarów planszy
const int BOARD_SIZE = 7;
const int TILES_NUMBER = BOARD_SIZE * BOARD_SIZE;
const int TILE_SIZE = 49;

// wyciągnięcie pierwszej współrzędnej z liczby opisującej pozycję na planszy
int numToCoord1(int num)
{
    return TILE_SIZE * (num % BOARD_SIZE);
}

// wyciągnięcie drugiej współrzędnej z liczby opisującej pozycję na planszy
int numToCoord2(int num)
{
    return TILE_SIZE * ((num / BOARD_SIZE) % BOARD_SIZE);
}

// przetworzenie współrzędnych na pozycję na planszy
int coordsToNum(int coord1, int coord2)
{
    return (coord1 + BOARD_SIZE * coord2) / TILE_SIZE;
}

// odległość między dwoma polami na planszy w metryce miejskiej
int dist(int num1, int num2)
{
    return (abs((numToCoord1(num1)) - (numToCoord1(num2))) +
            abs((numToCoord2(num1)) - (numToCoord2(num2)))) / TILE_SIZE;
}

// maksimum dwóch liczb
int max(int a, int b)
{
    if(a > b)
        return a;
    else
        return b;
}

// minimum dwóch liczb
int min(int a, int b)
{
    if(a < b)
        return a;
    else
        return b;
}
