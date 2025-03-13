#include <chrono>
#include <cstddef>
#include <iostream>
#include <memory>
#include <random>

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>

constexpr size_t width = 1000;
constexpr size_t height = 1000;

int main(int /*argc*/, const char** /*argv*/) {
    sf::RenderWindow window;
    window.create(
        sf::VideoMode({width, height}),
        "GOL",
        sf::Style::Titlebar | sf::Style::Close
    );
    // window.setFramerateLimit(100);

    sf::Texture texture({width, height});

    sf::Sprite sprite(texture);

    auto pixels = std::make_unique<uint8_t[][width][4]>(height);

    std::fill(
        reinterpret_cast<uint8_t*>(pixels.get()),
        reinterpret_cast<uint8_t*>(pixels.get()) + height * width * 4,
        0
    );

    auto past_state = std::make_unique<bool[][width]>(height);
    auto state = std::make_unique<bool[][width]>(height);

    auto rand = std::minstd_rand(
        std::chrono::steady_clock::now().time_since_epoch().count()
    );
    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            past_state[row][col] = rand() % 2 == 0;
            state[row][col] = false;
        }
    }

    const auto starttime = std::chrono::steady_clock::now();

    auto last_end_time = starttime;
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code
                    == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        for (size_t row = 0; row < height; row++) {
            const size_t row_min = row == 0 ? 0 : row - 1;
            const size_t row_max = row == height - 1 ? height - 1 : row + 1;

            for (size_t col = 0; col < width; col++) {
                const size_t col_min = col == 0 ? 0 : col - 1;
                const size_t col_max = col == width - 1 ? width - 1 : col + 1;

                size_t sum = 0;
                for (size_t row = row_min; row <= row_max; row++) {
                    for (size_t col = col_min; col <= col_max; col++) {
                        if (past_state[row][col]) {
                            sum++;
                        }
                    }
                }

                if (past_state[row][col]) {
                    sum--;
                }

                const bool result =
                    sum == 3 || (past_state[row][col] && sum == 2);
                state[row][col] = result;
                pixels[row][col][3] = result * 255;
            }
        }

        texture.update(reinterpret_cast<uint8_t*>(pixels.get()));
        window.clear(sf::Color(255, 255, 255, 255));
        window.draw(sprite);
        window.display();

        std::swap(state, past_state);

        auto end_time = std::chrono::steady_clock::now();

        static auto last_printed =
            std::chrono::time_point<std::chrono::steady_clock>(
                std::chrono::seconds(0)
            );

        if (end_time - last_printed > std::chrono::milliseconds(100)) {
            std::cout << "\033[s\033[K"
                      << 1
                    / std::chrono::duration_cast<std::chrono::duration<float>>(
                          end_time - last_end_time
                    )
                          .count()
                      << "\033[u" << std::flush;
            last_printed = end_time;
        }

        last_end_time = end_time;
    }

    return 0;
}
