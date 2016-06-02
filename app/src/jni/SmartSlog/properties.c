/**
 * @file properties.c
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   11 December, 2010
 * @brief  Functions for work with properties.
 *
 *
 * @section LICENSE
 *
 * PetrSU KP library is free software; you can redistribute it and/or modify
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
 * Contains functions for work with properties.
 * This file is part of SmartSlog KP Library.
 */

#include <string.h>
#include "structures.h"

#ifdef MTENABLE
#include <pthread.h>
#endif

#include "properties.h"
#include "utils/list.h"
#include "utils/util_func.h"

/******************************************************************************/
/************************ Static and global entities **************************/
/// @cond INTERNAL_STRUCTURES
/** @brief Mutex for functions: only one function (set/get/unset/update) can work */
#ifdef MTENABLE
#if defined(WIN32) || defined (WINCE)
static pthread_mutex_t g_func_work = PTHREAD_MUTEX_INITIALIZER;
//PTHREAD_RECURSIVE_MUTEX_INITIALIZER; 
//PTHREAD_MUTEX_INITIALIZER;
#else
static pthread_mutex_t g_func_work = PTHREAD_MUTEX_INITIALIZER;
#endif
#endif
/// @endcond


/******************************************************************************/
/***************************** Static functions *******************************/
static int set_property_for_class(class_t *ont_class, property_t *propname, const void *data);
static int set_property_for_individual(individual_t *ind, property_t *propname, const void *data);
static int set_property_for_pattern(pattern_t *p, property_t *propname, const void *data);

static prop_val_t* get_property(individual_t *ind, const char *propname);
static prop_val_t* get_property_value_by_value(individual_t *ind, prop_val_t *prop_value);
static list_t* get_property_node_by_value(individual_t *ind, prop_val_t *prop_value);

static int update_property_for_individual(individual_t *ind, const char *propname, \
        const void* old_data, const void *new_data);


static list_t* find_properties_values_by_name(individual_t *ind, const char *propname,\
        bool only_one);

static int verify_value_property(const property_t *prop, const void *data);


static void free_property_value_data(prop_val_t *prop_val);
int sslog_update_value_struct(prop_val_t* prop_value, void* new_data);

 
static int check_cardinality_for_unset(individual_t *ind, prop_val_t *prop_val);


/******************************************************************************/
/**************************** External functions ******************************/
/// @cond EXTERNAL_FUNCTIONS
int set_property_by_name(individual_t *ind, char *propname, void *value)
{
       property_t *prop = (property_t *) sslog_get_property_type(ind->parent_class,
            propname);
       
       return sslog_add_property((void *) ind, prop, value);
}


/**
 * @brief Set new property for entity.
 *
 * Sets property for one individual or whole class (not implemented yet).
 * Create new value from given if setting data property, or assigns given
 * value(individual) to property.
 *
 * @param ind individual or class.
 * @param property property.
 * @param value value to set.
 *
 * @return SSLOG_ERROR_NO on success or not otherwise.
 */
SSLOG_EXTERN int sslog_add_property(void *entity, property_t *prop, const void *value)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property START", \
                KPLIB_DEBUG_PROP_PREFIX);

    if (entity == NULL
            || verify_property(prop) != SSLOG_ERROR_NO) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property END: "\
                "No entity or property is incorrect.", KPLIB_DEBUG_PROP_PREFIX);
        return SSLOG_ERROR_INCORRECT_ARGUMENT;
    }

#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif

    int rtti_type = sslog_get_rtti_type(entity);

    int error_code = SSLOG_ERROR_NO;
    switch (rtti_type) {
        case RTTI_CLASS: {
            error_code = set_property_for_class((class_t *) entity, prop, value);
            break;
        }
        case RTTI_INDIVIDUAL: {
            error_code = set_property_for_individual((individual_t *) entity, prop, value);
            break;
        }
        case RTTI_PATTERN: {
            error_code = set_property_for_pattern((pattern_t *) entity, prop, value);
            break;
        }
        default: {
            error_code = SSLOG_ERROR_INCORRECT_ENTITY;
        }
    }

#ifdef MTENABLE
    pthread_mutex_unlock(&g_func_work);
#endif	
	
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_by_name END", \
        KPLIB_DEBUG_PROP_PREFIX);
    return error_code;
}



/**
 * @brief Set property with ginen prop_val_t struct.
 *
 * Set property for one individuals or for all individuals from class.
 *
 * @param entity class or individual.
 * @param prop_val value to set.
 *
 * @return 0 on success or not otherwise.
 */
int set_property_with_prop_val(void *entity, const prop_val_t *prop_val)
{
    return sslog_add_property(entity, prop_val->property, prop_val->prop_value);
}


// TODO: call internal function: get_property
/**
 * @brief Gets first found property by name.
 *
 * @param ind individual.
 * @param propname name of property.
 *
 * @return property value structure on success or NULL if there is no such property.
 */
SSLOG_EXTERN const prop_val_t* sslog_get_property(individual_t *ind, property_t *prop)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property START", \
        KPLIB_DEBUG_PROP_PREFIX);

#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif

	
	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: wrong property", \
			    KPLIB_DEBUG_PROP_PREFIX);
		return NULL;
	}

    list_t *list = find_properties_values_by_name(ind, prop->name, true);

    if (list == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);

#ifdef MTENABLE
			pthread_mutex_unlock(&g_func_work);
#endif			

        return NULL;
    }

    // 
    list_t* node = list_get_first_prev_node(list);

    if (node == NULL) {
        list_free_with_nodes(list, NULL);
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);

#ifdef MTENABLE			
			pthread_mutex_unlock(&g_func_work);
#endif
			
        return NULL;
    }

    // Gets data from node (first found property value) and remove list.
    void *data = node->data;
    list_free_with_nodes(list, NULL);

    if (data == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);
			
#ifdef MTENABLE			
			pthread_mutex_unlock(&g_func_work);
#endif
			
        return NULL;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END", \
        KPLIB_DEBUG_PROP_PREFIX);

#ifdef MTENABLE		
    pthread_mutex_unlock(&g_func_work);
#endif
    return (prop_val_t *) data;
}


/**
 * @brief Gets firts foundnode of properties list with given property name.
 *
 * @param ind individual.
 * @param propname name of property.
 *
 * @return node of list on success or NULL otherwise.
 */
const list_t* get_property_node(individual_t *ind, const char *propname)
{
    // Gets all properties with given name
    list_t *list = find_properties_values_by_name(ind, propname, true);

    if (list == NULL) {
        return NULL;
    }

    // Gets first and then return it.
    list_t* node = list_get_first_prev_node(list);

    if (node == NULL) {
        return NULL;
    }
    
    list_del(&node->links);
    list_free_with_nodes(list, NULL);
    
    return node;
}


/**
 * @brief Get property value with given data.
 *
 * @param ind individual.
 * @param propname name of property.
 * @param data value of property.
 *
 * @return property value struct on success or NULL otherwise.
 */
SSLOG_EXTERN const prop_val_t* sslog_get_property_by_value(individual_t *ind,
        property_t *prop, void *data)
{
#ifdef MTENABLE    
	pthread_mutex_lock(&g_func_work);
#endif
	
	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		return NULL;
	}

    property_t *prop_type = (property_t *) sslog_get_property_type(
            ind->parent_class, prop->name);
    prop_val_t *test_prop_val = sslog_new_value_struct(prop_type, data);
    prop_val_t *prop_val = get_property_value_by_value(ind, test_prop_val);

    sslog_free_value_struct_with_func(test_prop_val, NULL);
 
#ifdef MTENABLE 
    pthread_mutex_unlock(&g_func_work);
#endif
	
    return prop_val;
}


/**
 * @brief Get all properties with given name.
 *
 * @param ind individual.
 * @param propname name of property.
 *
 * @return properties values list on success or NULL otherwise.
 */
SSLOG_EXTERN list_t* sslog_get_property_all(individual_t *ind, property_t *prop)
{
#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif

	if (prop == NULL || prop->name == NULL) {
		set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
		return NULL;
	}

    list_t *properties = find_properties_values_by_name(ind, prop->name, false);

#ifdef MTENABLE	
    pthread_mutex_unlock(&g_func_work);
#endif	

    return properties;
}


/**
 * @brief Update property with given name.
 *
 * @param ind individual.
 * @param propname name of property.
 * @param new_data value of property.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_update_property(void *entity, const char *propname, void *new_data)
{
    return sslog_update_property_with_value(entity, propname, NULL, new_data);
}


/**
 * @brief Update property using old value.
 *
 * @param ind individual.
 * @param propname name of property.
 * @param old_data old value of property.
 * @param new_data new value of property.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_update_property_with_value(void *entity, const char *propname,
        const void *old_data, void *new_data)
{
    int rtti_type = sslog_get_rtti_type(entity);

    if (rtti_type == -1) {
        return -1;
    }
	
#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif
    int error_code = 0;
    switch(rtti_type) {
        case RTTI_INDIVIDUAL: {
            error_code = update_property_for_individual(
                    (individual_t *) entity, propname, old_data, new_data);
            break;
        }
        case RTTI_CLASS: {
            // No code, maybe write later, if needed.
            break;
        }
    }
	
#ifdef MTENABLE
    pthread_mutex_unlock(&g_func_work);
#endif    
	return error_code;
}


/**
 * @brief Update property value.
 *
 * @param prop_value value of property.
 * @param new_data new value of property.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_update_value_struct(prop_val_t* prop_value, void* new_data)
{
    if (prop_value == NULL) {
        return SSLOG_ERROR_INCORRECT_PROPERTY_VALUE;
    }

    // Checks value for given propertie.
    if (verify_value_property(prop_value->property, new_data) != 0) {
        return SSLOG_ERROR_INCORRECT_PROPERTY_VALUE;
    }

    // Remove data from value.
    free_property_value_data(prop_value);

    // Set new data.
    if (sslog_is_object_property(prop_value->property) == true) {
        prop_value->prop_value = new_data;
    } else {
		prop_value->prop_value = sslog_strndup((const char *) new_data, SS_SUBJECT_MAX_LEN); //inherit length from KPI library
    }

    return SSLOG_ERROR_NO;
}



SSLOG_EXTERN int sslog_remove_property(individual_t *ind, property_t *prop)
{
   KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s remove_property START", \
            KPLIB_DEBUG_CLASSES_PREFIX);
    reset_error();

    if (verify_individual(ind) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property END: incorrect individual", \
            KPLIB_DEBUG_CLASSES_PREFIX);
        return set_error(SSLOG_ERROR_INCORRECT_INDIVIDUAL);
    }

    if (verify_property(prop) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property END: incorrect property", \
            KPLIB_DEBUG_CLASSES_PREFIX);
        return set_error(SSLOG_ERROR_INCORRECT_PROPERTY);
    }

    list_t *prop_node = NULL;

#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif
	
	// TODO: Remove or modify else
    if (1) { 
        prop_node = (list_t *) get_property_node(ind, prop->name);
    } else {
        prop_val_t *old_prop_value = NULL;//sslog_new_value_struct(prop_type, (void *) data);

        prop_node = get_property_node_by_value(ind, old_prop_value);
        sslog_free_value_struct_with_func(old_prop_value, NULL);
    }

    if (prop_node == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property END: no property", \
            KPLIB_DEBUG_CLASSES_PREFIX);
#ifdef MTENABLE        
		pthread_mutex_unlock(&g_func_work);
#endif
        return 0;
    }

    // It's impossible to remove property 
    // when count of properties less than minimal cardinality.
    if (check_cardinality_for_unset(ind, (prop_val_t *) prop_node->data) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property END: min cardinality error", \
            KPLIB_DEBUG_CLASSES_PREFIX);
#ifdef MTENABLE
			pthread_mutex_unlock(&g_func_work);
#endif			
        return set_error(SSLOG_ERROR_MINCARDINALITY_VIOLATED);
    }

    //list_del(&prop_node->links);
    list_del_and_free_nodes_with_data(ind->properties, prop_node->data, 
            (void (*)(void*)) sslog_free_data_property_value_struct);
    //free_property_value_data((prop_val_t *) prop_node->data);
    list_free_node(prop_node, NULL);
    
#ifdef MTENABLE
    pthread_mutex_unlock(&g_func_work);
#endif	
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s remove_property END", \
                KPLIB_DEBUG_CLASSES_PREFIX);
    return 0;
}


/**
 * @brief Unset property value.
 *
 * @param ind individual.
 * @param propname name of property.
 * @param data value of property.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_remove_property_with_value(individual_t *ind, property_t *prop, void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s remove_property_with_data START", \
                KPLIB_DEBUG_CLASSES_PREFIX);
    reset_error();
    
    if (verify_individual(ind) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property_with_data END: incorrect individual", \
            KPLIB_DEBUG_CLASSES_PREFIX);

        return set_error(SSLOG_ERROR_INCORRECT_INDIVIDUAL);
    }

    //if (get_sbcr_status(ind) == SBCR_STATUS_SUBSCRIBED) {

    if (verify_property(prop) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property_with_data END: incorrect property", \
            KPLIB_DEBUG_CLASSES_PREFIX);

        return set_error(SSLOG_ERROR_INCORRECT_PROPERTY);
    }

    list_t *prop_node = NULL;
#ifdef MTENABLE
    pthread_mutex_lock(&g_func_work);
#endif
	
    if (data == NULL) {
        prop_node = (list_t *) get_property_node(ind, prop->name);
    } else {
        property_t *prop_type = (property_t *) sslog_get_property_type(ind->parent_class, prop->name);
        prop_val_t *old_prop_value = sslog_new_value_struct(prop_type, (void *) data);
        
        prop_node = get_property_node_by_value(ind, old_prop_value);
        // TODO: !!!CHECK!!!
        sslog_free_value_struct(old_prop_value);
        //sslog_free_value_struct_with_func(old_prop_value, NULL);
    }

    if (prop_node == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s remove_property_with_data END: no property", \
            KPLIB_DEBUG_CLASSES_PREFIX);
#ifdef MTENABLE
			pthread_mutex_unlock(&g_func_work);
#endif			
        return 0;
    }

    // It's impossible to remove property 
    // when count of properties less than minimal cardinality.
    if (check_cardinality_for_unset(ind, (prop_val_t *) prop_node->data) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) \
            ("\n%s unset_property_for_individual END: min cardinality error", \
            KPLIB_DEBUG_CLASSES_PREFIX);
#ifdef MTENABLE
			pthread_mutex_unlock(&g_func_work);
#endif			
        return set_error(SSLOG_ERROR_MINCARDINALITY_VIOLATED);
    }

    list_del_and_free_node(prop_node, (void (*)(void *)) sslog_free_value_struct);
    //list_del(&prop_node->links);
    //free_property_value_data((prop_val_t *) prop_node->data);
    //list_free_node(prop_node, sslog_free_value_struct);
#ifdef MTENABLE
    pthread_mutex_unlock(&g_func_work);
#endif	
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s unset_property_for_individual END", \
                KPLIB_DEBUG_CLASSES_PREFIX);
    return 0;
}


/**
 * @brief Get property struct by given name.
 *
 * @param ont_class class.
 * @param propname name of property.
 *
 * @return property struct on success or NULL otherwise.
 */
SSLOG_EXTERN const property_t* sslog_get_property_type(const class_t *ont_class, const char *propname)
{
    reset_error();
    int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
        set_error(error_code);
        return NULL;
    }

    if (is_str_null_or_empty(propname) == true
            || error_code != 0) {
        set_error(SSLOG_ERROR_INCORRECT_PROPERTY_NAME);
        return NULL;
    }

    if (ont_class->properties == NULL) {
        set_error(SSLOG_ERROR_CANT_FIND_PROPERTY);
        return NULL;
    }

    // Checking ontology class properties to equality with given name. 
    list_head_t *pos = NULL;
    list_for_each(pos, &ont_class->properties->links) {
         list_t *list = list_entry(pos, list_t, links);
         property_t *test_prop = (property_t *)list->data;

         if (strcmp(test_prop->name, propname) == 0) {
            return test_prop;
        }
    }

    set_error(SSLOG_ERROR_CANT_FIND_PROPERTY);
    return NULL;
}

// TODO: remove it or implement
/**
 * @brief Checks property for individual. (Not implemented)
 *
 * @param individual individual.
 * @param prop property.
 *
 * @return true if individual has property, otherwise false.
 */
bool is_individual_has_property(individual_t *individual, property_t *prop)
{
    return false;
}



SSLOG_EXTERN bool sslog_is_object_property(property_t *prop)
{
    if (prop == NULL) {
        return false;
    }

    if (prop->type == OBJECTPROPERTY) {
        return true;
    }

    return false;
}

/// @endcond


/// @cond INTERNAL_FUNCTIONS
/**
 * @brief Checks equality of given properties values.
 *
 * It checks names, about field, types and values. 
 *
 * @param a one property value.
 * @param b another property value.
 *
 * @return true if equals or false otherwise.
 */
bool is_prop_values_equal(prop_val_t *a, prop_val_t *b)
{
    if (a == NULL && b == NULL) {
        return true;
    } else if (a == NULL || b == NULL) {
        return false;
    }

    property_t *prop_a = a->property;
    property_t *prop_b = b->property;

    if (strcmp(prop_a->name, prop_b->name) != 0 ) {
        return false;
    }

    if (strcmp(prop_a->about, prop_b->about) != 0) {
        return false;
    }

    if (prop_a->type != prop_b->type) {
        return false;
    }

    if (a->prop_value == NULL && b->prop_value == NULL) {
        return true;
    } else if (a->prop_value == NULL || b->prop_value == NULL) {
        return false;
    }

    if (prop_a->type == DATATYPEPROPERTY
            && strcmp((const char *) a->prop_value, (const char *) b->prop_value) == 0) {
        return true;
    }

    if (prop_a->type == OBJECTPROPERTY
            && a->prop_value == b->prop_value) {
        return true;
    }
    return false;
}


// TODO: rename it, the name is incorrect
/**
 * @brief Checks equality of properties names of the given properties values.
 *
 * It checks names, about field and types.
 *
 * @param a one property value.
 * @param b another property value.
 *
 * @return true if equals or false otherwise.
 */
bool is_prop_names_equal(prop_val_t *a, prop_val_t *b)
{
	if (a == NULL && b == NULL) {
		return true;
	} else if (a == NULL || b == NULL) {
		return false;
	}

    property_t *prop_a = a->property;
	property_t *prop_b = b->property;

	if (strcmp(prop_a->name, prop_b->name) != 0 ) {
		return false;
	}

    if (strcmp(prop_a->about, prop_b->about) != 0) {
		return false;
	}

    if (prop_a->type != prop_b->type) {
		return false;
	}

        return true;
}
/// @endcond



/*****************************************************************************/
/***************************** Static functions *******************************/
// FIXME: is_prop_values_equal need add is_properties_equal
/**
 * @brief Count how many properties has individual by given property value struct.
 *
 * Checks only properties pointers equality, data is not checked.
 *
 * @param individual individual.
 * @param prop_val property value.
 *
 * @return count of properties.
 */
static int count_properties_by_prop_value(individual_t *ind, prop_val_t * prop_val)
{
    if (ind == NULL || prop_val == NULL) {
        return -1;
    }

    int prop_count = 0;
    
    list_head_t *list_walker = NULL;
    list_for_each (list_walker, &ind->properties->links) {
         list_t *list = list_entry(list_walker, list_t, links);
         prop_val_t *test_prop = (prop_val_t *)list->data;

         if (test_prop == NULL) {
             continue;
         }

         if (prop_val->property == test_prop->property) { //(is_prop_values_equal(prop_val, test_prop) == true) {
            ++prop_count;
        }
    }

    return prop_count;
}


/**
 * @brief Checks cardinality when property set.
 *
 * @param individual individual.
 * @param prop_val property value.
 *
 * @return SSLOG_ERROR_NO on success or SSLOG_ERROR_MAXCARDINALITY_VIOLATED if cardinality 
 * will be violated after setting new property .
 */
static int check_cardinality_for_set(individual_t *ind, prop_val_t *prop_val)
{
    if (ind == NULL || prop_val == NULL) {
        return SSLOG_ERROR_INCORRECT_ARGUMENT;
    }

    // Checks the amount of properties with maximal cardinality.
    int prop_count = count_properties_by_prop_value(ind, prop_val);

    int cardinality = prop_val->property->maxcardinality;
    
    if (cardinality < 0) {
        return SSLOG_ERROR_NO;
    }

    if (prop_count >= cardinality) {
        return SSLOG_ERROR_MAXCARDINALITY_VIOLATED;
    }

    return SSLOG_ERROR_NO;
}


/**
 * @brief Checks cardinality when property unset.
 *
 * @param individual individual.
 * @param prop_val property value.
 *
 * @return 0 on success or -1 if cardinality will be violated after removing 
 * property.
 */
static int check_cardinality_for_unset(individual_t *ind, prop_val_t *prop_val)
{
    if (ind == NULL || prop_val == NULL) {
        return -1;
    }

    int prop_count = count_properties_by_prop_value(ind, prop_val);

    int cardinality = prop_val->property->mincardinality;

    if (cardinality < 0) {
        return 0;
    }

    if (prop_count <= cardinality) {
        return -1;
    }

    return 0;
}



/**
 * @brief Set property for individual.
 *
 * @param individual individual.
 * @param propname property name.
 * @param data data for set.
 *
 * @return 0 on success or not otherwise.
 */
static int set_property_for_individual(individual_t *ind,
        property_t *prop, const void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_individual START", \
                KPLIB_DEBUG_PROP_PREFIX);

    if (verify_value_property(prop, data) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_individual END: "\
                "property not verifed", KPLIB_DEBUG_PROP_PREFIX);

        return SSLOG_ERROR_INCORRECT_PROPERTY_VALUE;
    }

    if (ind->properties == NULL) {
        ind->properties = list_get_new_list();
    }

    prop_val_t* value = sslog_new_value_struct(prop, (void *) data);

    if (check_cardinality_for_set(ind, value) == SSLOG_ERROR_NO) {
        list_add_data(value, ind->properties);
    } else {

        sslog_free_data_property_value_struct(value);
        
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_individual END: "\
                "cardinality error", KPLIB_DEBUG_PROP_PREFIX);
        
        return SSLOG_ERROR_MAXCARDINALITY_VIOLATED;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_individual END", \
                KPLIB_DEBUG_PROP_PREFIX);

    return SSLOG_ERROR_NO;
}



/**
 * @brief Set property for pattern.
 *
 * @param pattern pattern.
 * @param propname property name.
 * @param data data for set.
 *
 * @return 0 on success or not otherwise.
 */
static int set_property_for_pattern(pattern_t *p, property_t *prop, const void *data)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_pattern START", \
                KPLIB_DEBUG_PROP_PREFIX);

    if (verify_value_property(prop, data) != 0) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_pattern END: "\
                "property not verifed", KPLIB_DEBUG_PROP_PREFIX);

        return SSLOG_ERROR_INCORRECT_PROPERTY_VALUE;
    }

    if (p->check_prop_list == NULL) {
        p->check_prop_list = list_get_new_list();
    }

    prop_val_t* value = sslog_new_value_struct(prop, (void *) data);

    list_add_data(value, p->check_prop_list);

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s set_property_for_pattern END", \
                KPLIB_DEBUG_PROP_PREFIX);

    return SSLOG_ERROR_NO;
}


// TODO: Remove it!
/**
 * @brief Set property for class. (Not implemented)
 *
 * Set property for all individuals, owned by given class.
 *
 * @param class class.
 * @param propname property name.
 * @param data data for set.
 *
 * @return 0 on success or not otherwise.
 */
static int set_property_for_class(class_t *ind,
        property_t *prop, const void *data)
{
    return SSLOG_ERROR_NOT_IMPLEMENTED;
}


/**
 * @brief Gets first found property by name.
 *
 * @param ind individual.
 * @param propname name of property.
 *
 * @return struct of the property value on success or null if there is no such property.
 */
static prop_val_t* get_property(individual_t *ind, const char *propname)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property START", \
        KPLIB_DEBUG_PROP_PREFIX);

    list_t *list = find_properties_values_by_name(ind, propname, true);

    if (list == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);

        return NULL;
    }

    // 
    list_t* node = list_get_first_prev_node(list);

    if (node == NULL) {
        list_free_with_nodes(list, NULL);
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);
        return NULL;
    }

    // Gets data from node (first found property value) and remove list.
    void *data = node->data;
    list_free_with_nodes(list, NULL);

    if (data == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END: no property", \
            KPLIB_DEBUG_PROP_PREFIX);
        return NULL;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED) ("\n%s get_property END", \
        KPLIB_DEBUG_PROP_PREFIX);

    return (prop_val_t *) data;
}


/**
 * @brief Gets property value by given value.
 *
 * @param ind individual.
 * @param prop_value property value.
 *
 * @return property value on success or NULL otherwise.
 */
static prop_val_t* get_property_value_by_value(individual_t *ind, prop_val_t *prop_value)
{
    if (ind == NULL || prop_value == NULL) {
        return NULL;
    }

    if (list_is_null_or_empty(ind->properties) == 1) {
        return NULL;
    }

    list_head_t *pos = NULL;
    list_for_each(pos, &ind->properties->links) {
         list_t *list = list_entry(pos, list_t, links);
         prop_val_t *test_prop = (prop_val_t *)list->data;

         if (is_prop_values_equal(prop_value, test_prop) == true) {

            return test_prop;
        }
    }

    return NULL;
}


/**
 * @brief Gets property value list (node from list_t) node by given property value.
 *
 * @param ind individual.
 * @param prop_value property value.
 *
 * @return list of node on success or NULL otherwise.
 */
static list_t* get_property_node_by_value(individual_t *ind, prop_val_t *prop_value)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s get_property_node_by_value START", \
            KPLIB_DEBUG_PROP_PREFIX);

    if (list_is_null_or_empty(ind->properties) == 1 || prop_value == NULL) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s get_property_node_by_value END: " \
            "no properties or value not set", KPLIB_DEBUG_PROP_PREFIX);
        return NULL;
    }
    
    list_head_t *pos = NULL;

    list_for_each(pos, &ind->properties->links) {
         list_t *list = list_entry(pos, list_t, links);
         
         prop_val_t *test_prop = (prop_val_t *)list->data;

         if (is_prop_values_equal(prop_value, test_prop) == true) {
             KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s get_property_node_by_value END", \
                KPLIB_DEBUG_PROP_PREFIX);

            return list;
        }
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s get_property_node_by_value END: " \
            "no nodes", KPLIB_DEBUG_PROP_PREFIX);

    return NULL;
}


/**
 * @brief Gets properties values with properties with given property name.
 *
 * @param ind individual.
 * @param propname property name.
 * @param only_one if true gets only one firs property value.
 *
 * @return properties values list on success or NULL otherwise.
 */
static list_t* find_properties_values_by_name(individual_t *ind, const char *propname, bool only_one)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s find_properties_values_by_name START", \
        KPLIB_DEBUG_PROP_PREFIX);

     if (is_str_null_or_empty(propname) == true
            || verify_individual(ind) != 0 ) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s find_properties_values_by_name END: " \
        "invalid parameters", KPLIB_DEBUG_PROP_PREFIX);

        return NULL;
    }

    if (list_is_null_or_empty(ind->properties) == 1) {
        KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s find_properties_values_by_name END: " \
            "no properties", KPLIB_DEBUG_PROP_PREFIX);

        return NULL;
    }

    list_head_t *pos = NULL;
    list_t *prop_list = list_get_new_list();
    bool is_prop_find = false;

    // Find values for properties with given name.
    list_for_each(pos, &ind->properties->links) {
         list_t *list = list_entry(pos, list_t, links);
         prop_val_t *prop_val = (prop_val_t *)list->data;

         if (strcmp(prop_val->property->name, propname) == 0) {
            list_add_data(prop_val, prop_list);
            is_prop_find = true;
        }

         if (is_prop_find == true && only_one == true) {
             break;
         }
    }

    if (list_empty(&prop_list->links) == 1) {
        list_free(prop_list);
        prop_list = NULL;
    }

    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_LOW) ("\n%s find_properties_values_by_name END", \
                KPLIB_DEBUG_PROP_PREFIX);

    return prop_list;
}


/* FIXME: Add part for check property and individual, add free functions.
 * remove old data or return it.
 */
/**
 * @brief Update property.
 *
 * Find property with given old data and update it by new data. 
 * If old data is not set it update first find value with given name.
 *
 * @param ind individual.
 * @param propname property name.
 * @param old_data old property value data.
 * @param new_data new property value data.
 *
 * @return properties values list on success or NULL otherwise.
 */
static int update_property_for_individual(individual_t *ind,
        const char *propname, const void *old_data, const void *new_data)
{
    prop_val_t *prop_value = NULL;

    // Find first property with given name or find value with given data.
    if (old_data == NULL) {
        prop_value = (prop_val_t *) get_property(ind, propname);
    } else {
        property_t *prop_type = (property_t *) sslog_get_property_type(ind->parent_class, propname);
        prop_val_t *old_prop_value = sslog_new_value_struct(prop_type, (void *) old_data);
        prop_value = get_property_value_by_value(ind, old_prop_value);

        //sslog_free_value_struct_with_func(old_prop_value, (void(*)(void*)) sslog_free_data_property_value_struct);
        sslog_free_data_property_value_struct(old_prop_value);
    }
    
    return sslog_update_value_struct(prop_value, (void *) new_data);
}

// TODO: remove it, use function from structures.
/**
 * @brief Free property value.
 *
 * @param prop_val property value.
 */
static void free_property_value_data(prop_val_t *prop_val)
{
    if (prop_val == NULL) {
        return;
    }

    if (prop_val->property == NULL) {
        return;
    }

    if (prop_val->property->type == DATATYPEPROPERTY) {
        free(prop_val->prop_value);
        prop_val->prop_value = NULL;
    } else if (prop_val->property->type == OBJECTPROPERTY) {
        prop_val->prop_value = NULL;
    }
}


/**
 * @brief Checks data for property.
 *
 * Can be given data set for given property.
 *
 * @param property property.
 * @param data data for check.
 *
 * @return 0 on success or -1 otherwise.
 */
static int verify_value_property(const property_t *prop, const void *data)
{
    if (prop == NULL) {
        return -1;
    }

    // The data for object property must be an individual.
    if (prop->type == OBJECTPROPERTY) {
	    if (data != NULL)
	    {
	            int *rtti_type = (int *) data;

	            if (*rtti_type != RTTI_INDIVIDUAL) {
	                return -1;
	            }
	    }
    } else if (prop->type != DATATYPEPROPERTY) {
        return -1;
    }

    return 0;
}


/**
 * @brief Checks property value - depricated.
 *
 * @param prop_value property value.
 *
 * @return 0 on success or -1 otherwise.
 */
/* Not used

static int verify_property_value_struct(const prop_val_t *prop_val)
{
    if (prop_val == NULL) {
        return -1;
    }

    property_t *prop = prop_val->property;
    void *data = prop_val->prop_value;

    return verify_value_property(prop, data);
}

*/

