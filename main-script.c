#include <stdio.h>
#include <stdlib.h>
#include <helper-functions.h>

int main() {
    printf("%s", "Installing VLC \n");
    system("sudo pacman -S vlc --noconfirm");
    printf("%s", "Done installing vlc");
    return 0;
}
