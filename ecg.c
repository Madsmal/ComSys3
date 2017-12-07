/*
 * ecg.c
 *
 * 
 */

// Implements
#include "ecg.h"

#include "radio.h"
#include "alarm.h"

// Uses
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

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

int ecg_send(int  dst, char* packet, int len, int to_ms) {

    struct sockaddr_in sa;   // Structure to hold destination address
    int totNr = len/DATA_PAYLOAD_SIZE;
    pdu_frame_t buf;
    alarm_t timer1;
    int src, time_left;

    alarm_init(&timer1);    
   
    int done = 0;
    // Check that port and len are valid
    if ((dst<1024)|(dst >65535)){
        return ERR_INVAL;
    }
    
    if (len>4096){
        return ERR_INVAL;
    }

    //start overall timer
    alarm_set(&timer1, to_ms);
    
    int sentNow = 0;

    buf.req.type.tag = REQ;
    buf.req.nrPackage = totNr;
    int err;
    int ack1;
    if ((err = radio_send(dst, buf.raw, sizeof(buf))) != ERR_OK){
        printf("Req send failed with %d\n", err);
        return ERR_FAILED;
    }
    buf.ack.type.tag = ACK;
    ack1 = radio_recv(&src, buf.raw, alarm_rem(&timer1));
    
    while(ack1>=ERR_OK && !alarm_expired(&timer1)) 
    {   
        time_left = alarm_rem(&timer1);
        buf.data.type.tag = DATA;
        if (ack1 != sizeof(ack_pdu_t) || buf.pdu_type.tag != ACK) {
                    // Not an ACK packet -- ignore
                    printf("Non-ACK packcet with length %d received\n", ack1);
                    continue;
                }

                // Check sender
                if (src != dst) {
                    printf("Wrong sender: %d\n", src);
                    continue;
                }



            memcpy(buf.raw, &packet[sentNow*sizeof(buf.raw)],sizeof(buf.raw));

        if ( (err=radio_send(dst, buf.raw, sizeof(buf.raw))) != ERR_OK) {
            printf("radio_send in ecg_send failed with %d\n", err);
            return ERR_FAILED;
            }

        ack1 = radio_recv(&src, buf.raw, time_left);
        sentNow = sentNow++;

    }
    if (sentNow == totNr){
        buf.termination.type.tag = TERMINATION;
        if ( (err=radio_send(dst, buf.raw, sizeof(buf.raw))) != ERR_OK) {
            printf("radio_send in ecg_send failed with %d\n", err);
            return ERR_FAILED;
            }
    }

    return ERR_OK;
}

int ecg_recv( int * src , char * packet , int len , int to_ms) {

    pdu_frame_t buf;
    int  err;
    alarm_t timer2;
    alarm_init(&timer2);
    alarm_set(&timer2,to_ms) ; 
    int reqReceiv = 0;
    int msg[72];
        /* READY STATE */
    int msgReceived = 0;
    int totNr;
    
        while (1) {

            err=radio_recv(&src, buf.raw, alarm_rem(&timer2));

                // Somehting received -- check if REQ.
            if ((buf.req.type.tag = REQ)&& err == ERR_OK) {
                    reqReceiv = 1;
                    totNr = buf.req.nrPackage;
                    printf("REQ packet with length %d received\n", err);
                    break;
            }
            if (err == -3){
                return ERR_TIMEOUT;
            }
            
                
            }
            buf.ack.type.tag = ACK;
             if ( (err=radio_send(&src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
            printf("radio_send failed with %d\n", err);
            return ERR_FAILED;
            }
        
        while((reqReceiv == 1) && (msgReceived<totNr)){
            err=radio_recv(&src, buf.raw, alarm_rem(&timer2));
            if (err!=ERR_OK){ return ERR_FAILED;}
            if (buf.data.type.tag == DATA){
                printf("Received message from %d: %s\n", src,buf.raw );
                msgReceived++;
            }
             if (alarm_expired(&timer2)){
                return ERR_TIMEOUT;
            }
            if(msgReceived == totNr){
                reqReceiv = 0;
            }
        }

        /* ACKNOWLEDGE STATE */

        // Prepare frame seen as ACK PDU
        //buf.ack.type.tag = ACK;
        

        // Send acknowledgement to sender
        //=radio_send(src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
         //   printf("radio_send failed with %d\n", err);
          //  return ERR_FAILED;
        //}

        /* DONE STATE */

}
   
