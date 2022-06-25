/**
 * files.c contains all of the functionality for the download and upload commands.
 * @file files.c
 */

#include <files.h>
#include <stdio.h>
#include <stdlib.h>
#include <utils.h>
#include <string.h>
#include <arpa/inet.h>

static uint32_t deserialize_upload_file_path(char *upload_args, char **file_path_out);
static uint32_t deserialize_upload_file_contents(char *upload_args, uint32_t file_path_len, char **contents_out);

/**
 * @brief opens file specified by parameter passed, puts the contents in contents_out and returns the size.
 * @param filename filename to open
 * @param contents_out a NULL pointer that will contain the contents of the file or NULL on error
 * @return an integer representing the number of bytes read
 */ 
//TODO : Need to make sure this can read binary files. Add fgetc()
uint32_t read_file(char *filename, char **contents_out)
{
    // open the file handle in read bytes mode
    // allocate our initial read buffer
    // get file contents
    // uint32_t read_counter = 0;
    // uint32_t file_len = 0;
    // FILE *file = NULL;
    // char *file_contents = NULL;
    // file = fopen(filename, "rb");
    // int32_t fseek_status = 0;

    // if(NULL == file)
    // {
    //     checkclose(file);
    //     contents_out = NULL;
    //     read_counter = -1;
    //     return read_counter;
    // }

    // fseek_status = fseek(file, 0, SEEK_END);
    // printf("fseek status: %d\n", fseek_status);
    // printf("ftell: %d\n", ftell(file));
    // read_counter = ftell(file);
    // rewind(file);

    // file_contents = (char *)calloc(read_counter, sizeof(char));

    // fread(file_contents, read_counter, 1, file);
    // checkclose(file);
    // //TODO : Need to figure out why this works
    // *contents_out = file_contents;
    // printf("read_counter: %d\n", read_counter);
    // printf("fseek status: %d\n", fseek_status);

    // return read_counter;
    uint32_t read_counter = 0;
    int alloc_counter = 0;
    int alloc_multiplier = 2;

    // open the file handle in read bytes mode
    FILE *fp = NULL;
    fp = fopen(filename, "rb");
    if(NULL == fp) {
        printf("Unable to open file.\n");
        return read_counter;
    }

    // allocate our initial read buffer
    *contents_out = (char*)malloc(1024);
    if(NULL == *contents_out) {
        printf("Unable to allocate read buffer.\n");
        return read_counter;
    }

    // get file contents
    while(!feof(fp)) {
        (*contents_out)[read_counter] = fgetc(fp);
        read_counter++;
        alloc_counter++;

        // if 1024 have been read realloc
        if(alloc_counter == 1024) {
            char *tmpContent = (char*)realloc(*contents_out, 1024 * alloc_multiplier);
            if(tmpContent == NULL) {
                checkfree(*contents_out);
                return read_counter;
            }
            *contents_out = tmpContent;
            tmpContent = NULL;

            alloc_multiplier++;
            alloc_counter = 0;
        }
    }

    checkclose(fp);
    return read_counter -1;

}

// /**
//  * @brief Write the contents of a stream to a specified file.
//  * @param filename the name of the file to write to
//  * @param contents a pointer to the contents that will be written
//  * @param contents_size the size of the contents
//  * @return returns the number of bytes written to the file
//  */
uint32_t write_file(char *filename, char *contents, uint32_t contents_size)
{
    uint32_t bytes_written = 0;
    FILE *file = NULL;

    if(NULL == contents)
    {
        return 0;
    }
    printf("filename: %s\n", filename);
    printf("contents: %s\n", contents);
    
    file = fopen(filename, "wb");
    bytes_written = fwrite(contents, sizeof(char), contents_size, file);
    printf("bytes_written: %d\n", bytes_written);
    checkclose(file);
    if(bytes_written > 0)
    {
        return bytes_written;
    }
    return -1;
}

/**
 * @brief Download the file specified in the Command args.
 * @param cmd the download command with arguments
 * @return a Response where the message is the file byte stream
 */
Response *download_file_command(Command *cmd)
{
    // open the file handle in read bytes mode
    // Send contents
    FILE *file = NULL;
    Response *rsp = NULL;
    char *file_contents = NULL;
    int32_t ret_code = 0;
    long file_len = 0;
    char error_msg[23] = "error downloading file";

    file_len = read_file(cmd->args, &file_contents);

    if(NULL == file_contents)
    {
        checkfree(file_contents);
        ret_code = 1;
        rsp = alloc_response(ret_code, error_msg, sizeof(error_msg));
        return rsp;
    }
    rsp = alloc_response(ret_code, file_contents, file_len);
    checkfree(file_contents);

    return rsp;

}

/**
 * @brief deserialize the file path and file path length from upload arguments.
 * @param upload_args the full upload arguments passed in the command structure
 * @param file_path_out a NULL pointer that will contain the file path on success or NULL on error
 * @return the length of the file path
 */
static uint32_t deserialize_upload_file_path(char *upload_args, char **file_path_out)
{
    uint32_t file_path_len;
    uint32_t offset = 0;
    memcpy(&file_path_len, upload_args, sizeof(uint32_t));
    file_path_len = ntohl(file_path_len);
    offset += 4;
    *file_path_out = calloc(file_path_len, sizeof(char));
    memcpy(*file_path_out, upload_args + offset, file_path_len);
    printf("file_path_len: %d\n", file_path_len);
    printf("upload file path: %s\n", *file_path_out);

    // refer to the README.md to inform how to deserialize just the upload path
    return file_path_len;
}

/**
 * @brief deserialize the file contents and contents length from upload arguments.
 * @param upload_args the full upload arguments passed in the command structure
 * @param file_path_len length of the already deserialize file path
 * @param contents_out a NULL pointer that will contain the file contents on success or NULL on error
 * @return the length of the file path
 */
static uint32_t deserialize_upload_file_contents(char *upload_args, uint32_t file_path_len, char **contents_out)
{
    uint32_t content_len;
    uint32_t offset = file_path_len + sizeof(uint32_t);
    memcpy(&content_len, upload_args + offset, sizeof(uint32_t));
    content_len = ntohl(content_len);
    offset += 4;
    *contents_out = calloc(content_len, sizeof(char));
    memcpy(*contents_out, upload_args + offset, content_len);
    printf("contents_len: %d\n", content_len);
    printf("contents_out: %s\n", *contents_out);

    // refer to the README.md to inform how to deserialize just the file contents
    return content_len;
}

/**
 * @brief Upload the file specified in the command arguments.
 * @param cmd the command structure
 * @return a response indicating upload success or failure
 */
Response *upload_file_command(Command *cmd)
{
    Response *rsp = NULL;
    uint32_t file_path_len = 0;
    uint32_t content_len = 0;
    char *upload_file_path = NULL;
    char *upload_file_content = NULL;
    uint32_t bytes_written = 0;
    uint32_t ret_code = 0;
    char *upload_msg = "upload successful";
    // Deserialize path and contents
    printf("cmd->args: %s\n", cmd->args);
    file_path_len = deserialize_upload_file_path(cmd->args, &upload_file_path);
    if(0 == file_path_len)
    {
        upload_msg = "error with upload file path";
        ret_code = 1;
        rsp = alloc_response(ret_code, upload_msg, strlen(upload_msg) + 1);
        return rsp;
    }
    content_len = deserialize_upload_file_contents(cmd->args, file_path_len, &upload_file_content);
    if(0 == content_len)
    {
        upload_msg = "error with upload file contents";
        ret_code = 1;
        rsp = alloc_response(ret_code, upload_msg, strlen(upload_msg) + 1);
        return rsp;
    }    

    // Write file
    bytes_written = write_file(upload_file_path, upload_file_content, content_len);
    printf("bytes written: %d\n", bytes_written);
    if(bytes_written != content_len)
    {
        upload_msg = "error with upload file write";
        ret_code = 1;
    }
    rsp = alloc_response(ret_code, upload_msg, strlen(upload_msg) + 1);
    return rsp;
}
