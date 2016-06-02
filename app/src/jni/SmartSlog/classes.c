/**
 * @file classes.c
 * @Author Alexandr A. Lomov <lomov@cs.karelia.ru>
 * @Author Pavel I. Vanag <vanag@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Contains functions for work with classes.
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
 * Contains functions for work with classes.
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

#include <string.h>
#include "classes.h"
#include "structures.h"
#include "ss_func.h" // KPLIB_UUID_MAX_LEN
#include "utils/check_func.h"
#include "utils/kp_error.h"
#include "utils/util_func.h"
#include "utils/list.h"
#include "repository.h"

#ifndef _CLASSES_C
#define	_CLASSES_C


/******************************************************************************/
/*************************** Static functions list ****************************/
static void add_individual_to_class(class_t *ont_class, individual_t *individual);
static void add_individual_to_repository(individual_t *individual);


/******************************************************************************/
/**************************** External functions ******************************/
/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Create new individual.
 *
 * Create new individual without UUID.
 *
 * @param classtype type of class.
 *
 * @return individual of given class type on success or NULL otherwise.
 */
SSLOG_EXTERN individual_t* sslog_new_individual(const class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_individual START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

    int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_individual END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return NULL;
    }

    individual_t *individual = (individual_t *) malloc(sizeof(individual_t));

    if (individual == NULL) {
        set_error(SSLOG_ERROR_OUT_OF_MEMORY);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_individual END: error_code = %i (malloc returns NULL)\n", KPLIB_DEBUG_CLASSES_PREFIX, SSLOG_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    // Sets different fields of the new created individuals
    individual->rtti = RTTI_INDIVIDUAL;
    individual->parent_class = ont_class;
    individual->classtype = sslog_strndup(ont_class->classtype, SS_PREDICATE_MAX_LEN); // Classtype would be set as PREDIACTE during comminication so we inherit length
    individual->properties = NULL;
    individual->uuid = NULL; //sslog_generate_uuid();
    individual->subscribe_status = SBCR_STATUS_NO;

    error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        sslog_free_individual(individual);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_individual END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return NULL;
    }

    add_individual_to_class((class_t *) ont_class, individual);
    add_individual_to_repository(individual);

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_individual END: individual created (UUID: %s)\n", KPLIB_DEBUG_CLASSES_PREFIX, individual->uuid);

    return individual;
}


/**
 * @brief Returns a list of superclasses.
 *
 * @param class class.
 *
 * @return list of superclasses on success or NULL otherwise.
 */
SSLOG_EXTERN list_t* sslog_get_superclass_all(const class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_get_superclass_all START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

    int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_get_superclass_all END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return NULL;
    }

    if (ont_class->superclasses == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_get_superclass_all END: %s has no superclasses, NULL returned\n", KPLIB_DEBUG_CLASSES_PREFIX, ont_class->classtype);
        return NULL;
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_get_superclass_all END: superclasses of %s are returned\n", KPLIB_DEBUG_CLASSES_PREFIX, ont_class->classtype);

    return ont_class->superclasses;
}


/**
 * @brief Checks if class (named "subclass") is SUBCLASS (in ontology terms) of another class (named superclass).
 *
 * @param class class for check.
 * @param superclass alleged superclass of class.
 *
 * @return 1 if the class is a subclass of superclass, 0 if no and -1 on error.
 */
SSLOG_EXTERN int sslog_is_subclass_of(const class_t *subclass, const class_t *superclass)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_is_subclass_of START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

    int error_code = verify_class(subclass);

	if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_subclass_of END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return -1;
    }

	error_code = verify_class(superclass);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_subclass_of END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return -1;
    }

    if (subclass->superclasses == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_subclass_of END: NO %s IS NOT subclass of %s\n", KPLIB_DEBUG_CLASSES_PREFIX, subclass->classtype, superclass->classtype);
        return 0;
    }

    // Compare classatypes
    list_head_t *pos = NULL;
    list_for_each(pos, &subclass->superclasses->links) {
         list_t *list = list_entry(pos, list_t, links);
         class_t *test_class = (class_t *)list->data;

         if (strcmp(test_class->classtype, superclass->classtype) == 0) {
			KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_subclass_of END: YES %s IS subclass of %s\n", KPLIB_DEBUG_CLASSES_PREFIX, subclass->classtype, superclass->classtype);
            return 1;
        }
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_subclass_of END: NO %s IS NOT subclass of %s\n", KPLIB_DEBUG_CLASSES_PREFIX, subclass->classtype, superclass->classtype);

    return 0;
}


/**
 * @brief Checks is individual is instance of some class (classtype in ontology terms).
 *
 * @param individual object of some class for check.
 * @param ont_class allaged class of individual.
 *
 * @return returns 1 if the individual is object of given class, 0 if no and -1 on error.
 */
SSLOG_EXTERN int sslog_is_classtype_of(const individual_t *individual, const class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_is_classtype_of START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

	int error_code = verify_class(ont_class);
	
    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_classtype_of END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return -1;
    }

    error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_classtype_of END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return -1;
    }

    if (strcmp(ont_class->classtype, individual->classtype) == 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_is_classtype_of END: YES %s IS instance of %s\n", KPLIB_DEBUG_CLASSES_PREFIX, individual->classtype, ont_class->classtype);
        return 1;
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_is_classtype_of END: NO %s IS NOT instance of %s\n", KPLIB_DEBUG_CLASSES_PREFIX, individual->classtype, ont_class->classtype);

    return 0;
}


/**
 * @brief Create new property value structure.
 *
 * This function copy only value for data-property (i.e. char*), 
 * if property is object then value is pointer to given value (i.e. individual).
 *
 * @param prop property for value.
 * @param data data for value of property.
 *
 * @return new property value structure or NULL on error.
 */
SSLOG_EXTERN prop_val_t* sslog_new_value_struct(property_t *prop, const void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_value_struct START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

	int error_code = verify_property(prop);
	
    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_value_struct END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
        return NULL;
    }

    prop_val_t *value = (prop_val_t *) malloc(sizeof(prop_val_t));

	if (value == NULL) {
        set_error(SSLOG_ERROR_OUT_OF_MEMORY);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_value_struct END: error_code = %i (malloc returns NULL)\n", KPLIB_DEBUG_CLASSES_PREFIX, SSLOG_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

	value->property = prop;
    
    // If we create a value for data-property then we make copy. 
    if (sslog_is_object_property(prop) == true) {
        value->prop_value = (void *) data;
    } else {
        value->prop_value = sslog_strndup((const char *) data, SS_OBJECT_MAX_LEN);
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_value_struct END: new structure returned\n", KPLIB_DEBUG_CLASSES_PREFIX);

    return value;
}


/**
 * @brief Set UUID.
 *
 * Set new UUID for individuals. You need reinitialize individual to work with SS.
 *
 * @param ind individual.
 * @param uuid UUID.
 *
 * @return 1 on success or 0 on error.
 */
SSLOG_EXTERN int sslog_set_individual_uuid(individual_t *ind, const char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_individual_uuid START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(ind);

	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_individual_uuid END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
		set_error(error_code);
		return 0;
	}

	if (is_str_null_or_empty(uuid) == true) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_individual_uuid END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, SSLOG_ERROR_INCORRECT_UUID);
		set_error(SSLOG_ERROR_INCORRECT_UUID);
		return 0;
	}

	if (ind->uuid != NULL) {
		free(ind->uuid);
	}

	ind->uuid = sslog_strndup(uuid, KPLIB_UUID_MAX_LEN);  

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_individual_uuid END: UUID successfully set\n", KPLIB_DEBUG_CLASSES_PREFIX);

	return 1;
}

// FIXME: need get all classes, now only parent.
/**
 * @brief Gets all inheritance classes.
 *
 * @param individual individual for get parents.
 *
 * @return returns list with classes on success or NULL otherwise.
 */
SSLOG_EXTERN list_t* get_individual_inheritance_classes(individual_t *individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s get_individual_inheritance_classes START\n", KPLIB_DEBUG_CLASSES_PREFIX);

	reset_error();

	int error_code = verify_individual(individual);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s get_individual_inheritance_classes END: error_code = %i\n", KPLIB_DEBUG_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return NULL;
    }

    const class_t *parent = individual->parent_class;

    list_t *classes = list_get_new_list();
	
	if (classes == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s get_individual_inheritance_classes END: list_get_new_list return NULL\n", KPLIB_DEBUG_CLASSES_PREFIX);
		set_error(SSLOG_ERROR_OUT_OF_MEMORY);
        return NULL;
    }

    list_add_data((void *)parent, classes);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s get_individual_inheritance_classes END: classes composed and returned\n", KPLIB_DEBUG_CLASSES_PREFIX);

	return classes;
}
/// @endcond


/******************************************************************************/
/***************************** Static functions *******************************/
/**
 * @brief Add individual to instances list for given class.
 *
 * @param ont_class class.
 * @param individual individual;
 */
static void add_individual_to_class(class_t *ont_class, individual_t *individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW)
		("\n%s add_individual_to_class START\n", KPLIB_DEBUG_CLASSES_PREFIX);

    if (ont_class == NULL || individual == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW)
			("\n%s add_individual_to_class END: ont_class or individual is NULL\n", KPLIB_DEBUG_CLASSES_PREFIX);
        return;
    }

    ont_class->instances = list_get_new_list_if_null(ont_class->instances);

    if (ont_class->instances == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW)
			("\n%s add_individual_to_class END: list_get_new_list_if_null returns NULL\n", KPLIB_DEBUG_CLASSES_PREFIX);
        return;
    }

    list_add_data((void *) individual, ont_class->instances);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW)
		("\n%s add_individual_to_class END: individual is successfully added\n", KPLIB_DEBUG_CLASSES_PREFIX);
}


/**
 * @brief Add individual to repository.
 *
 * It used when new individual create.
 *
 * @param individual individual for insert.
 */
static void add_individual_to_repository(individual_t *individual)
{
    if (individual == NULL) {
        return;
    }

    list_t *repo = get_global_repository_by_rtti(RTTI_INDIVIDUAL);

    list_add_data(individual, repo);
}



#endif	/* _CLASSES_C */
