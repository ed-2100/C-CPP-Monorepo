# C/C++ Projects Monorepo

A mono repo for all of my desktop C/C++ projects.

## Projects

 - [`cleanup_stack`](cleanup_stack/README.md)
 - [`game_of_life`](game_of_life/README.md)
 - [`glowing_dots`](glowing_dots/README.md)
 - [`tictactoe`](tictactoe/README.md)
 - [`vk_test`](vk_test/README.md)

## Dependencies

The Vulkan SDK and SDL3 is required to build this project.

## Building

To build on Linux:
```sh
cmake -B builddir
make -C builddir
```

## Other Development Commands

- **Formatting** \
  Requires `clang-format` and `fd-find`.
  ```sh
  clang-format -i $(fd --glob '*.{cpp,hpp,c,h}' --exclude ./builddir/ --type file ./)
  ```

- **Retrieving the Line Count** \
  Requires `fd-find` from crates.io.
  ```sh
  wc -l $(fd --glob '*' --exclude ./builddir/ --type file ./)
  ```

- **Leak Checking** \
  Requires `valgrind`.
  ```sh
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,possible $EXECUTABLE_NAME
  ```

## Notes on Toolchain

- [This](https://github.com/mesonbuild/meson/issues/10590) is my reasoning for not using meson.

## TODO HERE

```sh
time cmake -B builddir -G Ninja
time cmake -B builddir -G Ninja -DCMAKE_BUILD_TYPE=Debug --toolchain ./clang_compile.cmake
time cmake -B builddir -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo --toolchain ./clang_compile.cmake
time ninja -C builddir
scan-build cmake -B builddir
scan-build make -C builddir -j $(($(nproc) - 2))
env SDL_VIDEODRIVER=wayland ./builddir/vk_test_v4/vk_test_v4
wc -l $(fd --glob '*' --exclude ./builddir/ --type file ./)
clang-format -i $(fd --glob '*.{cpp,hpp,c,h}' --exclude ./builddir/ --type file ./)
env SDL_VIDEODRIVER=wayland valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,possible ./builddir/vk_test_v3/vk_test_v3
```

I am using `auto` throughout my Vulkan code in `vk_test_v3`, because `vk::raii` types can be implicitly converted to `vk::` types, which causes typo bugs that are hard to find.

All of the vulkan projects in this repo are currently under construction.
