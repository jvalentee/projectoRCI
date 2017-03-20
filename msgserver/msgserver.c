#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user_functions.h"
#include "message_functions.h"

#define max(A,B) ((A)>=(B)?(A):(B))
#define PORT  59000
#define MAXCHAR 1024

int main(int argc, char *argv[])
{
  int n_messages, int_reg, fd_id, addrlen, n, select_ret_val, maxfd, id_socket=0, print_message=0;
  char *server_name, *ip_address, *udp_port, *tcp_port, *id_serverip, *id_serverport;
  char command[MAXCHAR], reg_message[MAXCHAR], buffer[MAXCHAR];
  struct hostent *hostptr;
  struct sockaddr_in serveraddr, clientaddr;
  fd_set rfds;

  //Collects the arguments to the corresponding variable.
  get_arguments(argc, argv, &server_name, &ip_address, &udp_port, &tcp_port, &id_serverip, &id_serverport, &n_messages, &int_reg);

  //Registration message.
  strcpy(reg_message, "REG ");
  strcat(reg_message, server_name);
  strcat(reg_message, ";");
  strcat(reg_message, ip_address);
  strcat(reg_message, ";");
  strcat(reg_message, udp_port);
  strcat(reg_message, ";");
  strcat(reg_message, tcp_port);

  //File descriptor UDP to ID server.
  fd_id = socket(AF_INET, SOCK_DGRAM,0);
  if(fd_id == -1)
  {
    exit(-1);
  }

  if((hostptr = gethostbyname("tejo.tecnico.ulisboa.pt")) == NULL)
   {
     //Error
     exit(1);
   }

  memset((void*)&serveraddr,(int) '\0', sizeof(serveraddr));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = ((struct in_addr *)(hostptr->h_addr_list[0]))->s_addr;
  serveraddr.sin_port = htons((u_short)PORT);

  printf("MSGSERVER\nPor favor, escolha uma das seguintes operações e pressione 'enter':\n1-'join'\n2-'show_servers'\n3-'show_messages'\n4-'exit'\n");


  while(1)
   {
     //Clears sets of watched file descriptors
     FD_ZERO(&rfds);

     //Watch stdin to see when it has input.
     FD_SET(0, &rfds);
     maxfd = 0;

     if(id_socket == 1)
     {
       FD_SET(fd_id, &rfds);
       maxfd = (maxfd, fd_id);
     }

     //Run select and get it's return value
     select_ret_val = select(maxfd+1, &rfds, (fd_set*)NULL, (fd_set*)NULL, (struct timeval *)NULL);

     //Check select return value for errors
     if (select_ret_val == -1)
     {
       printf("Error on select()\n");
       exit(1);
     }
       /*Find out the fd we are treating*/

       //We are treating user input
      if (FD_ISSET(0, &rfds))
      {
        //Reads the user input.
        fgets(command, MAXCHAR, stdin);

        //Identifies the user command.
        n = identify_user_commands(command);

        //Join.
        if(n==1)
        {
          addrlen = sizeof(serveraddr);
          //Send the registration message to the id server.
          sendto(fd_id, reg_message, strlen(reg_message)+1, 0, (struct sockaddr*) &serveraddr, addrlen);

          //Send the "GET_SERVERS" message to the id server.
          sendto(fd_id, "GET_SERVERS", strlen("GET_SERVERS")+1, 0, (struct sockaddr*) &serveraddr, addrlen);

          //It can now receive information from the server.
          id_socket = 1;
          //Blocks printing on the screen.
          print_message = 0;
        }

        if(n==2)
        {
          addrlen = sizeof(serveraddr);
          //Send the "GET_SERVERS" message to the id server.
          sendto(fd_id, "GET_SERVERS", strlen("GET_SERVERS")+1, 0, (struct sockaddr*) &serveraddr, addrlen);
          //It can now receive information from the server.
          id_socket = 1;
          //Prints on the screen.
          print_message = 1;
        }

        if(n==4)
        {
          //Closes the program.
          break;
        }

      }

      if (FD_ISSET(fd_id, &rfds))
      {
        addrlen = sizeof(serveraddr);
        recvfrom(fd_id, buffer, sizeof(buffer), 0, (struct sockaddr*)&serveraddr, &addrlen);

        //Controls printing the list of servers on screen.
        if(print_message == 1)
        {
          printf("%s\n", buffer);
        }
      }
  }

  return 0;

}
