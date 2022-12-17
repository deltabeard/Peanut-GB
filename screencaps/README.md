# Screencaps

These screencaps were made by using the "Dump BMP" feature of the Peanut-SDL
example. This feature dumps each frame as a 24-bit bitmap in the running folder.
This feature may be toggled on and off by pressing 'b' on the keyboard.

To convert the output bitmaps to a GIF, I used the following commands:
```
# Generate palette for selected bitmap. This bitmap should have all the colours
# that will appear in the sequence. This is to reduce the output file size of
the GIF later.
ffmpeg -y -i MEGAMANV_%010d.bmp -vf palettegen palette.png

# Convert the bitmaps to GIF. 50 FPS output is used since anything higher
# produces a GIF that is around 22 FPS for some reason.
ffmpeg -y -framerate 4194304/70224 -i MEGAMANV_%010d.bmp -i palette.png -filter_complex "paletteuse" -r 50 MEGAMANV.gif
```
