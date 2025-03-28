#include "ops.hpp"

#include <iostream>

/// Maps the player ID to a string representation of their game piece.
constexpr std::array player_map{
    "\u001b[31mX\u001b[0m",
    "\u001b[34mO\u001b[0m",
};

/// ANSI escape code to clear the screen.
constexpr char const* clear_screen = "\033[2J\033[H";

void Game::run() {
    // Initialize the game board here so that the game can be restarted.
    game_board = {
        0b000000000,
        0b000000000,
    };

    // The game loop.
    for (bool player = false;; player = !player) {
        printTiles();
        getInput(player);
        if (checkForWin(player)) { break; }
    }
}

void Game::printTiles() {
    // Compute the filled spots on the gameboard.
    auto filled_spots = game_board[0] | game_board[1];

    // Print the Game Board.
    std::cout << clear_screen << "+---+---+---+\n";
    for (int row = 0; row < 9; row += 3) {
        std::cout << '|';
        for (int col = 0; col < 3; col++) {
            int index = row + col;

            std::cout << ' ';
            if (filled_spots[index]) {
                std::cout << player_map[game_board[1][index]];
            } else {
                std::cout << "\u001b[32m" << index + 1 << "\u001b[0m";
            }
            std::cout << " |";
        }
        std::cout << "\n+---+---+---+\n";
    }
}

void Game::getInput(bool player) {
    // Prompt loop.
    while (true) {
        // Print out the prompt.
        std::cout << "What spot, player " << player_map[player] << "? ";

        // NOTE: Can't use an immediately invoked lambda here because of the
        // try-catch.
        int choice;
        {
            // Get the user's choice.
            std::string s_choice;
            std::getline(std::cin, s_choice, '\n');

            // Try to convert the choice to an integer while handling invalid
            // input.
            try {
                // This can throw an error, so catch it!
                choice = std::stoi(s_choice) - 1;
            } catch (const std::logic_error&) {
                std::cout << "Not a valid number!\n";
                continue;
            }
        }

        // Check if the choice is in the right range.
        if ((choice >= 0) && (choice < 9)) {
            // Check if the spot has already been taken.
            if (~(game_board[0] | game_board[1])[choice]) {
                // Mark the player's choice on the game board and exit the
                // prompt loop.
                game_board[player][choice] = true;
                break;
            } else {
                std::cout << "Spot Already Taken!\n";
            }
        } else {
            std::cout << "Out of Range!\n";
        }
    }
}

bool Game::checkForWin(bool player) {
    // All possible win conditions.
    constexpr std::array<std::bitset<9>, 8> win_states{
        0b111000000,
        0b000111000,
        0b000000111,
        0b100100100,
        0b010010010,
        0b001010100,
        0b100010001,
        0b001001001,
    };

    // Check if the game is a draw.
    if ((game_board[0] | game_board[1]) == 0b111111111) {
        printTiles();
        std::cout << "It's a draw!\n";
        return true;
    }

    // Check the board against each win-state.
    for (auto win_state : win_states) {
        if ((game_board[player] & win_state) == win_state) {
            printTiles();
            std::cout << "Player " << player_map[player] << " wins!\n";
            return true;
        }
    }

    // Continue the game.
    return false;
}
