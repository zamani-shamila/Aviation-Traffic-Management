#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dispatch.h>

/*
 * In this part of code, most of the code is based on the client/server code provided by POD in class
 * only some small implementation was done which is demonstrated below
 * */

#define ATTACH_POINT "CompSysToDisplay"

typedef struct _pulse msg_header_t;
//
///* Our real data comes after the header */
typedef struct _my_data {
    msg_header_t hdr;
    char aircraftinfo[256]; // Modify this structure to include the data fields that will be sent by the radar
} my_data_t;


int data_display() {
   name_attach_t *attach;
   my_data_t msg;
   int rcvid;
//   char buffer[20];
   if ((attach = name_attach(NULL, ATTACH_POINT, 0)) == NULL) {
       return EXIT_FAILURE;
   }

   /* Do your MsgReceive's here now with the chid */
   while (1) {
	   ///////////////////////////////////////////////////////////////////// No change
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
       if (msg.hdr.type == _IO_CONNECT ) {
           MsgReply( rcvid, EOK, NULL, 0 );
           continue;
       }
       /* Some other QNX IO message was received; reject it */
       if (msg.hdr.type > _IO_BASE && msg.hdr.type <= _IO_MAX ) {
           MsgError( rcvid, ENOSYS );
           continue;
       }
//In the data_display function (client ) we receive the message from the computer system and print it on our terminal

	   if (msg.hdr.type == 0x00) {
//	       printf("Radar data: ");
	       printf("%s", msg.aircraftinfo);

	       printf("\n");
	   }

       MsgReply(rcvid, EOK, 0, 0);
   }

   name_detach(attach, 0);
   return EXIT_SUCCESS;

}
int main(int argc, char **argv) {
    int ret;

//    while(1){
    printf("Running Server ...\n");
    ret = data_display();
//    }
    return ret;
}


/////////////////////////////////

