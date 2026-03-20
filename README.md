## build
You can build with `make` to build every tool. This project setup lends
itself well to parallel compilation, so `-j` is recommended. `make install`
exports all the built tools to `INSTALL_PREFIX/bin/apple-aut-bin/`
(`INSTALL_PREFIX` defaults to `$HOME/.local`), and you can change
`INSTALL_PREFIX` to specify the installation path. To remove exported tools
from the `INSTALL_PREFIX`, just run `make clean_installed`.

`make install` also installs the library file and headers to
`INSTALL_PREFIX/lib` and `INSTALL_PREFIX/include/autlib`, respectively, in case
you want to use them for your own projects.

Every executable will be placed in `INSTALL_PREFIX/bin/apple-aut-bin/`, except
for `apple-aut`, which is the tool used to run the others (to prevent
cluttering PATH), and is placed at `INSTALL_PREFIX/bin/apple-aut`. You call it
with `apple-aut <tool_name>`, or `apple-aut help` for a list of all installed
tools.

To pass extra flags along to compile commands, add `EXTRA_CFLAGS={your cflags}`
and/or `EXTRA_LDFLAGS={your ldflags}` for compiler and linker flags,
respectively, to your `make` command.


---

~~If it fails to build with a linker error about an 'expand_bindir_path' symbol, try
using `CC=` to set a different C compiler. This happens for me on x86_64 Linux, and
using `CC=zig cc` fixed it for me, so you can try that if you have Zig installed.~~
This should be fixed; please open an issue on Github if you still have problems.
