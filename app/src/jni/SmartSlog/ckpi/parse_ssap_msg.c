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
 * parse_ssap_msg.c
 *
 * \brief API for parsing the SSAP transaction messages using scew and expat.
 *
 * Author: Jussi Kiljander, VTT Technical Research Centre of Finland
 *         Matti Eteläperä, VTT Technical Research Centre of Finland
 */

#include <scew/scew.h>
#include "ckpi.h"
#include "parse_ssap_msg.h"
#include "ssap_msg_common.h"

bnode_tmp_t * parse_bnodes(scew_element * uri_list);
ss_triple_t * parse_rdf_triples(scew_element *triple_list);


int parse_ssap_msg(char *xml, int len, ssap_msg_t *msg)
{
  scew_parser* parser = NULL;
  scew_tree* tree = NULL;
  const XML_Char* name = NULL;
  const XML_Char* value = NULL;
  scew_element *root = NULL;
  scew_element *tag = NULL;
  scew_element *triple_list = NULL;
  scew_element *uri_list = NULL;
  scew_attribute *attribute = NULL;
  scew_reader *reader = NULL;

  /* Init results */
  msg->n_result = NULL;
  msg->o_result = NULL;

  parser = scew_parser_create();
  scew_parser_ignore_whitespaces(parser, 1);

  reader = scew_reader_buffer_create(xml, len);
  tree = scew_parser_load(parser,reader);



  if(tree == NULL)
  {
    scew_error code = scew_error_code();

    #if DEBUG
      printf("ERROR: Unable to load buffer (error #%d: %s)\n", code,scew_error_string(code));
    #endif

    if(code == scew_error_expat)
    {
      #if DEBUG
        enum XML_Error expat_code = scew_error_expat_code(parser);
        printf("ERROR: Expat error #%d (line %d, column %d): %s\n"
               , expat_code
               , scew_error_expat_line(parser)
               , scew_error_expat_column(parser)
               , scew_error_expat_string(expat_code));
      #endif
    }
    return -1;
  }

  root = scew_tree_root(tree);
  name= scew_element_name(root);

  if(name==NULL || strcmp("SSAP_message",name)!= 0)
  {
    SS_DEBUG_PRINT("ERROR: Not SSAP message\n");
    return -1;
  }

  scew_list *list = scew_element_children (root);

  while(list != NULL)
  {
    tag = scew_list_data (list);
    list = scew_list_next(list);

    name=scew_element_name(tag);

    if(strcmp(name,"message_type")==0)
    {
      value = scew_element_contents(tag);
      if(value==NULL)
      {
        SS_DEBUG_PRINT("No message type\n");
        return -1;
      }
        strncpy(msg->message_type,value, MAX_MSG_TYPE_LEN);
        continue;
    }
    if(strcmp(name,"transaction_type")==0)
    {
      value = scew_element_contents(tag);
      if(value==NULL)
      {
        SS_DEBUG_PRINT("ERROR: No transaction type\n");
        return -1;
      }
      strncpy(msg->transaction_type, value, MAX_ACTION_TYPE_LEN);
      continue;
    }
    if(strcmp(name,"transaction_id")==0)
    {
      value = scew_element_contents(tag);
      if(value==NULL)
      {
        SS_DEBUG_PRINT("ERROR: No transaction ID\n");
        return -1;
      }
      strncpy(msg->transaction_id, value, MAX_ACTION_ID_LEN);

      continue;
    }
    if(strcmp(name,"node_id")==0)
    {
      value = scew_element_contents(tag);
      if(value==NULL)
      {
        SS_DEBUG_PRINT("ERROR: No node ID\n");
        return -1;
      }
      strncpy(msg->node_id,value,SS_NODE_ID_MAX_LEN);
      continue;
    }
    if(strcmp(name,"space_id")==0)
    {
      value = scew_element_contents(tag);
      if(value==NULL)
      {
        SS_DEBUG_PRINT("ERROR: No space ID\n");
        return -1;
      }
      strncpy(msg->space_id,value,SS_SPACE_ID_MAX_LEN);
      continue;
    }
    if(strcmp(name,"parameter")==0)
    {
      /* Attributes are not used. What other attributes there are? */
      /* possible parameters should be defined as macros */

      scew_list *att_list = scew_element_attributes (tag);

      while (att_list != NULL)
      {
        if (strcmp(scew_attribute_name(scew_list_data(att_list)), "name") == 0)
        {
          attribute = (scew_attribute*)scew_list_data(att_list);
          break;
        }
        att_list = scew_list_next(att_list);
      }

      /* To obtain attribute value use: */
      /* possible attribute name values are:
        "status", "bnodes" "result", "subscription_id", "new_results", "obsolete_results"
      */
      value = scew_attribute_value(attribute);

      if(strcmp(value,"status")==0)
      {
        value = scew_element_contents(tag);

        if(value!=NULL)
        strncpy(msg->transaction_status, value, MAX_STATUS_LEN);
      }

      if(strcmp(value,"results")==0 || strcmp(value,"new_results")==0)
      {
         triple_list = scew_element_by_name(tag,"triple_list");
         if (triple_list != NULL)
           msg->n_result = parse_rdf_triples(triple_list);
         else
           return -1;
         triple_list = NULL;
      }
      if(strcmp(value,"obsolete_results")==0)
      {
        triple_list = scew_element_by_name(tag,"triple_list");
        if (triple_list != NULL)
          msg->o_result = parse_rdf_triples(triple_list);
        else
          return -1;
        triple_list = NULL;
       }
       if(strcmp(value,"subscription_id")==0)
       {
          value=scew_element_contents(tag);
          if(value!=NULL)
            strncpy(msg->subscribe_id, value, SS_SUB_ID_MAX_LEN);
       }
       if(strcmp(value,"bnodes")==0)
       {
         uri_list = scew_element_by_name(tag,"urilist");
         if (uri_list != NULL)
           msg->bnodes = parse_bnodes(uri_list);
         else
           return -1;
         uri_list = NULL;
       }
       continue;
    }
  }

  scew_tree_free(tree);
  scew_parser_free(parser);

  // SmartSlog Team: buffer is not released.
  scew_reader_free(reader);

  return 0;
}

bnode_tmp_t * parse_bnodes(scew_element * uri_list)
{
  scew_element *uri = NULL;
  const XML_Char* value = NULL;

  bnode_tmp_t * first_bnode = NULL;
  bnode_tmp_t * new_bnode = NULL;

  scew_list *list = scew_element_children (uri_list);
 
  while(list!=NULL)
  {
    uri = scew_list_data(list);
    list = scew_list_next(list);

    new_bnode = (bnode_tmp_t *)malloc(sizeof(bnode_tmp_t));

    if(!new_bnode)
    {
      SS_DEBUG_PRINT("ERROR: unable to reserve memory for bnode\n");
      return first_bnode;
    }

    /* Add the new bnode to the top */
    new_bnode->next = first_bnode;
    first_bnode = new_bnode;

    scew_list *att_list = scew_element_attributes (uri);
    while (att_list != NULL)
    {
      if (strcmp(scew_attribute_name(scew_list_data(att_list)), "tag") == 0)
      {
        value = scew_attribute_value((scew_attribute*)scew_list_data(att_list));
        if(value!=NULL)
          strncpy(new_bnode->label, value, SS_SUBJECT_MAX_LEN);
        break;
      }
      att_list = scew_list_next(att_list);
    }

    value = scew_element_contents(uri);
    if(value!=NULL)
      strncpy(new_bnode->uri, value, SS_URI_MAX_LEN);
  }

  return first_bnode;
}

ss_triple_t * parse_rdf_triples(scew_element *triple_list)
{
  scew_element *triple = NULL;
  scew_element *triple_member = NULL;
  scew_attribute *attribute = NULL;
  const XML_Char* value = NULL;

  ss_triple_t *first_triple = NULL;
  ss_triple_t *triple_new = NULL;

  scew_list *list = scew_element_children (triple_list);
  int list_size;

  list_size = scew_element_count(triple_list);

  while (list != NULL)
  {
    triple_new = (ss_triple_t *)malloc(sizeof(ss_triple_t));

    triple = scew_list_data(list);
    list = scew_list_next(list);

    if(!triple_new)
    {
      SS_DEBUG_PRINT("ERROR: unable to reserve memory for ss_triple_t\n");
      return first_triple;
    }

    /* Add the new triple to the top */
    triple_new->next = first_triple;
    first_triple = triple_new;
    triple_new->subject[0] = 0;
    triple_new->predicate[0] = 0;
    triple_new->object[0] = 0;

    triple_member = scew_element_by_name(triple,"subject");
    if (triple_member != NULL)
    {
      triple_new->subject_type = SS_RDF_TYPE_URI;
      value=scew_element_contents(triple_member);
      if(value!=NULL)
        strncpy(triple_new->subject, value, SS_SUBJECT_MAX_LEN);
    }

    triple_member = NULL;

    triple_member = scew_element_by_name(triple,"predicate");
    if (triple_member != NULL)
    {
      value=scew_element_contents(triple_member);
      if(value!=NULL)
        strncpy(triple_new->predicate, value, SS_PREDICATE_MAX_LEN);
    }

    triple_member = NULL;

    triple_member = scew_element_by_name(triple,"object");
    if (triple_member != NULL)
    {
      scew_list *att_list = scew_element_attributes (triple_member);
      while(att_list != NULL)
      {
        if (strcmp(scew_attribute_name(scew_list_data(att_list)), "type") == 0)
        {
          attribute = (scew_attribute*)scew_list_data(att_list);
          break;
        }
        att_list = scew_list_next(att_list);
      }

      value = scew_attribute_value(attribute);
      if(value!=NULL)
      {
        if(strcmp(value, URI_STRING) == 0)
          triple_new->object_type = SS_RDF_TYPE_URI;
        else triple_new->object_type = SS_RDF_TYPE_LIT;
      }
      value=scew_element_contents(triple_member);
      if(value!=NULL)
        strncpy(triple_new->object, value, SS_OBJECT_MAX_LEN);
    }
    triple_member = NULL;

  }

  return first_triple;
}
