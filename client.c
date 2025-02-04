#include "global.h"
#include "board.h"
#include "move.h"
#include "comm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <limits.h>

// maximum depth for minimax (don't crash it!)
#define MAX_DEPTH 5

// ab-pruning flag
#define AB_PRUNING TRUE

// maximum number of moves to search
#define MAX_MOVES_SEARCH 100
#define TOTAL_EMPTY_CELLS 169

// min(), max() macros
#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)


/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket
char msg;					// used to store the received message

char * agentName = "Pápou";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/


/* Minimax & Evaluation */

// --- Count the Legal moves available ---
int countAvailableMoves(Position *currentPosition, Move moves[], char color) {
    int total_moves = 0;

	// for each square on the board, search if its empty and legal then -> added to moves list
	for (int i = 0; i < ARRAY_BOARD_SIZE; i++) {
        for (int j = 0; j < ARRAY_BOARD_SIZE; j++) {

			// if square is empty and legal add the move to the list
			if (currentPosition->board[i][j] == EMPTY) {
				// parse the move
				Move move = {{i, j}, color};

				if(isLegal(currentPosition, i, j, color)) {
                    // store the move
					moves[total_moves] = move;

					// increase moves counter
					total_moves++;
                }
            }
        }
    }
	// return the total number of moves available on each game state
    return total_moves;
}


// --- Evaluation function (f) ---
// int evaluatePosition(Position *currentPosition, char playerColor) {
// 	// V(state) = #myDisks - #opponentDisks
// 	int myAgentScore = currentPosition->score[(int)playerColor];
// 	int opponentAgentScore = currentPosition->score[getOtherSide(playerColor)];
// 	int stateValue = myAgentScore - opponentAgentScore;

// 	return stateValue;
// }


// --- Evaluation function (f) ---
int evaluatePosition(Position *currentPosition, char color) {
	// we divide the game into 3 phases:
	// Start: 0-30% of the game
	// Middle: 30-70% of the game
	// End: 70-100% of the game

	// On each phase we adapt the weights of the evaluation function
	// We assume that sometimes our agent has to defend or to attack (by playing more aggressive or defensive)

	// F-score of state
	int stateValue = 0;

	// Enemy color
	char enemyColor = getOtherSide(color);

	// get the number of available moves for each player
    Move moves[100];
    int my_moves = countAvailableMoves(currentPosition, moves, color);
    int enemy_moves = countAvailableMoves(currentPosition, moves, enemyColor);


	// Game Progress
    int total_discs = currentPosition->score[WHITE] + currentPosition->score[BLACK];
    double gameProgress = (double)total_discs/TOTAL_EMPTY_CELLS;

    // Disc difference #myDisks - #opponentDisks
    int my_score = currentPosition->score[(int)color];
    int enemy_score = currentPosition->score[(int)enemyColor];


    int weight_factor = (gameProgress < 0.5) ? 1 : 3;
    stateValue += weight_factor * (my_score - enemy_score);

    // Mobility (More Important for White)
	// to counter enemy
    int mobilityWeight = (color == WHITE) ? 10 : 5;
    stateValue += mobilityWeight * (my_moves - enemy_moves);

    // corners
    int cornerWeight = 25;

	// On a 15x15 hexagonal board, there are 6 corners
	// {0, 7} {0, 14} {7, 0} {7, 14} {14, 0} {14, 7}
	int corners[6][2] = {{0, 7}, {0, 14}, {7, 0}, {7, 14}, {14, 0}, {14, 7}};

	for (int i = 0; i < 6; i++) {
		if (currentPosition->board[corners[i][0]][corners[i][1]] == color) {
			stateValue += cornerWeight;
		}
		if (currentPosition->board[corners[i][0]][corners[i][1]] == enemyColor) {
			stateValue -= cornerWeight;
		}
	}

	// corners control because they are the most important, not changeable, stable
    int stabilityScore = 0;
	int offset = (ARRAY_BOARD_SIZE - 1) / 2;

	for (int i = 0; i < ARRAY_BOARD_SIZE; i++) {
		for (int j = 0; j < ARRAY_BOARD_SIZE; j++) {
			if (currentPosition->board[i][j] == color) {
				// ignore out of bound spaces
				if (currentPosition->board[i][j] == OUT_OF_BOUND){
					continue;
				}

				// Top-edge
				if (i == 0 && (j >= offset && j <= ARRAY_BOARD_SIZE - offset - 1)) {
					stabilityScore += 2;
				}

				// Bottom-edge
				if (i == ARRAY_BOARD_SIZE - 1 && (j >= offset && j <= ARRAY_BOARD_SIZE - offset - 1)) {
					stabilityScore += 2;
				}

				// Left-edge
				if (j == offset - i || j == offset - (ARRAY_BOARD_SIZE - 1 - i)) {
					stabilityScore += 2;
				}

				// Right-edge
				if (j == offset + i || j == offset + (ARRAY_BOARD_SIZE - 1 - i)) {
					stabilityScore += 2;
				}

				// Near-edge stability (adjacent to a stable edge piece)
				if ((i > 0 && i < ARRAY_BOARD_SIZE - 1) && (j > 0 && j < ARRAY_BOARD_SIZE - 1)) {
					// check if the piece is stable
					// observe the geitonika boxes
					if (currentPosition->board[i-1][j] == color && currentPosition->board[i+1][j] == color && currentPosition->board[i][j-1] == color && currentPosition->board[i][j+1] == color) {
						stabilityScore += 1;
					}
				}
			}
		}
	}


	// black should capture corners, be more stable than white.
    int stabilityWeight = (color == BLACK) ? 8 : 3;
    stateValue += stabilityScore * stabilityWeight;

	// me trying to outperform the enemy on the tournament
	// STRATEGY: Make opponent apply their worst move by increasing the stability of the board
    return stateValue;
}


// --- Minimax Algorithm ---
int minimax(Position *currentPosition, int depth, int alpha, int beta, int maximizingPlayer) {

	// -> Break condition
	// -> Reached maximum depth or no more moves possible
	if (depth == 0 || (!canMove(currentPosition, WHITE) && !canMove(currentPosition, BLACK)))
        return evaluatePosition(currentPosition, myColor);


	// -> Get all available moves
    Move moves[MAX_MOVES_SEARCH];
    int total_available_moves = countAvailableMoves(currentPosition, moves, maximizingPlayer ? myColor : getOtherSide(myColor));

	// If not moves are available then, evaluate current position and return
    if (total_available_moves == 0)
        return evaluatePosition(currentPosition, myColor);

    if (maximizingPlayer) {
		// -> Maximize the score, starting from -infinity(or the lowest possible value)
        int max_f_score = INT_MIN;

		for (int i = 0; i < total_available_moves; i++) {
  			// copy the current position
			Position temporaryPosition = *currentPosition;

			// make the move on the temporary position
			doMove(&temporaryPosition, &moves[i]);

			// starting minimax on that move, with the other player turn
			int current_move_evaluation = minimax(&temporaryPosition, depth - 1, alpha, beta, FALSE);

            max_f_score = max(current_move_evaluation, max_f_score);

			if (AB_PRUNING) {
				alpha = max(current_move_evaluation, alpha);

				// pruning, saving time
				if (beta <= alpha)
					return max_f_score;
			}
        }

        return max_f_score;
    }
	else {
		// -> Minimize the score, starting from +infinity(or the highest possible value)
        int min_f_score = INT_MAX;

		for (int i = 0; i < total_available_moves; i++) {
  			// copy the current position
			Position temporaryPosition = *currentPosition;

			// make the move on the temporary position
			doMove(&temporaryPosition, &moves[i]);

			// starting minimax on that move, with the other player turn
            int current_move_evaluation = minimax(&temporaryPosition, depth - 1, alpha, beta, TRUE);

			min_f_score = min(current_move_evaluation, min_f_score);

			if (AB_PRUNING) {
            	beta = min(current_move_evaluation, beta);

				// pruning, saving time
				if (beta <= alpha)
					return min_f_score;
			}
        }

        return min_f_score;
    }
}



// --- Select Best Move ---
Move getBestMove(Position *currentPosition, char color) {

	// minimax parameters
    int best_move_scored = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;


	// get all available moves for the current player
    Move moves[MAX_MOVES_SEARCH];
    int total_available_moves = countAvailableMoves(currentPosition, moves, color);

	// assume a perfect move
	Move bestMove;

	// if no moves are available, return a null move
	if (total_available_moves == 0) {
		bestMove.tile[0] = NULL_MOVE;
		return bestMove;
	}

    for (int i = 0; i < total_available_moves; i++) {
        // copy the current position
		Position temporaryPosition = *currentPosition;
		doMove(&temporaryPosition, &moves[i]);

		// start minimax using the temporary position (the move is already applied) and see if it's a good move
		int current_move_evaluation = minimax(&temporaryPosition, MAX_DEPTH - 1, alpha, beta, FALSE);

        // if a better move is found update the best move
		if (current_move_evaluation > best_move_scored) {
            best_move_scored = current_move_evaluation;
            bestMove = moves[i];
        }

		if (AB_PRUNING) {
			// update alpha
			alpha = max(current_move_evaluation, alpha);

			// else prune occurs
			if (beta <= alpha)
				break;
		}
    }

    return bestMove;
}

/**********************************************************/

// --- Main ---
int main( int argc, char ** argv )
{
	int c;
	opterr = 0;

	while( ( c = getopt ( argc, argv, "i:p:h" ) ) != -1 )
		switch( c )
		{
			case 'h':
				printf( "[-i ip] [-p port]\n" );
				return 0;
			case 'i':
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case '?':
				if( optopt == 'i' || optopt == 'p' )
					printf( "Option -%c requires an argument.\n", ( char ) optopt );
				else if( isprint( optopt ) )
					printf( "Unknown option -%c\n", ( char ) optopt );
				else
					printf( "Unknown option character -%c\n", ( char ) optopt );
				return 1;
			default:
			return 1;
		}

	connectToTarget( port, ip, &mySocket );

	while(TRUE)
	{
		msg = recvMsg( mySocket );
		switch ( msg )
		{
			case NM_REQUEST_NAME:		//server asks for our name
				sendName( agentName, mySocket );
				break;

			case NM_NEW_POSITION:		//server is trying to send us a new position
				getPosition( &gamePosition, mySocket );
				printPosition( &gamePosition );
				break;

			case NM_COLOR_W:			//server informs us that we have WHITE color
				myColor = WHITE;
				break;

			case NM_COLOR_B:			//server informs us that we have BLACK color
				myColor = BLACK;
				break;

			case NM_PREPARE_TO_RECEIVE_MOVE:	//server informs us that he will now send us opponent's move
				getMove( &moveReceived, mySocket );
				moveReceived.color = getOtherSide( myColor );
				doMove( &gamePosition, &moveReceived );		//play opponent's move on our position
				printPosition( &gamePosition );
				break;

			case NM_REQUEST_MOVE:		//server requests our move
				myMove.color = myColor;

				if(!canMove(&gamePosition, myColor)){
					myMove.tile[ 0 ] = NULL_MOVE;		// we have no move ..so send null move
				}
				else
					myMove = getBestMove(&gamePosition, myColor);

				sendMove( &myMove, mySocket );			//send our move
				doMove( &gamePosition, &myMove );		//play our move on our position
				printPosition( &gamePosition );
				break;

			case NM_QUIT:			//server wants us to quit...we shall obey
				close( mySocket );
				return 0;
		}
	}

	return 0;
}
