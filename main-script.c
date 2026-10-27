#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/stat.h>
#include "helper-functions.h"

/* ------------------------------------------------------------------ */
/* Validators for user input that ends up in files or shell commands   */
/* ------------------------------------------------------------------ */

static bool validLocale(const char *s) {        // en_US, de_DE, sr_RS@latin ...
    size_t n = strlen(s);
    if (n < 2 || n > 24) return false;
    for (size_t i = 0; i < n; i++) {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_' && s[i] != '@') return false;
    }
    return true;
}

static bool validHostname(const char *s) {
    size_t n = strlen(s);
    if (n == 0 || n > 63 || s[0] == '-') return false;
    for (size_t i = 0; i < n; i++) {
        if (!isalnum((unsigned char)s[i]) && s[i] != '-') return false;
    }
    return true;
}

static bool validUsername(const char *s) {
    size_t n = strlen(s);
    if (n == 0 || n > 31) return false;
    if (!(islower((unsigned char)s[0]) || s[0] == '_')) return false;
    for (size_t i = 1; i < n; i++) {
        if (!(islower((unsigned char)s[i]) || isdigit((unsigned char)s[i]) || s[i] == '_' || s[i] == '-')) return false;
    }
    return true;
}

static bool validTimezone(const char *s) {      // Area/Location, must exist in the new system
    size_t n = strlen(s);
    if (n == 0 || n > 64 || s[0] == '/' || strstr(s, "..") != NULL) return false;
    for (size_t i = 0; i < n; i++) {
        if (!isalnum((unsigned char)s[i]) && s[i] != '/' && s[i] != '_' && s[i] != '+' && s[i] != '-') return false;
    }
    char path[256];
    snprintf(path, sizeof(path), "/mnt/usr/share/zoneinfo/%s", s);
    struct stat st;
    return stat(path, &st) == 0 && S_ISREG(st.st_mode);
}

/* ------------------------------------------------------------------ */
/* Small utilities                                                     */
/* ------------------------------------------------------------------ */

static void writeText(const char *path, const char *mode, const char *text) {
    FILE *f = fopen(path, mode);
    if (f == NULL) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    fputs(text, f);
    fclose(f);
}

static void setPassword(const char *user) {
    for (int attempt = 0; attempt < 3; attempt++) {
        if (runf("arch-chroot /mnt passwd %s", user) == 0) return;
        printf("Password was not set, please try again.\n");
    }
    printf("Warning: no password was set for %s. Set one after rebooting (passwd %s).\n", user, user);
}

/* ------------------------------------------------------------------ */

int main(void) {
    char diskSelection[64] = "";
    char rootDisk[64] = "";

    printWelcome();    // Print Welcome Text and Script Information

    green();
    if (askChoice("Would you like to continue with the installation of Arch Linux (y/n): ", "yn") == 'n') {
        printf("\nScript terminated by user\n");
        exit(EXIT_FAILURE);
    }

    printf("\n");
    green();
    int fwBits = uefiBits();
    bool uefi = (fwBits != 0);  // True if system has UEFI
    bool autoPart = (askChoice("Would you like to manually partition using FDISK or automatically partition (m/a): ", "ma") == 'a');

    if (autoPart) {
        runf("fdisk -l");
        green();
        printf("\n");
        promptValid("Select an installation disk by typing the disk e.g. sda: ",
                    "Disk not found: Select an installation disk by typing the disk e.g. sda: ",
                    diskSelection, sizeof(diskSelection), checkWholeDisk);
        white();

        printf("WARNING: ALL DATA on /dev/%s will be permanently erased!\n", diskSelection);
        char confirm[64];
        printf("Type the disk name again to confirm: ");
        fflush(stdout);
        readLine(confirm, sizeof(confirm));
        if (strcmp(confirm, diskSelection) != 0) {
            printf("Confirmation did not match. Nothing was changed.\n");
            exit(EXIT_FAILURE);
        }
        diskAutoFormat(diskSelection, uefi);     // partition, format and mount
        green();
    }
    else {  // Manual partitioning with FDISK
        green();
        printf("Use FDISK to create the following partitions: \n");
        printf("If the system is BIOS, press o when entering FDISK to make MBR partitions:\n");
        printf("1. A boot partition: 1GB (UEFI Systems Only) \n");
        printf("2. A SWAP Partition: Size Variable \n");
        printf("3. A Root Partition \n");
        printf("4. A Separate Home Partition (Not Required)\n");
        printf("Make Sure to Write Down the Address of Each Partition e.g. /dev/sda1 \n");
        askChoice("Type y and press enter to continue to FDISK: ", "y");
        runf("fdisk -l");

        do {
            char fdiskDisk[64];
            green();
            printf("\n");
            promptValid("Select an installation disk by typing the disk e.g. sda: ",
                        "Disk not found: Select an installation disk by typing the disk e.g. sda: ",
                        fdiskDisk, sizeof(fdiskDisk), checkWholeDisk);
            runf("fdisk /dev/%s", fdiskDisk);        // Open fdisk for the selected disk
            green();
        } while (askChoice("Do you want to format another disk? (y/n): ", "yn") == 'y');

        char bootPart[64] = "";
        char swapPart[64];
        char rootPart[64];
        char homePart[64];

        if (uefi) {
            promptValid("\nEnter The Boot Partition Location e.g. sda1: ",
                        "Partition not found: Enter the boot location e.g. sda1: ",
                        bootPart, sizeof(bootPart), checkPartition);
            // Don't wipe an existing EFI System Partition (dual boot) unless asked to
            if (askChoice("Format the boot partition? Answer n to keep an existing EFI partition (y/n): ", "yn") == 'y') {
                white();
                runOrDie("mkfs.fat -F32 -I /dev/%s", bootPart);
                green();
            }
        }

        promptValid("Enter the Swap Partition Location e.g. sda2: ",
                    "Partition not found: Enter the Swap location e.g. sda2: ",
                    swapPart, sizeof(swapPart), checkPartition);
        white();
        runOrDie("mkswap -f /dev/%s", swapPart);
        runOrDie("swapon /dev/%s", swapPart);
        green();

        promptValid("\nEnter the Root Partition Location e.g. sda3: ",
                    "Partition not found: Enter the Root location e.g. sda3: ",
                    rootPart, sizeof(rootPart), checkPartition);
        white();
        runOrDie("mkfs.ext4 -F /dev/%s", rootPart);
        runOrDie("mount /dev/%s /mnt", rootPart);
        if (!parentDisk(rootPart, rootDisk, sizeof(rootDisk))) {   // needed for GRUB on BIOS systems
            fprintf(stderr, "Could not determine which disk %s belongs to.\n", rootPart);
            exit(EXIT_FAILURE);
        }
        green();

        if (uefi) {    // mount boot partition if UEFI
            runOrDie("mount --mkdir /dev/%s /mnt/boot", bootPart);
        }

        const char *homePrompt = "\nEnter the Home Partition Location e.g. sda4 (Enter n if you don't want a separate home partition): ";
        for (;;) {
            printf("%s", homePrompt);
            fflush(stdout);
            readLine(homePart, sizeof(homePart));
            if (strcasecmp(homePart, "n") == 0) { homePart[0] = '\0'; break; }
            if (checkPartition(homePart)) break;
            clearPrevLine();
            homePrompt = "Partition not found: Enter the Home location e.g. sda4 (n for none): ";
        }
        if (homePart[0] != '\0') {
            if (askChoice("Format the home partition? Answer n to keep its existing data (y/n): ", "yn") == 'y') {
                white();
                runOrDie("mkfs.ext4 -F /dev/%s", homePart);
            }
            runOrDie("mount --mkdir /dev/%s /mnt/home", homePart);
            green();
        }
    }

    printf("\nPartitioning Has Been Completed!\n");
    green();
    if (askChoice("Continue to Base Install (y/n): ", "yn") == 'n') {
        printf("Script Terminated By User After Partitioning\n");
        exit(0);
    }

    printf("Select the kernel you want to install: \n");
    printf("1. Up to date Linux Kernel (linux)\n");
    printf("2. Long Term Support Linux Kernel (linux-lts)\n");
    printf("3. Install Both Kernels\n");
    char kernelOption = askChoice("Type the number with the corresponding option and press enter: ", "123");
    white();
    if (kernelOption == '1') {
        runOrDie("pacstrap -K /mnt base linux linux-firmware");
    }
    else if (kernelOption == '2') {
        runOrDie("pacstrap -K /mnt base linux-lts linux-firmware");
    }
    else {
        runOrDie("pacstrap -K /mnt base linux linux-lts linux-firmware");
    }
    runf("touch /mnt/etc/vconsole.conf");     // after pacstrap so /mnt/etc exists; silences a mkinitcpio warning

    green();
    printf("The base system has been installed! Now we can continue to configuring the new system.\n");
    printf("Generating fstab\n");
    white();
    runOrDie("genfstab -U /mnt >> /mnt/etc/fstab");
    green();

    printf("Entering Arch Chroot\n");
    char timeZone[128];
    promptValid("Lets set the timezone of the system! Enter the timezone as Area/Location e.g. America/Denver: ",
                "Unknown timezone: Enter the timezone as Area/Location e.g. America/Denver: ",
                timeZone, sizeof(timeZone), validTimezone);
    runOrDie("arch-chroot /mnt ln -sf /usr/share/zoneinfo/%s /etc/localtime", timeZone);
    runf("arch-chroot /mnt hwclock --systohc");        // can fail on VMs without an RTC, not fatal
    printf("Timezone has been set!\n");

    bool firstLocale = true;
    do {
        char locale[32];
        promptValid("Enter the locale you would like to use e.g. en_US: ",
                    "Invalid locale: Enter the locale you would like to use e.g. en_US: ",
                    locale, sizeof(locale), validLocale);

        char line[96];
        snprintf(line, sizeof(line), "\n%s.UTF-8 UTF-8\n", locale);       // uncommented entry for locale-gen
        writeText("/mnt/etc/locale.gen", "a", line);

        if (firstLocale) {                                                // LANG is the first locale entered
            snprintf(line, sizeof(line), "LANG=%s.UTF-8\n", locale);
            writeText("/mnt/etc/locale.conf", "w", line);
            firstLocale = false;
        }
    } while (askChoice("The locale has been added. Do you want to add another (y/n): ", "yn") == 'y');
    runOrDie("arch-chroot /mnt locale-gen");

    char hostname[64];
    promptValid("Type the hostname that you want to configure (Your computer's name on the network): ",
                "Invalid hostname (letters, digits and '-'): Type the hostname: ",
                hostname, sizeof(hostname), validHostname);
    char hostLine[80];
    snprintf(hostLine, sizeof(hostLine), "%s\n", hostname);
    writeText("/mnt/etc/hostname", "w", hostLine);

    printf("Installing CPU Microcode\n");   // Install the CPUs microcode
    white();
    int cpu = cpuBrand();
    if (cpu == 1) {
        runOrDie("arch-chroot /mnt pacman -S amd-ucode --noconfirm");
    }
    else if (cpu == 2) {
        runOrDie("arch-chroot /mnt pacman -S intel-ucode --noconfirm");
    }
    else {
        printf("CPU is neither AMD or Intel. Not installing any microcode!\n");
    }

    green();
    printf("Generate INITRAMFS\n");
    white();
    runOrDie("arch-chroot /mnt mkinitcpio -P");
    green();
    printf("Enter your root password:\n");
    white();
    setPassword("root");

    green();
    printf("Installing GRUB bootloader package\n");
    white();
    runOrDie("arch-chroot /mnt pacman -S grub --noconfirm");
    green();
    if (askChoice("Would you like to install os-prober to detect other operating systems for dual boot (y/n): ", "yn") == 'y') {
        white();
        runOrDie("arch-chroot /mnt pacman -S os-prober --noconfirm");
        // Current GRUB disables os-prober by default, so enable it explicitly
        runf("sed -i 's/^#\\?GRUB_DISABLE_OS_PROBER=.*/GRUB_DISABLE_OS_PROBER=false/' /mnt/etc/default/grub");
        runf("grep -q '^GRUB_DISABLE_OS_PROBER=' /mnt/etc/default/grub || echo 'GRUB_DISABLE_OS_PROBER=false' >> /mnt/etc/default/grub");
    }

    green();
    printf("Installing GRUB bootloader to the system\n");
    if (uefi) {
        white();
        runOrDie("arch-chroot /mnt pacman -S efibootmgr --noconfirm");
        runOrDie("arch-chroot /mnt grub-install --target=%s --efi-directory=/boot --bootloader-id=ARCH",
                 fwBits == 32 ? "i386-efi" : "x86_64-efi");
    }
    else {
        // auto partitioning: GRUB goes on the selected disk; manual: on the disk holding the root partition
        const char *grubDisk = autoPart ? diskSelection : rootDisk;
        if (grubDisk[0] == '\0') {
            fprintf(stderr, "No disk known for GRUB installation.\n");
            exit(EXIT_FAILURE);
        }
        runOrDie("arch-chroot /mnt grub-install --target=i386-pc /dev/%s", grubDisk);
    }
    runOrDie("arch-chroot /mnt grub-mkconfig -o /boot/grub/grub.cfg");
    green();
    printf("Grub has been installed into the system!\n");
    white();
    runOrDie("arch-chroot /mnt pacman -S networkmanager --noconfirm");
    runOrDie("arch-chroot /mnt systemctl enable NetworkManager");
    green();

    bool sudoInstalled = (askChoice("Would you like to install Sudo (y/n): ", "yn") == 'y');
    if (sudoInstalled) {
        white();
        runOrDie("arch-chroot /mnt pacman -S sudo --noconfirm");
        green();
    }

    if (askChoice("Would you like to create a user (y/n): ", "yn") == 'y') {
        bool wheelEnabled = false;       // has the wheel rule been written yet
        do {
            char userName[64];
            promptValid("Type the name for the user: ",
                        "Invalid name (lowercase letters, digits, '_' and '-'): Type the name for the user: ",
                        userName, sizeof(userName), validUsername);
            runOrDie("arch-chroot /mnt useradd -m %s", userName);
            setPassword(userName);
            green();

            if (sudoInstalled &&
                askChoice("Do you want to give this user sudo permissions (y/n): ", "yn") == 'y') {
                runOrDie("arch-chroot /mnt usermod -aG wheel %s", userName);
                if (!wheelEnabled) {
                    // A drop-in file is safer than editing /etc/sudoers directly
                    writeText("/mnt/etc/sudoers.d/10-wheel", "w", "%wheel ALL=(ALL:ALL) ALL\n");
                    chmod("/mnt/etc/sudoers.d/10-wheel", 0440);
                    wheelEnabled = true;
                }
            }
        } while (askChoice("Would you like to add another user (y/n): ", "yn") == 'y');
    }

    if (askChoice("Would you like to install YAY (AUR Package Manager) (y/n): ", "yn") == 'y') {
        white();
        runOrDie("arch-chroot /mnt pacman -S --needed --noconfirm git base-devel");   // also provides sudo
        runOrDie("arch-chroot /mnt useradd -m temp");

        // Temporary build user: may run pacman without a password and nothing else. No password is set.
        writeText("/mnt/etc/sudoers.d/99-temp-yay", "w", "temp ALL=(ALL) NOPASSWD: /usr/bin/pacman\n");
        chmod("/mnt/etc/sudoers.d/99-temp-yay", 0440);

        int rc = runf("arch-chroot /mnt sudo -i -u temp git clone https://aur.archlinux.org/yay.git /home/temp/yay");
        if (rc == 0) rc = runf("arch-chroot /mnt sudo -i -u temp sh -c 'cd /home/temp/yay && makepkg -s --noconfirm'");
        if (rc == 0) rc = runf("arch-chroot /mnt bash -c 'cd /home/temp/yay && pacman -U --noconfirm *.pkg.tar.zst'");

        // Cleanup always runs, even if the build failed
        remove("/mnt/etc/sudoers.d/99-temp-yay");
        runf("arch-chroot /mnt userdel -r temp");
        if (rc != 0) {
            printf("Warning: YAY could not be installed. You can install it manually after rebooting.\n");
        }
    }

    green();
    if (askChoice("The base system has been installed! Would you like to continue to installing a display-server, desktop-environment, and display manager? (y/n): ", "yn") == 'n') {
        printf("Script exited by user. Base system is installed and bootable!\n");
        exit(0);
    }

    printf("Choose a display server to install:\n");
    printf("1. Wayland\n");
    printf("2. Xorg\n");
    printf("3. Both\n");
    printf("-------------------------------------\n");
    char dServer = askChoice("Enter the number of your choice: ", "123");
    if (dServer == '1') {
        runOrDie("arch-chroot /mnt pacman -S wayland --noconfirm");
    }
    else if (dServer == '2') {
        runOrDie("arch-chroot /mnt pacman -S xorg-server --noconfirm");
    }
    else {
        runOrDie("arch-chroot /mnt pacman -S xorg-server wayland --noconfirm");
    }

    printf("Choose a desktop environment to install:\n");       // Installs xorg with ones that require it
    printf("1. KDE Plasma\n");
    printf("2. GNOME\n");
    printf("3. Cinnamon\n");
    printf("4. XFCE\n");
    printf("5. LXQt\n");
    printf("6. MATE\n");
    printf("-------------------------------------\n");
    char dEnv = askChoice("Enter the number of your choice: ", "123456");
    white();
    switch (dEnv) {
        case '1':
            runOrDie("arch-chroot /mnt pacman -S plasma-meta sddm --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable sddm");
            break;
        case '2':
            runOrDie("arch-chroot /mnt pacman -S gnome gdm --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable gdm");
            break;
        case '3':
            runOrDie("arch-chroot /mnt pacman -S xorg-server lightdm lightdm-gtk-greeter cinnamon --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable lightdm");
            break;
        case '4':
            runOrDie("arch-chroot /mnt pacman -S xorg-server xfce4 lightdm lightdm-gtk-greeter --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable lightdm");
            break;
        case '5':
            runOrDie("arch-chroot /mnt pacman -S xorg-server lxqt sddm --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable sddm");
            break;
        default:
            runOrDie("arch-chroot /mnt pacman -S xorg-server mate lightdm lightdm-gtk-greeter --noconfirm");
            runOrDie("arch-chroot /mnt systemctl enable lightdm");
            break;
    }

    green();
    printf("|---------------------------|\n");
    printf("| Installation is complete! |\n");
    printf("|---------------------------|\n");
    white();
    return 0;
}
