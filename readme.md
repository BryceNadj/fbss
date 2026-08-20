# fbss

### Frame Buffer Screenshot

A simple tool that will capture the contents of the linux framebuffer and save it as a bmp file in `~/Pictures/fbss`.

Wayland/X11 doesn't use the linux framebuffer so this won't screenshot your desktop,
there are already tools for that.
The linux virtual terminal (usually accessed through `ctrl + alt + F3` or something)
uses the framebuffer so this tool will allow you to screenshot your display there.

_Must run with `sudo` in order to be able to open the framebuffer._

### TODO
- [x] Increment filename to save a new image instead of overwriting the last saved image
- [x] Set uid of output file to user so they have persmission to do things like deleting the file
- [ ] Save disk space by ignoring the transparency color channel
