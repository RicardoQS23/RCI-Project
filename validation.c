#include "project.h"

/**
 * @brief App initialization
 */
void init(AppNode *app, NodeQueue *queue, NODE *temporaryExtern, char *regIP, char *regUDP, char **argv, int argc)
{
    if(argc != 5 && argc != 3)
    {
        printf("invalid inputs ./cot [IP] [TCP] [regIP] [regUDP]\n");
        exit(1);
    }
    
    if (validateCommandLine(argv, argc) < 0)
    {
        printf("invalid inputs ./cot [IP] [TCP] [regIP] [regUDP]\n");
        exit(1);
    }
    memset(app, 0, sizeof(struct AppNode));
    memset(queue, 0, sizeof(struct NodeQueue));
    strcpy(app->self.ip, argv[1]);
    strcpy(app->self.port, argv[2]);
    if ((app->self.socket.fd = openTcpServer(app)) < 0)
    {
        printf("Couldn't open TCP Server\n");
        exit(1);
    }
    if(argc == 5)
    {
    strcpy(regIP, argv[3]);
    strcpy(regUDP, argv[4]);
    }
    else
    {
    strcpy(regIP, "193.136.138.142");
    strcpy(regUDP, "59000");
    }
    temporaryExtern->socket.fd = -1;
}

/**
 * @brief This function counts the number of completed messages acording to the number  of '\n' characters 
 */
int countLFchars(char *buffer)
{
    int counter = 0;
    for(int i = 0; i < (int)strlen(buffer); i++)
    {
        if(buffer[i] == '\n')
            counter++;
    }
    return counter;
}

/**
 * @brief This function validates the user's input
 */
int validateUserInput(AppNode *app, enum commands *cmd, char *buffer, char *bootIP,  char *name, char *dest, char *bootID, char *bootTCP, char *net, char *fileName, int joinFlag)
{
    char *token;
    int cmd_code;
    if((token = strtok(buffer, " \n\r\t")) == NULL)
        return -1;
        
    cmd_code = compare_cmd(token);
    if(cmd_code == 0 && joinFlag == 1)
    {
        printf("Can't repeat join command\n");
        return -1;
    }
    if(cmd_code == 1 && joinFlag == 1)
    {
        printf("Can't repeat djoin command\n");
        return -1;
    }
    switch (cmd_code)
    {
    case 0: // Join
        *cmd = JOIN;

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;

        if (strlen(token) != 3 && ((atoi(token) > 100) || atoi(token) < 0))
            return -1;
        strcpy(net, token);

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;

        if (strlen(token) != 2 && ((atoi(token) > 99) || atoi(token) < 0))
            return -1;
        strcpy(app->self.id, token);
        break;
    case 1: // Djoin
        *cmd = DJOIN;
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strlen(token) != 3 && ((atoi(token) > 100) || atoi(token) < 0))
            return -1;
        strcpy(net, token);

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strlen(token) != 2 && ((atoi(token) > 99) || atoi(token) < 0))
            return -1;
        strcpy(app->self.id, token);

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strlen(token) != 2 && ((atoi(token) > 100) || atoi(token) < 0))
            return -1;
        strcpy(bootID, token);

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        strcpy(bootIP, token);

        if ((token = strtok(NULL, " \n\t\r")) == NULL)
            return -1;
        strcpy(bootTCP, token);
        if (strlen(bootTCP) != 5)
            return -1;

        if (validate_ip(bootIP) != 0)
            return -1;
        break;
    case 2:
        *cmd = CREATE;
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strlen(token) > 100)
            return -1;
        strcpy(name, token);
        break;
    case 3:
        *cmd = DELETE;
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strlen(token) > 100)
            return -1;
        strcpy(name, token);
        break;
    case 4:
        *cmd = GET;
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        strcpy(dest, token);

        if (strcmp(app->self.id, dest) == 0)
            return -1;

        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;

        if (strlen(token) > 100)
            return -1;
        strcpy(name, token);
        break;
    case 5:
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strcmp(token, "topology") == 0)
            *cmd = SHOW_TOPOLOGY;
        else if (strcmp(token, "names") == 0)
            *cmd = SHOW_NAMES;
        else if (strcmp(token, "routing") == 0)
            *cmd = SHOW_ROUTING;
        else
            *cmd = -1;
        break;
    case 6:
        *cmd = LEAVE;
        break;
    case 7:
        *cmd = EXIT;
        break;
    case 8:
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        if (strcmp(token, "names") == 0)
            *cmd = CLEAR_NAMES;
        else if (strcmp(token, "routing") == 0)
            *cmd = CLEAR_ROUTING;
        else
            *cmd = -1;
        break;
    case 9:
        *cmd = LOAD;
        if ((token = strtok(NULL, " \n\r\t")) == NULL)
            return -1;
        strcpy(fileName, token);
        break;
    case 10:
        *cmd = SHOW_NAMES;
        break;
    case 11:
        *cmd = SHOW_TOPOLOGY;
        break;
    case 12:
        *cmd = SHOW_ROUTING;
        break;
    case 13:
        *cmd = CLEAR_NAMES;
        break;
    case 14:
        *cmd = CLEAR_ROUTING;
        break;
    default:
        *cmd = -1;
        break;
    }
    return 0;
}

/**
 * @brief Command line validation. Returns -1 if the input is invalid
 */
int validateCommandLine(char **cmdLine, int argc)
{
    // Check machineIP
    if (validate_ip(cmdLine[1]) < 0)
        return -1;
    // Check TCP
    if (strlen(cmdLine[2]) > 5)
        return -1;
    // Check regIP
    if(argc == 5)
    {
        if (validate_ip(cmdLine[3]) < 0)
            return -1;
        // Check regUDP
        if (strlen(cmdLine[4]) > 5)
            return -1;
    }
    return 0;
}

/**
 * @brief Validates if the characters are digits, returning 0 if at least one is not
 */
int validate_number(char *str)
{
    while (*str != '\0')
    {
        if (!isdigit(*str))
            return 0; // if the character is not a number, return false
        str++;        // point to next character
    }
    return 1;
}

/**
 * @brief Validates the Ip adress, returning -1 if its invalid and 0 if valid.
 */
int validate_ip(char *ip)
{ // check whether the IP is valid or not
    int num, dots = 0;
    char *ptr, copy[64];
    strcpy(copy, ip);

    if (copy == NULL)
        return -1;
    ptr = strtok(copy, "."); // cut the string using dor delimiter
    if (ptr == NULL)
        return -1;

    while (ptr)
    {
        if (!validate_number(ptr))
            return -1;   // check whether the sub string is  holding only number or not
        num = atoi(ptr); // convert substring to number
        if (num >= 0 && num <= 255)
        {
            ptr = strtok(NULL, "."); // cut the next part of the string
            if (ptr != NULL)
                dots++; // increase the dot count
        }
        else
            return -1;
    }
    if (dots != 3)
        return -1; // if the number of dots are not 3, return false

    return 0;
}

/**
 * @brief Auxiliary function used to check if the input command is one of the commands present in the 'cmds' array
 */
int compare_cmd(char *cmdLine)
{
    char cmds[15][7] = {
        "join",
        "djoin",
        "create",
        "delete",
        "get",
        "show",
        "leave",
        "exit",
        "clear",
        "load",
        "sn",
        "st",
        "sr",
        "cn",
        "cr"
    };
    for (int i = 0; i < 15; i++)
    {
        if (strcmp(cmdLine, cmds[i]) == 0)
            return i;
    }
    return -1;
}

/**
 * @brief function used as a complement to the strtok
 */
char *advancePointer(char *token)
{
    return token + strlen(token) + 1;
}