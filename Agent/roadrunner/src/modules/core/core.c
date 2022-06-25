/**
 * core.c contains all of the functionality for the core RoadRunner commands, checkin and sleep.
 * @file core.c
 */

#include <core.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * @brief Check in with the RoadRunner command post.
 * @return a Response structure
 */
Response *checkin_command()
{
    char msg[] = "roadrunner checkin";
    Response *rsp = alloc_response(0, msg, sizeof(msg));
    return rsp;
}

/**
 * @brief Command to have RoadRunner sleep for a specified number of seconds.
 * @param cmd the command structure
 * @return a Response structure
 */
Response *sleep_command(Command *cmd)
{
    // Perform sleep and return base msg with how long the agent was commanded to sleep

    Response *rsp = NULL;
    uint32_t sleep_time;
    char *base_msg = "road runner slept for %s second(s)"; 
    char new_message[50];
    uint32_t return_code = 0;
    uint32_t msg_leng = 0;
    uint32_t i;
    
    if(NULL == cmd->args)
    {
        return_code = 1;
        base_msg = "arguments provided are NULL";
        rsp = alloc_response(return_code, base_msg, strlen(base_msg));
        return rsp;
    }
    printf("cmd->args: %s\n", cmd->args);
    printf("cmd->args_len: %d\n", cmd->args_len);
    for (i = 0; i < cmd->args_len-1; ++i)
    {
        printf("cmd->args[i]: %c\n", cmd->args[i]);
        if (!isdigit(cmd->args[i]))
        {
            return_code = 1;
            base_msg = "arguments are not of the correct type";
            rsp = alloc_response(return_code, base_msg, strlen(base_msg));
            return rsp;
        }
    }
    sscanf(cmd->args, "%d", &sleep_time);
    sleep(sleep_time);
    snprintf(new_message, sizeof(new_message), base_msg, cmd->args);

    rsp = alloc_response(return_code, new_message, sizeof(new_message));

    return rsp;
}
