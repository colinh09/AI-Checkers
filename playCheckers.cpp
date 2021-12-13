#include "checkers.cpp"

void playGame()
{
    movesList.reserve(200);
    int startingPlayer, timeLimit;
    //if the user wants to load a board or not
    bool userBoard = false;
    double input = 0;
    cout << "Welcome to checkers! This program was created by Colin Hwang." << endl << endl; 
    cout << "Would you like to:" << endl;
    cout << "     [1] Begin a new game?" << endl;
    cout << "     [2] Load a saved game?" << endl;
    cout << "Option Chosen --> ";
    //making sure that a valid input is entered
    while(input == 0)
    {
        cin >> input;
        cout << endl;
        if (input != 1 && input != 2)
        {
            cout << "That did not match any of the options given. Please enter a 1 or a 2!" << endl;
            cout << "Option Chosen --> ";
            input = 0;
            cin.clear();
            cin.ignore(numeric_limits <streamsize> ::max(), '\n');
        }
    }
    //user chose not to load board
    if(input == 1)
    {
        initStartBoard();
    }
    //user chose to load board
    //no need to have the user to insert starting time and time limit
    else 
    {
        string gameinfo = loadBoard();
        startingPlayer = static_cast<int>(gameinfo[0]) - 48;
        timeLimit = static_cast<int>(gameinfo[1]) - 48;
        userBoard = true;
    }
    //two options: ai vs ai or user vs user.
    //maybe implement user vs user functionality later for personal use.
    bool AIvsAI = false;
    cout << "Please choose a game mode:" << endl;
    cout << "     [1] User versus AI" << endl;
    cout << "     [2] AI versus AI" << endl;
    cout << "Option Chosen --> ";
    input = 0;
    //ensuring correct input is entered
    while(input == 0)
    {
        cin >> input;
        cout << endl;
        if (input != 1 && input != 2)
        {
            cout << "That did not match any of the options given. Please enter a 1 or a 2!" << endl;
            cout << "Option Chosen --> ";
            input = 0;
            cin.clear();
            cin.ignore(numeric_limits <streamsize> ::max(), '\n');
        }
    }
    AIvsAI = (input == 2);
    //need to enter time limit and set starting player if not loading in a board
    if (userBoard == false)
    {
        cout << "How many seconds do you want to give the AI to move? Please choose an integer value within the range of 3s-60s." << endl;
        cout << "Time Limit --> ";
        input = 0;
        //check if correct input is entered
        while(input == 0)
        {
            cin >> input;
            cout << endl;
            if (input > 60 || input < 3)
            {
                cout << "That did not match any of the options given. Please enter an integer value within the range of 3s-60s." << endl;
                cout << "Option Chosen --> ";
                cout << endl;
                input = 0;
                cin.clear();
                cin.ignore(numeric_limits <streamsize> ::max(), '\n');
            }
        }
        //decides which player starts the game
        if(player1Starts(!AIvsAI))
        {
            startingPlayer = 1;
        } 
        else 
        {
            startingPlayer = 2;
        }
        cout << "The starting player is player " << startingPlayer << "." << endl << endl;
    }
    printBoard();
    //ensures that the board is not in a gameover state
    if(gameOver() > 0)
    {
        return;
    }
    //AI vs AI is different from user vs AI.
    if(AIvsAI)
    {
        playAIvsAI(startingPlayer, input);
        return;
    }
    //if player 1 is starting, get all legal moves and play the move
    if(startingPlayer == 1)
    {
        getLegalMoves(1);
        // If no moves then game ends
        if (movesList.size() == 0) 
        {
            cout << "You have no possible moves. The AI wins. GG WP" << endl;
            return;
        }
        cout << "Here is the list of legal moves you can make:" << endl;
        printMoves();
        playMove(getMoveChoice());
        printBoard();
    }
    //if player 1 is not starting, player 2 will start (obviously). This will loop until there is a draw or a gameover state has been reached
    while(true)
    {
        getLegalMoves(2);
        // If no moves then game ends
        if(movesList.size() == 0)
        {
            cout << "The AI has no moves left. You win!" << endl;
            return; //exit out of loop
        }
        cout << "Here is the list of moves the AI can make:" << endl;
        printMoves();
        cout << endl;
        cout << "The AI is deciding on its move..." << endl << endl;
        //minimax to find optimal move
        playMove(iterativeDeepening(floor(input)));
        printBoard();
        //checks if game is a draw
        if(isDraw())
        {
            return;
        }
        //player 1's turn after player 2
        //alternate p1 and p2 turns
        getLegalMoves(1);
        if(movesList.size() == 0) 
        {
            cout << "You have no possible moves. The AI wins. GG WP" << endl;
            return;
        }
        cout << "Here is the list of legal moves you can make:" << endl;
        printMoves();
        playMove(getMoveChoice());
        printBoard();
        if(isDraw())
        {
            return;
        }
    }
}

int main(int argc, const char * argv[]) 
{
    //seeding the pseudo random number generator that rand() will use
    srand(time(nullptr));
    playGame(); //lets play some checkers!!
    return 0;
}