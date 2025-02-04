# HexThello-AI-Agent

## Overview
HexThello AI is an agent designed to play HexThello, a hexagonal variant of Othello/Reversi. The AI uses the Minimax algorithm with Alpha-Beta Pruning for decision-making.

## Requirements
* Any Linux distribution

## Installation & Setup
### Clone the Repository
```sh
git clone https://github.com/npapoutsakis/hexThello.git
cd hexThello
```

### Compile the Code
```sh
make all   # Builds both client and server
make client   # Builds only the client
make server   # Builds only the server
```

## Execution
* `./guiServer`
* `./server [-p port] [-g number_of_games] [-s (swap color after each game)]`
* `./client [-i ip] [-p port]`

## How It Works
The AI uses Minimax with Alpha-Beta Pruning to evaluate board positions efficiently. It dynamically adapts strategies for both players and prioritizes corner control, mobility, and stability.

## License
This project is licensed under the MIT License.

