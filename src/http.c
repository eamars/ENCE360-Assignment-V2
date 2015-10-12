#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "http.h"

#define BUF_SIZE 1024
#define DL_SIZE 1024

// HTTP request example
const char *HTTP_REQ_GET =
"GET %s HTTP/1.0\r\n"
"Host: %s\r\n"
"Connection: close\r\n"
"User-Agent: ENCE360-HTTP-AGENT\r\n"
"Accept: */*\r\n"
"\r\n";

int client_socket(char *hostname, int port_num)
{
  char port[6];
  struct addrinfo their_addrinfo; // server address info
  struct addrinfo *their_addr = NULL; // connector's address information

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&their_addrinfo, 0, sizeof(struct addrinfo));
  their_addrinfo.ai_family = AF_INET;        /* use an internet address */
  their_addrinfo.ai_socktype = SOCK_STREAM;  /* use TCP rather than datagram */

  sprintf(port, "%d", port_num);
  getaddrinfo( hostname, port, &their_addrinfo, &their_addr);/* get IP info */

  int rc = connect(sockfd, their_addr->ai_addr, their_addr->ai_addrlen); //connect to server

  // free allocated memory
  free(their_addr);


  if(rc == -1) {
    perror("connect");
    exit(1);
  }

  return sockfd;
}

Buffer* http_query(char *host, char *page, int port) {

  int client_fd;
  Buffer *buffer;
  char *request;
  char strbuf[DL_SIZE];
  int sz;

  // prepare http request header
  request = (char *) malloc(strlen(HTTP_REQ_GET) + strlen(page) + strlen(host) + 1);
  sprintf(request, HTTP_REQ_GET, page, host);

  // open connection to remote server
  client_fd = client_socket(host, port);

  // send http request to remote server
  if ((sz = write(client_fd, request, strlen(request))) < 0)
  {
      perror("write");
      fprintf(stderr, "Error: Failed to send http request to http://%s%s\n", host, page);
      exit(2);
  }

  // free http request
  free(request);

  // receive http response
  buffer = (Buffer *) calloc(1, sizeof(Buffer)); // allocate memory and fill with zeros
  memset(strbuf, 0, DL_SIZE);

  // read from socket
  while ((sz = read(client_fd, strbuf, DL_SIZE)) != 0)
  {
      if (sz < 0)
      {
          perror("read");
          fprintf(stderr, "Error: Failed to fetch http response from http://%s%s\n", host, page);
          exit(1);
      }
      else
      {
          // use realloc to increase size for variable length data
          // why use length+sz+1? because valgrind will complain about there is no space left for buffer->data
          buffer->data = realloc(buffer->data, buffer->length + sz + 1);

          // copy downloaded data from buffer to data with actual download size
          memcpy(buffer->data + buffer->length, strbuf, sz);

          // increase the recorded data size
          buffer->length += sz;
      }
      memset(strbuf, 0, DL_SIZE);
  }

  close(client_fd);

  // If the server return nothing, we will not create the object
  if (buffer->length == 0)
  {
      free(buffer);
      return NULL;
  }

  return buffer;

}

// split http content from the response string
char* http_get_content(Buffer *response) {

  char* header_end = strstr(response->data, "\r\n\r\n");

  if(header_end) {
    return header_end + 4;
  } else {
   return response->data;
  }
}


Buffer *http_url(const char *url) {
  char host[BUF_SIZE];
  char page_slash[BUF_SIZE];
  strncpy(host, url, BUF_SIZE);

  char *page = strstr(host, "/");
  if(page) {
    page[0] = '\0';

    ++page;
    sprintf(page_slash, "/%s", page);
    return http_query(host, page_slash, 80);
  } else {

    fprintf(stderr, "could not split url into host/page %s\n", url);
    return NULL;
  }
}
