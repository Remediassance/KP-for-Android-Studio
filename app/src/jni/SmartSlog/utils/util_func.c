/**
 * @file util_func.c
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>, Vanag Pavel <vanag@cs.karelia.ru>
 * @date   26 December, 2009
 * @brief  Useful functions.
 *
 *
 * @section LICENSE
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
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Some small useful functions.
 * This file is part of PetrSU KP library.
 *
 * Copyright (C) 2009 - Alexander A. Lomov. All rights reserved.
 */

#ifndef _UTIL_FUNC_C_
#define _UTIL_FUNC_C_

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#include <stdio.h>
#include "kp_error.h"
#include "../structures.h"
#include "util_func.h"
#include "list.h"
#include <sys/types.h>
#include <stdlib.h>
#include "../properties.h"


/******************************************************************************/
/**************************** External functions ******************************/
/** @cond EXTERNAL_FUNCTIONS */

/**
 *
 * @brief Duplicate string: allocates memory of defined size and copies string at most size characters.
 *        If string is longer than size, only size characters would be copied,
 *                                     terminating null byte ('\0') would be added.
 *        Internal function "strndup" not in ANSI C c99 standart, so SmartSlog has own implementation.
 *
 *
 * @param const char *str - string to copy
 * @param size_t len -  size of string to copy.
 *
 * @return returns duplicated string or NULL if error accures.
 */
SSLOG_EXTERN char *sslog_strndup(const char *str, size_t n)
{
    char *result;
    size_t len = strlen (str);

    if (n < len)
        len = n;

    result = (char *) malloc (len + 1);
    if (!result)
        return NULL;

    result[len] = '\0';
    return (char *) memcpy (result, str, len);
}

/// @endcond



/******************************************************************************/
/**************************** Internal functions ******************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @brief Checks is property in the list.
 *
 * @param property_t property to check.
 * @param list_t list with properties.
 *
 * @return returns true if item founded in list and false otherwise.
 */
bool is_in_property_list(property_t *item, list_t *property_list)
{
    list_head_t *pos = NULL;

    if ((item == NULL) || (property_list == NULL)) {
        return false;
    }

    if (list_empty(&property_list->links)) {
        return false;
    }

    // Check names and types of properties to find given property.    

    list_for_each(pos, &property_list->links)
    {
        list_t *node = list_entry(pos, list_t, links);
        property_t *prop = (property_t *) node->data;

        if ((strcmp(item->name, prop->name) == 0)
                && (item->type == prop->type)
                && (item->rtti == prop->rtti)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Checks is property value in the list.
 *
 * @param prop_val_t alue to check.
 * @param list_t list with values.
 *
 * @return returns true if the item founded in the list and false otherwise.
 */
bool is_in_property_values_list(prop_val_t *item, list_t *property_list)
{
    list_head_t *pos = NULL;

    if ((item == NULL) || (property_list == NULL)) {
        return false;
    }

    if (list_empty(&property_list->links)) {
        return false;
    }

    list_for_each(pos, &property_list->links)
    {
        list_t *node = list_entry(pos, list_t, links);
        prop_val_t *prop = (prop_val_t *) node->data;

        if (is_prop_names_equal(item, prop)) {
            return true;
        }
    }

    return false;
}

/**
 * @brief Concatenate two triples lists.
 *
 * This function only add triples list b to a. No new memory allocated.
 * If you delete/free list a, b will be deleted too.
 *
 * @param a first triples list.
 * @param b second triples list.
 *
 * @return list a with added b. If one of lists equal NULL, it returns another,
 * if both equals NULL it returns NULL.
 */
ss_triple_t *concat_triplets(ss_triple_t *a, ss_triple_t *b)
{
    if (a == NULL && b == NULL) {
        return NULL;
    } else if (a == NULL) {
        return b;
    } else if (b == NULL) {
        return a;
    }

    ss_triple_t *list_walker = a;

    while (true) {
        if (list_walker->next == NULL) {
            list_walker->next = b;
            break;
        }
        list_walker = list_walker->next;
    }

    list_walker = b;

    return a;
}
/// @endcond
#endif /* _UTIL_FUNC_C_ */
