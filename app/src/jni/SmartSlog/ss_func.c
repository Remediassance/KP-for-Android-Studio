/*
 * ss_func.c - functions for work with SS.
 * This file is part of PetrSU KP library.
 *
 * Copyright (C) 2009 - Alexandr A. Lomov. All rights reserved.
 *
 * PetrSU KP library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PetrSU KP library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PetrSU KP library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "ss_func.h"

#include "utils/kp_error.h"

#define UUID_HASH_LENGTH 3
#define NULL_BYTE_LENGTH 1

static int string_to_sharp_length(char *);

/**
 * @brief ss_info struct for KP.
 */
//static ss_info_t ss_info;

/******************************************************************************/
/**************************** External functions ******************************/
/**
 * @brief Get ss_info struct.
 *
 * ss_info used by KPI Low functions.
 *
 * @return ss_info on success or NULL otherwise.
 */
//ss_info_t* sslog_get_ss_info()
//{
//    return &ss_info;
//}


/**
 * @brief Resets SmartSlog and KPI errno.
 */
SSLOG_EXTERN void sslog_reset_errno()
{
	ss_info_t* ss_info = sslog_get_ss_info();
    
    if (ss_info != NULL)
	    ss_info->ss_errno = 0;

	reset_error();
}

/**
 * @fn sslog_generate_uuid(class_t *ont_class)
 *
 * @brief Generate UUID according given class (generates valid URI).
 *
 * @return new UUID on success or NULL otherwise.
 */
SSLOG_EXTERN char* sslog_generate_uuid(const class_t *ont_class)
{
    int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO)
        return NULL;

    int sharp_place = string_to_sharp_length(ont_class->classtype);

    if (sharp_place == 0) return NULL;

    char *uuid = (char *) malloc(sizeof(char) * (sharp_place+UUID_HASH_LENGTH+NULL_BYTE_LENGTH));
    if (uuid == NULL) return NULL;

    char *hash = sslog_generate_hash();
    if (hash == NULL) return NULL;

    strncpy(uuid, ont_class->classtype, sharp_place);
    strncat(uuid, hash, UUID_HASH_LENGTH);

    return uuid;
}


// TODO: Write better code, find uuid library.
/**
 * @brief Generate UUID.
 *
 * @return new UUID on success or NULL otherwise.
 */
SSLOG_EXTERN char* sslog_generate_hash()
{
    time_t t;
    srand((unsigned) time(&t));

    int rnd_num = rand() % 1000;

    char *hash = (char *) malloc(sizeof(char) * (UUID_HASH_LENGTH+NULL_BYTE_LENGTH));
    
    if (hash == NULL) return NULL;

    sprintf(hash, "%i", rnd_num);

    return hash;
}


/**
 * @brief Makes different library initialization.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_ss_init_session()
{
     ss_info_t* ss_info = sslog_get_ss_info();
     return ss_discovery(ss_info);
    //user@192.168.0.247
    //ss_info_t ss = {{0}, SS_SPACE_ID, {SS_ADDRESS, SS_PORT}, {0}};
    //*first_ss = ss;
}


/**
 * @brief Makes different library initialization using given data.
 *
 * @param space_id Smart Space id.
 * @param ss_address Smart Space address(ip or nota).
 * @param ss_port Smart Space port.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_ss_init_session_with_parameters(const char *space_id, const char *ss_address, int ss_port)
{
    ss_info_t* ss_info = sslog_get_ss_info();
    ss_discovery(ss_info);

    strncpy(ss_info->space_id, space_id, SS_SPACE_ID_MAX_LEN);
    strncpy(ss_info->address.ip, ss_address, MAX_IP_LEN);
    ss_info->address.port = ss_port;

    return 0;
}

/**
 * @brief Findes sharp ("#") in string and returns it place in given uri
 * Function returns length on given uri if there is no sharp
 *
 * @param uri URI to search in
 *
 * @return int >0 on success or 0 otherwise.
 */
static int string_to_sharp_length(char *uri)
{
    int i;
 
    if (uri == NULL) return 0;
    
    size_t size = strlen(uri);
    if (size == 0) return 0;
    
    for (i=size-1;i>=0;i--)
    {
        if (uri[i] == '#') break;
    }
    
    return (i==0)?size:i+1;
}

