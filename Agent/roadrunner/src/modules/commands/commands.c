/**
 * commands.c contains all of the functionality for serializeing and deserializing commands and responses.
 * @file commands.c
 */

#include <commands.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdbool.h>
#include <utils.h>

/**
 * @brief Deserialize a message stream of bytes into a Command structure.
 * @param msg_size (in) the total message stream size
 * @param msg_stream (in) a pointer to the message stream
 * @return a pointer to a Command structure or NULL on error
 */
Command *deserialize_command(uint32_t  msg_size, char *msg_stream)
{
    // Validate command received
    uint32_t command_length = 0;
    uint32_t n_command_length = 0;
    uint32_t n_args_len = 0;
    uint32_t mem_offset = 0;
    char *command = NULL;
    uint32_t args_len = 0;
    char *args = NULL;
    Command *cmd = NULL;
    // Get bytes
    // convert from network to host byte order
    if(msg_stream == NULL)
    {
        COMMANDS_DEBUG_PRINT("error: msg_stream must not be NULL\n");
        goto fail_out;
    }
    // check to see if we have a min sized stream
    if(msg_size < COMMAND_STREAM_MIN_SIZE)
    {
        COMMANDS_DEBUG_PRINT("error: command stream is to small to be valid\n");
        goto fail_out;
    }
    mem_offset += sizeof(uint32_t);
    memcpy(&n_command_length, msg_stream + mem_offset, sizeof(uint32_t));
    command_length = ntohl(n_command_length);
    command = calloc(command_length, sizeof(char));
    mem_offset += sizeof(uint32_t);
    memcpy(command, msg_stream + mem_offset, command_length);
    mem_offset += command_length;
    memcpy(&n_args_len, msg_stream + mem_offset, sizeof(uint32_t));
    args_len = ntohl(n_args_len);
    mem_offset += sizeof(uint32_t);
    args = calloc(args_len, sizeof(char));
    memcpy(args, msg_stream + mem_offset, args_len);
    cmd = alloc_command(command, command_length, args, args_len);
    checkfree(command);
    checkfree(args);

    return cmd;
fail_clean:
    checkfree(command);
    checkfree(args);
fail_out:
    return NULL;
}

/**
 * @brief Allocate the memory and store data in a new Command structure.
 * @param cmd (in) pointer to the cmd bytes
 * @param cmd_len (in) the size of the cmd
 * @param args (in) pointer to the args bytes
 * @param args_len (in) the size of the args
 * @return a pointer to an empty Command structure or NULL on error
 */
Command *alloc_command(char *cmd, uint32_t cmd_len, char *args, uint32_t args_len)
{
    Command *new_cmd = calloc(1, sizeof(Command));
    // if we get a null then there are memory issues and we need to bail
    if(new_cmd == NULL)
    {
        return new_cmd;
    }
    new_cmd->cmd = calloc(cmd_len, sizeof(char));
    memcpy(new_cmd->cmd, cmd, cmd_len);
    new_cmd->cmd_len = cmd_len;
    new_cmd->args = calloc(args_len, sizeof(char));
    memcpy(new_cmd->args, args, args_len);
    new_cmd->args_len = args_len;
    return new_cmd;
}

/**
 * @brief Free a Command structure and its components.
 * @param cmd a pointer to a Command structure
 */
void free_command(Command *cmd)
{
    if(cmd != NULL)
    {
        checkfree(cmd->cmd)         
        checkfree(cmd->args)
        free(cmd);
    }
}

/**
 * @brief Serialize a Response structure into a byte stream.
 * @param rsp (in) a pointer to a Response structure
 * @param stream_out (out) a NULL char pointer where the stream will be passed back out
 * @return the total size of the stream
 */
uint32_t serialize_response(Response *rsp, char **stream_out)
{
    // initialize variables
    uint32_t total_size = 0;
    *stream_out = (char *)calloc((sizeof(uint32_t) * 3) + rsp->msg_len, sizeof(char));
    uint32_t msg_len = rsp->msg_len;
    uint32_t ret_code = 0;
    uint32_t offset = sizeof(uint32_t);
    // calculate the total size of the message
    total_size = sizeof(uint32_t) * 3 + rsp->msg_len;
    // Convert to network byte order
    total_size = htonl(total_size);
    msg_len = htonl(msg_len);
    ret_code = htonl(rsp->ret_code);

    // copy each part of the message into the stream
    memcpy(*stream_out, &total_size, 4);
    memcpy(*stream_out + offset, &ret_code, 4);
    offset += 4;
    memcpy(*stream_out + offset, &msg_len, 4);
    offset += 4;
    memcpy(*stream_out + offset, rsp->msg, rsp->msg_len);

    total_size = ntohl(total_size);
    return total_size;
}

/**
 * @brief Allocate memory for a Response structure.
 * @param ret_code (in) the integer return code
 * @param msg (in) the message stream
 * @param msg_len (in) the size of the message stream
 * @return a pointer to a Response structure or NULL on error
 */
Response *alloc_response(int32_t ret_code, char* msg, uint32_t msg_len)
{
    Response *rsp;
    rsp = calloc(1, sizeof(Response));

    // if we get a null then there are memory issues and we need to bail
    if (rsp == NULL)
    {
        return rsp;
    }

    rsp->ret_code = ret_code;
    rsp->msg_len = msg_len;

    // need to include the null terminator
    rsp->msg = calloc(msg_len, sizeof(char));
    memcpy(rsp->msg, msg, msg_len);

    return rsp;
}

/**
 * @brief free a Response structure and its components.
 * @param rsp pointer to a Response structure
 */
void free_response(Response *rsp)
{
    
    // check to see if the Response structure is NULL
    if(rsp != NULL)
    {
        // free the message stream
        checkfree(rsp->msg)
        // free the whole structure  
        free(rsp);
    }
}

