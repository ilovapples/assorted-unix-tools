## build
you can build with `make` to build every tool, but this project setup lends itself well to parallel compilation, so `-j` is recommended.
`make export` exports all the built executables to `INSTALL_PREFIX/bin` (`INSTALL_PREFIX` defaults to `$HOME/.local`),
and you can change `INSTALL_PREFIX` to specify the installation path.
To remove exported tools from the `INSTALL_PREFIX`, just run `make clean_exported`.
