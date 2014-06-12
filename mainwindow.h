#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include<QMessageBox>
#include<QGraphicsView>
#include<QGraphicsItem>
#include<QtGui>
#include<QtCore>
#include<QStack>
#include<metrics.h>
#include<tile.h>
#include<player.h>
#include<QSignalMapper>
#include<pawn.h>
#include<QPair>
#include<QVector>
#include<QInputDialog>
#include<QFile>
#include<QFileDialog>

namespace Ui {
class MainWindow;
}

typedef bool team;

// struktura przechowująca stan danej tury
struct State{
    QVector<QPair<int, int> > moves;
    team currTeam;
    bool isEditMode;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    // często używane stałe
    static const team BLUE = true;
    static const team RED = false;
    static const int TILE_SIZE = 49;
    static const int PLAYERS_NUM = 2;
    static const int TEAM_NUMBER = 7;
    static const int PAWNS_NUMBER = 2 * TEAM_NUMBER;
    static const int PAWN_W_BALL = 3;
    static const int MAX_MOVES = 2;
    static const int MAX_PASSES = 1;

    //metoda wykonująca ruch
    void makeMove(Tile* a, Tile* b);

    /* metody zaznaczające pola - pierwsza te, na które ruch jest dozwolony,
     * druga - pomocnicza pierwszej, te, na które można podać piłkę
     * trzecia wszystkie, oprócz wybranego
     */
    void markAvailable();
    void markPassDirs(int num, team colour, int dir);
    void markAll();

    // metoda rozpoczynająca grę
    void newGame(bool red, bool blue);

    // początek tury
    void startTurn();
    // tura wykonywana przez sztuczną inteligencję
    void AITurn();

    // metody pomocnicze parseClick
    void parseClickNoHighlight(int i);
    void parseClickWHighlight(int i);
    void confirm(int i);

    /* metoda kończąca grę i jej pomocnicze metody, sprawdzające, czy gra
     * się zakończyła
     */
    void endgame(team winner);
    void checkRegularWin();
    void checkUnfairLine(team colour);
    void checkEndgame();

    // metody tworzące komunikaty okna statusu i je wyświetlające
    char parseCoord(int coord, char array[7]);
    char parseRow(int coord);
    char parseColumn(int coord);
    QString parseNum(int num);
    void dispCurrState();
    void dispState(State state);

    // wskaźniki i zmienne trzymające stan gry
    Player *players[PLAYERS_NUM];
    Player *currPlayer;
    bool doParse;
    bool gameOn;
    bool editModeOn;
    int moveCounter;
    int passCounter;
    Tile *tiles[TILES_NUMBER];
    Pawn *pawns[PAWNS_NUMBER];
    Tile *lastHighlighted;
    State currTurn;

    // stosy trzymające historię gry - do przeglądania i zapisu
    QStack<State> game;
    QStack<State> past;
    QStack<State> future;

    // wskaźniki do obsługi zdarzeń
    Ui::MainWindow *ui;
    QGraphicsScene *scene;
    QSignalMapper *clickMap;
    QWidget *viewport;

private slots:
    // slot czytający kliknięcia na planszy
    void parseClick(int i);

    // sloty rozpoczynające odpowiedni wariant nowej gry
    void HumVSHum();
    void HumVSAI();
    void AIVSHum();
    void AIVSAI();

    // sloty obsługujące przyciski bieżącej tury
    void on_endTurnButton_clicked();
    void on_undoButton_clicked();

    // sloty obsługujące przyciski przeglądania historii
    void on_prevTurnButton_clicked();
    void on_nextTurnButton_clicked();

    // sloty obsługujące przyciski oszustw
    void on_hintButton_clicked();
    void on_turnEditButton_clicked();

    // sloty obsługujące przyciski zapisu i wczytania gry
    void saveGame();
    void loadGame();

    // slot obsługujący wyświetlanie zasad gry
    void gameRules();

signals:
    // sygnał zatwierdzający kliknięcie i-tego pola
    void confirmClick(int i);
};

#endif // MAINWINDOW_H
