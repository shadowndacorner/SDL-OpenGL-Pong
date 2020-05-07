# SDL/OpenGL Pong

This is a super simple pong game I made because I was bored and wanted to see how long it would take.  It uses OpenGL 3.3, SDL2, OpenAL-Soft, and mostly C-style C++ (with a few cheats here and there, like the use of vectors and references in a few places).

It is not super well organized, but that was not the intention.  This was _not_ developed to follow proper software architecture, game architecture, or anything.  It's simple and gets the job done.  That is it.

All collisions are basic axis aligned rectangle tests.  There's nothing clever going on there (or anywhere else, really) - the ball just checks to see if its bounding rect intersects with that of the walls or paddles.

The only remotely clever thing happening here is that it uses instanced rendering for all of the rects, but even that is probably less efficient than it would be by just streaming the vertex/index buffers every frame since it's sending a model matrix per rect, which is 16 floats, compared to 4 floats + 4 shorts for streaming a VBO/IBO.  This was yet another choice that was made just because it seemed more interesting to do that way.

## Building
1. Clone submodules (`git submodule update --init --recursive`)
2. Make a build directory (`mkdir build; cd build`)
3. Run cmake (`cmake -GNinja ../`)
4. Build (`cmake --build .`)
5. Copy the `data` folder into `bin/data`

Or if you're on Windows, simply run `windows_scripts/build.bat` in a native tools command prompt for your version of Visual Studio (or any command prompt that has been initialized as such).  It will automatically do all of the above.  Alternatively, run `windows_scripts/run.bat`, which will do all of the above and launch the game.

Note: OpenAL-Soft was acquired with vcpkg.  I'd recommend using it, as in my experience the submodule approach for OpenAL doesn't work as well.

## Status
It's more or less done.  Guess I could add multiplayer or text rendering or something, but that seems like more work than it's worth.