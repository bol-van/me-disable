#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <linux/mei.h>
#include <unistd.h>
#include <errno.h>

// Disable Intel ME engine.
// This was tested on Z87 board.
// Payload data taken from reverse-engineered fpt.exe v9.5.


#define _countof(a) (sizeof(a)/sizeof(*(a)))

static const char *DEV_NAME[] = { "/dev/mei", "/dev/mei0", "/dev/mei1", "/dev/mei2" };

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

uint8_t disable_cmd[] = {0xff,0x10,0x00,0x00};

int main(int argc, char *argv[])
{
	int fd;
	int rc;
	int i;
	struct mei_connect_client_data meidata;

	for(int i=0;i<_countof(DEV_NAME);i++)
	{
	    printf("Opening %s ... ",DEV_NAME[i]);
	    fd = open(DEV_NAME[i], O_RDWR);
	    if (fd < 0) {
		printf("%s\n", strerror(errno));
	    }
	}
	if (fd<0)
	{
	    printf("ME device not found\n");
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
