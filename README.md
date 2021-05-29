# world_mapper
OLC:PGE application for making arbitrary resolution maps. Mostly a work in progress - this is not meant to work out of the box.

Instead of using classical raster graphics, image data is stored as a quadtree, allowing arbitrary resolution. Thus, savefiles are also currently stored as a custom file type (appropriately named ".cff")

It uses the OneLoneCoder's (PixelGameEngine)[https://github.com/OneLoneCoder/olcPixelGameEngine] for the interface. You will need it (save in working directory as "olc.h")

Controls:
- CTRL+mouseclick for color picking
- arrow keys for movement, SHIFT and CTRL increase speed
- 'R' and 'F' to zoom in/out
- '+' and '-' (on the numpad) to change brush size

- Escape to exit
- D to display quadtree
- L to recenter, '*' (on numpad) to reset zoom and offset
- B and S to switch between normal brush and spray brush
- 'O' to open file, 'M' to save


Known bugs:
- brush icon and actual brush do not align
- zoom is off-center ('L' was added as a temporary workaround)
- spray brush is currently very limited in functionality
- cannot export images


29.5.21: I've fixed most of these, I intend to port it to my [engine](https://github.com/alve1801/engine) and then I'll update
