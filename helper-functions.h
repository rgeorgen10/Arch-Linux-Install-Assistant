#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/sysinfo.h>

/* ------------------------------------------------------------------ */
/* Terminal helpers                                                    */
/* ------------------------------------------------------------------ */

static inline void green(void) {
    printf("\033[0;32m");
}

static inline void white(void) {
    printf("\033[0m\n");
}

static inline void clearPrevLine(void) {
    printf("\033[A\033[2K");
}

/* ------------------------------------------------------------------ */
/* Running commands (printf-style, output is checked)                  */
/* ------------------------------------------------------------------ */

/* Runs a shell command. Returns its exit status, or -1 on failure to run.
 * Only ever pass validated values (see validDevName etc.) into %s. */
static inline int vrunf(const char *fmt, va_list ap) {
    char cmd[1024];
    int n = vsnprintf(cmd, sizeof(cmd), fmt, ap);
    if (n < 0 || (size_t)n >= sizeof(cmd)) {
        fprintf(stderr, "Command too long, refusing to run it.\n");
        return -1;
    }
    fflush(stdout);   // make sure our output (and colors) appear before the command's
    int rc = system(cmd);
    if (rc == -1) return -1;
    return WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;
}

__attribute__((format(printf, 1, 2)))
static inline int runf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = vrunf(fmt, ap);
    va_end(ap);
    return rc;
}

/* Same as runf, but aborts the whole script if the command fails. */
__attribute__((format(printf, 1, 2)))
static inline void runOrDie(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int rc = vrunf(fmt, ap);
    va_end(ap);
    if (rc != 0) {
        printf("\033[0m\n");
        fprintf(stderr, "A required command failed (exit status %d). Aborting installation.\n", rc);
        exit(EXIT_FAILURE);
    }
}

/* ------------------------------------------------------------------ */
/* Input helpers (fgets based: no buffer overflows, no leftover '\n')  */
/* ------------------------------------------------------------------ */

/* Reads one line, trims whitespace, discards anything that didn't fit. */
static inline void readLine(char *buf, size_t size) {
    if (fgets(buf, (int)size, stdin) == NULL) {
        printf("\nInput closed. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    size_t len = strcspn(buf, "\n");
    if (buf[len] == '\0') {            // no newline: line was too long (or EOF), drop the rest
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    buf[len] = '\0';

    char *start = buf;
    while (isspace((unsigned char)*start)) start++;
    size_t n = strlen(start);
    while (n > 0 && isspace((unsigned char)start[n - 1])) start[--n] = '\0';
    if (start != buf) memmove(buf, start, n + 1);
}

/* Asks until the answer is a single character contained in `valid`
 * (case-insensitive). Returns it lowercased. */
static inline char askChoice(const char *prompt, const char *valid) {
    char line[16];
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        readLine(line, sizeof(line));
        if (line[0] != '\0' && line[1] == '\0') {
            char c = (char)tolower((unsigned char)line[0]);
            if (strchr(valid, c) != NULL) return c;
        }
        clearPrevLine();
    }
}

/* Asks until ok(buf) is true. */
static inline void promptValid(const char *prompt, const char *errPrompt,
                               char *buf, size_t size, bool (*ok)(const char *)) {
    const char *p = prompt;
    for (;;) {
        printf("%s", p);
        fflush(stdout);
        readLine(buf, size);
        if (ok(buf)) return;
        clearPrevLine();
        p = errPrompt;
    }
}

/* ------------------------------------------------------------------ */
/* Device checks (sysfs instead of lsblk | grep)                       */
/* ------------------------------------------------------------------ */

/* Only letters, digits, '_' and '-' (sda, sda1, nvme0n1p2, mmcblk0p1...) */
static inline bool validDevName(const char *s) {
    size_t n = strlen(s);
    if (n == 0 || n > 31) return false;
    for (size_t i = 0; i < n; i++) {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_' && s[i] != '-') return false;
    }
    return true;
}

/* A whole disk, e.g. sda or nvme0n1 */
static inline bool checkWholeDisk(const char *name) {
    if (!validDevName(name)) return false;
    char path[128];
    snprintf(path, sizeof(path), "/sys/block/%s", name);
    return access(path, F_OK) == 0;
}

/* Any block device (disk or partition) */
static inline bool checkDiskSelect(const char *name) {
    if (!validDevName(name)) return false;
    char path[128];
    snprintf(path, sizeof(path), "/sys/class/block/%s", name);
    return access(path, F_OK) == 0;
}

/* A partition: exists and is NOT a whole disk (so typing "sda" instead of "sda1" is rejected) */
static inline bool checkPartition(const char *name) {
    return checkDiskSelect(name) && !checkWholeDisk(name);
}

/* Finds the parent disk of a partition (sda1 -> sda, nvme0n1p2 -> nvme0n1) */
static inline bool parentDisk(const char *part, char *out, size_t size) {
    char link[128], resolved[PATH_MAX];
    snprintf(link, sizeof(link), "/sys/class/block/%s", part);
    if (realpath(link, resolved) == NULL) return false;
    char *slash = strrchr(resolved, '/');
    if (slash == NULL) return false;
    *slash = '\0';                       // drop the partition name
    char *base = strrchr(resolved, '/');
    if (base == NULL) return false;
    snprintf(out, size, "%s", base + 1);
    return checkWholeDisk(out);
}

/* ------------------------------------------------------------------ */
/* System info                                                         */
/* ------------------------------------------------------------------ */

/* Returns 64 or 32 for UEFI systems, 0 for BIOS */
static inline int uefiBits(void) {
    char buf[8] = "";
    FILE *fp = fopen("/sys/firmware/efi/fw_platform_size", "r");
    if (fp == NULL) return 0;
    if (fgets(buf, sizeof(buf), fp) == NULL) { fclose(fp); return 0; }
    fclose(fp);
    if (strncmp(buf, "64", 2) == 0) return 64;
    if (strncmp(buf, "32", 2) == 0) return 32;
    return 0;
}

static inline bool uefiCheck(void) {
    return uefiBits() != 0;
}

/* return 1 for AMD, 2 for Intel, 3 for Neither/Unknown */
static inline int cpuBrand(void) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (fp == NULL) return 3;
    char line[256];
    int brand = 3;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "vendor_id", 9) == 0) {
            if (strstr(line, "AuthenticAMD") != NULL) brand = 1;
            else if (strstr(line, "GenuineIntel") != NULL) brand = 2;
            break;
        }
    }
    fclose(fp);
    return brand;
}

/* Total RAM in GB, rounded up. Returns -1 on error. */
static inline int get_memory_gb(void) {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        return -1;
    }
    unsigned long long bytes = (unsigned long long)info.totalram * info.mem_unit;
    return (int)(bytes / (1024ULL * 1024ULL * 1024ULL) + 1);
}

/* ------------------------------------------------------------------ */
/* Automatic partitioning                                              */
/* ------------------------------------------------------------------ */

/* Wipes `disk` (e.g. "sda") and creates: [boot (UEFI only)] + swap (= RAM) + root.
 * Everything is mounted under /mnt afterwards. Aborts on any failure. */
static inline void diskAutoFormat(const char *disk, bool uefi) {
    if (!checkWholeDisk(disk)) {
        fprintf(stderr, "Invalid disk: %s\n", disk);
        exit(EXIT_FAILURE);
    }

    int memGB = get_memory_gb();
    if (memGB < 1) memGB = 4;            // couldn't detect RAM, fall back to 4 GB of swap
    int swapMiB = memGB * 1024;

    char dev[64];
    snprintf(dev, sizeof(dev), "/dev/%s", disk);
    // Disks ending in a digit (nvme0n1, mmcblk0) name partitions with a 'p' (nvme0n1p1)
    const char *sep = isdigit((unsigned char)disk[strlen(disk) - 1]) ? "p" : "";
    char p1[80], p2[80], p3[80];
    snprintf(p1, sizeof(p1), "%s%s1", dev, sep);
    snprintf(p2, sizeof(p2), "%s%s2", dev, sep);
    snprintf(p3, sizeof(p3), "%s%s3", dev, sep);

    // Release anything on the target disk (mounts and swap) so the table can be rewritten
    runf("umount -R /mnt 2>/dev/null");
    runf("for p in %s*; do umount \"$p\" 2>/dev/null; swapoff \"$p\" 2>/dev/null; done", dev);

    if (uefi) {
        int swapStart = 1025;                    // boot partition: 1MiB - 1025MiB
        int swapEnd = swapStart + swapMiB;       // root starts right where swap ends
        runOrDie("parted -s %s mklabel gpt", dev);
        runOrDie("parted -s %s mkpart ESP fat32 1MiB %dMiB", dev, swapStart);
        runOrDie("parted -s %s set 1 esp on", dev);
        runOrDie("parted -s %s mkpart swap linux-swap %dMiB %dMiB", dev, swapStart, swapEnd);
        runOrDie("parted -s %s mkpart root ext4 %dMiB 100%%", dev, swapEnd);
        runf("partprobe %s", dev);
        runf("udevadm settle");

        runOrDie("mkfs.fat -F32 -I %s", p1);
        runOrDie("mkswap -f %s", p2);
        runOrDie("mkfs.ext4 -F %s", p3);
        runOrDie("mount %s /mnt", p3);
        runOrDie("swapon %s", p2);
        runOrDie("mount --mkdir %s /mnt/boot", p1);
    }
    else {  // Legacy BIOS: msdos table, swap = partition 1, root = partition 2
        int swapEnd = 1 + swapMiB;
        runOrDie("parted -s %s mklabel msdos", dev);
        runOrDie("parted -s %s mkpart primary linux-swap 1MiB %dMiB", dev, swapEnd);
        runOrDie("parted -s %s mkpart primary ext4 %dMiB 100%%", dev, swapEnd);
        runf("partprobe %s", dev);
        runf("udevadm settle");

        runOrDie("mkswap -f %s", p1);
        runOrDie("mkfs.ext4 -F %s", p2);
        runOrDie("mount %s /mnt", p2);
        runOrDie("swapon %s", p1);
    }
}

static inline void printWelcome(void) {
    green();
    printf("|----------------------------------------------------| \n");
    printf("|            Arch Linux Install Assistant            | \n");
    printf("|          --------------------------------          | \n");
    printf("|      This script is designed to assist in the      | \n");
    printf("|    installation of Arch Linux. This script will    | \n");
    printf("|           assist with the following:               | \n");
    printf("| 1) Formatting Disks                                | \n");
    printf("| 2) Installing base system                          | \n");
    printf("| 3) Configuring base system                         | \n");
    printf("| 4) Installing a bootloader (GRUB)                  | \n");
    printf("| 5) Adding Sudo and Users                           | \n");
    printf("| 6) Installing a Display Server                     | \n");
    printf("| 7) Installing a Display Manager                    | \n");
    printf("| 8) Installing a Desktop Environment                | \n");
    printf("|----------------------------------------------------| \n");
    white();
}

#endif /* HELPER_FUNCTIONS_H */
