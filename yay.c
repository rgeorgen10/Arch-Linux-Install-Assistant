#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

int main() {
    system("arch-chroot /mnt useradd -mG wheel temp");
    system("arch-chroot /mnt pacman -S --needed --noconfirm git base-devel");
    system("echo 'temp:passwd' | arch-chroot /mnt chpasswd");
    system("arch-chroot /mnt sudo -i -u temp git clone https://aur.archlinux.org/yay.git /home/temp/yay && arch-chroot /mnt sudo -i -u temp sh -c 'cd /home/temp/yay && makepkg -s'");
    system("arch-chroot /mnt pacman -U /home/temp/yay/*.pkg.tar.zst");
    // system("arch-chroot /mnt rm -r /home/temp");
    // system("arch-chroot /mnt userdel temp");
    return 0;
}