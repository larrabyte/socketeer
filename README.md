# Socketeer
Socketeer is a small utility program aimed at cross-platform data transfer using [ASIO](https://github.com/chriskohlhoff/asio). Tested on macOS and Linux, untested on Windows.

## Prerequisites and Compilation
Install ASIO as a standalone library by either using a package manager of your choice (pacman and homebrew are my favourites, so: `sudo pacman -S asio` or `brew install asio`) or by getting it from their [downloads page](http://think-async.com/Asio/Download.html). Add an include flag to the Makefile if necessary. Once you have ASIO installed, run `make` to compile Socketeer!
