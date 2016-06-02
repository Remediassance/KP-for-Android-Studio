/*

  ANSI C KPI library is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
 
  ANSI C KPI library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with ANSI C KPI library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor,
  Boston, MA  02110-1301  USA

  Copyright (C) 2012 -  SmartSlog Team (Aleksandr A. Lomov). All rights reserved.
  
  This library is based on KPI_Low sources from VTT, see the copyright below.
  New features, functions or modifications are marked with relevant comments, 
  other source codes are written by VTT.
  KPI_Low project is available here: http://sourceforge.net/projects/kpilow/

  
  VTT Copyrights: 
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
 * @file ckpi.c
 *
 * @brief API provides Smart Space discovery and SSAP format transaction operations with
 *  the discovered Smart Space for the KP application.
 *
 * Authors: Jussi Kiljander, VTT Technical Research Centre of Finland, 
 *          SmartSlog Team (Aleksandr A. Lomov - lomov@cs.karelia.ru)
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ckpi.h"
#include "compose_ssap_msg.h"
#include "parse_ssap_msg.h"
#include "process_ssap_cnf.h"
#include "sskp_errno.h"

#ifdef ACCESS_NOTA
#include "sib_access_nota.h"
#else
#include "sib_access_tcp.h"
#endif

#if defined(WIN32) || defined (WINCE)
#include "pthread.h"
#endif

/*
*****************************************************************************
*  EXPORTED FUNCTION IMPLEMENTATIONS
*****************************************************************************
*/

/**
 * \fn int ss_discovery(ss_info_t * first_ss)
 *
 * \brief Discoveres the existing Smart Spaces.
 *
 * This version supports only Hardcoded Smart Space discovery.
 *
 * \param[in] ss_info_t * first_ss. Pointer to the ss info struct.
 * \return int. Currently returns always 1.
 */
EXTERN int ss_discovery(ss_info_t * first_ss)
{
  //ss_info_t ss = {0, {0}, SS_SPACE_ID, 0, 0, {SS_ADDRESS, SS_PORT_N}, {0}};
  first_ss->free = 0;
  memset(first_ss->node_id, 0, SS_NODE_ID_MAX_LEN);
  strcpy(first_ss->space_id,SS_SPACE_ID);
  first_ss->transaction_id = 0;
  first_ss->socket = 0;
#if defined(ACCESS_NOTA)
  strcpy(first_ss->address.sid, SS_ADDRESS);
#else
  strcpy(first_ss->address.ip, SS_ADDRESS);
#endif
  first_ss->address.port = SS_PORT_N;
  memset(first_ss->ssap_msg, 0, SS_MAX_MESSAGE_SIZE);
  first_ss->ss_errno = 0;

  //*first_ss = ss;
  return 1;
}

/**
 * \fn int ss_join(ss_info_t * ss_info, char * node_id)
 *
 * \brief Executes the SSAP format join operation.
 *
 * \param[in]  ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and space_id information.
 * \param[in]  char * node_id. ID of your node.

 * \return int status. Status of the operation when completed (0 if successfull,
 *                     otherwise - 1).
 */
EXTERN int ss_join(ss_info_t * ss_info, char * node_id)
{
  ssap_msg_t msg_i;
  int socket,status;

  strcpy(ss_info->node_id, node_id);
  ss_info->transaction_id = 1;
  make_join_msg(ss_info);

  if((socket = ss_open(&(ss_info->address))) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_OPEN;
    return -1;
  }
  ss_info->socket = socket;

  if(ss_send(socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }
  if((status = ss_recv(socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;

    ss_close(socket);
    return -1;
  
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("JOIN", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_join_response(ss_info, &msg_i);
}

/**
 * \fn int ss_leave(ss_info_t * ss_info)
 *
 * \brief Executes the SSAP format leave operation.
 *
 * \param[in]  ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *             space_id information.
 *
 * \return int status. Status of the operation when completed (0 if successfull,
 *                     otherwise -1).
 */
EXTERN int ss_leave(ss_info_t * ss_info)
{
  ssap_msg_t msg_i;
  int status;

  ss_info->transaction_id++;
  make_leave_msg(ss_info);

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }
  if((status = ss_recv(ss_info->socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    ss_close(ss_info->socket);
    return -1;
  }

  if(ss_close(ss_info->socket) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_CLOSE;
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("LEAVE", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_leave_response(ss_info, &msg_i);
}

/**
 * \fn int ss_query(ss_info_t * ss_info, ss_triple_t * requested_triples, ss_triple_t ** returned_triples)
 *
 * \brief  Executes the SSAP format query operation.
 *
 *  Function composes and send SSAP query messsage to the SIB, whose address information is
 *  found in the ss_info struct. Function also extracts the RDF triples from the query
 *  response. Function returns pointer to the first triple and the triples form a linked
 *  list that can be traversed.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_triple_t * requested_triples. Pointer to the first triple requested from the SIB.
 * \param[out] ss_triple_t * returned_triples. Pointer to the first triple returned by the SIB.
 *
 * \return int status. Status of the operation when completed (0 if successfull, otherwise -1).
 */
EXTERN int ss_query(ss_info_t * ss_info, ss_triple_t * requested_triples, ss_triple_t ** returned_triples)
{
  ssap_msg_t msg_i;
  int status;

  ss_info->transaction_id++;
  make_query_msg(ss_info, requested_triples);

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }

  if((status = ss_recv(ss_info->socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("QUERY", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_query_response(ss_info, &msg_i, returned_triples);
}
 
/**
 * \fn int ss_insert(ss_info_t * ss_info, ss_triple_t * first_triple, ss_bnode_t * bnodes)
 *
 * \brief Executes the SSAP format insert operation.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_triple_t * first_triple. Pointer to the first triple in the triple list to be inserted.
 * \param[out] ss_bnode_t * bnodes. Pointer to the bnode struct(s), where bnode label and URI are copied.
 * \return int status. Status of the operation when completed (0 if successfull,
 *         otherwise < 0).
 */
EXTERN int ss_insert(ss_info_t * ss_info, ss_triple_t * first_triple, ss_bnode_t * bnodes)
{
  ssap_msg_t msg_i;
  int status = 0;

  ss_info->transaction_id++;
  make_insert_msg(ss_info, first_triple);

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }
  if((status = ss_recv(ss_info->socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("INSERT", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_insert_response(ss_info, &msg_i, bnodes);
}

/**
 * \fn int ss_update(ss_info_t * ss_info, ss_triple_t * inserted_triples, ss_triple_t * removed_triples, ss_bnode_t * bnodes)
 *
 * \brief Executes the SSAP format update operation.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_triple_t * inserted_triples. Pointer to the first triple in the triple list to be inserted.
 * \param[in] ss_triple_t * removed_triples. Pointer to the first triple in the triple list to be removed.
 * \param[out] ss_bnode_t * bnodes. Pointer to the bnode struct(s), where bnode label and URI are copied.
 * \return int status. Status of the operation when completed (0 if successfull,
 *         otherwise -1).
 */
EXTERN int ss_update(ss_info_t * ss_info, ss_triple_t * inserted_triples, ss_triple_t * removed_triples, ss_bnode_t * bnodes)
{
  ssap_msg_t msg_i;
  int status;

  ss_info->transaction_id++;
  make_update_msg(ss_info, inserted_triples, removed_triples);

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }

  if((status = ss_recv(ss_info->socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("UPDATE", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_update_response(ss_info, &msg_i, bnodes);
}

/**
 * \fn int ss_remove(ss_info_t * ss_info, ss_triple_t * removed_triples)
 *
 * \brief Executes the SSAP format remove operation.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_triple_t * removed_triples. Pointer to the first triple in the triple list to be removed.
 * \return int status. Status of the operation when completed (0 if successfull,
 *         otherwise -1).
 */
EXTERN int ss_remove(ss_info_t * ss_info, ss_triple_t * removed_triples)
{
  ssap_msg_t msg_i;
  int status;

  ss_info->transaction_id++;
  make_remove_msg(ss_info, removed_triples);

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }

  if((status = ss_recv(ss_info->socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("REMOVE", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  return handle_remove_response(ss_info, &msg_i);
}

/**
 * \fn int ss_subscribe(ss_info_t * ss_info, ss_subs_info_t * subs_info, ss_triple_t * requested_triples, ss_triple_t ** returned_triples)
 *
 * \brief  Executes the SSAP format subsrcibe operation.
 *
 *  Function composes and send SSAP subsrcibe messsage to the SIB, whose address
 *  information is found in the ss_info struct. Function returns the requested information
 *  in triple format. Subscribe / unsubscribe indications can be later checked using ss_subscribe_indication function.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_subs_info_t * subs_info. A pointer to subscribe info struct.
 * \param[in] ss_triple_t * requested_triples. Pointer to the first triple requested from the SIB.
 * \param[out] ss_triple_t ** returned_triples. Pointer to the first triple returned by the SIB.
 *
 * \return int status. Status of the operation when completed (0 if successfull,
 *         otherwise -1).
 */
EXTERN int ss_subscribe(ss_info_t * ss_info, ss_subs_info_t * subs_info, ss_triple_t * requested_triples, ss_triple_t ** returned_triples)
{
  ssap_msg_t msg_i;
  int socket, status;

  ss_info->transaction_id++;
  make_subscribe_msg(ss_info, requested_triples);

  if((socket = ss_open(&(ss_info->address))) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_OPEN;
    return -1;
  }

  if(ss_send(socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }
  if((status = ss_recv(socket, ss_info->ssap_msg, SS_RECV_TIMEOUT_MSECS)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;
    else
      ss_info->ss_errno = SS_ERROR_RECV_TIMEOUT;
    ss_close(socket);
    return -1;
  }

  if(parse_ssap_msg(ss_info->ssap_msg, strlen(ss_info->ssap_msg), &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
    return -1;
  }

  if(strcmp("SUBSCRIBE", msg_i.transaction_type) != 0)
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    return -1;
  }

  if((handle_subscribe_response(ss_info, &msg_i, subs_info, returned_triples)) < 0)
    return -1;

  subs_info -> socket = socket;

  return 0;
}

/**
 * \fn int ss_subscribe_indication(ss_info_t * ss_info, ss_subs_info_t * subs_info, ss_triple_t ** new_triples, ss_triple_t ** obsolete_triples, int to_msecs)
 *
 * \brief  Function checks if the subscribe or unsubscribe indication has been received.
 *
 *  This function is used to check whether subscribe or unsubscribe indication has been received. Function waits on the select until a message
 *  is received to the socket or timeout occurs. The timeout value can be given as a parameter. If indication has been received function handles the indication.
 *  The results are passed to user via two pointers called new_triples and obsolete_triples. The memory for these triples is reserved dynamically
 *  and must be freed using ss_delete_triples function.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_subs_info_t * subs_info.
 * \param[out] ss_triple_t ** new_results. Pointer to the first triple of the new result list.
 * \param[out] ss_triple_t ** obsolete_results. Pointer to the first triple of the obsolete result list.
 * \param[in] int to_msecs. Timeout value in milliseconds.
 *
 * \return int status. Unsubscribe: 2
 *                     Indication:  1
 *                     Timeout:     0
 *                     Error:      -1
 */

EXTERN int ss_subscribe_indication(ss_info_t * ss_info, ss_subs_info_t * subs_info, ss_triple_t ** new_triples, ss_triple_t ** obsolete_triples, int to_msecs)
{
  ssap_msg_t msg_i;
  int status = -1;
  int offset = 0;
  multi_msg_t * m = NULL;
  multi_msg_t * m_prev = NULL;

  *new_triples = NULL; 
  *obsolete_triples = NULL;
  subs_info->fmsg = NULL;

  if((status = ss_mrecv(&subs_info->fmsg, subs_info->socket, ss_info->ssap_msg, to_msecs)) <= 0)
  {
    if(status < 0)
      ss_info->ss_errno = SS_ERROR_SOCKET_RECV;

    return status;
  }

  for(m = subs_info->fmsg; m; m = m->next)
  {
    if(parse_ssap_msg(&(ss_info->ssap_msg[offset]), m->size, &msg_i) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SSAP_MSG_FORMAT;
      status = -1;
      break;
  }

    offset += m->size;

  if(strcmp("SUBSCRIBE", msg_i.transaction_type) == 0)
  {
    if(handle_subscribe_indication(ss_info, &msg_i, subs_info, new_triples, obsolete_triples) == 0)
      status = 1;
    else
      status = -1;
  }
  else if(strcmp("UNSUBSCRIBE", msg_i.transaction_type) == 0)
  {
    if(handle_unsubscribe_response(ss_info, &msg_i, subs_info) == 0)
    {
      if(ss_close(subs_info->socket) < 0)
      {
        ss_info->ss_errno = SS_ERROR_SOCKET_CLOSE;
          status = -1;
          break;
      }
      
      /*  SmartSlog Team: reset subscription info. */
        subs_info->id[0] = '\0';
        subs_info->socket = -1; 
      
      status = 2;
        break;
    }
    else
      {
      status = -1;
        break;
      }
  }
  else
  {
    ss_info->ss_errno = SS_ERROR_TRANSACTION_TYPE;
    status = -1;
      break;
    }
  }

  m = subs_info->fmsg;
  while(m)
  {
     m_prev = m;
     m = m->next;
     free(m_prev);
  }

  return status;
}

/**
 * \fn int ss_unsubscribe(ss_info_t * ss_info, ss_subs_info_t * subs_info)
 *
 * \brief  Terminates the SSAP format subsrcibe operation.
 *
 * \param[in] ss_info_t * ss_info. A pointer to the struct holding neccessary node_id and
 *            space_id information.
 * \param[in] ss_subs_info_t * subs_info. Subscribe to be terminated.
 *
 * \return int status. Status of the operation when completed (0 if successfull,
 *                     otherwise -1).
 */
EXTERN int ss_unsubscribe(ss_info_t * ss_info, ss_subs_info_t * subs_info)
{
  //int socket;

  ss_info->transaction_id++;
  make_unsubscribe_msg(ss_info, subs_info->id);

  /* SIB sends response to the subscribe message to the socket where subscribe
     message was sent. */
  /*
  if((socket = ss_open(&(ss_info->address))) < 0)
  {
     ss_info->ss_errno = SS_ERROR_SOCKET_OPEN;
     return -1;
  }
  */

  if(ss_send(ss_info->socket, ss_info->ssap_msg) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_SEND;
    return -1;
  }
  /*
  if(ss_close(socket) < 0)
  {
    ss_info->ss_errno = SS_ERROR_SOCKET_CLOSE;
    return -1;
  }
  */
  return 0;
}


/**
 * \fn int ss_add_triple(ss_triple_t ** first_triple, char * subject, char * predicate, char * object, char * rdf_subject_type, char * rdf_object_type)
 *
 * \brief Adds new triple to the triple list pointed by first_triple.
 *
 * Function adds the new triple to the top of the triple list pointed by first_triple. The memory
 * is reserved dynamically and must be freed using ss_delete_triples function. Make sure that the first_triple
 * is not pointing to anything (points to NULL) when there is no previous triples.
 *
 * \param[in/out]  ss_triple_t ** first_triple. A pointer to the first ss_triple
 *                 struct pointer.
 * \param[in] char * subject.
 * \param[in] char * predicate.
 * \param[in] char * object.
 * \param[in] char * rdf_subject_type. Possible values are macros RDF_TYPE_URI and
 *            RDF_TYPE_BNODE.
 * \param[in] char * rdf_object_type. Possible values are macros RDF_TYPE_URI,
 *            RDF_TYPE_BNODE and RDF_TYPE_LIT.
 *
 * \return int. 0 when successfull, otherwise -1.
 */
EXTERN int ss_add_triple(ss_triple_t ** first_triple, char * subject, char * predicate, char * object, int rdf_subject_type, int rdf_object_type)
{
  ss_triple_t * triple_new = NULL;


  triple_new = (ss_triple_t *)malloc(sizeof(ss_triple_t));

  if(!triple_new)
  {
    SS_DEBUG_PRINT(("ERROR: unable to reserve memory for ss_triple_t\n"));
    return -1;
  }

  strcpy(triple_new->subject, subject);
  strcpy(triple_new->predicate, predicate);
  strcpy(triple_new->object, object);
  triple_new->subject_type = rdf_subject_type;
  triple_new->object_type = rdf_object_type;

  /* Add the new triple to the top */
  triple_new -> next = *first_triple;
  *first_triple = triple_new;

  return 0;
}

/**
 * \fn void ss_delete_triples(ss_triple_t * first_triple)
 *
 * \brief Deletes all ss_triple struct nodes.
 *
 * \param[in] ss_triple_t * first_triple. A pointer to the first ss_triple.
 */
EXTERN void ss_delete_triples(ss_triple_t * first_triple)
{
  ss_triple_t * triple_current = first_triple;
  ss_triple_t * triple_next = NULL;

  while(triple_current)
    {
      triple_next = triple_current->next;
      free(triple_current);
      triple_current = triple_next;
    }

}

/*** Functions from the SmartSlog team ***/


/**
 * \fn void init_ss_info()
 *
 * \brief Initialize information about the smart space with given information.
 *
 * \param[in] ss_info_t *info. Structure for initializing.
 * \param[in] const char *ss_id. SmartSpace identifier.
 * \param[in] const char *ss_address. Address of the SmartSpace.
 * \param[in] int ss_port. Port of the SmartSpace.
 *
 */
EXTERN void ss_init_space_info(ss_info_t *info, 
            const char *ss_id, const char *ss_address, int ss_port)
{
    if (info == NULL) {
        return;
    }
 
    info->free = 0;
    info->transaction_id = 0;
    info->socket = 0;
    info->node_id[0] = '\0';
    info->ssap_msg[0] = '\0';
    info->ss_errno = 0;
    
    strncpy(info->space_id, ss_id, SS_SPACE_ID_MAX_LEN);
    strncpy(info->address.ip, ss_address, MAX_IP_LEN);
    info->address.port = ss_port;
} 





/**
 * \fn int ss_close_subscription()
 *
 * \brief Closes the subscription.  Use this function to force close subscription.  
 *
 * \param[in] iss_subs_info_t *subs_info. Subscribe to be closed.
 *
 * \return int. 0 if successful or if the subscription is not active, otherwise -1 
 * (error while closing or if the given subscription is null).
 */
EXTERN int ss_close_subscription(ss_subs_info_t *subs_info)
{
    if (subs_info == NULL) {
        return -1;
    }

    int status =  ss_close(subs_info->socket);
    
    subs_info->id[0] = '\0';
    subs_info->socket = -1;

    return status;
}


#if defined(WIN32) || defined (WINCE)
#if defined(ACCESS_NOTA)
EXTERN void init()
{
  pthread_win32_process_attach_np();
}
#endif
#endif
