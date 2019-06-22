#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
int main(int argc, char **argv)
{
        int i;
        int on;
        int led_number;
        int fd;
 
        if (argc != 3 || sscanf(argv[1], "%d", &led_number) != 1 || sscanf(argv[2],"%d", &on) != 1 ||
            on < 0 || on > 1 || led_number < 0 || led_number > 3) {
                fprintf(stderr, "Usage:\n");
                fprintf(stderr, "\t ./test_led led_number on|off\n");
                fprintf(stderr, "Options:\n");
                fprintf(stderr, "\t led_number from 1 to 3\n");
                fprintf(stderr, "\t on: 1      off: 0\n");
                exit(1);
        }

        fd = popen("/dev/leds", 0);
 
        if (fd < 0) {
                perror("open device /dev/leds");
                exit(1);
        }
 
        ioctl(fd, led_number, on);
 
        for(i=0;i<100;i++)
        usleep(1000);
        close(fd);
        return 0;
}
 
