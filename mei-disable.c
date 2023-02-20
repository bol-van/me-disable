#include <stdio.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/mei.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// Disable Intel ME engine.
// This was tested on Z87 board.
// Payload data taken from reverse-engineered fpt.exe v9.5.

#define NUM_DEV_NAMES
char *DEF_DEV_NAMES[NUM_DEV_NAMES] = {"/dev/mei0", "/dev/mei", "/dev/mei1", "/dev/mei2", "/dev/mei3"};

struct guid
{
        uint32_t   data1;
        uint16_t   data2;
        uint16_t   data3;
        uint8_t   data4[8];
};

static const struct guid mkhi_guid = {
	0x8E6A6715,
	0x9ABC,
	0x4043,
	{0x88, 0xEF, 0x9E, 0x39, 0xC6, 0xF6, 0x3E, 0x0F}
};

char *DEV_NAME = NULL;

uint8_t disable_cmd[] = {0xff,0x10,0x00,0x00};

int imeCheck() {
    struct stat st;

    for (int i = 0; i < NUM_DEV_NAMES; i++) {
        char path[20];
        snprintf(path, sizeof(path), "%s", DEF_DEV_NAMES[i]);

        if (stat(path, &st) == 0) {
            DEV_NAME = DEF_DEV_NAMES[i];
            break;
        }
    }

    if (DEV_NAME == NULL) {
        perror("ME device not found");
        return 1;
    }

    printf("ME device found: %s\n", DEV_NAME);

    return 0;
}

int main(int argc, char *argv[])
{
	int fd;
	int rc;
	int i;
	struct mei_connect_client_data meidata;
	imeCheck(); //new IME device checker, auto-checks for ime device from array.
	printf("Opening %s ... ",DEV_NAME);
	fd = open(DEV_NAME, O_RDWR);
	if (fd < 0) {
		printf("error\n"); fflush(stdout);
		perror("mei device open");
		return 1;
	}
	printf("opened\n");

	memcpy(&meidata.in_client_uuid,&mkhi_guid,sizeof(mkhi_guid));
	
	printf("Sending IOCTL_MEI_CONNECT_CLIENT .. ");
	rc = ioctl(fd, IOCTL_MEI_CONNECT_CLIENT, &meidata);
	if (rc < 0) {
		printf("error\n"); fflush(stdout);
		perror("ioctl");
		close(fd);
		return 1;
	}
	printf("ok\n");

	printf("Writing disableme payload .. ");
	rc = write(fd, disable_cmd, sizeof(disable_cmd));
	if (rc < 0) {
		printf("error\n"); fflush(stdout);
		perror("write");
		close(fd);
		return 1;
	}
	fsync(fd);
	printf("written %d bytes\n",rc);

	printf("Sleep 1 sec\n",rc);
	sleep(1);

	close(fd);

	return 0;
}
