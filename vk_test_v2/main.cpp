#include "Engine.hpp"

#include <iostream>

int main(int argc, char* argv[]) {
    try {
        std::cout << "Initializing." << std::endl;

        auto engine = vke::VulkanEngine();

        std::cout << "Initialized." << std::endl;

        engine.run();

        std::cout << "Deinitializing." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << std::format("Caught {}: {}", typeid(e).name(), e.what()) << std::endl;
    } catch (...) { std::cerr << "Unknown Exception" << std::endl; }

    std::cout << "Deinitialized." << std::endl;
    return 0;
}
