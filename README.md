# SDL/OpenGL Pong

This is a super simple pong game I made because I was bored and wanted to see how long it would take.  It uses OpenGL 3.3, SDL2, and orthodox C++.

It is not particularly well organized, but that was not the intention.  This was _not_ developed to follow proper software architecture, game architecture, or anything.  It's simple and gets the job done.  That is it.

## Building
1. Clone submodules (`git submodule update --init --recursive`)
2. Make a build directory (`mkdir build; cd build`)
3. Run cmake (`cmake -GNinja ../`)
4. Build (`cmake --build .`)

Or if you're on Windows, simply run `windows_scripts/run.bat` in a native tools command prompt for your version of Visual Studio (or any command prompt that has been initialized as such).

## Status
Basic structure is set up, implementing game logic and finishing rendering logic tomorrow.