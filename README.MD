# TODO HERE

```sh
time cmake -B builddir -G Ninja
time cmake -B builddir -G Ninja -DCMAKE_BUILD_TYPE=Debug --toolchain ./clangd_compile.cmake
time cmake -B builddir -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo --toolchain ./clangd_compile.cmake
time ninja -C builddir
scan-build cmake -B builddir
scan-build make -C builddir -j $(($(nproc) - 2))
```

[This](https://github.com/mesonbuild/meson/issues/10590) is my reasoning for not using meson.

I am using `auto` throughout my Vulkan code, because vk::raii types can be implicitly converted to vk:: types, which causes typo bugs that are hard to find.

All of the vulkan projects in this repo are currently under construction.
