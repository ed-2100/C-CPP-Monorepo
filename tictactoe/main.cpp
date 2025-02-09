#include "ops.hpp"

#include <iostream>

int main() {
  // Initialize the Game Board.
  // arr[0] = Player X pieces
  // arr[1] = Player O pieces
  GameState game_board{
      0b000000000,
      0b000000000,
  };

  // Introduction.
  std::cout << "TicTacToe!\n";

  // The game loop.
  for (bool player = false;; player = !player) {
    printTiles(game_board);
    getInput(game_board, player);
    if (checkForWin(game_board, player)) {
      break;
    }
  }

  return 0;
}
