# C/C++ Projects Monorepo

A monorepo for all of my desktop C/C++ projects.

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
  valgrind --tool=memcheck --leak-check=full --show-leak-kinds=definite,possible ./builddir/my_exe/my_exe
  ```

- **Static Analysis** \
  Requires `clang-analyzer`.

  ```sh
  scan-build cmake -B builddir
  scan-build make -C builddir -j $(($(nproc) - 2)
  ```

## Notes on Toolchain

- [This](https://github.com/mesonbuild/meson/issues/10590) is my reasoning for not using meson.
- To use clang explicitly, add `--toolchain clang_compile.cmake` to your cmake command.
- To use wayland explicitly with SDL programs, prepend `env SDL_VIDEODRIVER=wayland` to your shell command.
