#include <iomanip>
#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <time.h> //easier than chrono
#include <string>
//i know it's bad practice...
using namespace std;

//map class to help map moves
class Map 
{
public:
    int x, y;
    Map(int i, int j) : x{i}, y{j} {}
};
//Vector of legal moves without jumps
vector<vector<Map>> movesList;
//Vector of legal moves with jumps
vector<vector<Map>*> jumpsList;

//global variable to keep track of the time limit
bool timeLimitPassed = false;

//using same identifier rule of pieces as Sable (my prof).
//0 = empty space, 1 = p1, 2 = p2, 3 = p1 king, 4 = p2 king
//8x8 checker board
int currentBoard[8][8] = {0};
// Global variables for heuristic (evaluation) function
int currentTotalPcs = 0;
int currentP1Pcs = 0;
int currentP2Pcs = 0;
int currentP1Kings = 0;
int currentP2Kings = 0;
int turnCount = 0;

//only need to set 1s and 2s to board if its the start of the game
void initStartBoard()
{
    int player = 2;
    for(int i = 0, j = 0; j < 8; i++)
    {
        if ((i+j+1)%2 == 0)
        {
            currentBoard[j][i] = player;
        }
        if (i == 7)
        {
            i = -1;
            if(j == 2)
            {
                j = 5;
                player = 1;
            } 
            else 
            {
                j++;
            }
        }
    }
    currentTotalPcs = 24;
    currentP1Pcs = 12;
    currentP2Pcs = 12;
}

// Initializes user-set game board
string loadBoard(){
    int count = 0;
    string gameinfo;
    //reads in board. Follows Sable's board schematic
    string file;
    cout << "Please enter the file name of your saved game." << endl;
    cout << "NOTE: the time limit for the AI and the starting player should be included in the file. Therefore, these paramters will not be entered." << endl;
    cout << "File name --> ";
    cin >> file;
    cout << endl;
    ifstream fin;
    fin.open(file);
    //checks if the user entered a proper saved game file
    while(!fin.is_open())
    {
        cout << "Entered file failed to be opened." << endl;
        cout << "Please enter a new file." << endl;
        cout << "File name --> ";
        cin >> file;
        cout << endl;
        fin.open(file);
    }
    int input;
    int i = 1; int j = 0;
    while(fin >> input && count < 31)
    {
        count++;
        if ((i+j+1)%2 == 0)
        {
            if(j == 0 && input == 1)
            {
                input = 3;
            } 
            else if(j == 7 && input == 2)
            {
                input = 4;
            }
            
            currentBoard[j][i] = input;
            //keep track of pieces for heuristic
            if(input == 1 || input == 3)
            {
                currentP1Pcs++;
                currentTotalPcs++;
                if(input == 3)
                {
                    currentP1Kings++;
                }
            } 
            else if(input == 2 || input == 4)
            {
                currentP2Pcs++;
                currentTotalPcs++;
                if(input == 4)
                {
                    currentP2Kings++;
                }
            }
        }
        do
        {
            i++;
            if(i == 8)
            {
                i = 0;
                j++;
            }
            if(j == 8)
            {
                break;
            }
        } while ((i+j+1)%2 != 0);
    }
    //this will be the starting player
    fin >> input;
    gameinfo += to_string(input);
    //this will be the time limit
    fin >> input;
    gameinfo += to_string(input);
    //return both pieces of info as a string
    return gameinfo;
}

//making sure that a spot does not get jumped more than once
bool legalJump(int jumpedI, int jumpedJ, const vector<Map> & move)
{
    for (int i = 0; i < move.size()-1; i++) 
    {
        if(((abs(move.at(i).x + move.at(i+1).x)) / 2) == jumpedI && ((abs(move.at(i).y + move.at(i+1).y)) / 2) == jumpedJ)
        {
            return false;
        }
    }
    return true;
}

//deals with jump moves
void jump(vector<Map>& move, int i, int j, int player, int otherPlayer, bool king, int board[8][8], vector<vector<Map>>& moves = movesList, vector<vector<Map>*>& jumps = jumpsList)
{
    //if there are more than one jump that can be made
    bool multiOptions = false;
    vector<Map> moveCopy = move;
    
    //if the piece thats jumping becomes a king, it stops there
    if (((j == 0 && player == 1) || (j == 7 && player == 2)) && !king) 
    {
        return;
    }
    
    //northwest
    if(i > 1 && j > 1 && (player == 1 || king))
    {
        if((board[j-1][i-1] == otherPlayer || board[j-1][i-1] == otherPlayer+2)
           && (board[j-2][i-2] == 0 || (i-2 == move.at(0).x && j-2 == move.at(0).y))
           && legalJump(i-1, j-1, moveCopy))
        {
            move.push_back(Map(i-2, j-2));
            multiOptions = true;
            jump(move, i-2, j-2, player, otherPlayer, king, board, moves, jumps);
        }
    }
    
    //northeast
    if(i < 6 && j > 1 && (player == 1 || king))
    {
        if((board[j-1][i+1] == otherPlayer || board[j-1][i+1] == otherPlayer+2)
           && (board[j-2][i+2] == 0 || (i+2 == move.at(0).x && j-2 == move.at(0).y))
           && legalJump(i+1, j-1, moveCopy))
        {
            if (multiOptions) 
            {
                moves.push_back(moveCopy);
                jumps.push_back(&moves.at(moves.size()-1));
                (moves.at(moves.size()-1)).push_back(Map(i+2, j-2));
                jump(moves.at(moves.size()-1), i+2, j-2, player, otherPlayer, king, board, moves, jumps);
            } 
            else 
            {
                move.push_back(Map(i+2, j-2));
                multiOptions = true;
                jump(move, i+2, j-2, player, otherPlayer, king, board, moves, jumps);
            }
        }
    }
    
    //southwest
    if(i > 1 && j < 6 && (player == 2 || king))
    {
        if((board[j+1][i-1] == otherPlayer || board[j+1][i-1] == otherPlayer+2)
           && (board[j+2][i-2] == 0 || (i-2 == move.at(0).x && j+2 == move.at(0).y))
           && legalJump(i-1, j+1, moveCopy))
        {
            if (multiOptions) 
            {
                moves.push_back(moveCopy);
                jumps.push_back(&moves.at(moves.size()-1));
                (moves.at(moves.size()-1)).push_back(Map(i-2, j+2));
                jump(moves.at(moves.size()-1), i-2, j+2, player, otherPlayer, king, board, moves, jumps);
            } 
            else 
            {
                move.push_back(Map(i-2, j+2));
                multiOptions = true;
                jump(move, i-2, j+2, player, otherPlayer, king, board, moves, jumps);
            }
        }
    }
    
    //southeast
    if(i < 6 && j < 6 && (player == 2 || king))
    {
        if((board[j+1][i+1] == otherPlayer || board[j+1][i+1] == otherPlayer+2)
           && (board[j+2][i+2] == 0 || (i+2 == move.at(0).x && j+2 == move.at(0).y))
           && legalJump(i+1, j+1, moveCopy))
        {
            if (multiOptions) 
            {
                moves.push_back(moveCopy);
                jumps.push_back(&moves.at(moves.size()-1));
                (moves.at(moves.size()-1)).push_back(Map(i+2, j+2));
                jump(moves.at(moves.size()-1), i+2, j+2, player, otherPlayer, king, board, moves, jumps);
            } 
            else 
            {
                move.push_back(Map(i+2, j+2));
                multiOptions = true;
                jump(move, i+2, j+2, player, otherPlayer, king, board, moves, jumps);
            }
        }
    }
}

//retrieves all legal moves that a player can make
void getLegalMoves(int player, int board[8][8] = currentBoard, vector<vector<Map>>& moves = movesList, vector<vector<Map>*>& jumps = jumpsList)
{
    moves.clear();
    jumps.clear();
    int otherPlayer;
    bool king;
    //if there is a jump, the player MUST take it. Therefore, other non jump moves can be ignored.
    bool jumped = false; 
    if(player == 1)
    {
        otherPlayer = 2;
    } 
    else 
    {
        otherPlayer = 1;
    }
    for(int i = 0, j = 0; j < 8; i++)
    {
        if(board[j][i] == player || board[j][i] == player +2)
        {
            
            king = board[j][i] > 2;
            
            //North
            if((player == 1 || king) && j != 0)
            {
                //northwest
                if(i != 0)
                {
                    if(!jumped && board[j-1][i-1] == 0)
                    {
                        //add the move onto list
                        moves.push_back({Map(i, j), Map(i-1, j-1)});
                    } 
                    else if((board[j-1][i-1] == otherPlayer || board[j-1][i-1] == otherPlayer + 2) && i != 1 && j != 1)
                    {
                        if(board[j-2][i-2] == 0)
                        {
                            //if there is a jump to be made
                            jumped = true;
                            moves.push_back({Map(i, j), Map(i-2, j-2)});
                            jumps.push_back(&moves.at(moves.size()-1));
                            jump(moves.at(moves.size()-1), i-2, j-2, player, otherPlayer, king, board, moves, jumps);
                        }
                    }
                }
                
                //northeast
                if(i != 7)
                {
                    if(!jumped && board[j-1][i+1] == 0)
                    {
                        //add move onto list
                        moves.push_back({Map(i, j), Map(i+1, j-1)});
                    } 
                    else if((board[j-1][i+1] == otherPlayer ||board[j-1][i+1] == otherPlayer + 2) && i != 6 && j != 1)
                    {
                        if(board[j-2][i+2] == 0)
                        {
                            //same thing
                            jumped = true;
                            moves.push_back({Map(i, j), Map(i+2, j-2)});
                            jumps.push_back(&(moves.at(moves.size()-1)));
                            jump(moves.at(moves.size()-1), i+2, j-2, player, otherPlayer, king, board, moves, jumps);
                        }
                    }
                }
            }
            
            //south
            if((player == 2 || king) && j != 7)
            {
                //southwest
                if(i != 0)
                {
                    if(!jumped && board[j+1][i-1] == 0)
                    {
                        moves.push_back({Map(i, j), Map(i-1, j+1)});
                    } 
                    else if((board[j+1][i-1] == otherPlayer ||board[j+1][i-1] == otherPlayer + 2) && i != 1 && j != 6)
                    {
                        if(board[j+2][i-2] == 0)
                        {
                            jumped = true;
                            moves.push_back({Map(i, j), Map(i-2, j+2)});
                            jumps.push_back(&moves.at(moves.size()-1));
                            jump(moves.at(moves.size()-1), i-2, j+2, player, otherPlayer, king, board, moves, jumps);
                        }
                    }
                }  
                //southeast
                if(i != 7)
                {
                    if(!jumped && board[j+1][i+1] == 0)
                    {
                        moves.push_back({Map(i, j), Map(i+1, j+1)});
                    } 
                    else if((board[j+1][i+1] == otherPlayer ||board[j+1][i+1] == otherPlayer + 2) && i != 6 && j != 6)
                    {
                        if(board[j+2][i+2] == 0)
                        {
                            jumped = true;
                            moves.push_back({Map(i, j), Map(i+2, j+2)});
                            jumps.push_back(&moves.at(moves.size()-1));
                            jump(moves.at(moves.size()-1), i+2, j+2, player, otherPlayer, king, board, moves, jumps);
                        }
                    }
                }
            }
        }
        if(i == 7)
        {
            i = -1;
            j++;
        }
    }
}

//displays all legal moves to the interface
void printMoves()
{
    if(jumpsList.size() > 0)
    {
        for(int i = 0; i < jumpsList.size(); i++)
        {
            cout << " (" << i+1 << "): ";
            for(int j = 0; j < jumpsList.at(i)->size(); j++)
            {
                cout << "[" << jumpsList.at(i)->at(j).x << ", " << jumpsList.at(i)->at(j).y << "]";
                if (jumpsList.at(i)->size() > 2)
                {
                //making sure to add --> between multiple jumps!!
                    if (j != jumpsList.at(i)->size() - 1 && (jumpsList.at(i)->at(j).x >= 0 && jumpsList.at(i)->at(j).x <= 7))
                    {
                        cout << " --> ";
                    }
                }
                //if no multiple jumps
                else
                {
                    if (j % 2 == 0)
                    {
                        cout << " --> ";
                    }
                }
            }
            cout << endl;
        }
    }
    //non jumps
    else 
    {
        for(int i = 0; i < movesList.size(); i++)
        {
            cout << " (" << i+1 << "): ";
            for(int j = 0; j < movesList.at(i).size(); j++)
            {
                cout << "[" << movesList.at(i).at(j).x << ", " << movesList.at(i).at(j).y << "]";
                if (j % 2 == 0)
                {
                    cout << " --> ";
                }
            }
            cout << endl;
        }
    }
}

//ooo pretty board :D
void printBoard(){
    int i, j;
    cout << endl;
    cout << "     Counter Key:" << endl;
    cout << "     Player 1 Counter: " << "\033[1;31m" << "a" << "\033[0m" << endl;
    cout << "     Player 1 King: " << "\033[1;31m" << "A" << "\033[0m" << endl;
    cout << "     Player 2 Counter: " << "\033[1;37m" << "z" << "\033[0m" << endl;
    cout << "     Player 2 King: " << "\033[1;37m" << "Z" << "\033[0m" << endl;
    cout << endl << std::setw(20);
    for(i = 0; i < 8; i++) 
    {
        cout << "   " << "\033[32;40m" << "\033[1m" << i << "\033[0m" << "   ";
    }
    cout << endl << endl;
    for(i = 0; i < 8; i++) 
    {
        cout << std::setw(23);
        for(int j = 0; j < 8; j++) 
        {
            if((j + i) & 1) 
            {
                cout << "\033[;40m" << "       ";
            } 
            else 
            {
                cout << "\033[;47m" << "       ";
            }
        }
        cout << "\033[0m" << endl << std::setw(13);
        //color green for row labeling
        cout << " " << "\033[32;40m" << "\033[1m" << i << "\033[0m" << "   ";
        //middle section of each square in board. Also include checker piece identifier
        for(j = 0; j < 8; j++) 
        {
            if((j + i) & 1) 
            {
                cout << "\033[;40m" << "   ";
            } 
            else 
            {
                cout << "\033[;47m" << "   ";
            }
            if (currentBoard[i][j] >= 1 && currentBoard[i][j] <= 4)
            {
                if(currentBoard[i][j] == 1)
                {
                    cout << "\033[1;31m" << "a" << "\033[;40m" << "   " << "\033[0m";
                } 
                else if(currentBoard[i][j] == 3)
                {
                    cout << "\033[1;31m" << "A" << "\033[;40m" << "   " << "\033[0m";
                } 
                else if(currentBoard[i][j] == 2)
                {
                    cout << "\033[1;37m" << "z" << "\033[;40m" << "   " << "\033[0m";
                } 
                else if(currentBoard[i][j] == 4)
                {
                    cout << "\033[1;37m" << "Z" << "\033[;40m" << "   " << "\033[0m";
                }
            }
            if (!(currentBoard[i][j] >= 1 && currentBoard[i][j] <= 4))
            {
                cout <<  "    ";
            }
        }
        //finish bottom section of square with same checkered pattern of black and white
        cout << "\033[0m" << endl << std::setw(23);
        for(j = 0; j < 8; ++j) 
        {
            if((j + i) & 1) 
            {
                cout << "\033[;40m" << "       ";
            } 
            else 
            {
                cout << "\033[;47m" << "       ";
            }
        }
        //reset
        cout << "\033[0m" << endl;
    }
    cout << endl;
}

//takes the choice from either the user or the AI and plays the move
void playMove(int moveNum, int board[8][8] = currentBoard, vector<vector<Map>>& moves = movesList, vector<vector<Map>*>& jumps = jumpsList)
{
    moveNum--;
    bool jump = jumps.size() > 0;
    int piece;
    if(!jump)
    {
        piece = board[moves.at(moveNum).at(0).y][moves.at(moveNum).at(0).x];
        //if piece reached opponent end of the board, becomes king
        if ((moves.at(moveNum).at(1).y == 0 && piece == 1) || (moves.at(moveNum).at(1).y == 7 && piece == 2)) 
        {
            board[moves.at(moveNum).at(1).y][moves.at(moveNum).at(1).x] = piece + 2;
            //keeping track of pieces for heuristic!!
            if(piece == 1)
            {
                currentP1Kings++;
            } 
            else
            {
                currentP2Kings++;
            }
        }
        else 
        {
            board[moves.at(moveNum).at(1).y][moves.at(moveNum).at(1).x] = piece;
        }
        board[moves.at(moveNum).at(0).y][moves.at(moveNum).at(0).x] = 0;
    } 
    else 
    {
        piece = board[jumps.at(moveNum)->at(0).y][jumps.at(moveNum)->at(0).x];
        int jumpedX;
        int jumpedY;
        //starting square no longer has a piece. Remove it.
        board[jumps.at(moveNum)->at(0).y][jumps.at(moveNum)->at(0).x] = 0;
        for (int i = 0; i < jumps.at(moveNum)->size(); i++) 
        {
            if(i == jumps.at(moveNum)->size() - 1)
            {
                //king when reaching opponent's end of board
                if ((jumps.at(moveNum)->at(i).y == 0 && piece == 1) || (jumps.at(moveNum)->at(i).y == 7 && piece == 2))
                {
                    board[jumps.at(moveNum)->at(i).y][jumps.at(moveNum)->at(i).x] = piece + 2;
                } 
                else 
                {
                    board[jumps.at(moveNum)->at(i).y][jumps.at(moveNum)->at(i).x] = piece;
                }
            } 
            else 
            {
                //retrieves coordinate of jumped piece. clears its path because it got eliminated
                jumpedX = (abs(jumps.at(moveNum)->at(i).x + jumps.at(moveNum)->at(i+1).x)) / 2;
                jumpedY = (abs(jumps.at(moveNum)->at(i).y + jumps.at(moveNum)->at(i+1).y)) / 2;
                //keep track of pieces for heuristic
                int jumpedPc = board[jumpedY][jumpedX];
                if(board == currentBoard)
                {
                    currentTotalPcs--;
                    if(jumpedPc == 1 || jumpedPc == 3)
                    {
                        currentP1Pcs--;
                        if(jumpedPc == 3)
                        {
                            currentP1Kings--;
                        }
                    } 
                    else 
                    {
                        currentP2Pcs--;
                        if(jumpedPc == 4)
                        {
                            currentP2Kings--;
                        }
                    }
                }
                board[jumpedY][jumpedX] = 0;
            }
        }
    }
}

//determines if the board is in a game over state
int gameOver(int board[8][8] = currentBoard)
{
    bool player1Wins = true;
    bool player2Wins = true;
    for(int i = 0, j = 0; j < 8; i++)
    {
        if(player2Wins && (board[j][i] == 1 || board[j][i] == 3))
        {
            player2Wins = false;
        } 
        else if(player1Wins && (board[j][i] == 2 || board[j][i] == 4))
        {
            player1Wins = false;
        }
        
        if (!player1Wins && !player2Wins) 
        {
            return 0;
        }
        if(i == 7)
        {
            i = -1;
            j++;
        }
    }
    
    if (player1Wins) 
    {
        return 1;
    } 
    else 
    {
        return 2;
    }
}

void copyBoard(int from[8][8], int to[8][8])
{
    for(int i = 0, j = 0; j < 8; i++)
    {
        to[j][i] = from[j][i];
        if(i == 7)
        {
            i = -1;
            j++;
        }
    }
}

//heuristic (evaluation) function to improve minimax search
int heuristic(int board[8][8], int player, int depth, bool noMoves, int numMoves)
{
    int val = 0;
    int p1Pawns = 0;
    int p2Pawns = 0;
    int p1Kings = 0;
    int p2Kings = 0;
    int p1Bottom = 0;
    int p2Top = 0;
    int p1MidBox = 0;
    int p2MidBox = 0;
    int p1MidRows = 0;
    int p2MidRows = 0;
    int p1Pcs = 0;
    int p2Pcs = 0;
    int totalpcs = 0;
    for(int i = 0, j = 0; j < 8; i++)
    {
        //how many pieces are at the top and bottom of the board
        if(j == 0 && (board[j][i] == 2 || board[j][i] == 4))
        {
            p2Top++;
        } 
        else if(j == 7 && (board[j][i] == 1 || board[j][i] == 3))
        {
            p1Bottom++;
        }
        //most important thing to the AI should be the number of pieces and the number of kings
        //pawns are worth +- 100
        if(board[j][i] == 1)
        {
            val-=100;
            p1Pawns++;
            p1Pcs++;
            totalpcs++;
        } 
        else if(board[j][i] == 2)
        {
            val+=100;
            p2Pawns++;
            p2Pcs++;
            totalpcs++;
        } 
        //kings are worth +- 100
        else if(board[j][i] == 3)
        {
            val-=155;
            p1Kings++;
            p1Pcs++;
            totalpcs++;
        } 
        else if(board[j][i] == 4)
        {
            val += 155;
            p2Kings++;
            p2Pcs++;
            totalpcs++;
        }
        //how many pieces are in the middle box of the board
        if((i >= 2 && i <= 5) && (j >= 2 && j <= 5))
        {
            if (board[j][i] == 2 || board[j][i] == 4) 
            {
                p2MidBox++;
            } 
            else if(board[j][i] == 2 || board[j][i] == 4)
            {
                p1MidBox++;
            }
        }
        //keep track of pieces in the middle section of board, but not middle box
        if((i < 2 || i > 5) && (j >= 2 && j <= 5))
        {
            if (board[j][i] == 2 || board[j][i] == 4) 
            {
                p2MidRows++;
            } 
            else if(board[j][i] == 2 || board[j][i] == 4)
            {
                p1MidRows++;
            }
        }
        if(i == 7)
        {
            i = -1;
            j++;
        }
    }
    //heuristic changes for end-game. The AI should care more about how many pieces it has relative to the other player and the number of kings
    //"end game" is arbitrary, however, I am deciding that end game is reached either when
    //one player has less than 2/3 of the other player's pieces
    //there are <=10 pieces left on the board
    //<=15 pieces and there are an uneven number of kings between the players
    if(currentTotalPcs <= 10 || (currentP2Pcs*(2/3) >= currentP1Pcs) || (currentP1Pcs*(2/3) >= currentP2Pcs) || (currentTotalPcs <= 14 && abs(currentP1Kings - currentP2Kings) > 0))
       {
        //the player that has more pieces should be looking to make trades. If done right, this will lead to a win
        if(currentP2Pcs > currentP1Pcs && p2Pcs > p1Pcs)
        {
            val += ((currentTotalPcs - totalpcs)*30);
        } 
        else if(currentP1Pcs > currentP2Pcs && p1Pcs > p2Pcs) 
        {
            val -= ((currentTotalPcs - totalpcs)*30);
        }
        //kings become more valuable to the player.
        val += p2Kings*20;
        val -= p1Kings*20;
    } 
    //if it is still early in the game, the positioning of pieces (staying clumped, avoiding forced jumps resulting in uneven trades) and # of pieces matters more
    else 
    {
        //pieces at top and bottom (home ground) of the board are +- 50
        //pieces that are in the middle box are the same as top and bottom. +- 50
        //if not in the middle box, but in the middle rows, pieces have less agency. +-10
        val += ((p2Top*50) + (p2MidBox*50) + (p2MidRows*10));
        val -= ((p1Bottom*50) + (p1MidBox*50) + (p1MidRows*10));
    }
    //if the player still has pieces to king, the opponent's home row becomes much more valuable
    //any of the opponent's pieces in the home row guards against new kings , +- 40
    if(p1Pawns > 0)
    {
        val+=40*p2Top;
    } 
    else if(p2Pawns > 0)
    {
        val-=40*p1Bottom;
    }
    //makes AI take the closer win and push off loss
    //will also make AI try to block their opponent if it means winning fastest (blocking off the opponent means there are no moves left, resulting in a win)
    if(player == 2 && (noMoves || gameOver(board) == 1))
    {
        val -= (500 + (700 - (depth*10)));
    } 
    else if (player == 1 && (noMoves || gameOver(board) == 2))
    {
        val += (500 + (700 - (depth*10)));
    }
    //to choose between equal moves, choose randomly
    val += rand() % 10;
    return val;
}

//minimax search with alpha beta pruning
//derived from textbook's implementation!
int minimax(int board[8][8], int depthLeft, int alpha, int beta, bool maxPlayer, time_t endTime,int currentDepth, bool root = false)
{
    //if AI ran out of time
    if(timeLimitPassed || time(nullptr) >= endTime)
    {
        timeLimitPassed = true;
        return 0;
    }
    int boardCopy[8][8] = {0};
    vector<vector<Map>> nodeMoves;
    vector<vector<Map>*> nodeJumps;
    nodeMoves.reserve(200);
    int value;
    int tmpValue;
    int bestMove = 1;
    int player = maxPlayer ? 2 : 1;
    getLegalMoves(player, board, nodeMoves, nodeJumps);
    int moveAmt = nodeJumps.size() > 0 ? nodeJumps.size() : nodeMoves.size();
    //run hueristic function at either the max depth of an end-game board
    //however, will continue evaluating a branch if there is a forced jump
    if((depthLeft <= 0 && (nodeJumps.size() == 0)) || moveAmt == 0 || gameOver(board) > 0)
    {
        return heuristic(board, player, currentDepth, (moveAmt == 0), moveAmt);
    }
    if(maxPlayer)
    {
        value = -90000;
        for(int i = 1; i <= moveAmt; i++)
        {
            copyBoard(board, boardCopy);
            playMove(i, boardCopy, nodeMoves, nodeJumps);
            tmpValue = minimax(boardCopy, depthLeft-1, alpha, beta, false, endTime, currentDepth+1);
            if(tmpValue > value)
            {
                value = tmpValue;
                if(root)
                {
                    bestMove = i;
                }
            }
            alpha = max(alpha, value);
            if(alpha >= beta)
            {
                break;
            }
        }
        if(root)
        {
            return bestMove;
        } 
        else 
        {
           return value;
        }
    } 
    else 
    {
        value = 90000;
        for(int i = 1; i <= moveAmt; i++)
        {
            copyBoard(board, boardCopy);
            playMove(i, boardCopy, nodeMoves, nodeJumps);
            
            tmpValue = minimax(boardCopy, depthLeft-1, alpha, beta, true, endTime, currentDepth+1);
            if(tmpValue < value)
            {
                value = tmpValue;
                
                if(root)
                {
                    bestMove = i;
                }
            }
            beta = min(beta, value);
            if(alpha >= beta)
            {
                break;
            }
        }
        if(root)
        {
            return bestMove;
        } 
        else 
        {
            return value;
        }
    }
}

//iterative deepening for minmax. If only there wasn't a time limit....
int iterativeDeepening(int seconds, bool player2 = true)
{
    timeLimitPassed = false;
    int bestMove = 1;
    int move;
    time_t startTime = time(nullptr);
    time_t endTime = startTime + seconds;
    int depth = 1;
    //if there is only one move option, no need for minimax
    if(movesList.size() == 1 || jumpsList.size() == 1)
    {
        cout << "Maximum depth searched to completion: 0" << endl;
        cout << "Time Spent Searching: " << time(nullptr) - startTime << " Seconds" << endl;
        return 1;
    }
    //after searching a depth, if there is less than half the timelimit left, no point in searching next depth
    while((time(nullptr) + (seconds/2)) <= endTime)
    {
        move = minimax(currentBoard, depth, -90000, 90000, player2, endTime, 0, true);
        if(!timeLimitPassed)
        {
            bestMove = move;
            depth++;
        }
    }
    cout << "AI has chosen move " << bestMove << endl;
    cout << "Maximum depth searched to completion: " << depth << endl;
    if (time(nullptr) - startTime == seconds)
    {
        cout << "Stopped searching at depth " << depth+1 << endl;
    }
    cout << "Time Spent Searching: " << time(nullptr) - startTime << " Seconds" << endl;
    return bestMove;
}

//keeps track of number of turns that did not have a jump. After 200 turns, game calls a draw
bool isDraw()
{
    if(jumpsList.size() == 0)
    {
        turnCount++;
    } 
    else 
    {
        turnCount = 0;
    }
    if(turnCount >= 200)
    {
        cout << "This is clearly going nowhere.... no jumps have been made in 200 moves!!" << endl;
        cout << "The game is a draw!" << endl;
        return true;
    } 
    else 
    {
        return false;
    }
}

//if AIvsAI was selected
void playAIvsAI(int startingPlayer, int seconds)
{
    //if player 1 is the starting player, get all the legal moves and AI's decision
    if(startingPlayer == 1)
    {
        getLegalMoves(1);
        //no moves? game ends!
        if (movesList.size() == 0) 
        {
            cout << "Player 1 has no legal moves left. Player 2 wins." << endl;
            return;
        }
        cout << "Here is the list of moves player 1 AI can make:" << endl;
        printMoves();
        cout << endl;
        cout << "Player 1 AI is deciding on its move..." << endl << endl;
        playMove(iterativeDeepening(floor(seconds), false));
        printBoard();
    }
    //loop until there is a draw or game over state
    while(true)
    {
        //p2's turn
        getLegalMoves(2);
        //no moves? game ends!
        if(movesList.size() == 0)
        {
            cout << "Player 2 has no legal moves left. Player 1 wins. GG WP" << endl;
            return;
        }
        cout << "Here is the list of moves player 2 AI can make:" << endl;
        printMoves();
        cout << endl;
        cout << "Player 2 AI is deciding on its move..." << endl << endl;
        playMove(iterativeDeepening(floor(seconds)));
        printBoard();
        //draw???
        if(isDraw())
        {
            return;
        }
        //p1's turn
        getLegalMoves(1);
        //no moves? game ends!
        if(movesList.size() == 0) 
        {
            cout << "Player 1 has no legal moves left. Player 2 wins. GG WP" << endl;
            return;
        }
        cout << "Here is the list of moves player 1 AI can make:" << endl;
        printMoves();
        cout << endl;
        cout << "Player 1 AI is deciding on its move..." << endl << endl;
        playMove(iterativeDeepening(floor(seconds), false));
        printBoard();
        //draw???
        if(isDraw())
        {
            return;
        }
    }
}

//retrieve what move the user wants to make
int getMoveChoice()
{
    int moveChoice;
    int numChoices = jumpsList.size() > 0 ? jumpsList.size() : movesList.size();
    cout << "Which move would you like choose? (Enter the numerical value associated with the move)" << endl;
    cout << "Move Chosen --> ";
    cin >> moveChoice;
    cout << endl;
    //testing for invalid input from user
    while(cin.fail() || (moveChoice < 1 || moveChoice > numChoices))
    {
        cout << "That was an invalid choice." << endl;
        cout << "Please give a number choice associated with one of the legal moves." << endl;
        cout << "Move Chosen --> ";
        cin.clear();
        cin.ignore(256, '\n');
        cin >> moveChoice;
        cout << endl;
    }
    return moveChoice;
}

//prompts user to see which player starts the game
bool player1Starts(bool notAI)
{
    char input;
    if(notAI)
    {
        cout << "Would you like to have the first move? Enter y for yes and n for no." << endl;
        cout << "[Y/N] : ";
    } 
    else 
    {
        cout << "Would you like player 1 to go first? Enter y for yes and n for no." << endl;
        cout << "[Y/N] : ";
    }
    cin >> input;
    while(!(input == 'y' || input == 'Y' || input == 'n' || input == 'N'))
    {
        cout << "That was an invalid response. Please respond with y or n." << endl;
        cout << "[Y/N] : ";
        cin >> input;
    }
    if(input == 'y' || input == 'Y')
    {
        return true;
    } 
    else 
    {
        return false;
    }
}
