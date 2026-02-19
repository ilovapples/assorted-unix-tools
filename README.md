## build
You can build with `make` to build every tool, but this project setup lends itself well to parallel compilation, so `-j` is recommended.
`make install` exports all the built tools to `INSTALL_PREFIX/bin/apple-aut-bin/` (`INSTALL_PREFIX` defaults to `$HOME/.local`),
and you can change `INSTALL_PREFIX` to specify the installation path.
To remove exported tools from the `INSTALL_PREFIX`, just run `make clean_installed`.

Every executable will be placed in `INSTALL_PREFIX/bin/apple-aut-bin/`, except for `apple-aut`, which is the tool used to
run the others, and is placed at`INSTALL_PREFIX/bin/apple-aut`. You call it with `apple-aut <tool_name>`, or `apple-aut help`
for a list of all installed tools.
