#pragma once

#include <array>
#include <bitset>

/// The Tic Tac Toe game.
class Game {
  /// Represents the game board.
  ///
  /// arr[0] = Player X pieces
  /// arr[1] = Player O pieces
  std::array<std::bitset<9>, 2> game_board;

public:
  /// The main entry point for the game.
  void run();

private:
  /// Prints the current state of the game board.
  void printTiles();

  /// Gets the player's input and updates the game board.
  void getInput(bool player);

  /// Checks if the current player has won or if the game is a draw.
  bool checkForWin(bool player);
};
