/**
 * @file subscription_changes.c
 * @author Aleksandr A. Lomov <lomov@cs.karelia.ru>
 * @date   Created on April 28, 2012, 5:17 PM
 * @brief  Interface for changes of the property.
 *
 *
 * @section LICENSE
 *
 * SmartSlog KP Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * SmartSlog KP Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartSlog KP Library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * subscription_changes.c - interface for changes of the subscription.
 * Subscription changes contains iformation about chnages that were perfomed
 * under subscribed data.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2012 - SmartSlog Team (Aleksandr A. Lomov). All rights reserved.
 */


#include <string.h>
#include "ss_func.h"

#include "property_changes.h"

#include "utils/list.h"
#include "utils/kp_error.h"
#include "utils/kp_debug.h"
#include "utils/check_func.h"
#include "utils/util_func.h"

#include "subscription_changes_internal.h"



#define SBCR_CH_PRINT_FORMAT "\nSubscription chnages (seq = '%i'):" \
    "\n\tInserted individuals: %i" \
    "\n\tUpdated individuals: %i" \
    "\n\tRemoved individuals: %i" \
    "\n\tInserted properties: %i" \
    "\n\tUpdated properties: %i" \
    "\n\tRemoved properties: %i"



/******************************************************************************/
/*************************** Static functions list ****************************/
static list_t *get_individual_list_by_action(subscription_changes_data_t *changes, action_type action);
static list_t *get_property_list_by_action(subscription_changes_data_t *changes, action_type action);
static int is_uuid_equals(const char *a, const char *b);
static int is_prop_changes_for_uuid(property_changes_data_t *p_ch, const char *uuid);



/******************************************************************************/
/**************************** External functions ******************************/
/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Gets a suquence number of the changes.
 *
 * Changes are made based on data that were received with notification.
 * This number is sequence of indication that was used to make changes.
 * Firts changes are made while perfoming subscription operation, 
 * this changes has '1' as this number.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes subscription chnages data structure.
 *
 * @return sequence number of the chnages.
 */
SSLOG_EXTERN int sslog_sbcr_ch_get_sequence_number(subscription_changes_data_t *changes)
{
    int error_code = verify_subscription_changes(changes);

    if (error_code != SSLOG_ERROR_NO) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_individuals END: incorrect changes struct.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        set_error(error_code);
        return -1;
    }

    return changes->sequence_number;
}

/**
 * @brief Gets all individual's UUID by action.
 * 
 * Individuals can be inserted, removed or updated. Updated means that some 
 * properties of the individual were changed.
 *
 * <b>Do not modify or free returned list. </b>
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 * 
 * @param[in] changes subscription chnages data structure.
 * @param[in] action action that was perfom under individuals: insert, remove, 
 * update.
 *
 * @return list with individuals UUIDs (list with strings) or
 * NULL if given changes or action is incorrect.
 */
SSLOG_EXTERN const list_t *sslog_sbcr_ch_get_individual_by_action(subscription_changes_data_t *changes, action_type action)
{

    if (verify_subscription_changes(changes) != SSLOG_ERROR_NO) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_individuals END: incorrect changes struct.",                \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return NULL;
    }

    return get_individual_list_by_action(changes, action);
}

/**
 * @brief Gets all individual's UUID.
 *
 * It make one list with all individual's UUIDs, that are were inserted,
 * updated or removed.
 *
 * <b>You need to free the returned list, but without data relesed.</b>
 * Use #list_free_with_nodes with NULL as argument for free function.
 *
 * <b>Known bug:</b> if under the individual was perfomed more that one actions,
 * for example it was inserted and updated, then UUID will be inserted
 * to list more that once. This will be fixed later.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes changes to get individuals UUIDs.
 *
 * @return list with UUIDs (list with strings) on success or NULL otherwise.
 */
SSLOG_EXTERN list_t *sslog_sbcr_ch_get_individual_all(subscription_changes_data_t *changes)
{
    if (verify_subscription_changes(changes) != SSLOG_ERROR_NO) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_all_individuals END: incorrect changes struct.",                \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return NULL;
    }

    list_t *list = get_individual_list_by_action(changes, ACTION_INSERT);
    list_t *inserted_list = list_copy_list(list);

    list = get_individual_list_by_action(changes, ACTION_UPDATE);
    list_t *updated_list = list_copy_list(list);

    list = get_individual_list_by_action(changes, ACTION_REMOVE);
    list_t *removed_list = list_copy_list(list);

    if (updated_list == NULL || inserted_list == NULL || updated_list == NULL) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_all_individuals END: not enough memory.",                \
                KPLIB_DEBUG_SBCR_CH_PREFIX);

        list_free_with_nodes(inserted_list, NULL);
        list_free_with_nodes(updated_list, NULL);
        list_free_with_nodes(removed_list, NULL);
        return NULL;
    }

    list_t *full_list = inserted_list;
    list_add_list(updated_list, full_list);
    list_add_list(removed_list, full_list);

    list_free(updated_list);
    list_free(removed_list);

    return full_list;
}

/**
 * @brief Gets all changed properties by action.
 *
 * <b>Do not modify or free returned list. </b>
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes changes to get properties.
 * @param[in] action action that was perfom under properties: insert, remove,
 * update.
 *
 * @return list with properties changes (list with #property_changes_data_s) or
 * NULL if given changes or action is incorrect.
 *
 * @see property_changes_data_s
 */
SSLOG_EXTERN const list_t *sslog_sbcr_ch_get_property_by_action(subscription_changes_data_t *changes, action_type action)
{
    int error_code = verify_subscription_changes(changes);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    return get_property_list_by_action(changes, action);
}

/**
 * @brief Gets all changed properties.
 *
 * <b>You need to free the returned list, but without data relesed.</b>
 * Use #list_free_with_nodes with NULL as argument for free function.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes changes to get properties.
 *
 * @return list with properties changes (list with #property_changes_data_s)
 * on success or NULL otherwise.
 *
 * @see property_changes_data_s
 */
SSLOG_EXTERN list_t *sslog_sbcr_ch_get_property_all(subscription_changes_data_t *changes)
{
    reset_error();

    int error_code = verify_subscription_changes(changes);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    list_t *list = get_property_list_by_action(changes, ACTION_INSERT);
    list_t *inserted_list = list_copy_list(list);

    list = get_property_list_by_action(changes, ACTION_UPDATE);
    list_t *updated_list = list_copy_list(list);

    list = get_property_list_by_action(changes, ACTION_REMOVE);
    list_t *removed_list = list_copy_list(list);

    if (updated_list == NULL || inserted_list == NULL || updated_list == NULL) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_all_properties_by_uuid END: not enough memory.",                \
                KPLIB_DEBUG_SBCR_CH_PREFIX);

        list_free_with_nodes(inserted_list, NULL);
        list_free_with_nodes(updated_list, NULL);
        list_free_with_nodes(removed_list, NULL);
        return NULL;
    }

    list_t *full_list = inserted_list;
    list_add_list(updated_list, full_list);
    list_add_list(removed_list, full_list);

    list_free(updated_list);
    list_free(removed_list);

    return full_list;
}

/**
 * @brief Gets all changed properties by action for individual's UUID.
 *
 * This function can returns list for updated individuals, for other returned
 * list will be empty.
 *
 * <b>You need to free the returned list, but without data relesed.</b>
 * Use #list_free_with_nodes with NULL as argument for free function.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes changes to get properties.
 * @param[in] uuid individual's UUID to get properties.
 * @param[in] action action that was perfom under properties: insert, remove,
 * update.
 *
 * @return list with properties changes (list with #property_changes_data_s) or
 * NULL if given changes or action is incorrect.
 *
 * @see property_changes_data_s
 */
SSLOG_EXTERN list_t *sslog_sbcr_ch_get_property_by_uuid(subscription_changes_data_t *changes,
        const char *uuid, action_type action)
{
    reset_error();

    int error_code = verify_subscription_changes(changes);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    if (is_str_null_or_empty(uuid) == true) {
        set_error(SSLOG_ERROR_NULL_ARGUMENT);
        return NULL;
    }

    list_t *list = get_property_list_by_action(changes, action);

    if (list == NULL) {
        set_error(SSLOG_ERROR_UNKNOWN);
        return NULL;
    }

    list_t *uuid_prop = list_copy_list_with_filter(list,
            (int (*)(void*, void *))is_prop_changes_for_uuid,
            (void *) uuid);

    if (uuid_prop == NULL) {
        set_error(SSLOG_ERROR_UNKNOWN);
        return NULL;
    }
    /*

        list_head_t *list_walker = NULL;

        list_for_each(list_walker, &list->links)
        {
            list_t *node = list_entry(list_walker, list_t, links);
            property_changes_data_t *prop_ch = (property_changes_data_t *) node->data;

            if (prop_ch == NULL) {
                continue;
            } else if (is_str_null_or_empty(prop_ch->owner_uuid) == true) {
                KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_properties_by_uuid: incorrect uuid.",  \
                    KPLIB_DEBUG_SBCR_CH_PREFIX);
                continue;
            } else if (strncmp(prop_ch->owner_uuid, uuid, KPLIB_UUID_MAX_LEN) == 0) {
                list_add_data(prop_ch, uuid_prop);
            }
        }
     */

    return uuid_prop;
}

/**
 * @brief Gets all changed properties for individual's UUID.
 *
 * This function can returns list for updated individuals, for other returned
 * list will be empty.
 *
 * <b>You need to free the returned list, but without data relesed.</b>
 * Use #list_free_with_nodes with NULL as argument for free function.
 *
 * This function sets a global error if it can't perfome operation,
 * you can get an error code using #sslog_get_error_code function.
 *
 * @param[in] changes changes to get properties.
 * @param[in] uuid individual's UUID to get properties.
 *
 * @return list with properties changes (list with #property_changes_data_s) or
 * NULL if given changes or action is incorrect.
 *
 * @see property_changes_data_s
 */
SSLOG_EXTERN list_t *sslog_sbcr_ch_get_property_by_uuid_all(subscription_changes_data_t *changes,
        const char *uuid)
{
    reset_error();

    int error_code = verify_subscription_changes(changes);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    if (is_str_null_or_empty(uuid) == true) {
        set_error(SSLOG_ERROR_INCORRECT_ARGUMENT);
        return NULL;
    }

    list_t *list = get_property_list_by_action(changes, ACTION_INSERT);
    list_t *inserted_list = list_copy_list_with_filter(list,
            (int (*)(void*, void *))is_prop_changes_for_uuid, (void *) uuid);

    list = get_property_list_by_action(changes, ACTION_UPDATE);
    list_t *updated_list = list_copy_list_with_filter(list,
            (int (*)(void*, void *))is_prop_changes_for_uuid, (void *) uuid);

    list = get_property_list_by_action(changes, ACTION_REMOVE);
    list_t *removed_list = list_copy_list_with_filter(list,
            (int (*)(void*, void *))is_prop_changes_for_uuid, (void *) uuid);

    if (updated_list == NULL || inserted_list == NULL || updated_list == NULL) {
        KPLIB_DEBUG_PRINT("\n%s sbcr_ch_get_all_properties_by_uuid END: not enough memory.",                \
                KPLIB_DEBUG_SBCR_CH_PREFIX);

        list_free_with_nodes(inserted_list, NULL);
        list_free_with_nodes(updated_list, NULL);
        list_free_with_nodes(removed_list, NULL);

        set_error(SSLOG_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    list_t *full_list = inserted_list;
    list_add_list(updated_list, full_list);
    list_add_list(removed_list, full_list);

    list_free(updated_list);
    list_free(removed_list);

    return full_list;
}

/**
 * @brief Prints information about changes.
 *
 * It prints sequence number and how manu individuals and properties were
 * inserted, updated or removed.
 *
 * @param[in] changes changes to print information about changes.
 */
SSLOG_EXTERN void sslog_sbcr_ch_print(subscription_changes_data_t *changes)
{
    if (verify_subscription_changes(changes) != SSLOG_ERROR_NO) {
        return;
    }

    printf(SBCR_CH_PRINT_FORMAT,
            changes->sequence_number,
            list_count(&changes->inserted_individuals),
            list_count(&changes->updated_individuals),
            list_count(&changes->removed_individuals),
            list_count(&changes->inserted_props),
            list_count(&changes->updated_props),
            list_count(&changes->removed_props)
            );
}


/** @endcond */


/******************************************************************************/
/**************************** Internal functions ******************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @fn subscription_changes_data_t* new_subscription_changes()
 * @brief Creates new subscription chnages data structure (@see subscription_changes_data_s).
 *
 * @return new struct on success or NULL otherwise.
 */
subscription_changes_data_t* new_subscription_changes()
{
    subscription_changes_data_t *changes =
            (subscription_changes_data_t *) malloc(sizeof (subscription_changes_data_t));

    if (changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s new_subscription_changes: no memory.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);

        return NULL;
    }

    changes->rtti = RTTI_SUBSCRIPTION_CHANGES;

    changes->sequence_number = 0;

    INIT_LIST_HEAD(&changes->inserted_individuals.links);
    INIT_LIST_HEAD(&changes->updated_individuals.links);
    INIT_LIST_HEAD(&changes->removed_individuals.links);

    INIT_LIST_HEAD(&changes->inserted_props.links);
    INIT_LIST_HEAD(&changes->updated_props.links);
    INIT_LIST_HEAD(&changes->removed_props.links);

    return changes;
}

/**
 * @fn free_subscription_changes(subscription_changes_data_t *changes)
 * @brief Free subscription changes data structure (@see subscription_changes_data_s).
 *
 * @param[in] changes structure for free, if parameter is NULL then it does nothing.
 */
void free_subscription_changes(subscription_changes_data_t *changes)
{
    if (changes == NULL) {
        return;
    }

    changes->rtti = RTTI_MIN_VALUE;

    clean_changes_data(changes);

    free(changes);
}

void clean_changes_data(subscription_changes_data_t *changes)
{
    if (changes == NULL) {
        return;
    }

    // Lists have UUID as data, release all data with free.
    list_del_and_free_nodes(&changes->inserted_individuals, free);
    list_del_and_free_nodes(&changes->updated_individuals, free);
    list_del_and_free_nodes(&changes->removed_individuals, free);

    // Lists with properties have struct as data, release data with appropriate function.
    list_del_and_free_nodes(&changes->inserted_props, (void (*)(void *))free_property_changes);
    list_del_and_free_nodes(&changes->updated_props, (void (*)(void *))free_property_changes);
    list_del_and_free_nodes(&changes->removed_props, (void (*)(void *))free_property_changes);
}

void set_sequence_number(subscription_changes_data_t *changes, int sequence_number)
{
    if (changes == NULL) {
        return;
    }

    changes->sequence_number = sequence_number;
}

void inc_sequence_number(subscription_changes_data_t *changes)
{
    if (changes == NULL) {
        return;
    }

    ++changes->sequence_number;
}

/**
 * @fn void add_individual_to_changes(subscription_changes_data_t *changes, individual_t *individual, action_type action)
 * @brief Adds individual to the changes by action.
 *
 * This function adds an individual only once, if you pass an individual
 * twice, then it does nothing.
 *
 * @param[in] changes subscription changes to add (@see subscription_changes_data_s).
 * @param[in] individual individual that will be added to the changes (@see individual_s)
 * @param[in] action action that was perfomed under individual: insert, remove, update.
 *
 * @return true on success(individual has been added or was already in the changes) or false otherwise.
 */
bool add_individual_to_changes(subscription_changes_data_t *changes,
        individual_t *individual, action_type action)
{
    if (changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: changes data is NULL.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    if (individual == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: individual is NULL.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    return add_individual_uuid_to_changes(changes, individual->uuid, action);
}

/**
 * @fn void add_individual_to_changes(subscription_changes_data_t *changes, individual_t *individual, action_type action)
 * @brief Adds individual to the changes by action.
 *
 * This function adds an individual only once, if you pass an individual
 * twice, then it does nothing.
 *
 * @param[in] changes subscription changes to add (@see subscription_changes_data_s).
 * @param[in] individual individual that will be added to the changes (@see individual_s)
 * @param[in] action action that was perfomed under individual: insert, remove, update.
 *
 * @return true on success(individual has been added or was already in the changes) or false otherwise.
 */
bool add_individual_uuid_to_changes(subscription_changes_data_t *changes,
        const char *uuid, action_type action)
{
    if (changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: changes data is NULL.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    if (is_str_null_or_empty(uuid) == true) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: individual has no UUID.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    list_t *ind_list = get_individual_list_by_action(changes, action);

    if (ind_list == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: no list for changes.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    // Check: individual's uuid is in the list.
    if (list_has_data_with_equals(ind_list, uuid,
            (int (*)(const void *, const void *)) is_uuid_equals) > 0) {
        return true;
    }

    char *copy_uuid = sslog_strndup(uuid, KPLIB_UUID_MAX_LEN);

    if (copy_uuid == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_individual_to_changes END: not enough memory.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    list_add_data(copy_uuid, ind_list);

    return true;
}

/**
 * @fn bool add_property_to_changes(subscription_changes_data_t *changes, individual_t *owner, property_t* property, const void *new_data, const void *old_data, action_type action)
 * @brief Adds property to the changes by action.
 *
 * Also the function adds owner(individual) to changes as updated
 * individual. You do not need to add individual by youself. Owner will be
 * add to the changes only once.
 *
 * @param[in] changes subscription changes to add (@see subscription_changes_data_s).
 * @param[in] owner individual who has changed property (@see individual_s).
 * @param[in] property property that was changed (@see property_s).
 * @param[in] new_data current data(string or individual) of the property.
 * @param[in] old_data previous data(string or individual) of the property.
 * @param[in] action action that was perfomed under property: insert, update, remove.
 *
 * @return true on success or false otherwise.
 */
bool add_property_to_changes(subscription_changes_data_t *changes,
        individual_t *owner, property_t* property,
        const void *new_data, const void *old_data,
        action_type action)
{
    if (property == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_property_to_changes END: property is NULL.",                  \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    list_t *props_list = get_property_list_by_action(changes, action);

    if (props_list == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_property_to_changes END: no list for changes.",                  \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }

    property_changes_data_t *prop_changes = new_property_changes(property, owner, old_data, new_data, action);

    if (prop_changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s add_property_to_changes END: can't create property changes struct.",                  \
            KPLIB_DEBUG_SBCR_CH_PREFIX);
        return false;
    }


    if (add_individual_to_changes(changes, owner, ACTION_UPDATE) == false) {
        KPLIB_DEBUG_PRINT("\n%s add_property_to_changes END: can't add individual to changes.",                  \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
        free_property_changes(prop_changes);
        return false;
    }

    list_add_data(prop_changes, props_list);

    return true;
}

/** @endcond */


/******************************************************************************/
/***************************** Static functions *******************************/

static list_t *get_individual_list_by_action(subscription_changes_data_t *changes, action_type action)
{
    if (changes == NULL) {
        return NULL;
    }

    switch (action) {
        case ACTION_INSERT:
            return &changes->inserted_individuals;
            break;
        case ACTION_UPDATE:
            return &changes->updated_individuals;
        case ACTION_REMOVE:
            return &changes->removed_individuals;
        default:
            KPLIB_DEBUG_PRINT("\n%s get_individual_list_by_action: incorrect  action.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
            break;
    }

    return NULL;
}

static list_t *get_property_list_by_action(subscription_changes_data_t *changes, action_type action)
{
    if (changes == NULL) {
        return NULL;
    }

    switch (action) {
        case ACTION_INSERT:
            return &changes->inserted_props;
            break;
        case ACTION_UPDATE:
            return &changes->updated_props;
            break;
        case ACTION_REMOVE:
            return &changes->removed_props;
            break;
        default:
            KPLIB_DEBUG_PRINT("\n%s get_property_list_by_action: incorrect action.",                     \
                KPLIB_DEBUG_SBCR_CH_PREFIX);
            break;
    }

    return NULL;
}

static int is_uuid_equals(const char *a, const char *b)
{
    return strncmp(a, b, KPLIB_UUID_MAX_LEN) == 0 ? 1 : 0;
}

static int is_prop_changes_for_uuid(property_changes_data_t *p_ch, const char *uuid)
{
    if (p_ch == NULL) {
        return 0;
    }

    if (is_str_null_or_empty(uuid) == true) {
        return 0;
    }

    if (strncmp(p_ch->owner_uuid, uuid, KPLIB_UUID_MAX_LEN) == 0) {
        return 1;
    }

    return 0;
}
