/**
 * @file classes.h
 * @Author Pavel I. Vanag <vanag@cs.karelia.ru>
 * @Author Alexandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Describe functions for work with knowledge patterns.
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
 * Describe functions for work with knowledge patterns.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2009 - SmartSlog. All rights reserved.
 *
 */

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#include <stdio.h>
#include <string.h>
#include "utils/list.h"

#include "structures.h"
#include "utils/kp_error.h"
#include "utils/kp_bool.h"

#ifndef _PATTERNS_H
#define	_PATTERNS_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif


/**
 * @brief rdf:type.
 */
#ifndef RDF_TYPE
#define RDF_TYPE "http://www.w3.org/1999/02/22-rdf-syntax-ns#type"
#endif


#ifdef	__cplusplus
extern "C" {
#endif


/******************************************************************************/
/****************************** Structures list *******************************/
/// @cond EXTERNAL_STRUCTURES

/**
 * @brief Structure for condition.
 *
 * Use for pattern.
 */
typedef struct pattern_condition_s {
    int rtti;                   /**< Run-time type information. */

    property_t *prop;           /**< Property to used in condition. */
    void *value;                /**< Value for property. */

    int condition;              /**< Condition for property and value ( @see pattern_conditions_types ). */
} pattern_condition_t;



/**
 * @brief Pattern structure.
 */
typedef struct pattern_s {
    int rtti;                   /**< Run-time type information. */

    class_t *ont_class;             /**< Class for pattern. */
    char *uuid; 		/**< UUID for pattern. */

    list_t *check_prop_list;    /**< List for properties to check. */
    list_t *uncheck_prop_list;  /**< List for properties to uncheck. */
} pattern_t;


/**
 * @brief Conditions for pattern.
 */
enum pattern_conditions_types {
	PATTERN_COND_MIN_VALUE = 0,     /**< Use for checking min value of conditions type field. */
    PATTERN_COND_NO,        /**< Condition not set. */
    PATTERN_COND_E,         /**< Equal condition. */
    PATTERN_COND_EL,        /**< Equal or less condition. */
    PATTERN_COND_EA,        /**< Equal or above condition. */
    PATTERN_COND_L,         /**< Less condition. */
    PATTERN_COND_A,         /**< Above condition. */
    PATTERN_COND_NE,        /**< Not equal condition. */
	PATTERN_COND_MAX_VALUE,     /**< Use for checking max value of conditions type field. */
};

/// @endcond
/******************************************************************************/



/******************************************************************************/
/****************************** Functions list ********************************/
/// @cond EXTERNAL_FUNCTIONS

SSLOG_EXTERN pattern_t* sslog_new_pattern(class_t *ont_class, const char *uuid);
SSLOG_EXTERN void sslog_free_pattern(pattern_t *pattern);
SSLOG_EXTERN void sslog_clean_pattern(pattern_t *pattern);
SSLOG_EXTERN int sslog_set_pattern_class(pattern_t *pattern, class_t *ont_class);
SSLOG_EXTERN int sslog_set_pattern_uuid(pattern_t *pattern, const char *uuid);

/**
* @fn list_t* sslog_ss_get_individual_by_pattern_all(individual_t *individual_pattern)
*
* @brief Finds all individuals matches given pattern in SS
*
* Tries to ﬁnd individual by pattern.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual_pattern. Pointer to the individual struct (pattern).
* @return list_t*. Null if there is no individuals of such class in SS,
*                 list of founded individuals otherwise.
*/
SSLOG_EXTERN list_t* sslog_ss_get_individual_by_pattern_all(pattern_t *individual_pattern);

SSLOG_EXTERN int sslog_add_unproperty_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition);

SSLOG_EXTERN int sslog_add_property_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition);


/// @endcond
/******************************************************************************/

#ifdef	__cplusplus
}
#endif

#endif	/* _PATTERNS_H */
