#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

#define GPIO_CLK 60
#define GPIO_DAT 30
#define SYSFS_GPIO_DIR "/sys/class/gpio"

int sysfs_write_file(char *path, char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    write(fd, value, strlen(value));
    close(fd);
    return 0;
}

void gpio_export(int gpio) {
    char b[50], p[50];
    sprintf(p, SYSFS_GPIO_DIR "/gpio%d/value", gpio);
    if (access(p, F_OK) != -1) return;
    int fd = open(SYSFS_GPIO_DIR "/export", O_WRONLY);
    if (fd < 0) return;
    sprintf(b, "%d", gpio);
    write(fd, b, strlen(b));
    close(fd);
}

void gpio_direction(int gpio, char *dir) {
    char p[50];
    sprintf(p, SYSFS_GPIO_DIR "/gpio%d/direction", gpio);
    sysfs_write_file(p, dir);
}

void gpio_write(int gpio, int val) {
    char p[50];
    sprintf(p, SYSFS_GPIO_DIR "/gpio%d/value", gpio);
    sysfs_write_file(p, val ? "1" : "0");
}

unsigned char gray(unsigned char bin) {
    pid_t pids[8];
    int status;
    unsigned char gray = 0;

    for (int i = 0; i < 8; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            int current_bit = (bin >> i) & 1;
            int gray_bit;
            if (i == 7) {
                gray_bit = current_bit;
            } else {
                int left_bit = (bin >> (i + 1)) & 1;
                gray_bit = current_bit ^ left_bit;
            }

            exit(gray_bit);
        }
    }

    for (int i = 0; i < 8; i++) {
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status)) {
            gray |= (WEXITSTATUS(status) << i);
        }
    }
    return gray;
}

void driver_send_byte(unsigned char data) {
    for (int i = 7; i >= 0; i--) {
        gpio_write(GPIO_DAT, (data >> i) & 1);
        gpio_write(GPIO_CLK, 1);
        usleep(1);
        gpio_write(GPIO_CLK, 0);
        usleep(1);
    }
}

int main() {
    gpio_export(GPIO_CLK);
    gpio_export(GPIO_DAT);
    gpio_direction(GPIO_CLK, "out");
    gpio_direction(GPIO_DAT, "out");
    gpio_write(GPIO_CLK, 0);

    unsigned char input = 0xE7; 
    driver_send_byte(convert_to_gray_parallel(input));

    return 0;
}