#include "hal/time.h"
#include "hal/spi.h"
#include "udp.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <math.h>
#include "beatbox.h"

#define MAX_LEN 1500
#define PORT 12345
#define MAX_PACKET_SIZE 1500
#define MAX_SAMPLE_STR_LEN 30

static pthread_mutex_t UDPmutex = PTHREAD_MUTEX_INITIALIZER; /* mutex lock for history */

static pthread_t tidUDP;

static bool exitSignal = false;
static int socketDescriptor;
static struct sockaddr_in sinRemote;

static int UDP_process_message(char command[]);

void UDP_join_thread(void) {
    pthread_join(tidUDP, NULL);
}
static int stopping = 0;
static void *UDPThreadFunc(void *pArg)
{
    bool *exitSignal = ((bool *)pArg);
    while (1)
    {
        if (stopping) {
            break;
        }
        // recieve a message
        unsigned int sin_len = sizeof(sinRemote);
        char messageRx[MAX_LEN];
        int bytesRx = recvfrom(socketDescriptor, messageRx, MAX_LEN - 1, 0, (struct sockaddr *)&sinRemote, &sin_len);
        if (bytesRx < MAX_LEN - 1)
        {
            messageRx[bytesRx - 1] = 0;
        }
        else
        {
            messageRx[MAX_LEN - 1] = 0;
        }

        //printf("Message Recieved(%d bytes): '%s'\n", bytesRx, messageRx);
        *exitSignal = UDP_process_message(messageRx);

        time_sleep_ms(100);
        if (*exitSignal)
        {
            break;
        }
    }
    UDP_cleanup();
    return 0;
}

void UDP_init(void)
{
    struct sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(PORT);

    socketDescriptor = socket(PF_INET, SOCK_DGRAM, 0);
    bind(socketDescriptor, (struct sockaddr *)&sin, sizeof(sin));

    pthread_create(&tidUDP, NULL, UDPThreadFunc, &exitSignal);
}

void UDP_cleanup(void)
{
    close(socketDescriptor);
    stopping = 1;
    pthread_join(tidUDP, NULL);
    pthread_mutex_destroy(&UDPmutex);
}

static void UDP_send_message(char message[])
{
    // send a message
    unsigned int sin_len = sizeof(sinRemote);
    sendto(socketDescriptor, message, strlen(message), 0, (struct sockaddr *)&sinRemote, sin_len);
}

static char prevCommand[MAX_LEN];
static char *helpMessage = "\nAccepted command examples:\n"
                           "beat_rock      -- sets the track to standard rock\n"
                           "beat_custom    -- sets the track to custom rock.\n"
                           "beat_none      -- sets the track to none\n"
                           "volume    -- set the volume.\n"
                           "tempo    -- set the tempo (bpm).\n"
                           "play_base    -- play the base drum.\n"
                           "play_snare    -- play the snare drum.\n"
                           "play_hi_hat    -- play the hi-hat.\n"
                           "stop       -- cause the server program to end.\n"
                           "<enter>    -- repeat last command.\n";

static char prevCommand[MAX_LEN] = "help";
static int UDP_process_message(char command[])
{
    char returnMessage[MAX_LEN];
    if (strcmp(command, "help") == 0 || strcmp(command, "?") == 0)
    {

        strncpy(returnMessage, helpMessage, MAX_LEN - 1);
        returnMessage[MAX_LEN - 1] = '\0';
        UDP_send_message(returnMessage);
    }
    else if (strcmp(command, "beat_rock") == 0)
    {
        beatbox_set_track(1);
    }else if (strcmp(command, "beat_custom") == 0)
    {
        beatbox_set_track(2);
    }else if (strcmp(command, "beat_none") == 0)
    {
        beatbox_set_track(0);
    }
    else if (strcmp(command, "volume") == 0)
    {
        // need to interpret volume value somehow
    }
    else if (strcmp(command, "tempo") == 0)
    {
        // need to interpret volume value somehow
    }
    else if (strcmp(command, "play_base") == 0)
    {
        beatbox_base();
    }
    else if (strcmp(command, "play_snare") == 0)
    {
        beatbox_snare();
    }
    else if (strcmp(command, "play_hi_hat") == 0)
    {
        beatbox_hi_hat();
    }
    else if (strcmp(command, "") == 0 && strcmp(prevCommand, "") != 0)
    {
        UDP_process_message(prevCommand);
        return 0;
    }
    else if (strcmp(command, "stop") == 0)
    {
        //UDP_send_message("exit the program gracefully, closing all open sockets threads dynamic memmory, etc");
        return 1;
    }else {
        return 0;
    }
    //printf("prevCommand: %s\n", prevCommand);
    char commandBuffer[MAX_LEN];
    strcpy(commandBuffer, command);
    strcpy(prevCommand, commandBuffer);
    return 0;
}