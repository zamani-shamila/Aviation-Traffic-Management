#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <unordered_map>
#include <iostream>
#include <time.h>

/*
 * In this part of code, most of the code is based on the client/server code provided by POD in class
 * only some small implementation was done which is demonstrated below
 * */
#define ATTACH_POINT1 "RadarToCompSys"
#define ATTACH_POINT2 "CompSysToDisplay"


typedef struct _pulse msg_header_t;

///* Our real data comes after the header */
typedef struct _my_data {
	msg_header_t hdr;
	char aircraftinfo[256];

} my_data_t;

int flag_messagReceived = 0;

char buffer[256];


/*
 * In the RadarToCompSys function(server) we receive the message from the Radar,
 * set the flag_messagReceived if a new message is received, and copy the message in a global variable buffer.
 *
 * */

void* RadarToCompSys(void *arg) {
	name_attach_t *attach;
	my_data_t msg;
	int rcvid;
	/* Create a local name (/dev/name/local/...) */
	if ((attach = name_attach(NULL, ATTACH_POINT1, 0)) == NULL) {
//		return EXIT_FAILURE;
	}
	while (1) {

		rcvid = MsgReceive(attach->chid, &msg, sizeof(msg), NULL);
		if (rcvid == -1) {/* Error condition, exit */
			break;
		}
		if (rcvid == 0) {/* Pulse received */
			switch (msg.hdr.code) {
			case _PULSE_CODE_DISCONNECT:
				ConnectDetach(msg.hdr.scoid);
				break;
			case _PULSE_CODE_UNBLOCK:
				break;
			default:
				break;
			}
			continue;
		}
		/* name_open() sends a connect message, must EOK this */
		if (msg.hdr.type == _IO_CONNECT) {
			MsgReply(rcvid, EOK, NULL, 0);
			continue;
		}
		/* Some other QNX IO message was received; reject it */
		if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX) {
			MsgError(rcvid, ENOSYS);
			continue;
		}
		if (msg.hdr.type == 0x00) {
			//	       printf("Radar data: ");
//			printf("\n");
			std::sscanf(msg.aircraftinfo, "%[^\n]", buffer);
			usleep(100000);

			flag_messagReceived=1;
			printf("%d\n",strlen(buffer));
			printf("%s \n", buffer);
		}

		MsgReply(rcvid, EOK, 0, 0);
	}

	name_detach(attach, 0);
//	return EXIT_SUCCESS;

}


/*
 * In the CompSysToDisplay function (client ) we use the buffer to copy it into the message we want to send to the display system,
 *  by using the flag_messagReceived  check if it is a new message or not, and then send it to display system. *
 * */

struct timespec sleep_time2;

//int client() {
//void* client() {
void* CompSysToDisplay(void *arg) {
//	std::cout<<"Print" <<std::endl;
	my_data_t msg1;

//	std::cout<<"Print" <<std::endl;
	int server_coid; //server connection ID

	if ((server_coid = name_open(ATTACH_POINT2, 0)) == -1) {

//		return EXIT_FAILURE;
	}

	/* We would have pre-defined data to stuff here */
	msg1.hdr.type = 0x00;
	msg1.hdr.subtype = 0x01;

	/* Do whatever work you wanted with server connection */
	while (true) {

		sscanf(buffer, "%[^\n]", msg1.aircraftinfo);
		usleep(100000);

		if(flag_messagReceived==1){
			flag_messagReceived=0;
		if (MsgSend(server_coid, &msg1, sizeof(msg1), NULL, 0) == -1) {
			printf("Error sending message to server.\n");
			name_close(server_coid);
//		return EXIT_FAILURE;
		}
	}
	}

	/* Close the connection */
//	name_close(server_coid);
	return NULL;
}

int main(int argc, char **argv) {
	int ret;

	printf("Running Computer System ...\n");
//	ret = server();
//	server();
//	int ret2;
//	printf("Running Client ...\n");
//	ret2 = client();


	// Thread for revceivig data from radar
	pthread_t RadarToCompSysThread;
	int rc = pthread_create(&RadarToCompSysThread, NULL, RadarToCompSys, NULL);
	if (rc) {
		printf("ERROR when creating RadarToCompSys thread; Code is %d\n", rc);
	}
	// Thread for sendig data to display
	pthread_t CompSysToDisplayThread;

	int rc2 = pthread_create(&CompSysToDisplayThread, NULL, CompSysToDisplay, NULL);
	if (rc2) {
		printf("ERROR when creating CompSysToDisplay thread; Code is %d\n", rc2);
	}

	pthread_join(RadarToCompSysThread, NULL);
	pthread_join(CompSysToDisplayThread, NULL);

}
