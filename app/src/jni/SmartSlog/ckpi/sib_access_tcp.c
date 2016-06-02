/*

  Copyright (c) 2009, VTT Technical Research Center of Finland
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.
  * Neither the name of the VTT Technical Research Center of Finland nor the
  names of its contributors may be used to endorse or promote products
  derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS ''AS IS'' AND ANY
  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

/**
 * \fn sib_access_tcp.c
 *
 * \brief Implementations  of function for sending/reveiving the SSAP transaction messages
 *        using TCP/IP.
 *
 * Author: Jussi Kiljander, VTT Technical Research Centre of Finland
 */


#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h> /* close() */
#include <stdlib.h>
#include <sys/time.h>

#include "ckpi.h"
#include "sib_access_tcp.h"


/*
*****************************************************************************
*  LOCAL FUNCTION PROTOTYPES
*****************************************************************************
*/

/**
 * \fn timeout_recv()
 *
 * \brief recv() with timeout implemented using select().
 *
 * \param[in] int socket. File descriptor of the socket to receive from.
 * \param[in/out] char * recv_buf. Received data is copied to this buffer.
 * \param[in] int len. Length of the recv_buf.
 * \param[in] int to_msecs. Timeout value in milliseconds.
 */
static int timeout_recv(int socket, char * recv_buf, int len, int to_msecs);

/*
*****************************************************************************
*  EXPORTED FUNCTION IMPLEMENTATIONS
*****************************************************************************
*/

/**
 * \fn int ss_open()
 *
 * \brief Creates a socket and connects socket to the IP and port specified in
 *        sib_address_t struct.
 *
 * \param[in] sib_address_t *tcpip_i. A pointer to the struct holding neccessary address
 *            information.
 *
 * \return int. If successfull returns the socket descriptor, otherwise -1.
 */
int ss_open(sib_address_t *tcpip_i)
{
  int sockfd;
  struct sockaddr_in sib_addr;

  sockfd = socket(PF_INET, SOCK_STREAM, 0);

  if(sockfd < 0)
  {
    SS_DEBUG_PRINT("ERROR: unable to create socket\n");
    return -1;
  }

  sib_addr.sin_family = AF_INET;
  sib_addr.sin_port = htons(tcpip_i->port);
  sib_addr.sin_addr.s_addr = inet_addr(tcpip_i->ip);

  memset(sib_addr.sin_zero, '\0', sizeof sib_addr.sin_zero);

  if(connect( sockfd, (struct sockaddr *)&sib_addr, sizeof(sib_addr)) < 0)
  {
    close(sockfd);
    SS_DEBUG_PRINT("ERROR: unable to connect socket\n");
    return -1;
  }

  return sockfd;
}

/**
 * \fn int ss_send()
 *
 * \brief Sends data to the Smart Space (SIB).
 *
 * \param[in] int socket. File descriptor of the socket to send.
 * \param[in] char * send_buf. A pointer to the data to be send.
 *
 * \return int. 0 if successful, otherwise -1.
 */
int ss_send(int socket, char * send_buf)
{

 int bytes_sent = 0;
 int bytes_left = strlen(send_buf);
 int bytes;

 while(bytes_left > 0)
 {
   bytes = send(socket, send_buf + bytes_sent, bytes_left, 0);
   if(bytes < 0)
     return -1;

   bytes_sent += bytes;
   bytes_left -= bytes;
 }

 return 0;  
}

/**
 * \fn int ss_recv()
 *
 * \brief Receives data to the Smart Space (SIB).
 *
 * \param[in] int socket. The socket descriptor of the socket where data is received from.
 * \param[in] char * recv_buf. A pointer to the data to be received.
 * \param[in] int to_msecs. Timeout value in milliseconds.
 *
 * \return int. Success: 1
 *              Timeout: 0
 *              ERROR:  -1
 */
int ss_recv(int socket, char * recv_buf, int to_msecs)
{
  int offset = 0;
  int bytes = 0;
  char * msg_end = NULL;
  int len = SS_MAX_MESSAGE_SIZE - 1;

  do{
    bytes = timeout_recv(socket, recv_buf + offset, len, to_msecs);
    if(bytes <= 0)
      return bytes;

    offset += bytes;
    len -= bytes;

    msg_end = strstr(recv_buf, SS_END_TAG);

  }while(!msg_end && len > 1);


  return bytes;
}

/**
 * \fn int ss_mrecv()
 *
 * \brief Receives (possibly) multiple SSAP messages from the Smart Space (SIB).
 *
 * \param[in] multi_msg_t * m. Pointer to the multi_msg_t struct.
 * \param[in] int socket. The socket descriptor of the socket where data is received from.
 * \param[in] char * recv_buf. A pointer to the data to be received.
 * \param[in] int to_msecs. Timeout value in milliseconds.
 *
 * \return int. Success: 1
 *              Timeout: 0
 *              ERROR:  -1
 */
int ss_mrecv(multi_msg_t ** mfirst, int socket, char * recv_buf, int to_msecs)
{
  int offset = 0;
  int bytes = 0;
  int len = SS_MAX_MESSAGE_SIZE - 1;
  char * msg_begin = recv_buf;
  char * msg_end = NULL;
  multi_msg_t * m = NULL;
  multi_msg_t * m_prev = NULL;
  
  while(1)
  {
    bytes = timeout_recv(socket, recv_buf + offset, len, to_msecs);
    if(bytes <= 0)
      return bytes;

    offset += bytes;
    len -= bytes;

    while(1)
    {
      msg_end = strstr(msg_begin, SS_END_TAG);

      if(msg_end == NULL)
        break;
      else
      {
        m = (multi_msg_t *)malloc(sizeof(multi_msg_t));

        if(!m)
        {
          SS_DEBUG_PRINT("ERROR: malloc()\n");
          return -1;
        }
        m->size = msg_end + strlen(SS_END_TAG) - msg_begin;
        m->next = NULL;
        
        if(*mfirst == NULL)
          *mfirst = m;
        else
        {
          for(m_prev = *mfirst; m_prev; m_prev = m_prev->next)
            if(m_prev->next == NULL) break;

          m_prev->next = m;
        }

        if(offset == (msg_end + strlen(SS_END_TAG) - recv_buf)) /* whole message received */
          return bytes;

        /* move to new message */
        msg_begin = msg_end + strlen(SS_END_TAG);

      } 
    }

    if(len <= 1)
      return -1;

  }

  return bytes;
}

/**
 * \fn ss_close()
 *
 * \brief Closes the socket.
 *
 * \param[in] int socket. File descriptor of the socket to be closed
 *
 * \return int. 0 if successful, otherwise -1.
 */
int ss_close(int socket)
{
  return close(socket);
}

/*
*****************************************************************************
*  LOCAL FUNCTION IMPLEMENTATIONS
*****************************************************************************
*/

/**
 * \fn timeout_recv()
 *
 * \brief recv() with timeout implemented using select().
 *
 * \param[in] int socket. File descriptor of the socket to receive from.
 * \param[in/out] char * recv_buf. Received data is copied to this buffer.
 * \param[in] int len. Length of the recv_buf.
 * \param[in] int to_msecs. Timeout value in milliseconds.
 */
static int timeout_recv(int socket, char * recv_buf, int len, int to_msecs)
{
  int recv_bytes;
  int err;
  struct timeval tv;
  fd_set readfds;

  tv.tv_sec = (int)(to_msecs / 1000);
  tv.tv_usec = (int)(((to_msecs / 1000) - tv.tv_sec) * 1000000);
  FD_ZERO(&readfds);
  FD_SET(socket, &readfds);

  err = select(socket + 1, &readfds, NULL, NULL, &tv);

  if(err < 0)
  {
    SS_DEBUG_PRINT("ERROR: select()\n");
    return -1;
  }
  if(err == 0)
    return 0;
  if(FD_ISSET(socket, &readfds))
  {
    recv_bytes = recv(socket, recv_buf, len-1, 0);

    if(recv_bytes < 0)
      return -1;

    recv_buf[recv_bytes] = 0;
  }

  return recv_bytes;
}
