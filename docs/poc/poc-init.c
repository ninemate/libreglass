#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <unistd.h>

static const char marker[] = "LG-G1-POC: init reached\n";

int main(void)
{
    if (getpid() != 1)
        return fputs(marker, stdout) == EOF;

    if (mount("devtmpfs", "/dev", "devtmpfs", MS_NOSUID | MS_NOEXEC, NULL) == 0) {
        int console = open("/dev/console", O_WRONLY | O_NOCTTY);
        if (console >= 0) {
            write(console, marker, sizeof(marker) - 1);
            close(console);
        }
    }

    int fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    int broadcast = 1;
    struct sockaddr_in destination = {
        .sin_family = AF_INET,
        .sin_port = htons(45555),
        .sin_addr.s_addr = htonl(INADDR_BROADCAST),
    };
    if (fd >= 0)
        setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast));

    for (;;) {
        if (fd >= 0)
            sendto(fd, marker, sizeof(marker) - 1, 0,
                   (const struct sockaddr *)&destination, sizeof(destination));
        sleep(3);
    }
}
