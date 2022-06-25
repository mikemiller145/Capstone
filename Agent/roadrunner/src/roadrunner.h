/**
 * roadrunner.h contains the all the defines and declarations needed to run the agent.
 * @file roadrunner.h
 */

// header guard, keeps things from being defined multiple times
#ifndef ROADRUNNER_H
#define ROADRUNNER_H

// we can toggle print statements by defining (or not) RR_DEBUG. Additionally,
// can toggle all debug prints in all other files by defining RR_DEBUG_ALL.
// the defining can be done at compile time which gives us the flexibility
// to change the functionality of the agent without changing the code itself.
#if defined(RR_DEBUG) || defined(RR_DEBUG_ALL)
#include <stdio.h>
#define RR_DEBUG_PRINT(msg, ...) printf(msg, ##__VA_ARGS__);
#else
#define RR_DEBUG_PRINT(msg, ...) 
#endif

// include headers that we need for our declarations
#include <commands.h>

// define things we need for the agent to run
// 10.27.10.10
// localhost
#define HOST "10.27.10.10"
#define PORT "1338"
#define SLEEP_INTERVAL 3

#define SHUTDOWN_MSG "shutting down"
#define BAD_COMMAND_MSG "error: bad command"
#define SHUTDOWN_CMD_STR "shutdown"

#endif
