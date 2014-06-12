#include "player.h"

typedef bool team;

// reprezentant braku ruchu
static QVector<QPair<int, int> > PASS = QVector<QPair<int, int> >();

Player::Player(bool col, bool ai, Tile *board[TILES_NUMBER], Pawn *pwns[PAWNS_NUMBER], QMainWindow *window)
{
    colour = col;
    isAi = ai;

    // podpięcie planszy
    for(int i = 0; i < TILES_NUMBER; i++) {
        tiles[i] = board[i];

        // wylosowanie tablicy hashującej
        for(int j = 0; j < RESIDENTS_NUMBER; j++) {
            hashTable[i][j] = qrand();
        }
    }

    // podpięcie pionków
    for(int i = 0; i < PAWNS_NUMBER; i++) {
        pawns[i] = pwns[i];
    }

    doProcess = true;
    ui = window;
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

// ustalenie wskaźnika na przeciwnika
void Player::setOpponent(Player *opp)
{
    opponent = opp;
}

// zwrócenie ruchu, który ma być wykonany
QVector<QPair<int, int> > Player::makeMove()
{
    QVector<QVector<QPair<int, int> > > children = getChildren();
    QVector<QPair<int, int> > bestChild = QVector<QPair<int, int> >();
    int bestGrade = -START_GRADE;

    // wywołanie alfabety dla każdego dziecka stanu obecnego
    while(!children.isEmpty()) {
        QVector<QPair<int, int> > temp = children.front();
        children.pop_front();
        parseTurn(temp, true);
        int grade = opponent->alphabeta(-START_GRADE, START_GRADE,
                                        ALPHABETA_DEPTH - 1, colour);
        parseTurn(temp,false);
        QEventLoop loop;
        loop.setParent(ui);
        loop.processEvents(QEventLoop::AllEvents);
        // jeśli obliczenia są przerwane, funkcja zwraca pas
        if(!doProcess)
            return PASS;
        if(grade > bestGrade) {
            bestGrade = grade;
            bestChild = temp;
        }
    }

    return bestChild;
}

// implementacja alfa-beta odcięcia
int Player::alphabeta(int alpha, int beta, int depth, team currPlayer)
{
    int r = 0;
    if (depth == 0)
        // funkcja oceniająca nie sprawdza, czy została wykonana linia
        // nie do przejścia, więc sprawdzenie wykonywane jest w algorytmie
        if(checkUnfairLine(currPlayer))
            return -START_GRADE;
        else
            return grade(currPlayer);
    else {
        QVector<QVector<QPair<int, int> > > children = this->getChildren();
        if(currPlayer == colour) {
            r -= START_GRADE;
            while(!children.isEmpty()){

                QVector<QPair<int, int> > temp = children.front();
                children.pop_front();

                parseTurn(temp, true);
                int grade = opponent->alphabeta(max(r, alpha), beta, depth - 1, currPlayer);
                parseTurn(temp, false);

                if(grade >= beta)
                    return grade;

                r = max(r, grade);
            }
        } else {
            r += START_GRADE;
            while(!children.isEmpty()){

                QVector<QPair<int, int> > temp = children.front();
                children.pop_front();

                parseTurn(temp, true);
                int grade = opponent->alphabeta(alpha, min(r, beta), depth - 1, currPlayer);
                parseTurn(temp, false);

                if(grade <= alpha)
                    return grade;

                r = min(r, grade);
            }
        }
        return r;
    }
}

// sprawdzenie czy gracz player ułożył linię nie do przejścia
bool Player::checkUnfairLine(team player)
{
    Pawn* columns[TEAM_NUMBER];
    for(int i = 0; i < BOARD_SIZE; i++) {
        columns[i] = NULL;
    }
    for(int i = 0; i < TEAM_NUMBER; i++) {
        if(player) {
            columns[numToCoord1(pawns[i]->getPos()) / TILE_SIZE] = pawns[i];
        } else {
            int j = PAWNS_NUMBER - 1 - i;
            columns[numToCoord1(pawns[j]->getPos()) / TILE_SIZE] = pawns[j];
        }
    }

    // sprawdzenie, czy w każdej kolumnie znajduje się pionek gracza
    bool checkFilling = true;
    for(int i = 0; i < BOARD_SIZE; i++) {
        checkFilling = checkFilling && columns[i] != NULL;
    }
    if(checkFilling) {

        //sprawdzenie, czy pionki tworzą linię nie do przejścia
        bool checkLine = true;
        for(int i = 0; i < BOARD_SIZE - 1; i++) {
            checkLine = checkLine && dist(columns[i]->getPos(),
                                          columns[i + 1]->getPos()) <= 2;
        }
        if(checkLine) {

            //sprawdzenie, czy 3 pionki przeciwnika sąsiadują z linią
            int countAdjacent = 0;
            int dir;
            if(player) dir = DOWN;
            else dir = UP;
            for(int i = 0; i < BOARD_SIZE; i++) {
                if(tiles[columns[i]->getPos() + dir]->getResident() != NULL){
                    countAdjacent++;
                }
            }
            if(countAdjacent >= 3){
                return true;
            }
        }
    }
    return false;
}

// funkcja pomocnicza makeMove - rozpatrzenie przesunięć
void Player::pushAvailwoBall(Pawn *pawn, QVector<QPair<int, int> > *results)
{
    for(int i = 0; i < DIRS_NUM; i++) {
        int temp = pawn->getPos() + DIRS[i];
        if(dist(temp, pawn->getPos()) == 1 && temp >= 0) {
            if(tiles[temp]->getResident() == NULL) {
                results->push_back(QPair<int, int>(pawn->getPos(), temp));
            }
        }
    }
}

/* funkcja pomocnicza makeMove - rozpatrzenie podań - zamiany miejscami
 * pionka oznakowanego kółkiem i zwykłego pionka
 */
void Player::pushAvailwBall(Pawn *pawn, QVector<QPair<int, int> > *results)
{
    for(int i = 0; i < ALL_DIRS_NUM; i++) {
        QVector<int> targs =
                passDirs(pawn->getColour(), ALL_DIRS[i], pawn->getPos());
        for(int j = 0; j < targs.size(); j++) {
            results->push_back(QPair<int, int>(pawn->getPos(), targs[j]));
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

// funkcja zwracająca wszystkie pojedyncze ruchy możliwe w danej chwili
QVector<QPair<int, int> > Player::getMoves(bool movesValid, bool passesValid)
{
    QVector<QPair<int, int> > *results = new QVector<QPair<int, int> >();
    for(int i = 0; i < TEAM_NUMBER; i++) {
        int temp;
        if(colour) temp = 0;
        else temp = TEAM_NUMBER;
        if(i == PAWN_W_BALL) {
            if(passesValid) {
                pushAvailwBall(pawns[i+temp], results);
            }
        }
        else {
            if(movesValid) {
                pushAvailwoBall(pawns[i+temp], results);
            }
        }
    }
    return *results;
}

// funkcja zwracająca wszystkie stany, do których można dojść w danej chwili
QVector<QVector<QPair<int, int> > > Player::getChildren()
{
    // rozpatruję tylko stany o różnych hashach
    currentHash = hash();
    QSet<unsigned long long> doneHashes = QSet<unsigned long long>();
    doneHashes.insert(currentHash);

    passesCounter = 0;
    movesCounter = 0;

    QVector<QVector<QPair<int, int> > > result =
            QVector<QVector<QPair<int, int> > >();
    result.push_back(QVector<QPair<int, int> >());

    // przy użyciu getMoves tworzę wszystkie możliwe kolejne stany,
    // przed dodaniem każdego sprawdzam, czy jego hash nie znajduje się
    // w doneHashes
    for(int k = 0; k < MAX_PASSES + MAX_MOVES; k++) {
        QVector<QVector<QPair<int, int> > > nextIter =
                QVector<QVector<QPair<int, int> > >();
        while(!result.isEmpty()){
            QVector<QPair<int, int> > temp = result.front();
            result.pop_front();
            nextIter.push_back(temp);

            parseTurn(temp, true);

            QVector<QPair<int, int> > queue =
                    getMoves(movesCounter < MAX_MOVES,
                             passesCounter < MAX_PASSES);

            while(!queue.isEmpty()) {
                QPair<int, int> nextMove = queue.front();
                queue.pop_front();

                move(tiles[nextMove.first],
                        tiles[nextMove.second], true);

                if(!doneHashes.contains(currentHash)) {

                    doneHashes.insert(currentHash);
                    QVector<QPair<int, int> > turnToAdd =
                            QVector<QPair<int, int> >();
                    for(int j = 0; j < temp.size(); j++) {
                        turnToAdd.push_back(temp[j]);
                    }
                    turnToAdd.push_back(nextMove);
                    nextIter.push_back(turnToAdd);
                }

                move(tiles[nextMove.second],
                        tiles[nextMove.first], false);
            }

            parseTurn(temp, false);
        }

        result = nextIter;
    }

    return result;
}

// funkcja zwracająca ocenę danego stanu planszy dla danego gracza
// (podanie gracza niezbędne ze względu na obiektową implementację alfabety)
int Player::grade(team currPlayer)
{
    int result = 0;

    for(int i = 0; i < TILES_NUMBER; i++) {
        Pawn* temp = tiles[i]->getResident();
        if(temp != NULL) {

            int coord;
            int multiplier;

            if(temp->getColour())
                coord = i;
            else
                coord = TILES_NUMBER - 1 - i;

            if(temp->getColour() == currPlayer)
                multiplier = 1;
            else
                multiplier = -1;

            if(temp->hasBall())
                result += multiplier * BALL_GRADE[coord];
            else
                result += multiplier * NO_BALL_GRADE[coord];
        }
    }

    return result;
}

// wprowadzenie/usunięcie ruchu
void Player::move(Tile *a, Tile *b, bool isForward)
{
    int inc;
    if(isForward)
        inc = 1;
    else
        inc = -1;

    Pawn* fst = (a->getResident());
    Pawn* snd = (b->getResident());
    b->setResident(fst, false);
    a->setResident(snd, false);

    if(snd != NULL)
        passesCounter += inc;
    else
        movesCounter += inc;

    int analyzeFst = analyzePawn(fst);
    int analyzeSnd = analyzePawn(snd);
    currentHash +=
            (hashTable[a->getPosition()][analyzeSnd] -
            hashTable[a->getPosition()][analyzeFst]);
    currentHash +=
            (hashTable[b->getPosition()][analyzeFst] -
            hashTable[b->getPosition()][analyzeSnd]);
}

// wprowadzenie/usunięcie tury
void Player::parseTurn(QVector<QPair<int, int> > turn, bool isForward)
{
    for(int i = 0; i < turn.size(); i++) {
        int index = i;
        if(!isForward)
            index = turn.size() - 1 - i;
        int first = turn[index].first;
        int second = turn[index].second;
        if(isForward)
            move(tiles[first], tiles[second], true);
        else
            move(tiles[second], tiles[first], false);
    }
}

// funkcja hashująca dany stan planszy
unsigned long Player::hash()
{
    unsigned long result = 0;
    for(int i = 0; i < TILES_NUMBER; i++) {
        Pawn *temp = tiles[i]->getResident();
        result += hashTable[i][analyzePawn(temp)];
    }
    return result;
}

// zwrócenie typu rezydenta pola
int Player::analyzePawn(Pawn *pawn)
{
    int result;
    if(pawn == NULL)
        result = EMPTY;
    else {
        if(pawn->getColour()){
            if(pawn->hasBall())
                result = BLUE_W_BALL;
            else
                result = BLUE_WO_BALL;
        } else {
            if(pawn->hasBall())
                result = RED_W_BALL;
            else
                result = RED_WO_BALL;
        }
    }
    return result;
}

// przerwanie obliczeń
void Player::interrupted()
{
    doProcess = false;
}
