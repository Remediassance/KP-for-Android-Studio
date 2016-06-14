/*
 * ss_multisib.c - functions to control work with different SIBs in one KP.
 * This file is part of SmartSlog KP library.
 *
 * Copyright (C) 2009 - Pavel I. Vanag. All rights reserved.
 *
 * SmartSlog KP library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SmartSlog KP library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartSlog KP library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ss_multisib.h"
#ifdef MTENABLE
#include <pthread.h>
#endif

/**
 * @brief Property structure, represents OWL property.
 */
typedef struct msib_name_s {
	char *name_id;          /**< Run-time type information. */
	ss_info_t ss_info;      /**< Maximum cardinality. */
} msib_name_t;


#ifndef MTENABLE

/**
 * @brief Property structure, represents OWL property.
 */
typedef struct msib_thread_s {
	pthread_t pid;          /**< Run-time type information. */
	ss_info_t *ss_info;     /**< Maximum cardinality. */
} msib_thread_t;

#endif



/******************************************************************************/
/*************************** Static functions list ****************************/
static int msib_add_to_list(msib_name_t* name);
static ss_info_t* msib_get_by_name(const char *name_id);
static msib_name_t* new_msib_name(const char *name_id);
static void free_msib_name( msib_name_t* name);
static msib_name_t* msib_default();
static ss_info_t* msib_first_or_deafult();
static ss_info_t* msib_get_first();

#ifndef MTENABLE	
static ss_info_t* msib_get_by_pid(pthread_t *pid);
static int msib_set_for_pid(ss_info_t* ss_info);
static int msib_reset_pids();
#endif


/**
 * @var list_t msibs_list
 * @brief ss_info struct for KP.
 * @see msib_name_t
 */
static list_t msibs_list;

/**
 * @var ss_info_t* ss_info_current
 * @brief Pointer to current ss_info structure.
 * @see ss_info_t
 */
static ss_info_t* ss_info_current = NULL;

#ifndef MTENABLE
/**
 * @var list_t msib_threads
 * @brief ss_info struct for KP.
 * @see msib_thread_t
 */
static list_t msib_threads;
#endif

/******************************************************************************/
/**************************** External functions ******************************/

/** @cond EXTERNAL_FUNCTIONS */

/**
 * @fn int sslog_ss_init_new_session(const char *name_id)
 *
 * @brief Adds new SIB to the global list and discover it.
 *
 * Creates new structure with ss_info and given name if it not already exists
 * and adds it to global list and discover. Pointer to current SIB would be set to new SIB.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] const char * name_id. Pointer to the local name of the new SIB.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
SSLOG_EXTERN int sslog_ss_init_new_session(const char *name_id)
{
    // Create new item for msibs_list
    msib_name_t* name = new_msib_name(name_id);

    if (name == NULL) {
        return -1;
    }

    // Add it to list and discover
    msib_add_to_list(name);

    if (ss_discovery(&name->ss_info) != 1) {
        //set_error(...);
        return -1;
    }
    
    // Set current SIB pointer to added SIB
    ss_info_current = &name->ss_info;

    return 0;
}


/**
 * @fn int sslog_ss_init_new_session_with_parameters(const char *space_id, const char *ss_address, int ss_port, const char *name_id)
 *
 * @brief Adds new SIB to the global list and discover it with given access.
 *
 * Creates new structure with ss_info and given name if it not already exists
 * and adds it to global list and discover with given access. Pointer to current SIB would be set to new SIB.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] const char * space_id. Pointer to the Smart Space ID.
 * @param[in] const char * ss_address. Pointer to the Smart Space IP address.
 * @param[in] int ss_port. Pointer to the Smart Space TCP port.
 * @param[in] const char * name_id. Pointer to the local name of the new SIB.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
SSLOG_EXTERN int sslog_ss_init_new_session_with_parameters(const char *space_id, const char *ss_address, int ss_port, const char *name_id)
{
    // Create new item for msibs_list
    msib_name_t* name = new_msib_name(name_id);

    if (name == NULL) {
        return -1;
    }

    // Add it to list and discover
    msib_add_to_list(name);

    if (ss_discovery(&name->ss_info) != 1) {
        //set_error(...);
        return -1;
    }

    // Set given access to SIB (Smart Space ID, IP address and port)
    (void) strncpy(name->ss_info.space_id, space_id, SS_SPACE_ID_MAX_LEN);
    (void) strncpy(name->ss_info.address.ip, ss_address, MAX_IP_LEN);
    name->ss_info.address.port = ss_port;
   
    // Set current SIB pointer to added SIB
    ss_info_current = &name->ss_info;

    return 0;
}


/**
 * @fn int sslog_switch_sib(const char *name_id)
 *
 * @brief Set current SIB pointer to SIB with given name.
 *
 * Tries to find SIB with given name in global list
 * and set current SIB pointer to it .
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] const char * name_id. Pointer to the local name of the SIB.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
SSLOG_EXTERN int sslog_switch_sib(const char *name_id)
{
    // Find SIB with given name
    ss_info_t* ss_info = msib_get_by_name(name_id);

    if (ss_info == NULL) {
        return -1;
    }

    // Set current SIB pointer to founded SIB for PID (if pthread support turned on)...
#ifndef MTENABLE
    msib_set_for_pid(ss_info);
    
    return 0;
#endif

    // ... or for all  (if pthread support turned off)
    ss_info_current = ss_info;

    return 0;
}


/**
 * @fn int ss_leave_by_name(const char* name_id)
 *
 * @brief Leaves SIB with given name.
 *
 * Tries to find SIB with given name in global list
 * and leaves it. If it was current SIB, current SIB would be swithed to first founded or to deafult.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] const char * name_id. Pointer to the local name of the SIB.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
SSLOG_EXTERN int ss_leave_by_name(const char* name_id)
{
    // Find SIB with given name
    ss_info_t* ss_info = msib_get_by_name(name_id);
    
    if (ss_info == NULL) {
        return -1;
    }
    
    // Leave this SIB
    int status = sslog_ss_leave_session(ss_info);
    
    if (status != 0) {
        //set_error(...);
        return status;
    }

    // Get first founded SIB or create deafult
    ss_info = msib_first_or_deafult();

    if (ss_info == NULL) {
        //set_error(...);
        return -1;
    }
    
    // Set current SIB pointer for PID (if pthread support turned on)...
#ifndef MTENABLE
    msib_set_for_pid(ss_info);
    
    return 0;
#endif
    
    // ... or for all  (if pthread support turned off)
    ss_info_current = ss_info;
    
    return 0;
}

/**
* @fn int sslog_ss_leave_session_all()
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_leave_session_all()
{
    list_head_t *list_walker = NULL;
    list_head_t *tmp = NULL;

    list_for_each_safe (list_walker, tmp, &msibs_list.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_name_t *msib_data = (msib_name_t *) node->data;
        
        if (msib_data == NULL) {
            list_del(list_walker);
            continue;
        }
        
        // Leave this SIB
        int status = sslog_ss_leave_session(&msib_data->ss_info);
    
        if (status != 0) {
            //set_error(...);
            return status;
        }
        
        if (msib_data->name_id != NULL) free(msib_data->name_id);
      
        free(msib_data);

        list_del_and_free_node(node, NULL);
        //list_del(list_walker);
        //free(list_walker);
    }

#ifndef MTENABLE
    int status = msib_reset_pids();
#endif
    ss_info_current = NULL;
    
    return 0;
}

SSLOG_EXTERN int sslog_ss_leave_session(ss_info_t * ss_info)
{
    return ss_leave(ss_info);
}

/// @endcond


/**
 * @brief Get ss_info struct.
 *
 * ss_info used by KPI Low functions.
 *
 * @return ss_info on success or NULL otherwise.
 */
/**
* @fn int sslog_ss_insert_individual(individual_t *individual)
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN ss_info_t* sslog_get_ss_info()
{
    ss_info_t* ss_info_pthread = NULL;
#ifndef MTENABLE
    pthread_t pid = pthread_self();
    ss_info_pthread = msib_get_by_pid(&pid);
#endif

    if (ss_info_pthread == NULL)
    {
        if (ss_info_current == NULL)
        {
            msib_name_t* name = msib_default();
            if (name == NULL) {
                return NULL;
            }
            ss_info_current = &name->ss_info;
        }
        return ss_info_current;
    }
    else
        return ss_info_pthread;
}


#ifndef MTENABLE
/**
* @fn int sslog_ss_insert_individual(individual_t *individual)
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
static int msib_set_for_pid(ss_info_t* ss_info)
{
    list_head_t *list_walker = NULL;
    list_head_t *tmp = NULL;
    pthread_t pid = pthread_self();
    
    if (list_empty(&msib_threads.links)) {
        //set_error(...);
        return -1;
    }
    
    list_for_each_safe (list_walker, tmp, &msib_threads.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_thread_t *msib_data = (msib_thread_t *) node->data;
        
        if (msib_data == NULL || msib_data->pid == NULL || msib_data->ss_info == NULL) continue;
        
        if (pthread_equal(msib_data->pid, pid)) {
            msib_data->ss_info = ss_info;
            return 0;
        }
    }

    msib_thread_t *for_thread = (msib_thread_t *) malloc(sizeof(msib_thread_t));
    
    for_thread->ss_info = ss_info;
    for_thread->pid = pid;
    list_add_data((void *) for_thread, &msib_threads);
    
    return 0;
}


/**
* @fn int sslog_ss_insert_individual(individual_t *individual)
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
static ss_info_t* msib_get_by_pid(pthread_t *pid)
{
    list_head_t *list_walker = NULL;

    if (list_empty(&msib_threads.links)) {
        //set_error(...);
        return NULL;
    }

    list_for_each (list_walker, &msib_threads.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_thread_t *msib_data = (msib_thread_t *) node->data;
        
        if (msib_data == NULL || msib_data->pid == NULL || msib_data->ss_info == NULL) continue;
        
        if (pthread_equal(msib_data->pid, *pid)) {
            return msib_data->ss_info;
        }
    }
    
    return NULL;
}


/**
 * @fn int sslog_ss_insert_individual(individual_t *individual)
 *
 * @brief Inserts given individual to SS
 *
 * Checks individual correctness: cardinality, property values limitation, and
 * converts the individual to triplets, which will be placed to SS.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] individual_t * individual. Pointer to the individual struct.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
static int msib_reset_pids()
{
    list_head_t *list_walker = NULL;
    list_head_t *tmp = NULL;
    
    if (list_empty(&msib_threads.links)) {
        return 0;
    }
    
    list_for_each_safe (list_walker, tmp, &msib_threads.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_thread_t *msib_data = (msib_thread_t *) node->data;
        
        if (msib_data == NULL) {
            list_del(list_walker);
            continue;
        }
        
        free(msib_data);

        list_del(list_walker);
    }
    
    return 0;
}
#endif




static msib_name_t* msib_default()
{
    msib_name_t* name = new_msib_name("__d_e_f");
    if (name == NULL) {
        return NULL;
    }

    if (msib_add_to_list(name) != 0) {
        (void) free_msib_name(name);
        return NULL;
    }

    return name;
}


static ss_info_t* msib_first_or_deafult()
{
    // Get first founded SIB...
    ss_info_t* ss_info = msib_get_first();
    
    // ... or create deafult
    if (ss_info == NULL) {
        msib_name_t* name = msib_default();
        if (name == NULL) {
            //set_error(...);
            return NULL;
        }
        ss_info = &name->ss_info;
    }

    return ss_info;
}

/**
* @fn int sslog_ss_insert_individual(individual_t *individual)
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
static ss_info_t* msib_get_by_name(const char *name_id)
{
    list_head_t *list_walker = NULL;
    
    if (list_empty(&msibs_list.links)) {
        //set_error(...);
        return NULL;
    }    

    list_for_each (list_walker, &msibs_list.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_name_t *msib_data = (msib_name_t *) node->data;
        
        if (msib_data == NULL || msib_data->name_id == NULL) continue;
        
        if (strcmp(msib_data->name_id, name_id) == 0) {
            return &msib_data->ss_info;
        }
    }
    
    return NULL;
}


/**
 * @fn int sslog_ss_insert_individual(individual_t *individual)
 *
 * @brief Inserts given individual to SS
 *
 * Checks individual correctness: cardinality, property values limitation, and
 * converts the individual to triplets, which will be placed to SS.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] individual_t * individual. Pointer to the individual struct.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 */
static ss_info_t* msib_get_first()
{
    list_head_t *list_walker = NULL;
    
    if (list_empty(&msibs_list.links)) {
        //set_error(...);
        return NULL;
    }    
    
    list_for_each (list_walker, &msibs_list.links) {
        list_t *node = list_entry(list_walker, list_t, links);
        if (node == NULL || node->data == NULL) continue;
        msib_name_t *msib_data = (msib_name_t *) node->data;
        
        if (msib_data == NULL || msib_data->name_id == NULL) continue;
        
        return &msib_data->ss_info;
    }
    
    return NULL;
}


/**
* @fn int sslog_ss_insert_individual(individual_t *individual)
*
* @brief Inserts given individual to SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
static int msib_add_to_list(msib_name_t* name)
{
    if (name == NULL) {
        return -1;
    }
 
    if (ss_info_current == NULL)
    {
        INIT_LIST_HEAD(&msibs_list.links);
    }
   
    if (msib_get_by_name(name->name_id) != NULL)
        return -1;

    list_add_data((void *) name, &msibs_list);
    return 0;
}


static msib_name_t* new_msib_name(const char *name_id)
{
    if (name_id == NULL)
        return NULL;

    msib_name_t *name = (msib_name_t *) malloc(sizeof(msib_name_t));

    if (name == NULL)
    return NULL;

    name->name_id = strdup(name_id);

    return name;
    
}


static void free_msib_name(msib_name_t* name)
{
    
}
