#include <array>
#include <chrono>
#include <cmath>
#include <iostream>
#include <memory>

#include <immintrin.h>
#include <xmmintrin.h>

#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>

// See the rust workspace for a more improved version of this code.

constexpr uint32_t width = 1000;
constexpr uint32_t height = 1000;

int main(int /*argc*/, const char ** /*argv*/) {
    sf::RenderWindow window;
    window.create(
        sf::VideoMode({width, height}),
        "Glowing Dots",
        sf::Style::Titlebar | sf::Style::Close
    );
    // window.setFramerateLimit(100);

    sf::Texture texture({width, height});

    sf::Sprite sprite(texture);

    auto pixels = std::make_unique<uint8_t[][width][4]>(height);

    std::fill(
        reinterpret_cast<uint8_t *>(pixels.get()),
        reinterpret_cast<uint8_t *>(pixels.get()) + height * width * 4,
        255
    );

    const auto starttime = std::chrono::steady_clock::now();

    auto last_end_time = starttime;
    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            } else if (event->is<sf::Event::KeyPressed>()) {
                if (event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
            }
        }

        const float time_elapsed =
            std::chrono::duration_cast<std::chrono::duration<float>>(last_end_time - starttime)
                .count();

        const float rotation = 0.5 * M_PI * time_elapsed;
        const __m128 c = _mm_set1_ps(std::cos(rotation));
        const __m128 s = _mm_set1_ps(std::sin(rotation));

        // Original dot coords.
        constexpr __m128 dot_x = {0.5, -0.5, -0.5, 0.5};
        constexpr __m128 dot_y = {0.5, 0.5, -0.5, -0.5};

        // Rotated dot coords.
        __m128 r_dot_x = (dot_x * c) - (dot_y * s);
        __m128 r_dot_y = (dot_y * c) + (dot_x * s);

        for (size_t pix_y = 0; pix_y < height; pix_y++) {
            const float y = static_cast<float>(pix_y) / height * 2 - 1;

            for (size_t pix_x = 0; pix_x < width; pix_x++) {
                const float x = static_cast<float>(pix_x) / width * 2 - 1;

                const __m128 delta_x = r_dot_x - x;
                const __m128 delta_y = r_dot_y - y;
                const __m128 dist_sqrd = delta_x * delta_x + delta_y * delta_y;
                const __m128 mag = 1 / (dist_sqrd * 30 + 1);

                const union {
                    __m128 simd;
                    std::array<float, 4> indices;
                } pixarray = {
                    .simd =
                        _mm_min_ps((mag + _mm_permute_ps(mag, 0xFF)) * 255 + 0.5, _mm_set1_ps(255))
                };

                // If only there were a better way to do this...
                auto pixel = pixels[pix_y][pix_x];
                pixel[0] = pixarray.indices[0];
                pixel[1] = pixarray.indices[1];
                pixel[2] = pixarray.indices[2];
            }
        }

        texture.update(reinterpret_cast<uint8_t *>(pixels.get()));
        // window.clear();
        window.draw(sprite);
        window.display();

        auto end_time = std::chrono::steady_clock::now();

        static auto last_printed =
            std::chrono::time_point<std::chrono::steady_clock>(std::chrono::seconds(0));

        if (end_time - last_printed > std::chrono::milliseconds(100)) {
            std::cout << "\033[s\033[K"
                      << 1 / std::chrono::duration_cast<std::chrono::duration<float>>(
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
