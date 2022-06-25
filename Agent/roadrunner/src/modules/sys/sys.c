/**
 * sys.c contains all of the functionality for the system commands.
 * @file sys.c
 */

#include <sys.h>

#define PLACEHOLDER "replace"
#define PLACEHOLDER_SIZE 7

// processlist helper functions
static uint32_t is_pid_folder(char *dir);
static uint32_t parse_pid_stat(char *proc_pid_dir, pid_stat_t *pid_stats);
static uint32_t parse_pid_owner(char *proc_pid_dir, pid_stat_t *pid_stats);
static uint32_t parse_pid_cmdline(char *proc_pid_dir, pid_stat_t *pid_stats);
static uint32_t return_plist_line(char *proc_pid_dir, char *proc_line);

// netstat helper functions
static uint32_t parse_conn_line(char *line, tcp_conn_t *connections);
static char *hex_to_ipaddr(u_int32_t ip_h);
static void free_connections(tcp_conn_t *connections);
static void print_connections(tcp_conn_t *connections, char *print_line);


/************************INSTRUCTIONS*****************************

Build 4 commands:
    hostname                Returns hostname
    invoke                  Returns shell command output
    proclist                Returns process list or proc info
    netstat                 Returns netstat

Hostname Functions:
    hostname_command

Invoke Functions
    invoke_command

Proclist Functions:
    process_list_command
        is_pid_folder
        return_pslist
            parse_pid_stat
            parse_pid_owner
            parse_pid_cmdline

Netstat Functions:
    get_netstat_command
        parse_conn_line
            hex_to_ipaddr
        print_connections
        free_connections

If you want to make use of the test harness, do not remove the MACROs. 
The macros will perform the required functionality for the function. 
See the defined macros in sys.h for more information.

*****************************END**********************************/

/**
 * @brief Returns hostname of host machine
 * @param cmd (in) a pointer to the command
 * @return Response where the message is the hostname
 * @return HOSTNAME_ERROR_MSG on error
 */
Response *hostname_command(Command *cmd)
{
    int32_t ret_code;
    char hostname[BUFF_MAX + 1];
    Response *rsp = NULL;

    // Get hostname
    HOSTNAME_MACRO(hostname, BUFF_MAX + 1, ret_code)

    if(-1 == ret_code)
    {
        ret_code = 1;
        rsp = alloc_response(ret_code, HOSTNAME_ERROR_MSG, sizeof(HOSTNAME_ERROR_MSG));
        return rsp;
    }
    else
    {
        rsp = alloc_response(ret_code, hostname, sizeof(hostname));
        return rsp;
    }
}

/**
 * @brief Builds formatted process list in one heap buffer and passed to Response
 * arg[1] = "" (empty) - Return full process list
 * arg[1] = pid - Return process information for just 1 pid
 * 
 * @param cmd (in) Command selected with args
 * @return Response* Formatted process list on success
 * @return Response* PROCESS_ERROR_MSG on error
 */
Response *process_list_command(Command *cmd)
{
    FILE *fp = NULL;
    Response *rsp = NULL;
    uint32_t msg_len = 0;
    char *msg = NULL;

    char ps[] = "ps";
    char space[] = " ";
    char *pid = NULL;
    char *owner = NULL;
    char loginuid_char[] = "/loginuid";
    uint32_t loginuid_int = 0;
    char p[] = "/";
    /* /proc/<process id>/loginuid */
    char *proc_pid_dir = NULL;
    int32_t content_size = 0;
    struct passwd *pwduid = NULL;



    // proc_pid_dir = (char *)calloc(sizeof(PROC_DIR) + sizeof(p) + cmd->args_len + sizeof(loginuid_char), sizeof(char));
    // strcat(proc_pid_dir, PROC_DIR);
    // strcat(proc_pid_dir, p);
    // strcat(proc_pid_dir, cmd->args);
    // strcat(proc_pid_dir, loginuid_char);
    // READ_LOGINID_MACRO(proc_pid_dir, &owner, content_size);
    // printf("%s", proc_pid_dir);
    // loginuid_int = atoi(owner);
    // pwduid = getpwuid(loginuid_int);





    // if(strlen(cmd->args) != 0)
    // {
    //     pid = (char *)calloc(sizeof(ps) +sizeof(space) + cmd->args_len, sizeof(char));
    //     strcat(pid, ps);
    //     strcat(pid, space);
    //     strcat(pid, cmd->args);
    // }
    // else
    // {
    //     pid = (char *)calloc(sizeof(ps), sizeof(char));
    // }

    // printf("%s\n", pid);
    // fp = popen("ps 55166", "r");
    // fseek(fp, 0, SEEK_END);
    // msg_len = ftell(fp);
    // rewind(fp);
    // msg = (char *)calloc(msg_len, sizeof(char));
    // fscanf(fp, "%s", msg);
    // printf("%s\n", msg);
    rsp = alloc_response(0, PLACEHOLDER, PLACEHOLDER_SIZE + 1);
    return rsp;  

}

/**
 * @brief Checks if folder contains all digits
 * @param entry (in) dirent structure - returned from readdir
 * @return uint32_t Returns 1 on success, 0 on failure
 */
static uint32_t is_pid_folder(char *dir)
{    
    return 1;
}

/**
 * @brief Loads struct pid_stat_t with information for 1 process from /proc
 *  processid -> pid
 *  parentpid -> ppid
 *  process name -> comm
 *  State of process -> state
 *
 * @param proc_pid_dir (in) Directory of process
 * @param pid_stats (out) pointer to struct pid_stat_t
 * @return Returns 0 if unable to process information
 * @return Returns 1 if successful
 */
static uint32_t parse_pid_stat(char *proc_pid_dir, pid_stat_t *pid_stats)
{
    // read process statistics /proc/<pid>/stat
    char *raw_pid_stats = NULL;
    uint32_t contents_size = 0;

    /* Open and read /proc/<pid>/stat, Output are statistics, delimited by " "
     *  "man proc" for details */
    READ_PROC_MACRO(proc_pid_dir, &raw_pid_stats, contents_size);

    // load pid_stats with information

    return 1;
}

/**
 * @brief Opens /proc/<pid>/loginuid to determine owner of process.
 * Sometimes loginuid returns -1 -> sets owner to "unk"
 * Loads pid_stats->owner with the owner of the process
 * NOTE: Loginuid isn't always accurate to the owner of the process
 * 
 * @param proc_pid_dir (in) loginuid path (ex. /proc/<pid>/loginuid)
 * @param pid_stats (out) struct with process information - defined in sys.h
 * @return uint32_t returns 0 if unable to parse owner
 * @return uint32_t returns 1 on success
 */
static uint32_t parse_pid_owner(char *proc_pid_dir, pid_stat_t *pid_stats)
{
    // Initialize Variables
    char *owner = NULL;
    uint32_t contents_size = 0;

    // call getpwuid(uid)
    READ_LOGINID_MACRO(proc_pid_dir, &owner, contents_size);
    return 1;
}

/**
 * @brief Opens /proc/<pid>/cmdline to get the exact command that kicked off the process
 * Loads pid_stats->cmdline with the command
 * 
 * @param proc_pid_dir (in) path to cmdline (ex. /proc/<pid>/cmdline)
 * @param pid_stats (out) struct with process information - defined in sys.h
 * @return uint32_t returns 0 if unable to parse cmdline
 * @return uint32_t returns 1 on success
 */
static uint32_t parse_pid_cmdline(char *proc_pid_dir, pid_stat_t *pid_stats)
{
    // Initialize variables
    char *cmdline = NULL;
    uint32_t contents_size = 0;

    // read proc_dir and returns the cmdline string
    READ_CMDLINE_MACRO(proc_pid_dir, &cmdline, contents_size);
    return 1;
}

/**
 * @brief Returns a formatted line with all corresponding pid information 
 * @param pid (in) process id (string of digits)
 * @param proc_line (out) formatted string with all process information
 * @return uint32_t length of proc_line (0 on failure)
 * *NOTE: Caller needs to free return
 */
static uint32_t return_plist_line(char *pid, char *proc_line)
{
    //parse_pid_stat
    //parse_pid_owner
    //parse_pid_cmdline

    /* Format process line for return
    snprintf(proc_line, PROCESS_LINE_MAX, "%-10s %8d %8d %-20s %s\n",
             pid_stats->owner,
             pid_stats->pid,
             pid_stats->ppid,
             pid_stats->comm,
             pid_stats->cmdline);*/
    return 1;
}

/**
 * @brief Calls Popen to execute shell commmand, not built for interactive cmds
 * 
 * @param cmd (in) 
 * @return Response* Returns command output on success or failure
 */
Response *invoke_command(Command *cmd)
{
    // Use the man pages to learn about he Popen command
    // Use the function to invoke the user specified command
    // Read the output from the command into a buffer and return it to the user
    // in a response
    Response *rsp = NULL;
    FILE *fp = NULL;
    char *msg = NULL;
    int32_t msg_len = 0;
    int32_t ret_code = 0;
    // fp = popen(cmd->args, "r");
    // if(NULL == fp)
    // {
    //     ret_code = 1;
    //     msg = "Command Failed";
    //     msg_len = 15;
    //     rsp = alloc_response(ret_code, msg, msg_len);
    //     return rsp;
    // }
    // fseek(fp, 0, SEEK_END);
    // msg_len = ftell(fp);
    // rewind(fp);
    // msg = (char *)calloc(msg_len, sizeof(char));
    // fscanf(fp, "%s", msg);
    // printf("%s\n", msg);

    // rsp = alloc_response(0, msg, msg_len + 1);
    // pclose(fp);
    // free(msg);
    uint32_t read_counter = 0;
    int alloc_counter = 0;
    int alloc_multiplier = 2;

    // open the file handle in read bytes mode
    fp = popen(cmd->args, "r");
    if(NULL == fp)
    {
        ret_code = 1;
        msg = "Command Failed";
        msg_len = 15;
        rsp = alloc_response(ret_code, msg, msg_len);
        return rsp;
    }

    // allocate our initial read buffer
    msg = (char*)malloc(1024);
    if(NULL == msg) {
        printf("Unable to allocate read buffer.\n");
        return read_counter;
    }

    // get file contents
    while(!feof(fp)) {
        (msg)[read_counter] = fgetc(fp);
        read_counter++;
        alloc_counter++;

        // if 1024 have been read realloc
        if(alloc_counter == 1024) {
            char *tmpContent = (char*)realloc(msg, 1024 * alloc_multiplier);
            if(tmpContent == NULL) {
                // checkfree(msg);
                return read_counter;
            }
            msg = tmpContent;
            tmpContent = NULL;

            alloc_multiplier++;
            alloc_counter = 0;
        }
    }
    rsp = alloc_response(0, msg, read_counter - 1);

    // checkclose(fp);


    return rsp;
}

/**
 * @brief Opens /proc/net/tcp and sends current tcp connections
 * 
 * @param cmd (in)
 * @return Response* returns formatted tcp socket information on success
 * @return Response* returns NETSTAT_ERROR_MSG on failure
 */
Response *get_netstat_command(Command *cmd)
{
    char *tcp_stream = NULL;
    uint32_t contents_size = 0;
    tcp_conn_t *connections = NULL;
    char netstat_buff[255] = {NULL};
    char *next_tcp_connection = NULL;
    int32_t parse_connection_status = 0;
    unsigned count = 0;
    char *print_line = (char *)calloc(PROCESS_LINE_MAX, sizeof(char));
    char *msg = NULL;
    char *netstat_header = "LOCAL                 REMOTE                STATUS     OWNER                     INODE\n";

    // (char *)calloc(BUFF_MAX, sizeof(char));
    
    
    // Read /proc/net/tcp and get stat information
    contents_size = read_file("/proc/net/tcp", &tcp_stream);
    // printf("hiiii\n");
    printf("Netstat content_size: %d\n", contents_size);
    
    if(0 == contents_size)
    {
        Response *rsp = alloc_response(0, NETSTAT_ERROR_MSG, sizeof(NETSTAT_ERROR_MSG));
        return rsp;
    }

    // Set up header for netstat, use this line for the test harness
    // Uncomment the line below
    snprintf(netstat_buff, BUFF_MAX, "%-21s %-21s %-10s %-20s %10s\n", "LOCAL", "REMOTE", "STATUS", "OWNER", "INODE");
    char* token;
    next_tcp_connection = tcp_stream;
    token = strtok_r(next_tcp_connection, "\n", &next_tcp_connection);
    // printf("%s\n",next_tcp_connection);

    tcp_conn_t *head = (tcp_conn_t *)calloc(1, sizeof(tcp_conn_t));
    tcp_conn_t *start = head;
 
    while ((token = strtok_r(next_tcp_connection, "\n", &next_tcp_connection)))
    {
        printf("%s\n",next_tcp_connection);
        parse_connection_status = parse_conn_line(token, head);  
    }
    head = start;
    head = head->next;
    msg = (char *)calloc(contents_size, sizeof(char));

    // Walk through linked list
    strcat(msg, netstat_header);

    while (NULL != head)
    {
        printf("Start Loop\n");
        if( 0 >= head->state)
        {
            break;
        }
        printf("print connections\n");
        print_connections(head, print_line);
        printf("strcat\n");
        strcat(msg, print_line);
        printf("head\n");
        head = head->next;
    }

    // set up response
    printf("alloc response, strlen: %d\n", strlen(msg) + 1);
    Response *rsp = alloc_response(0, msg, strlen(msg) + 1);
    printf("End of Netstat\n");
    return rsp;
}

/**
 * @brief Parse network connections from /proc/net/tcp and set relevant fields in tcp_conn_t struct
 * 
 * @param line (in) Line of network connections from /proc/net/tcp (1 network connection at a time)
 * @param connections (out) linked list structure to hold network info for each connection
 * @return uint32_t returns 1 on success
 * @return uint32_t returns 0 on failure
 */
static uint32_t parse_conn_line(char *line, tcp_conn_t *connections)
{

    uint32_t uid = 0;
    struct passwd *pwd = NULL;
    uint8_t count = 0;

    char* line_token = NULL;
    char* line_cp = line;
    char* temp = NULL;
    char* temp2 = NULL;

    char *local_address = NULL;
    uint32_t local_address_int = 0;
    char *local_port = NULL;


    char *remote_address = NULL;
    uint32_t remote_address_int = 0;
    char *remote_port = 0;


    unsigned long inode = 0;

    int16_t i = 0;
    tcp_conn_t *temp_Node = (tcp_conn_t *)calloc(1, sizeof(tcp_conn_t));
    
    if(temp_Node == NULL)
    {
        return 0;
    }
    while ((line_token = strtok_r(line_cp, " ", &line_cp)))
    {
        if(1 == count)
        {
            temp = line_token;
            local_address = strtok_r(temp, ":", &temp);
            local_address_int = strtol(local_address, NULL, 16);
            local_port = strtok_r(temp, ":", &temp);
            strcpy(temp_Node->local_addr, hex_to_ipaddr(local_address_int));
            temp_Node->local_port = strtol(local_port, NULL, 16);
        }
        if(2 == count)
        {
            temp2 = line_token;
            remote_address = strtok_r(temp2, ":", &temp2);
            remote_address_int = strtol(remote_address, NULL, 16);
            remote_port = strtok_r(temp2, ":", &temp2);
            strcpy(temp_Node->remote_addr, hex_to_ipaddr(remote_address_int));
            temp_Node->remote_port = strtol(remote_port, NULL, 16);
        }
        else if(3 == count)
        {
           temp_Node->state = strtol(line_token, NULL, 16);   
        }
        else if(7 == count)
        {
            uid = atoi(line_token);
        }
        else if(9 == count)
        {
            temp_Node->inode = strtol(line_token, NULL, 10);
        }
        count++;      
    }
    count = 0;
    temp = NULL;
    temp2 = NULL;
    // Get username from uid - calls getpwuid
    GET_UID_MACRO(uid, pwd);

    strcpy(temp_Node->owner, pwd->pw_name);
    temp_Node->next = NULL;

    while(NULL != connections->next)
    {
        connections = connections->next;
    }
    connections->next = temp_Node;
    connections = connections->next;
    // printf("%s\n", connections->owner);

    return 1;
}

/**
 * @brief Format a netstat connection
 * 
 * @param connections (in) structure with connection information - defined in sys.h
 * @param print_line (out) formatted connection line
 */
static void print_connections(tcp_conn_t *connections, char *print_line)
{
    /* Format line ("-" makes it left justified)
    * Need this format for the test harness
    * Uncomment the line below
    */
    const char *states[11] = {"ESTAB","SYN_SENT","SYN_RECV","FIN_WAIT1","FIN_WAIT2",
"TIME_WAIT","CLOSED","CLOSE_WAIT","LAST_ACK","LISTENING","CLOSING","UNKNOWN",};
    // printf("state: %d\n", connections->state-1);
    char *state = states[connections->state-1];
    // printf("snprintf\n");
    snprintf(print_line, BUFF_MAX, "%15s:%-5d %15s:%-5d %-10s %-20s %10ld\n",
                connections->local_addr,
                connections->local_port,
                connections->remote_addr,
                connections->remote_port,
                state,
                connections->owner,
                connections->inode);
    return;
}

/**
 * @brief Free connection linked list
 * 
 * @param connections (in) structure with connection information - defined in sys.h
 */
static void free_connections(tcp_conn_t *connections)
{
    return;
}

/**
 * @brief convert network order hex to ipv4 string notation
 *       This function is useful to make the hex representation found in
 *       /proc/net/tcp human readable
 * 
 * @param ip_h (in) hex value found in /proc/net/tcp
 * @return char* ipv4 string notation
 */
static char *hex_to_ipaddr(uint32_t ip_h)
{    
    struct in_addr addr;
    addr.s_addr = ip_h;
    return inet_ntoa(addr);
}
