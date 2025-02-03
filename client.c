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
#define AB_PRUNING FALSE

#define max(a, b) ((a > b) ? a : b)
#define min(a, b) ((a < b) ? a : b)


/**********************************************************/
Position gamePosition;		// Position we are going to use

Move moveReceived;			// temporary move to retrieve opponent's choice
Move myMove;				// move to save our choice and send it to the server

char myColor;				// to store our color
int mySocket;				// our socket
char msg;					// used to store the received message

char * agentName = "Papou!";		//default name.. change it! keep in mind MAX_NAME_LENGTH

char * ip = "127.0.0.1";	// default ip (local machine)
/**********************************************************/


/* Minimax & Evaluation */

// --- Evaluation function (f) ---
int evaluatePosition(Position *currentPosition, char playerColor) {
	// V(state) = #myDisks - #opponentDisks
	int myAgentScore = currentPosition->score[playerColor];
	int opponentAgentScore = currentPosition->score[getOtherSide(playerColor)];
	int stateValue = myAgentScore - opponentAgentScore;

	return stateValue;
}

int getAvailableMoves(Position *pos, char color, Move moves[]) {
    int count = 0;
    for (int i = 0; i < ARRAY_BOARD_SIZE; i++) {
        for (int j = 0; j < ARRAY_BOARD_SIZE; j++) {
            if (pos->board[i][j] == EMPTY) {  // Look for empty spaces
                Move move = {{i, j}, color};
                if (isLegal(pos, i, j, color)) {  // Check if the move is legal
                    moves[count++] = move;
                }
            }
        }
    }
    return count;  // Return number of valid moves found
}

// copy the board state and apply the move
void makeMove(Position *src, Position *dst, Move *move) {
    *dst = *src;
    doMove(dst, move);
}


// --- Minimax Algorithm ---
int minimax(Position *currentPosition, int depth, int alpha, int beta, int maximizingPlayer) {

	// -> Break condition
	// -> Reached maximum depth or no more moves possible
	if (depth == 0 || (!canMove(currentPosition, WHITE) && !canMove(currentPosition, BLACK))) {
        return evaluatePosition(currentPosition, myColor);
    }

	// -> Get all available moves
    Move moves[100];
    int numMoves = getAvailableMoves(currentPosition, maximizingPlayer ? myColor : getOtherSide(myColor), moves);

    if (numMoves == 0)
        return evaluatePosition(currentPosition, myColor);

    if (maximizingPlayer) {
        int maxEval = INT_MIN;
        for (int i = 0; i < numMoves; i++) {
            Position newPosition = *currentPosition;
            makeMove(currentPosition, &newPosition, &moves[i]);
            int eval = minimax(&newPosition, depth - 1, alpha, beta, FALSE);
            maxEval = max(eval, maxEval);

			if (AB_PRUNING) {
				alpha = max(eval, alpha);
				if (beta <= alpha) return maxEval;  // Prune
			}
        }

        return maxEval;
    }
	else {
        int minEval = INT_MAX;
        for (int i = 0; i < numMoves; i++) {
            Position newPosition = *currentPosition;


			// should i copy with function or do ti here?
            makeMove(currentPosition, &newPosition, &moves[i]);
            int eval = minimax(&newPosition, depth - 1, alpha, beta, TRUE);
            minEval = min(eval, minEval);

			if (AB_PRUNING) {
            	beta = min(eval, beta);
            	if (beta <= alpha)
					return minEval;
			}
        }

        return minEval;
    }
}



// --- Selecting the Best Move ---
Move getBestMove(Position *pos, char color) {
	// assume best move
    Move bestMove;
    int bestEval = INT_MIN;
    int alpha = INT_MIN;
    int beta = INT_MAX;

    Move moves[100];

	// get all available moves for the current player
    int numMoves = getAvailableMoves(pos, color, moves);

    for (int i = 0; i < numMoves; i++) {
        Position newPosition = *pos;
        makeMove(pos, &newPosition, &moves[i]);

		// evaluate the move
		int eval = minimax(&newPosition, MAX_DEPTH - 1, alpha, beta, FALSE);

        // if a better move is found update the best move
		if (eval > bestEval) {
            bestEval = eval;
            bestMove = moves[i];
        }

		// update alpha
		alpha = max(eval, alpha);
        if (beta <= alpha) {
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
				else {
					myMove = getBestMove(&gamePosition, myColor);
				}

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
