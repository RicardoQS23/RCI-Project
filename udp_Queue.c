#include "project.h"

/**
 * @brief In this function, a random node from the node's list is chosen to be connected to the entering node
 */
int chooseRandomNodeToConnect(char *buffer, char *my_id)
{
    char buffer_cpy[MAX_BUFFER_SIZE];
    char *token, id[3];
    int counter = 0, i, num, nodeOnNet = 0;
    srand(time(0));

    memset(buffer_cpy, 0, MAX_BUFFER_SIZE);
    strcpy(buffer_cpy, buffer);
    token = strtok(buffer_cpy, "\n");
    token = strtok(NULL, "\n");
    while (token != NULL)
    {
        if(sscanf(token, "%s %*s %*s", id) == 1)
        {
            if(strcmp(id, my_id) == 0)
                nodeOnNet = 1;
        }
        counter++;
        token = strtok(NULL, "\n");
    }

    if (counter == 0) // lista de nos na rede vazia
    {
        return 0;
    }

    token = strtok(buffer, "\n");
    token = strtok(NULL, "\n");

    num = (rand() % counter);
    for (i = 0; i < counter - 1; i++)
    {
        if (i == num)
            break;
        token = strtok(NULL, "\n");
    }
    strcpy(buffer, token);
    return nodeOnNet;
}

/**
 * @brief 
 */
void udpClient(char *buffer, char *ip, char *port)
{
    int errcode, fd;
    ssize_t n;
    struct addrinfo hints, *res;
    socklen_t addrlen;
    struct sockaddr_in addr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1)
    {
        printf("Cant communicate with udp server\n");
        freeaddrinfo(res);
        close(fd);
        return;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    errcode = getaddrinfo(ip, port, &hints, &res);
    if (errcode != 0)
    {
        printf("Cant communicate with udp server\n");
        freeaddrinfo(res);
        close(fd);
        return;
    }

    n = sendto(fd, buffer, strlen(buffer), 0, res->ai_addr, res->ai_addrlen);
    if (n == -1)
    {
        printf("Cant communicate with udp server\n");
        freeaddrinfo(res);
        close(fd);
        return;
    }

    addrlen = sizeof(addr);
    n = recvfrom(fd, buffer, MAX_BUFFER_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
    if (n == -1)
    {
        printf("Cant communicate with udp server\n");
    }
    freeaddrinfo(res);
    close(fd);
}

/**
 * @brief This function registers the node on the network
 */
void regNetwork(AppNode *app, fd_set *currentSockets, char *regIP, char *regUDP, char *net)
{
    char buffer[64] = "\0";
    sprintf(buffer, "REG %s %s %s %s", net, app->self.id, app->self.ip, app->self.port);
    udpClient(buffer, regIP, regUDP);
    FD_SET(app->self.socket.fd, currentSockets);
}

/**
 * @brief This function unregisters the node from the network
 */
void unregNetwork(AppNode *app, fd_set *currentSockets, char *regIP, char *regUDP, char *net)
{
    char buffer[32] = "\0"; 
    sprintf(buffer, "UNREG %s %s", net, app->self.id);
    udpClient(buffer, regIP, regUDP);
    FD_CLR(app->self.socket.fd, currentSockets);
}

/**
 * @brief This function clears the queue 
 */
void cleanQueue(NodeQueue *queue, fd_set *currentSockets)
{
    for (int i = queue->numNodesInQueue; i >= 0; i--)
    {
        FD_CLR(queue->queue[i].socket.fd, currentSockets);
        close(queue->queue[i].socket.fd);
    }
    memset(queue, 0, sizeof(NodeQueue));
}

/**
 * @brief Pops the queue
 */
void popQueue(NodeQueue *queue, int pos)
{
    memmove(&queue->queue[pos], &queue->queue[queue->numNodesInQueue - 1], sizeof(NODE));
    memset(&queue->queue[queue->numNodesInQueue - 1], 0, sizeof(NODE));
    queue->numNodesInQueue--;
}


void promoteQueueToIntern(AppNode *app, NodeQueue *queue, int pos)
{
    app->interns.numIntr++;
    memmove(&app->interns.intr[app->interns.numIntr - 1], &queue->queue[pos], sizeof(NODE));
    popQueue(queue, pos);
    //updateExpeditionTable(app, app->interns.intr[app->interns.numIntr - 1].id, app->interns.intr[app->interns.numIntr - 1].id, app->interns.intr[app->interns.numIntr - 1].socket.fd);
}


void promoteTemporaryToExtern(AppNode *app, NODE *temporaryExtern)
{
    memmove(&app->ext, &(*temporaryExtern), sizeof(NODE));
    temporaryExtern->socket.fd = -1;
    memset(temporaryExtern->socket.buffer, 0, MAX_BUFFER_SIZE);
    //updateExpeditionTable(app, app->ext.id, app->ext.id, app->ext.socket.fd);
}


/**
 * @brief resets the temporary Extern node
 */
void resetTemporaryExtern(NODE *temporaryNode, fd_set *currentSockets)
{
    FD_CLR(temporaryNode->socket.fd, currentSockets);
    close(temporaryNode->socket.fd);
    temporaryNode->socket.fd = -1;
    memset(temporaryNode->socket.buffer, 0, MAX_BUFFER_SIZE);
}