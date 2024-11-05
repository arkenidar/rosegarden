
# ChatGPT for app idea and code
<https://chatgpt.com/share/672a2899-bfb8-800a-8129-caec3ac063cd>

# use of app
use A S D F Keys ... to play sinusoidal sounds

# [BUILD app] GCC builds ( both in Windows or Linux )
- gcc -o app sound.c $(pkgconf --cflags --libs SDL2 SDL2_mixer) -lm

# [WIN setup] install using pacman in MSYS / MinGW ( Windows )
- pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-pkgconf # build tools for producing app
- pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_mixer # used libSDL libraries

# [LIN setup] install using APT ( Debian GNU / Linux )
- sudo apt install libsdl2-dev libsdl2-mixer-dev # used libSDL libraries

# folk-lore note
The rosegarden name is a reference to the real one:
<https://www.rosegardenmusic.com/>