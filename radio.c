/*
 * radio.c
 *
 * Emulation of radio node using UDP (skeleton)
 */

// Implements
#include "radio.h"

// Uses
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>

#define port 2500
#define BUFLEN 4096


int radio_init(int addr) {

    // Check validity of address
    if (addr<1024 | addr >65535){
        return ERR_INVAL;
    }

    int sock;    // UDP Socket used by this node

    // Create UDP socket
      if ((sock=socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        return ERR_INVAL;
    }

    struct sockaddr_in sa;   // Structure to set own address

    // Prepare address structure
    memset(&sa, 0, sizeof(sa));
     
    sa.sin_family = AF_INET;
    sa.sin_port = htons(addr);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to port
    if(bind(sock, (struct sockaddr *)&sa, sizeof(sa)) == -1)
    {   

        return 33;
    }
    return ERR_OK;
}

int radio_send(int  dst, char* data, int len) {

    struct sockaddr_in sa;   // Structure to hold destination address
    

    // Check that port and len are valid
    if (dst<1024 |dst >65535){
        return ERR_INVAL;
    }
    
    // Emulate transmission time
    usleep((len*1000)/2400);
    // Prepare address structure
    memset((char *) &sa, 0, sizeof(sa));
     
    sa.sin_family = AF_INET;
    sa.sin_port = htons(dst);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    // Send the message
    int result;

        // This part fucks up

        if ((result = sendto(dst, data,len , 0 , (struct sockaddr *) &sa,
         sizeof(sa)))==-1)
        {
            
            return ERR_FAILED;

        } 
        
         
         // Check if fully sent
        if (result != len) {
            
            return ERR_FAILED;
        }
    return ERR_OK;
}

int radio_recv(int* src, char* data, int to_ms) {


    struct sockaddr_in sa;   // Structure to receive source address

    memset((char *) &sa, 0, sizeof(sa));
     
    sa.sin_family = AF_INET;
    sa.sin_port = htons(src);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    int len = 100;      // Size of received packet (or error code)

    struct pollfd pfd; 
    char msg[512];
    pfd.fd = src;
    pfd.events = POLLIN;

    int rv = poll(&pfd,1,to_ms);

    if (rv == 0){
        
        return ERR_TIMEOUT;
    } else if (rv == -1){
        printf("whistleboy");
        return ERR_FAILED;
    } 
    // First poll/select with timeout (may be skipped at first)
    
    // Then get the packet

    // Zero out the address structure

    // Receive data
    
        if ((len = recvfrom(&src, msg, sizeof(msg), 0 , (struct sockaddr *) &sa,
         sizeof(sa)))==-1)
        {
            printf("failed receive, length = %d \n", len);
            return ERR_FAILED;

        } 
        
         
         
        if ( 0 > len  ) {
            printf("itschanigga");
            return ERR_FAILED;
        }
    // Set source from address structure
     *src = ntohs(sa.sin_port);

    return len;
}

