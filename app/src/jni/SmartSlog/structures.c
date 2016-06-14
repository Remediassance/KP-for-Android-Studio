/**
 * @file structures.c
 * @Author Aleksandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Functions for work with C structures for OWL entities, KP and SS.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
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
 * Contains functions for work with C structures for OWL entities, KP and SS.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2009 - Aleksander A. Lomov. All rights reserved.
 */

#include "structures.h"
#include "properties.h"
#include "classes.h"
#include "repository.h"


/******************************************************************************/
/*************************** Static functions list ****************************/
static void remove_instance_from_class(class_t *ont_class, individual_t *individual);
static void free_data_property_value_data(prop_val_t *prop_val);
static void free_object_property_value_data(prop_val_t *prop_val);



/******************************************************************************/
/***************************** External functions *****************************/
/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Free property structure.
 *
 * Free all fields, subproperties are not freed, only the list structure and all nodes, data stay  untouched.
 *
 * @param property property for free.
 */
SSLOG_EXTERN void sslog_free_property(property_t *property)
{
    if (property == NULL) {
        return;
    }

    property->rtti = RTTI_MIN_VALUE;

    sslog_repo_remove_entity((void *) property);

    free(property->name);
	free(property->domain);
	free(property->about);

    property->name = NULL;
	property->domain = NULL;
	property->about = NULL;

    list_free_with_nodes(property->oneof, NULL);
    list_free_with_nodes(property->subpropertyof, NULL);

    property->oneof = NULL;
    property->subpropertyof = NULL;

    free(property);
}


/**
 * @brief Free property value struct.
 *
 * Property is not freed.
 *
 * @param prop_val property value structure.
 * @param free_data_func function for free property value or NULL.
 */
SSLOG_EXTERN void sslog_free_value_struct_with_func(prop_val_t *prop_val, void (*free_data_func)(void*))
{
    if (prop_val == NULL) {
        return;
    }

    if (free_data_func != NULL && prop_val->prop_value != NULL) {
        free_data_func(prop_val->prop_value);

    }
    
    prop_val->prop_value = NULL;
    prop_val->property = NULL;
    
    free(prop_val);
}


/**
 * @brief Free property value struct.
 *
 * It checks object or data property given and then free data or object.
 * Oject (individual) not freed if it has references from other individuals.
 * Property is not freed.
 * This function doesn't free value if  property not set, in this case only structure
 * (@see prop_val_t) will be freed.
 *
 * @param prop_val property value structure.
 */
SSLOG_EXTERN void sslog_free_value_struct(prop_val_t *prop_val)
{
    if (prop_val == NULL) {
        return;
    }

    if (prop_val->property == NULL) {
        free(prop_val);
        return;
    }

    if (prop_val->property->type == DATATYPEPROPERTY) {
        free_data_property_value_data(prop_val);
    } else if (prop_val->property->type == OBJECTPROPERTY) {
        free_object_property_value_data(prop_val);
    }

    free(prop_val);
}


/**
 * @brief Free property value struct.
 *
 * It checks object or data property given and then free only data-property.
 * Property is not freed.
 * This function doesn't free value if  property not set, in this case only structure
 * (@see prop_val_t) will be freed.
 *
 * @param prop_val property value structure.
 */
SSLOG_EXTERN void sslog_free_data_property_value_struct(prop_val_t *prop_val)
{
    if (prop_val == NULL) {
        return;
    }

    if (prop_val->property == NULL) {
        free(prop_val);
        return;
    }

    if (prop_val->property->type == DATATYPEPROPERTY) {
        free_data_property_value_data(prop_val);
    }
    
    free(prop_val);
}


/**
 * @brief Free class structure.
 *
 * Properties, instances and superclasses not freed, only list struct.
 *
 * @param ont_class class structure.
 */
SSLOG_EXTERN void sslog_free_class(class_t *ont_class)
{
    if (ont_class == NULL) {
        return;
    }

    ont_class->rtti = RTTI_MIN_VALUE;

    sslog_repo_remove_entity((void *) ont_class);

    free(ont_class->classtype);
    list_free_with_nodes(ont_class->instances,  (void (*)(void *)) sslog_free_individual);
    list_free_with_nodes(ont_class->oneof, NULL);
    list_free_with_nodes(ont_class->superclasses, NULL);
    list_free_with_nodes(ont_class->properties, NULL);

    free(ont_class);
}


// FIXME: free prop_value data field, need check object or data property.
// FIXME: remove individual from global repository if no references and from parent class list.
/**
 * @brief Free individual structure.
 *
 * Parent class not free.
 *
 * @param class class structure.
 */
SSLOG_EXTERN void sslog_free_individual(individual_t *individual)
{
    if (individual == NULL) {
        return;
    }

    if (sslog_repo_count_individual_references(individual) > 0) {
        //return;
    }

    // Remove this individual from other containers such as parent class and repository
    remove_instance_from_class((class_t *) individual->parent_class, individual);
    sslog_repo_remove_entity((void *) individual);

    individual->rtti = RTTI_MIN_VALUE;
    
    free(individual->uuid);
    free(individual->classtype);

    individual->classtype = NULL;
    individual->uuid = NULL;
    individual->parent_class = NULL;

    list_free_with_nodes(individual->properties, (void (*)(void *))  sslog_free_value_struct);
    individual->properties = NULL;

    free(individual);
    }


/**
 * @brief Gets RTTI.
 *
 * @param entity some entity, for example: individual, class or property.
 *
 * @return RTTI entity type on success or RTTI_MIN_VALUE (@see rtti_types) otherwise.
 */
SSLOG_EXTERN int sslog_get_rtti_type(const void* entity)
{
    if (entity == NULL) {
        return RTTI_MIN_VALUE;
    }

    int *rtti_type = (int *) entity;

    if (*rtti_type >= RTTI_MAX_VALUE
            || *rtti_type <= RTTI_MIN_VALUE) {
        return RTTI_MIN_VALUE;
    }

    return *rtti_type;
}

/// @endcond

/// @cond INTERNAL_FUNCTIONS


// TODO: return MIN or MAX as error
/**
 * @brief Gets subscription status of individuals.
 *
 * @param individual individuals to check.
 *
 * @return status (@see subscribe_status) or -1 otherwise.
 */
int get_sbcr_status(individual_t *individual)
{
    if (individual == NULL) {
        return -1;
    }

    return individual->subscribe_status;
}


/**
 * @brief Sets subscription status.
 *
 * @param individual individuals to set.
 * @param status subscription status (@see subscribe_status).
 *
 * @return 0 on success or -1 otherwise.
 */
int set_sbcr_status(individual_t *individual, int status)
{
    if (individual == NULL) {
        return -1;
    }

    if (status >= SBCR_STATUS_MAX || status <= SBCR_STATUS_MIN) {
        return -1;
    }

    individual->subscribe_status = status;

    return 0;
}

/// @endcond
/******************************************************************************/


/******************************************************************************/
/****************************** Static functions ******************************/
/**
 * @brief Remove individual from class instances list.
 *
 * @param ont_class class for search instance.
 * @param individual individual for remove.
 */
static void remove_instance_from_class(class_t *ont_class, individual_t *individual)
{
    if (ont_class == NULL || individual == NULL) {
        return;
    }

    if (list_is_null_or_empty(ont_class->instances) == 1) {
        return;
    }

    list_del_and_free_nodes_with_data(ont_class->instances, (void *) individual, NULL);
}


/**
 * @brief Free object property value.
 *
 * It free only data field.
 *
 * @param prop_val value of property.
 */
static void free_object_property_value_data(prop_val_t *prop_val)
{
    if (prop_val->prop_value == NULL) {
        return;
    }
/*
    individual_t *ind = (individual_t *) prop_val->prop_value;


    int count = sslog_repo_count_individual_references(ind);
    
    if (count == 0) {
        sslog_free_individual(ind);
    }
*/

    prop_val->prop_value = NULL;
}


/**
 * @brief Free data property value.
 *
 * It free only data field.
 *
 * @param prop_val value of property.
 */
static void free_data_property_value_data(prop_val_t *prop_val)
{
    if (prop_val->prop_value == NULL) {
        return;
    }

    free(prop_val->prop_value);

    prop_val->prop_value = NULL;
}
