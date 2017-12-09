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
    char str[64];
 } data_pdu_t;

 typedef struct { 
    tag_t type;
    //int nr;
 } termination_pdu_t;

 typedef struct { 
    tag_t type;
    int totalLen;
 } req_pdu_t;

 typedef struct { 
    tag_t type;
 } ack_pdu_t;

 typedef union {
    char raw[72];

    tag_t        pdu_type;
    data_pdu_t   data;
    ack_pdu_t    ack;
    req_pdu_t    req;
    termination_pdu_t termination;
} pdu_frame_t;

#define DATA_PAYLOAD_SIZE 64
 
    
int ecg_init(int addr) {

    return radio_init(addr);
}

int ecg_send(int  dst, char* packet, int len, int to_ms) {

    struct sockaddr_in sa;   // Structure to hold destination address
    
    pdu_frame_t buf;
    alarm_t timer1;

    int src, time_left;
    alarm_init(&timer1);    
    int err;
    int ack1 = 1;
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
    int temp = 0;

    while (temp != 1) {
    buf.req.type.tag = REQ;
    buf.req.totalLen = len;
    
    
    
    if ((err = radio_send(dst, buf.raw, (sizeof(req_pdu_t)))) == ERR_OK){
        //printf("Req send failed with %d %d %d %d \n " ,dst, err, buf.raw, (sizeof(req_pdu_t)+1));
        //return ERR_FAILED;
        temp = 1;
    }
    }
    // ack1 = radio_recv(&src, buf.raw, alarm_rem(&timer1));
    

    while(ack1>=ERR_OK && !alarm_expired(&timer1)&& (sentNow>=len)) 
    {   
        time_left = alarm_rem(&timer1);
        if (buf.pdu_type.tag != ACK) {
                    // Not an ACK packet -- ignore
                    printf("Non-ACK packcet with length %d received\n", ack1);
                    continue;
                }

                // Check sender
                if (src != dst) {
                    printf("Wrong sender: %d\n", src);
                    continue;
                }

            buf.data.type.tag = DATA;
            if ( !(sentNow+64) > len){
                memcpy(buf.data.str, packet[sentNow], 64   ); 
                sentNow = sentNow+64;
                continue;
            } else {
                memcpy(buf.data.str, packet[sentNow], (len-sentNow));
                sentNow = len;
                continue;
            }

            
        if ( (err=radio_send(dst, buf.raw, sizeof(buf.raw))) != ERR_OK) {
            printf("radio_send in ecg_send failed with %d\n", err);
            return ERR_FAILED;
            }
        
        ack1 = radio_recv(&src, buf.raw, time_left);
        

    }
    if (alarm_expired(&timer1)){
        return ERR_TIMEOUT;
    }
    if (sentNow == len){
            buf.termination.type.tag = TERMINATION;
            if ( (err=radio_send(dst, buf.raw, sizeof(buf.raw))) != ERR_OK) 
            {
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
    int reqReceiv = 1;
    
        /* READY STATE */
    int msgReceived = 0;
    int totLen = 4096;
    char msg[4096];
    int temp1 = 1;
    while(temp1){
    err=radio_recv(src, buf.raw, -1);

                // Somehting received -- check if REQ.
            if ((buf.req.type.tag == REQ)) {
                    reqReceiv = 1;
                    if (buf.req.totalLen >4066){
                        return ERR_INVAL;
                    }
                    printf("REQ packet with length %d received and total packet length %d\n", err, buf.req.totalLen);
                    totLen = buf.req.totalLen;
                    
                    
            }
            char msg[totLen];
            if (err == -3){
                return ERR_TIMEOUT;
            }

            buf.ack.type.tag = ACK;

             if ( (err=radio_send(src, buf.raw, sizeof(buf.raw))) != ERR_OK) {
                printf("radio_send failed to send ack with %d\n", err);
                return ERR_FAILED;
            }
            printf("Her \n");
            temp1 = 0;
        }
        temp1 = 1;
        while   ((reqReceiv == 1) && (msgReceived<totLen)){
                        printf("Her2 \n");

                err=radio_recv(&src, buf.raw, alarm_rem(&timer2));
            
            if (buf.pdu_type.tag == DATA){
                memcpy(msg[msgReceived], buf.data.str ,sizeof(buf.raw));
                msgReceived= msgReceived+sizeof(buf.raw);
            }
             if (alarm_expired(&timer2)){
                return ERR_TIMEOUT;
            }
            if (buf.pdu_type.tag == TERMINATION){
                printf("Received message from %d: %s\n", src, msgReceived);
            }
            buf.ack.type.tag = ACK;
           

             // Send acknowledgement to sender
             if ((err=radio_send(src, buf.raw, sizeof(ack_pdu_t))) != ERR_OK) {
                 printf("radio_send failed with %d\n", err);
                return ERR_FAILED;
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
        return msgReceived;
}
   
