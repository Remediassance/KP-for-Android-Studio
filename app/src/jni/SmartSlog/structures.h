/**
 * @file structures.h
 * @Author Alexandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Describe C structures for OWL entities, KP and SS.
 *
 *
 * @section LICENSE
 *
 * PetrSU KP Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PetrSU KP Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PetrSU KP Library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Describe C structures for OWL entities, KP and SS.
 * This file is part of PetrSU KP Library.
 *
 * Copyright (C) 2009 - Alexander A. Lomov. All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "utils/kp_bool.h"
#include "utils/list.h"

#ifndef _STRUCTURES_H
#define	_STRUCTURES_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

#ifdef	__cplusplus
extern "C" {
#endif	

/******************************************************************************/
/******************************** Enums list **********************************/
/// @cond EXTERNAL_STRUCTURES

/**
 * @brief Run-time type information.
 */
enum rtti_types {
    RTTI_MIN_VALUE = 0,     /**< Use for checking min value of rtti field. */
    RTTI_INDIVIDUAL = 1,    /**< Individual type. */
    RTTI_CLASS = 2,         /**< Class type. */
    RTTI_PROPERTY = 3,      /**< Property type. */
    RTTI_SUBSCRIPTION,      /**< Subscription type. */
    RTTI_SUBSCRIPTION_CHANGES,      /**< Subscription changes type (#subscription_changes_data_s). */
    RTTI_SUBSCRIPTION_CALLBACK,      /**< Subscription's callback type. */
    RTTI_PATTERN,           /**< Pattern type. */
    RTTI_PATTERN_CONDITION, /**< Pattern condition type. */
    RTTI_MAX_VALUE          /**< Use for checking max value of rtti field. */
};


/**
 * @brief Types of properties.
 */
enum property_types {
    DATATYPEPROPERTY = 1,   /**< Data-property type. */
    OBJECTPROPERTY = 2      /**< Object-property type. */
};


/** @brief Statses of triples changing */
typedef enum action_type_enum {
    ACTION_UPDATE, /**< Triple was updated. */
    ACTION_REMOVE, /**< Triple was deleted. */
    ACTION_INSERT, /**< New triple was inserted. */
    ACTION_UNKNOWN /**< Maybe something wrong. */
} action_type;

/// @endcond

/// @cond INTERNAL_STRUCTURES

/**
 * @brief Types of subscription process.
 */
enum subscribe_status {
    SBCR_STATUS_MIN,
    SBCR_STATUS_NO,
    SBCR_STATUS_SUBSCRIBED,
    SBCR_STATUS_UPDATING,
    SBCR_STATUS_MAX
};

/// @endcond

/******************************************************************************/



/******************************************************************************/
/****************************** Structures list *******************************/
/// @cond EXTERNAL_STRUCTURES

/**
 * @brief Property structure, represents OWL property.
 */
typedef struct property_s {
	int rtti;               /**< Run-time type information. */
	int type;               /**< Property type: object, data. */
	char *name;             /**< Name of property. */
	char *domain;           /**< Property domain. */
	char *about;            /**< About field. */
	list_t *subpropertyof;  /**< Parent properties list. */
	list_t *oneof;          /**< Propertie's oneof value (OWL oneof). */
	int mincardinality;     /**< Minimal cardinality. */
	int maxcardinality;     /**< Maximum cardinality. */
} property_t;


/**
 * @brief Property value structure.
 *
 * Use for individuals.
 */
typedef struct prop_val_s {
    property_t *property;       /**< Reference to property struct. */
    void *prop_value;           /**< Value for property. */
} prop_val_t;


/**
 * @brief Class structure, represents OWL class.
 */
typedef struct class_s {
	int rtti;               /**< Run-time type information. */
	char *classtype;        /**< Type of class, name. */
	list_t *superclasses;   /**< List of superclasses. */
	list_t *oneof;          /**< Class oneof value (OWL oneof). */
	list_t *properties;     /**< Properties list for class. */
	list_t *instances;      /**< List of individuals. */
} class_t;


/**
 * @brief Individual structure.
 */
typedef struct individual_s {
	int rtti;                       /**< Run-time type information. */
	char *uuid;                     /**< UUID of individual. */
	char *classtype;                /**< Individual's class type. */
	list_t *properties;             /**< Properties values list. */
	const class_t *parent_class;    /**< Parent class. */
        int subscribe_status;           /**< Status of subscribe process. */
} individual_t;


/// @endcond
/******************************************************************************/



/******************************************************************************/
/****************************** Functions list ********************************/
/// @cond EXTERNAL_FUNCTIONS

SSLOG_EXTERN void sslog_free_property(property_t *property);
SSLOG_EXTERN void sslog_free_value_struct(prop_val_t *prop_val);
SSLOG_EXTERN void sslog_free_value_struct_with_func(prop_val_t *prop_val, void (*free_data_func)(void*));
SSLOG_EXTERN void sslog_free_data_property_value_struct(prop_val_t *prop_val);
SSLOG_EXTERN void sslog_free_class(class_t *ont_class);
SSLOG_EXTERN void sslog_free_individual(individual_t *individual);

SSLOG_EXTERN prop_val_t* sslog_new_value_struct(property_t *prop, const void *data);

SSLOG_EXTERN int sslog_get_rtti_type(const void* entity);

/// @endcond


/// @cond INTERNAL_FUNCTIONS

int get_sbcr_status(individual_t *individual);
int set_sbcr_status(individual_t *individual, int status);

/// @endcond
/******************************************************************************/

#ifdef	__cplusplus
}
#endif

#endif	/* _STRUCTURES_H */
