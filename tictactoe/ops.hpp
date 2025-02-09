#pragma once

#include <array>
#include <bitset>

constexpr std::array player_map{
    "\u001b[31mX\u001b[0m",
    "\u001b[34mO\u001b[0m",
};

using GameState = std::array<std::bitset<9>, 2>;

void printTiles(GameState game_board);
void getInput(GameState& game_board, bool player);
bool checkForWin(GameState game_board, bool player);
