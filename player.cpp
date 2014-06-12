#include "player.h"

typedef bool team;

// reprezentant braku ruchu
static QPair<int, int> PASS = QPair<int, int>(-1, -1);

Player::Player(bool col, bool ai, Tile *board[TILES_NUMBER], Pawn *pwns[PAWNS_NUMBER])
{
    colour = col;
    isAi = ai;
    for(int i = 0; i < TILES_NUMBER; i++) {
        tiles[i] = board[i];
    }
    for(int i = 0; i < PAWNS_NUMBER; i++) {
        pawns[i] = pwns[i];
    }
}

// sprawdzenie, czy graczem steruje komputer
bool Player::isArti()
{
    return isAi;
}

// sprawdzenie koloru gracza
team Player::getCol()
{
    return colour;
}

// zwrócenie ruchu, który ma być wykonany
QPair<int, int> Player::makeMove(bool movesValid, bool passesValid)
{
    bestGrade = START_GRADE;
    bestPair = PASS;
    for(int i = 0; i < TEAM_NUMBER; i++) {
        int temp;
        if(colour) temp = 0;
        else temp = TEAM_NUMBER;
        if(i == PAWN_W_BALL) {
            if(passesValid) {
                pushAvailwBall(pawns[i+temp]);
            }
        }
        else {
            if(movesValid) {
                pushAvailwoBall(pawns[i+temp]);
            }
        }
    }
    return bestPair;
}

// funkcja pomocnicza makeMove - rozpatrzenie przesunięć
void Player::pushAvailwoBall(Pawn *pawn)
{
    for(int i = 0; i < DIRS_NUM; i++) {
        int temp = pawn->getPos() + DIRS[i];
        if(dist(temp, pawn->getPos()) == 1 && temp >= 0) {
            if(tiles[temp]->getResident() == NULL) {
                QPair<int, int> pair = QPair<int, int>(pawn->getPos(), temp);
                if(gradePair(pair) > bestGrade) {
                    bestPair = pair;
                    bestGrade = gradePair(pair);
                }
            }
        }
    }
}

/* funkcja pomocnicza makeMove - rozpatrzenie podań - zamiany miejscami
 * pionka oznakowanego kółkiem i zwykłego pionka
 */
void Player::pushAvailwBall(Pawn *pawn)
{
    for(int i = 0; i < ALL_DIRS_NUM; i++) {
        QVector<int> targs =
                passDirs(pawn->getColour(), ALL_DIRS[i], pawn->getPos());
        for(int j = 0; j < targs.size(); j++) {
            QPair<int, int> pair = QPair<int, int>(pawn->getPos(), targs[j]);
            if(gradePair(pair) > bestGrade) {
                bestPair = pair;
                bestGrade = gradePair(pair);
            }
        }
    }
}

/* funkcja pomocnicza pushAvailwBall - zwraca nry pól, na które można wykonać
 * podanie w danym kierunku
 */
QVector<int> Player::passDirs(team colour, int dir, int now)
{
    QVector<int> stack = QVector<int>();
    if(dist(now, now + dir) <= 2 &&
            now + dir >= 0 && now + dir <= TILES_NUMBER) {
        Pawn *temp = tiles[now + dir]->getResident();
        if(temp != NULL) {
            if(temp->getColour() == colour) {
                QVector<int> iterStack = passDirs(colour, dir, now + dir);
                for(int i = 0; i < iterStack.size(); i++) {
                    stack.push_back(iterStack[i]);
                }
                stack.push_back(now + dir);
                return stack;
            } else return stack;
        } else {
            return passDirs(colour, dir, now + dir);
        }
    } else {
        return stack;
    }
}

// wywołanie funkcji oceniającej - analiza ruchu
int Player::gradePair(QPair<int, int> pair)
{
    int first, second;
    if(tiles[pair.first]->getResident()->getColour()) {
        first = pair.first;
        second = pair.second;
    } else {
        first = TILES_NUMBER - 1 - pair.first;
        second = TILES_NUMBER - 1 - pair.second;
    }
    if(tiles[pair.second]->getResident() != NULL) {
        int gradeAfter = NO_BALL_GRADE[first] + BALL_GRADE[second];
        int gradeBefore = NO_BALL_GRADE[second] + BALL_GRADE[first];
        return gradeAfter - gradeBefore;
    } else {
        return NO_BALL_GRADE[second] - NO_BALL_GRADE[first];
    }
}
