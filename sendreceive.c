/*
Matthew Pham
mnp190003
Project 3 dbclient.c
This client sends messages to the server to perform various commands
such as put, get, and delete. Then returns the success or failure of the command.
*/
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdint.h>    // for int32_t
#include <inttypes.h>  // for SCNd32, PRId32
#include <fcntl.h>     // for open, O_RDONLY
#include "msg.h"

#define BUF 256

void Usage(char *progname);

int LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen);

int Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd);
             
void clear_newlines(); // clears new lines after scanf calls


int 
main(int argc, char **argv) {

    if (argc != 3) {
        Usage(argv[0]);
    }
  
    unsigned short port = 0;
    if (sscanf(argv[2], "%hu", &port) != 1) {
        Usage(argv[0]);
    }
    
    // Get an appropriate sockaddr structure.
    struct sockaddr_storage addr;
    size_t addrlen;
    if (!LookupName(argv[1], port, &addr, &addrlen)) {
        Usage(argv[0]);
    }
  
    // Connect to the remote host.
    int socket_fd;
    if (!Connect(&addr, addrlen, &socket_fd)) {
        Usage(argv[0]);
    }
    
    // Sets a while loop of options that run until the user chooses to 0(quit)
    int type;
    while (1) {
        printf("Enter your choice (1 to put, 2 to get, 3 to delete, 0 to quit): ");
        
        scanf("%d", &type);
        clear_newlines();
        if(type > 3 || type < 0){
            printf("invalid option");
            break;
        }
    
        struct msg message;
        message.type = type;
        
        // depending on choice, the program will create a msg struct 
        // to send information to the server on what to do
        // put, get, or delete and will also prompt the user 
        // for the appropriate information
        switch(type){
          case 1:
              printf("Enter the student name: ");	
              fgets(message.rd.name, MAX_NAME_LENGTH, stdin);
              printf("Enter the student id: ");
              scanf("%"SCNd32 "%*c", &(message.rd.id));
              clear_newlines();
              break;
          case 2:
              printf("Enter the student id: ");
              scanf("%"SCNd32 "%*c", &(message.rd.id));
              clear_newlines();
              break;
          case 3:
              printf("Enter the student id: ");
              scanf("%"SCNd32 "%*c", &(message.rd.id));
              clear_newlines();
              break;
          case 0:
              close(socket_fd);
              return EXIT_SUCCESS;
        }
        struct msg received_msg;
        int mres;
        // The client will send the msg struct to the server
        while(1){
            int wres = write(socket_fd, &message, sizeof(message));
          
            mres = read(socket_fd, &received_msg, sizeof(received_msg));
            if (wres == 0) {
                 printf("socket closed prematurely \n");
                 close(socket_fd);
                 return EXIT_FAILURE;
            }
            if (wres == -1) {
                if (errno == EINTR)
                    continue;
            printf("socket write failure \n");
            close(socket_fd);
            return EXIT_FAILURE;
            }
            
            // After sending to the server the client will then 
        // receive the results of the sent msg from the server
        //int res;
        
            //res = read(socket_fd, &received_msg, sizeof(received_msg));
            if (mres == 0) {
                printf("socket closed prematurely \n");
                close(socket_fd);
                return EXIT_FAILURE;
            }
            if (mres == -1) {
                if (errno == EINTR)
                    continue;
                printf("socket read failure \n");
                close(socket_fd);
                return EXIT_FAILURE;
            }
            
            // depending on the type of command sent and the 
            // success of the command the client will provide
            // information about the results of the sent command
            switch(type){
              case 1:
                  if(received_msg.type == SUCCESS)
                      printf("put success\n");  
                  else
                      printf("put failed\n");
                  break;
              case 2:
                  if(received_msg.type == SUCCESS){
                      printf("name: %s\n", received_msg.rd.name);
                      printf("id: %lu\n", (unsigned long)received_msg.rd.id);  
                  }
                  else
                      printf("get failed\n");
                  break;
              case 3:
                  if(received_msg.type == SUCCESS){
                      printf("name: %s\n", received_msg.rd.name);
                      printf("id: %lu\n", (unsigned long)received_msg.rd.id); 
                  } 
                  else
                      printf("delete failed\n");
                  break;
              case 0:
                  close(socket_fd);
                  return EXIT_SUCCESS;
            }
            break;
        }
        
        
  
      }
   
  // Clean up.
  close(socket_fd);
  return EXIT_SUCCESS;
}

void 
Usage(char *progname) {
    printf("usage: %s  hostname port \n", progname);
    exit(EXIT_FAILURE);
}

int 
LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen) {
  struct addrinfo hints, *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  // Do the lookup by invoking getaddrinfo().
  if ((retval = getaddrinfo(name, NULL, &hints, &results)) != 0) {
    printf( "getaddrinfo failed: %s", gai_strerror(retval));
    return 0;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr =
            (struct sockaddr_in *) (results->ai_addr);
    v4addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr =
            (struct sockaddr_in6 *)(results->ai_addr);
    v6addr->sin6_port = htons(port);
  } else {
    printf("getaddrinfo failed to provide an IPv4 or IPv6 address \n");
    freeaddrinfo(results);
    return 0;
  }

  // Return the first result.
  assert(results != NULL);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return 1;
}

int 
Connect(const struct sockaddr_storage *addr,
             const size_t addrlen,
             int *ret_fd) {
  // Create the socket.
  int socket_fd = socket(addr->ss_family, SOCK_STREAM, 0);
  if (socket_fd == -1) {
    printf("socket() failed: %s", strerror(errno));
    return 0;
  }

  // Connect the socket to the remote host.
  int res = connect(socket_fd,
                    (const struct sockaddr *)(addr),
                    addrlen);
  if (res == -1) {
    printf("connect() failed: %s", strerror(errno));
    return 0;
  }

  *ret_fd = socket_fd;
  return 1;
}

// clears new lines for use after scanf call
void clear_newlines()
{
    int c;
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}
