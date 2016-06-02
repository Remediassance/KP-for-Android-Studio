/**
 * @file property_changes.c
 * @Author Aleksandr A. Lomov <lomov@cs.karelia.ru>
 * @date   4/26/12 7:46 PM
 * @brief  Changes of the property.
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
 * property_changes.c - Functions to manipulate with changes of the property.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2012 - SmartSlog Team (Aleksandr A. Lomov). All rights reserved.
 */

#include <string.h>

#include "utils/kp_debug.h"
#include "utils/util_func.h"
#include "ss_func.h"
#include "property_changes.h"


/******************************************************************************/
/*************************** Static functions list ****************************/
static char* prepare_value(property_t * property, const void *value);
static const char* get_uuid(const individual_t* individual);



/******************************************************************************/
/***************************** Internal functions *****************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @brief New data for subscription container.
 *
 * Subscription data contains individual and list of properties or NULL for all
 * properties of individual.
 *
 * @param ind individual.
 * @param properties list of properties for subscribe or NULL for all properties.
 *
 * @return new subscription data on success or NULL otherwise.
 */
property_changes_data_t* new_property_changes(
        property_t *property,
        individual_t *owner,
        const void *old_value,
        const void *new_value,
        action_type action)
{
    if (property == NULL || owner == NULL) {
        return NULL;
    }

    property_changes_data_t *changes =
            (property_changes_data_t *) malloc(sizeof (property_changes_data_t));

    if (changes == NULL) {
        KPLIB_DEBUG_PRINT("\n%s new_property_changes: no memory",
                KPLIB_DEBUG_PROP_CH_PREFIX);

        return NULL;
    }

    changes->property = property;

    const char *uuid = get_uuid(owner);
    changes->owner_uuid = sslog_strndup(uuid, KPLIB_UUID_MAX_LEN);

    changes->current_value = prepare_value(property, new_value);
    changes->previous_value = prepare_value(property, old_value);

    changes->action = action;
    
    return changes;
}

/**
 * @brief Free subscription container data.
 *
 * Free subscription data structure, individual and properties not freed. You
 * can free properties list (only list structure will be freed, not properties),
 * using parameter free_prop_list.
 *
 * @param data subscription data for free.
 * @param free_prop_list if it equals true - list structure will be freed, without properties.
 */
void free_property_changes(property_changes_data_t *changes)
{
    changes->rtti = RTTI_MIN_VALUE;

    free(changes->current_value);
    free(changes->previous_value);
    free(changes->owner_uuid);

    changes->current_value = NULL;
    changes->previous_value = NULL;
    changes->owner_uuid = NULL;

    free(changes);
}

/// @endcond
/******************************************************************************/


/******************************************************************************/
/***************************** Static functions *******************************/

static const char* get_uuid(const individual_t *individual)
{
    if (verify_individual(individual) != SSLOG_ERROR_NO) {
        return NULL;
    }

    return individual->uuid;
}

static char* prepare_value(property_t * property, const void *value)
{
    if (value == NULL) {
        return NULL;
    }

    if (property->type == DATATYPEPROPERTY) {
        return sslog_strndup((const char *) value, SS_OBJECT_MAX_LEN);
    }

    const char *uuid = get_uuid((const individual_t *)value);

    if (uuid == NULL) {
        return NULL;
    }

    return sslog_strndup(uuid, KPLIB_UUID_MAX_LEN);

}

