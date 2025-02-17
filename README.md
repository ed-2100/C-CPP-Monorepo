# TODO HERE

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

[This](https://github.com/mesonbuild/meson/issues/10590) is my reasoning for not using meson.

I am using `auto` throughout my Vulkan code in `vk_test_v3`, because `vk::raii` types can be implicitly converted to `vk::` types, which causes typo bugs that are hard to find.

All of the vulkan projects in this repo are currently under construction.
