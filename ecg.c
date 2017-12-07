/*
 * ecg.c
 *
 * 
 */

// Implements
#include "ecg.h"

// Uses
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>


#define timeout 200
#define BUFLEN 4096
#define DATA 0
#define ACK  1
#define REQ 2
#define TERMINATION 3

int sock;    // UDP Socket used by this node


typedef struct { char tag ; } tag_t ;

typedef struct { 
    tag_t type;
    //int nr;
    char str[0];
 } data_pdu_t;

 typedef struct { 
    tag_t type;
    //int nr;
 } termination_pdu_t;

 typedef struct { 
    tag_t type;
    int nrPackage;
 } req_pdu_t;

 typedef struct { 
    tag_t type;
 } ack_pdu_t;

 typedef union {
    char raw[FRAME_PAYLOAD_SIZE];

    tag_t        pdu_type;
    data_pdu_t   data;
    ack_pdu_t    ack;
    req_pdu_t    req;
    termination_pdu_t termination;
} pdu_frame_t;

#define DATA_PAYLOAD_SIZE (FRAME_PAYLOAD_SIZE - sizeof(data_pdu_t));

int ecg_init(int addr) {

    return radio_init(addr);
}

int ecg_send(int  dst, char* data, int len) {

    struct sockaddr_in sa;   // Structure to hold destination address
    totNr = len/DATA_PAYLOAD_SIZE;
    pdu_frame_t buf;
    

    int src, err;
   
    int done = 0;
    // Check that port and len are valid
    if (dst<1024 |dst >65535){
        return ERR_INVAL;
    }
    
    if (data>4096){
        return ERR_INVAL;
    }


    
    int sentNow = 0;

    buf.req.type.tag = REQ;
    buf.req.nrPackage = totNr;
    int ack1;
    if ((err = radio_send(dst, buf, sizeof(buf))) != ERR_OK){
        printf("Req send failed with %d\n", err);
        return ERR_FAILED;
    }
    buf.ack.type.tag = ACK;
    ack1 = radio_recv(&sa, buf.raw, timeout);
    
    while(ack1>=ERR_OK) //add alarm later.
    {   
         buf.data.type.tag = DATA;
       
        if (ack1 != sizeof(ack_pdu_t) || buf.pdu_type.tag != ACK) {
                    // Not an ACK packet -- ignore
                    printf("Non-ACK packcet with length %d received\n", err);
                    continue;
                }

                // Check sender
                if (src != dst) {
                    printf("Wrong sender: %d\n", src);
                    continue;
                };

                // Check fingerprint
                if (buf.ack.seal != fingerprint(msg, KEY2)) {
                    printf("Wrong fingerprint: 0x%08x\n", buf.ack.seal);
                    continue;
                };
        strcpy(buf.data.str, data[(sentNow*DATA_PAYLOAD_SIZE)
                :((sentNow*DATA_PAYLOAD_SIZE)+DATA_PAYLOAD_SIZE)];

        if ( (err=radio_send(dst, buf.raw, len)) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return ERR_FAILED;
            }

        ack1 = radio_recv(&sa, buf.raw, time_left);
        sentNow = sentNow++;

    }
    if (sentNow == totNr){
        buf.termination.type.tag = TERMINATION;
        if ( (err=radio_send(dst, buf.raw, len)) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return ERR_FAILED;
            }
    }

    return ERR_OK;
}

int ecg_recv( int * src , char * packet , int len , int to_ms) {


    struct sockaddr_in sa;   // Structure to receive source address
    pdu_frame_t buf;
    int  err;
    

    

        /* READY STATE */

        while (1) {

            err=radio_recv(&src, buf.raw, -1);
            if (err!=ERR_OK) {
                return ERR_FAILED;
            }
                // Somehting received -- check if REQ.
                if (buf.pdu_type.tag = REQ)) {
                    // Not a DATA or REQ packet -- ignore
                    printf("REQ packcet with length %d received\n", err);
                    break;
                }

                // DATA PDU ok
                
            }
        }
        

        /* ACKNOWLEDGE STATE */

        // Prepare frame seen as ACK PDU
        buf.ack.type.tag = ACK;
        

        // Send acknowledgement to sender
        if ( (err=radio_send(src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return ERR_FAILED;
        }

        /* DONE STATE */

        printf("Received message from %d: %s\n", src, msg);

    }
    
}

