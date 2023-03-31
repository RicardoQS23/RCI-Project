#include "project.h"

/**
 * ./cot 172.26.180.206 59002 193.136.138.142 59000
 * ./cot 172.26.180.206 59002 193.136.138.142 59000
 * djoin 008 20 11 127.0.0.1 59011
**/
int main(int argc, char *argv[]){
    AppNode app;
    NodeQueue queue;
    NODE temporaryExtern;
    char regIP[16], regUDP[6];
    char net[4], name[16], dest[16], bootIP[16], bootID[3], bootTCP[6], fileName[24];
    fd_set readSockets, currentSockets;
    int counter = 0;
    struct timeval timeout;
    enum commands cmd;

    init(&app, &queue, &temporaryExtern, regIP, regUDP, argv, argc);
    FD_ZERO(&currentSockets);
    FD_SET(0, &currentSockets);

    while (1)
    {
        readSockets = currentSockets;
        memset((void *)&timeout, 0, sizeof(timeout));
        timeout.tv_sec = 2;
        counter = select(FD_SETSIZE, &readSockets, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)&timeout);
       switch (counter)
        {
        case 0:
            if (queue.numNodesInQueue > 0)
                cleanQueue(&queue, &currentSockets);
            if(temporaryExtern.socket.fd != -1)
            {
                FD_CLR(temporaryExtern.socket.fd, &currentSockets);
                close(temporaryExtern.socket.fd);
                temporaryExtern.socket.fd = -1;
                memset(temporaryExtern.socket.buffer, 0, MAX_BUFFER_SIZE);
            }
            break;

        case -1:
            perror("select");
            exit(1);

        default:
            handleInterruptions(&app, &queue, &temporaryExtern, &readSockets, &currentSockets, &cmd, bootIP, name, dest, bootID, bootTCP, net, regIP, regUDP, fileName);
            break;
        }
        counter = 0;
    }
}