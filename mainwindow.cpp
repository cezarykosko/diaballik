#include "mainwindow.h"
#include "ui_mainwindow.h"

typedef bool team;

// stała reprezentująca brak ruchu
static QPair<int, int> PASS = QPair<int, int>(-1, -1);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // inicjalizacja wyświetlania planszy
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    viewport = ui->graphicsView->viewport();

    // wyłączenie wszystkich przycisków
    ui->endTurnButton->setEnabled(false);
    ui->undoButton->setEnabled(false);
    ui->nextTurnButton->setEnabled(false);
    ui->prevTurnButton->setEnabled(false);
    ui->turnEditButton->setEnabled(false);
    ui->hintButton->setEnabled(false);
    ui->actionSaveGame->setEnabled(false);

    // mapper sygnałów z pól
    clickMap = new QSignalMapper(this);

    // zmienne dot. stanu gry
    lastHighlighted = NULL;
    players[RED] = NULL;
    players[BLUE] = NULL;
    doParse = false;
    gameOn = false;
    editModeOn = false;

    // ustawienie pól na planszy
    for(int i = 0; i < TILES_NUMBER; i++) {
        tiles[i] = new Tile(i);
        tiles[i]->setZValue(-1);
        clickMap->setMapping(tiles[i], i);
        connect(tiles[i], SIGNAL(onClick()), clickMap, SLOT(map()));
        scene->addItem(tiles[i]);
    }

    // podłączenie sygnałów z pól i przycisków menu

    connect(clickMap, SIGNAL(mapped(int)), this, SLOT(parseClick(int)));

    connect(ui->actionSaveGame, SIGNAL(triggered()), this, SLOT(saveGame()));
    connect(ui->actionLoadGame, SIGNAL(triggered()), this, SLOT(loadGame()));

    connect(ui->actionHuman_vs_Human,SIGNAL(triggered()),
            this, SLOT(HumVSHum()));
    connect(ui->actionHuman_vs_AI, SIGNAL(triggered()), this, SLOT(HumVSAI()));
    connect(ui->actionAI_vs_Human, SIGNAL(triggered()), this, SLOT(AIVSHum()));
    connect(ui->actionAI_vs_AI, SIGNAL(triggered()), this, SLOT(AIVSAI()));

    connect(ui->actionGameRules, SIGNAL(triggered()), this, SLOT(gameRules()));


    //ustawienie pionków na planszy
    for(int i = 0; i < TEAM_NUMBER; i++) {
        pawns[i] = new Pawn(BLUE, i, (i == PAWN_W_BALL),
                            ui->graphicsView, viewport);
        scene->addItem(pawns[i]);
        pawns[PAWNS_NUMBER - 1 - i] =
                new Pawn(RED, TILES_NUMBER - 1 - i,
                         (i == PAWN_W_BALL), ui->graphicsView, viewport);
        scene->addItem(pawns[PAWNS_NUMBER - 1 - i]);
        tiles[i]->setResident(pawns[i]);
        tiles[TILES_NUMBER - 1 - i]->setResident(pawns[PAWNS_NUMBER - 1 - i]);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    for(int i = 0; i < TILES_NUMBER; i++) {
        delete tiles[i];
    }
    for(int i = 0; i < PAWNS_NUMBER; i++) {
        delete pawns[i];
    }
    if(players[0] != NULL)
        delete players[0];
    if(players[1] != NULL)
        delete players[1];
}

// wykonanie ruchu między polami a i b
void MainWindow::makeMove(Tile* a, Tile* b)
{
    Pawn* temp = (a->getResident());
    a->setResident(b->getResident());
    b->setResident(temp);
}

// oznaczenie pól, na które można wykonać ruch
void MainWindow::markAvailable()
{
    int temp = lastHighlighted->getPosition();
    if(lastHighlighted->getResident()->hasBall()) {
        if(passCounter < MAX_PASSES) {
            for(int i = 0; i < ALL_DIRS_NUM; i++) {
                markPassDirs(lastHighlighted->getPosition(),
                             lastHighlighted->getResident()->getColour(),
                             ALL_DIRS[i]);
            }}
    } else {
        if(moveCounter < MAX_MOVES) {
            for(int i = 0; i < DIRS_NUM; i++) {
                if(dist(temp, temp + DIRS[i]) == 1 && temp + DIRS[i] >= 0 &&
                        tiles[temp + DIRS[i]]->getResident() == NULL)
                    tiles[temp + DIRS[i]]->makeAvailable();
            }
        }
    }
}


// oznaczenie wszystkich wykonalnych zamian pionka z kółkiem w danym kierunku
void MainWindow::markPassDirs(int num, team colour, int dir)
{
    int temp = num + dir;
    if(dist(num, temp) <= 2 && temp >= 0) {
        if(tiles[temp]->getResident() == NULL) {
            markPassDirs(temp, colour, dir);
        } else if (tiles[temp]->getResident()->getColour() == colour) {
            tiles[temp]->makeAvailable();
            markPassDirs(temp, colour, dir);
        }
    }
}

// oznaczenie wszystkich pól oprócz wybranego
void MainWindow::markAll()
{
    for(int i = 0; i < TILES_NUMBER; i++) {
        if(tiles[i] != lastHighlighted)
            tiles[i]->makeAvailable();
    }
}

/* zainicjowanie nowej gry - zmienne red i blue informują o tym,
 * czy dany gracz jest graczem komputerowym, czy nie
 */
void MainWindow::newGame(bool red, bool blue)
{
    // zainicjowanie graczy
    if(players[0] != NULL)
        delete players[0];
    players[0] = new Player(0, red, tiles, pawns);
    if(players[1] != NULL)
        delete players[1];
    players[1] = new Player(1, blue, tiles, pawns);

    // ustawienie pionków na pozycjach startowych
    for(int i=0; i < TEAM_NUMBER; i++) {
        tiles[pawns[i]->getPos()]->setResident(tiles[i]->getResident());
        tiles[pawns[PAWNS_NUMBER - 1 - i]->getPos()]->
                setResident(tiles[TILES_NUMBER - 1 - i]->getResident());
        tiles[i]->setResident(pawns[i]);
        pawns[i]->setPos(i);
        tiles[TILES_NUMBER - 1 - i]->setResident(pawns[PAWNS_NUMBER - 1 - i]);
        pawns[PAWNS_NUMBER - 1 - i]->setPos(TILES_NUMBER - 1 - i);
    }

    // zainicjowanie stosów przechowujących stany gry
    State start = {QVector<QPair<int, int> >(), BLUE, false};
    past = QStack<State>();
    past.push(start);
    game = QStack<State>();
    game.push(start);
    future = QStack<State>();

    // ustawienie przycisków
    ui->endTurnButton->setEnabled(true);
    ui->nextTurnButton->setEnabled(false);
    ui->prevTurnButton->setEnabled(false);
    ui->turnEditButton->setEnabled(true);
    ui->actionSaveGame->setEnabled(true);

    // rozpoczęcie pierwszej tury
    currPlayer = players[0];
    currTurn = {QVector<QPair<int, int> >(), currPlayer->getCol(), false};
    gameOn = true;
    startTurn();
}

// rozpoczęcie nowej tury
void MainWindow::startTurn()
{
    // ustawienie stanów przycisków
    ui->undoButton->setEnabled(false);
    ui->hintButton->setEnabled(!currPlayer->isArti());

    // wyzerowanie liczników ruchów
    moveCounter = 0;
    passCounter = 0;

    // inicjalizacja stanu aktualnej tury
    currTurn = {QVector<QPair<int, int> >(), currPlayer->getCol(), false};

    /* rozpoczęcie - jeśli obecny gracz jest graczem sterowanym przez komputer,
     * zostanie wykonany ruch
     */
    dispCurrState();
    if(currPlayer->isArti()) {
        doParse = false;
        AITurn();
        ui->undoButton->setEnabled(false);
    } else {
        doParse = true;
    }
}

// wykonanie ruchu przez AI
void MainWindow::AITurn()
{
    for(int i = 0; i < MAX_PASSES + MAX_MOVES; i++) {
        QPair<int, int> move =
                currPlayer->makeMove(moveCounter < MAX_MOVES,
                                     passCounter < MAX_PASSES);
        if(move != PASS) {
            doParse = true;
            parseClick(move.first);
            parseClick(move.second);
            doParse = !(currPlayer->isArti());
        }
    }
}

// parsowanie kliknięcia, gdy nie ma wybranego pola
void MainWindow::parseClickNoHighlight(int i)
{
    if(tiles[i]->getResident() != NULL) {
        if(!editModeOn){
            if(tiles[i]->getResident()->getColour() == currPlayer->getCol()) {
                if(tiles[i]->getResident()->hasBall()) {
                    if(passCounter < MAX_PASSES) confirm(i);
                } else if(moveCounter < MAX_MOVES) confirm(i);
            }
        } else {
            tiles[i]->confirmClick();
            lastHighlighted = tiles[i];
            markAll();
        }
    }
}

// parsowanie kliknięcia po wybraniu pola
void MainWindow::parseClickWHighlight(int i)
{
    if(!editModeOn) {
        if(tiles[i]->isAvailable()) {
            lastHighlighted->confirmClick();
            markAvailable();
            makeMove(lastHighlighted, tiles[i]);
            currTurn.moves.push_back(QPair<int, int>
                                     (lastHighlighted->getPosition(),i));
            lastHighlighted = NULL;
            if(tiles[i]->getResident()->hasBall()) {
                passCounter += 1;
            } else {
                moveCounter += 1;
            }
            checkEndgame();
            ui->undoButton->setEnabled(gameOn);
            dispCurrState();
        } else if(tiles[i] == lastHighlighted) {
            tiles[i]->confirmClick();
            markAvailable();
            lastHighlighted = NULL;
        } else {
            lastHighlighted->confirmClick();
            markAvailable();
            lastHighlighted = NULL;
            parseClick(i);
        }
    } else {
        lastHighlighted->confirmClick();
        markAll();
        makeMove(lastHighlighted, tiles[i]);
        currTurn.moves.push_back(QPair<int, int>
                                 (lastHighlighted->getPosition(),i));
        lastHighlighted = NULL;
        dispCurrState();
    }
}


// zatwierdzenie kliknięcia i-tego pola
void MainWindow::confirm(int i)
{
    tiles[i]->confirmClick();
    lastHighlighted = tiles[i];
    markAvailable();
}

// zakończenie gry
void MainWindow::endgame(team winner)
{
    // komunikat o zakończeniu gry
    QString temp;
    if(winner) {
        temp = "niebieski";
    } else {
        temp = "czerwony";
    }
    QMessageBox::information(this,"Koniec gry","Wygrywa "+temp+"!");

    // odłączenie funkcji dostępnych w trakcie rozgrywki
    gameOn = false;
    ui->endTurnButton->setEnabled(false);
    ui->undoButton->setEnabled(false);
    ui->turnEditButton->setEnabled(false);
}

// sprawdzenie, czy któryś z graczy wygrał w standardowy sposób
void MainWindow::checkRegularWin()
{
    if(numToCoord2(pawns[PAWN_W_BALL]->getPos()) ==
            TILE_SIZE * (BOARD_SIZE - 1)) {
        endgame(BLUE);
    } else if(numToCoord2(pawns[PAWNS_NUMBER - 1 -
                          PAWN_W_BALL]->getPos()) ==  0) {
        endgame(RED);
    }
}

// sprawdzenie czy gracz przegrał przez stworzenie linii nie do przejścia
void MainWindow::checkUnfairLine(team colour)
{
    Pawn* columns[TEAM_NUMBER];
    for(int i = 0; i < BOARD_SIZE; i++) {
        columns[i] = NULL;
    }
    for(int i = 0; i < TEAM_NUMBER; i++) {
        if(colour) {
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
            if(colour) dir = DOWN;
            else dir = UP;
            for(int i = 0; i < BOARD_SIZE; i++) {
                if(tiles[columns[i]->getPos() + dir]->getResident() != NULL){
                    countAdjacent++;
                }
            }
            if(countAdjacent >= 3){
                endgame(!colour);
            }
        }

    }
}

// sprawdzenie, czy gra została zakończona
void MainWindow::checkEndgame()
{
    checkRegularWin();
    checkUnfairLine(currPlayer->getCol());
    checkUnfairLine(!currPlayer->getCol());
}

// przypisanie współrzędnej znaku
char MainWindow::parseCoord(int coord, char array[7])
{
    return array[coord / TILE_SIZE];
}

// przypisanie współrzędnej wiersza
char MainWindow::parseRow(int coord)
{
    char temp[BOARD_SIZE] = {'7', '6', '5', '4', '3', '2', '1'};
    return parseCoord(coord, temp);
}

// przypisanie współrzędnej kolumny
char MainWindow::parseColumn(int coord)
{
    char temp[BOARD_SIZE] = {'A', 'B', 'C', 'D', 'E', 'F', 'G'};
    return parseCoord(coord, temp);
}

// przypisanie pozycji informacji o kolumnie i wierszu
QString MainWindow::parseNum(int num)
{
    return QString(parseColumn(numToCoord1(num))) +
            QString(parseRow(numToCoord2(num)));
}

// wyświetlenie obecnego stanu gry
void MainWindow::dispCurrState()
{
    QString output;
    if(currTurn.isEditMode) {
        output = "Tryb edycji planszy\n";
    } else {
        output = "Tura ";
        if(currTurn.currTeam) {
            output += "niebieskiego\n";
        } else {
            output += "czerwonego\n";
        }
        output += "wykonane ruchy: " + QString::number(moveCounter) + "\n";
        output += "wykonane podania: " + QString::number(passCounter) + "\n";
    }
    for(int i = 0; i<currTurn.moves.size(); i++) {
        int from = currTurn.moves[i].first;
        int to = currTurn.moves[i].second;
        output += parseNum(from) + " - " + parseNum(to) + "\n";
    }
    ui->statusDisplay->setText(output);
}

// wyświetlenie danego stanu
void MainWindow::dispState(State state)
{
    if(past.isEmpty()) {
        ui->statusDisplay->setText("początek rozgrywki");
    } else {
        QString output;
        if(state.isEditMode) {
            output = "Tryb edycji planszy\n";
        } else {
            output = "Tura ";
            if(state.currTeam) {
                output += "niebieskiego\n";
            } else {
                output += "czerwonego\n";
            }
        }
        for(int i = 0; i<state.moves.size(); i++) {
            int from = state.moves[i].first;
            int to = state.moves[i].second;
            output += parseNum(from) + " - " + parseNum(to) + "\n";
        }
        ui->statusDisplay->setText(output);}
}

// parsowanie kliknięcia na i-te pole
void MainWindow::parseClick(int i)
{
    if(doParse && gameOn) {
        if(lastHighlighted != NULL) {
            parseClickWHighlight(i);
        } else {
            parseClickNoHighlight(i);
        }
    }
}

// rozpoczęcie gry człowiek vs człowiek
void MainWindow::HumVSHum()
{
    newGame(false, false);
}

// rozpoczęcie gry człowiek vs komputer
void MainWindow::HumVSAI()
{
    newGame(false, true);
}

// rozpoczęcie gry komputer vs człowiek
void MainWindow::AIVSHum()
{
    newGame(true, false);
}

// rozpoczęcie gry komputer vs komputer
void MainWindow::AIVSAI()
{
    newGame(true, true);
}

// obsługa przycisku końca tury
void MainWindow::on_endTurnButton_clicked()
{
    if(lastHighlighted != NULL)
        parseClick(lastHighlighted->getPosition());
    ui->prevTurnButton->setEnabled(true);
    past.push(currTurn);
    game.push(currTurn);
    if(gameOn) {
        currPlayer =  players[!currPlayer->getCol()];
        startTurn();
    }
}

// obsługa przycisku cofania ruchu
void MainWindow::on_undoButton_clicked()
{
    QPair<int, int> temp = currTurn.moves.last();
    currTurn.moves.pop_back();
    int from = temp.first;
    int to = temp.second;
    makeMove(tiles[to], tiles[from]);
    if(tiles[to]->getResident() != NULL &&
            tiles[from]->getResident() != NULL) {
        passCounter -= 1;
    } else {
        moveCounter -= 1;
    }
    dispCurrState();
    if(passCounter + moveCounter == 0) {
        ui->undoButton->setEnabled(false);
    }
}

// obsługa przycisku wyświetlenia poprzedniej tury
void MainWindow::on_prevTurnButton_clicked()
{
    doParse = false;
    ui->undoButton->setEnabled(false);
    ui->endTurnButton->setEnabled(false);
    if(lastHighlighted != NULL)
        parseClick(lastHighlighted->getPosition());

    State change;
    if(future.isEmpty()) change = currTurn;
    else change = future.top();
    future.push(past.pop());

    int length = change.moves.size();
    for(int i = 0; i < length; i++){
        int from = change.moves[length - 1 - i].first;
        int to = change.moves[length - 1 - i].second;
        makeMove(tiles[to], tiles[from]);
    }

    dispState(future.top());

    if(past.isEmpty()) ui->prevTurnButton->setEnabled(false);
    ui->nextTurnButton->setEnabled(true);
}

// obsługa przycisku wyświetlenia następnej tury
void MainWindow::on_nextTurnButton_clicked()
{
    past.push(future.pop());
    State change;
    if(future.isEmpty()) change = currTurn;
    else change = future.top();
    for(int i = 0; i < change.moves.size(); i++){
        int from = change.moves[i].first;
        int to = change.moves[i].second;
        makeMove(tiles[from], tiles[to]);
    }

    if(future.isEmpty()){
        doParse = true;
        dispCurrState();
        ui->nextTurnButton->setEnabled(false);
        ui->endTurnButton->setEnabled(gameOn);
        ui->undoButton->setEnabled(gameOn && currTurn.moves.size() > 0);
    } else dispState(change);

    ui->prevTurnButton->setEnabled(true);
}

// obsługa przycisku hinta
void MainWindow::on_hintButton_clicked()
{
    while(ui->undoButton->isEnabled()){
        on_undoButton_clicked();
    }
    AITurn();
}

// obsługa trybu edycji planszy
void MainWindow::on_turnEditButton_clicked()
{
    if(lastHighlighted != NULL)
        parseClick(lastHighlighted->getPosition());

    if(editModeOn) {
        editModeOn = false;

        QStringList items;
        items << tr("niebieski") << tr("czerwony");
        bool ok;
        QString player =
                QInputDialog::getItem(this, tr("Wybór Gracza"),
                                      tr("Który gracz ma teraz rozgrywać?"),
                                      items, 0, false, &ok);
        if(ok && !player.isEmpty()) {
            currTurn.currTeam = player != "niebieski";
            currPlayer = players[player != "niebieski"];
        }

        if (QMessageBox::Yes ==
                QMessageBox::question(this, "Usunięcie historii gry",
                                      "Czy usunąć historię?",
                                      QMessageBox::Yes|QMessageBox::No))
        {
            past = QStack<State>();
            game = QStack<State>();
            future = QStack<State>();
        }

        ui->turnEditButton->setText("Edytuj Planszę");
        on_endTurnButton_clicked();
    } else {
        doParse = true;
        editModeOn = true;
        ui->turnEditButton->setText("Zakończ Edycję");
        ui->hintButton->setEnabled(false);
        ui->endTurnButton->setEnabled(false);
        currTurn.isEditMode = true;
        dispCurrState();
    }
}

// obsługa zapisu gry
void MainWindow::saveGame()
{
    if(gameOn) {
        QFile outputFile;
        QString outputFileName = QFileDialog::getSaveFileName(this,tr("Zapisz Grę"),".",tr("Diaballik game file (*.dia)"));
        outputFile.setFileName(outputFileName);
        outputFile.open(QIODevice::WriteOnly);
        QDataStream out(&outputFile);

        out << players[RED]->isArti() << players[BLUE]->isArti();
        out << currTurn.currTeam << currTurn.isEditMode;
        out << currTurn.moves;
        out << passCounter << moveCounter;
        out << game.size();

        for(int i = 1; i < game.size(); i++){
            State temp = game[i];
            out << temp.currTeam << temp.isEditMode;
            out << temp.moves;
        }

        outputFile.close();
    }
}

// obsługa wczytania gry
void MainWindow::loadGame()
{
    bool red, blue, isEditMode;
    int size;
    team currTeam;
    QVector<QPair<int, int> > moves;

    QFile inputFile;
    QString inputFileName = QFileDialog::getOpenFileName(this,tr("Wczytaj Grę"),".",tr("Diaballik game file (*.dia)"));
    if(!inputFileName.isEmpty()) {
        inputFile.setFileName(inputFileName);
        inputFile.open(QIODevice::ReadOnly);
        QDataStream in(&inputFile);

        in >> red;
        in >> blue;
        newGame(red, blue);

        in >> currTeam;
        in >> isEditMode;
        in >> moves;
        currTurn = {moves, currTeam, isEditMode};
        currPlayer = players[currTeam];

        in >> passCounter;
        in >> moveCounter;

        in >> size;
        for(int i = 1; i < size; i++){
            in >> currTeam;
            in >> isEditMode;
            in >> moves;
            for(int j = 0; j < moves.size(); j++) {
                makeMove(tiles[moves[j].first], tiles[moves[j].second]);
            }
            State temp = {moves, currTeam, isEditMode};
            game.push(temp);
            past.push(temp);
        }

        for(int i = 0; i < currTurn.moves.size(); i++){
            int from = currTurn.moves[i].first;
            int to = currTurn.moves[i].second;
            makeMove(tiles[from], tiles[to]);
        }

        dispCurrState();
        ui->undoButton->setEnabled(moveCounter + passCounter > 0);
        ui->prevTurnButton->setEnabled(past.size() > 1);
        ui->hintButton->setEnabled(!(editModeOn || currPlayer->isArti()));
    }
}

// wyświetlenie zasad gry
void MainWindow::gameRules()
{
    QMessageBox::information(this, "Zasady gry", "Celem gry jest doprowadzenie pionka oznaczonego kółkiem na drugi koniec planszy, zanim zrobi to przeciwnik.");
    QMessageBox::information(this, "Zasady gry", "Dozwolone ruchy to: poruszenie pionka nieoznaczonego kółkiem na wolne sąsiednie pole (2 na turę) oraz zamiana pionka oznaczonego kółkiem i pionka nieoznaczonego kółkiem miejscami (1 na turę).");
    QMessageBox::information(this, "Zasady gry", "By dokonać zamiany, dwa pionki muszą leżeć w tym samym wierszu, tej samej kolumnie lub na tej samej diagonali, a pomiędzy nimi nie może się znajdować żaden pionek drużyny przeciwnej.");
    QMessageBox::information(this, "Zasady gry", "Jeśli któryś gracz stworzy z pionków linię nie do przejścia, a co najmniej 3 pionki drugiego gracza będą z nią sąsiadowały (od strony linii startowej drugiego gracza), gracz ten przegrywa grę.");
}

