/**
 * @file patterns.c
 * @Author Pavel I. Vanag <vanag@cs.karelia.ru>
 * @Author Alexandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Contains functions for work with knowledge patterns.
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
 * Contains functions for work with knowledge patterns.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2009 - SmartSlog. All rights reserved.
 *
 */


#ifndef _PATTERNS_C
#define	_PATTERNS_C

#include "patterns.h"
#include "properties.h"
#include "kpi_interface.h"
#include "ss_properties.h"

static void filter_list_for_property(list_t* inds, pattern_condition_t *cond);
static bool condition_type_check(int condition);


/**
 * @brief depricated function
 * Create and prepare new condition pattern.
 *
 * @param property_t *prop condition property.
 * @param void *value valud of condition property.
 * @param int condition condition type.
 *
 * @return new condition pattern.
 */
pattern_condition_t* new_pattern_condition(property_t *prop, void *value, int condition)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s new_pattern_condition START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

    pattern_condition_t *cond = (pattern_condition_t *) malloc(sizeof(pattern_condition_t));

	if (cond == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s new_pattern_condition END: error_code = %i (malloc return NULL)\n", KPLIB_DEBUG_PATTERNS_PREFIX, SSLOG_ERROR_OUT_OF_MEMORY);
		set_error(SSLOG_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

	cond->rtti = RTTI_PATTERN_CONDITION;
	cond->prop = prop;
	cond->value = NULL;

	if (value != NULL)
	{
		if (sslog_is_object_property(cond->prop) == true) {
			cond->value = value;
		} else if (value != NULL) {
			cond->value = sslog_strndup((const char *) value, SS_SUBJECT_MAX_LEN);
		}
	}

	cond->condition = condition;

	return cond;
}

/**
  * @brief depricated function
  */
void free_pattern_condition(pattern_condition_t *cond)
{
    if (cond == NULL) {
        return;
    }

    if (sslog_is_object_property(cond->prop) == true) {
        cond->value = NULL;
    } else if (cond->value != NULL) {
        free(cond->value);
    }

    cond->rtti = RTTI_MIN_VALUE;

    free(cond);
}

/**
 * @fn pattern_t* sslog_new_pattern(class_t *ont_class, const char *uuid)
 *
 * @brief Create and prepare new pattern.
 *
 * @param ont_class class of a pattern.
 * @param uuid ientifier fro pattern.
 *
 * @return new created pattern.
 */
SSLOG_EXTERN pattern_t* sslog_new_pattern(class_t *ont_class, const char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_pattern START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_class(ont_class);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s new_pattern_condition END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return NULL;
    }
	pattern_t *pattern = (pattern_t *) malloc(sizeof(pattern_t));

	if (pattern == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_new_pattern END: error_code = %i (malloc return NULL)\n", KPLIB_DEBUG_PATTERNS_PREFIX, SSLOG_ERROR_OUT_OF_MEMORY);
		set_error(SSLOG_ERROR_OUT_OF_MEMORY);
		return NULL;
	}

    pattern->rtti = RTTI_PATTERN;
	pattern->uuid = NULL;

    pattern->check_prop_list = NULL;
    pattern->uncheck_prop_list = NULL;

    sslog_set_pattern_class(pattern, ont_class);
    sslog_set_pattern_uuid(pattern, uuid);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_new_pattern END: pattern created and returned\n", KPLIB_DEBUG_PATTERNS_PREFIX);

    return pattern;
}

/**
 * @fn void sslog_clean_pattern(pattern_t *pattern)
 *
 * @brief Clean entire fields of pattern (pattern scructure still exists).
 *
 * @param pattern_t pattern knowledge pattern structure.
 */
SSLOG_EXTERN void sslog_clean_pattern(pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_clean_pattern START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_clean_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return;
    }

    list_free_with_nodes(pattern->check_prop_list, (void (*) (void*)) free_pattern_condition);
    list_free_with_nodes(pattern->uncheck_prop_list, (void (*) (void*)) free_pattern_condition);

    pattern->uncheck_prop_list = NULL;
    pattern->check_prop_list = NULL;

    if (pattern->uuid != NULL) free(pattern->uuid);
    pattern->uuid = NULL;
    pattern->ont_class = NULL;

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_clean_pattern END: pattern is cleaned\n", KPLIB_DEBUG_PATTERNS_PREFIX);
}

/**
 * @fn void sslog_free_pattern(pattern_t *pattern)
 *
 * @brief Free pattern scructure with entire.
 *
 * @param pattern_t pattern knowledge pattern structure.
 */
SSLOG_EXTERN void sslog_free_pattern(pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_free_pattern START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	sslog_clean_pattern(pattern);

    free(pattern);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_free_pattern END: pattern is free\n", KPLIB_DEBUG_PATTERNS_PREFIX);
}

/**
 * @fn int sslog_set_pattern_class(pattern_t *pattern, class_t *ont_class)
 *
 * @brief Set class for pattern.
 *
 * @param pattern_t *pattern knowledge pattern to set property for
 * @param class_t *ont_class ontology class that would be set
 *
 * @return 0 on success, -1 otherwise
 */
SSLOG_EXTERN int sslog_set_pattern_class(pattern_t *pattern, class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_pattern_class START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_pattern_class END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	error_code = verify_class(ont_class);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_pattern_class END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

    pattern->ont_class = ont_class;

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_pattern_class END: class is set\n", KPLIB_DEBUG_PATTERNS_PREFIX);

    return 0;
}

/**
 * @fn int sslog_set_pattern_uuid(pattern_t *pattern, const char *uuid)
 *
 * @brief Set UUID for pattern.
 *
 * @param pattern_t *pattern knowledge pattern to set property for
 * @param const char *uuid UUID that would be set
 *
 * @return 0 on success, -1 otherwise
 */
SSLOG_EXTERN int sslog_set_pattern_uuid(pattern_t *pattern, const char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_pattern_uuid START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_pattern_uuid END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }


    if (uuid == NULL) {
        if (pattern->uuid != NULL) free(pattern->uuid);
        pattern->uuid = NULL;
    } else if (strlen(uuid) == 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_set_pattern_uuid END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, SSLOG_ERROR_INCORRECT_UUID);
		set_error(SSLOG_ERROR_INCORRECT_UUID);
        return -1;
    } else {
        free(pattern->uuid);
        pattern->uuid = sslog_strndup(uuid, SS_OBJECT_MAX_LEN);
    }

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_pattern_uuid END: uuid is set\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	return 0;
}

/**
 * @fn int sslog_add_unproperty_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition)
 *
 * @brief Set uncheck property to pattern.
 *
 * @param pattern_t *pattern knowledge pattern to set property for
 * @param property_t *prop property that would be set
 * @param void *value value that would be set
 * @param int condition condition type
 *
 * @return 0 on success, -1 otherwise
 */
SSLOG_EXTERN int sslog_add_unproperty_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_add_unproperty_to_pattern START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_unproperty_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	error_code = verify_property(prop);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_unproperty_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	if (!condition_type_check(condition)) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_unproperty_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, SSLOG_ERROR_INCORRECT_CONDITION_TYPE);
		set_error(SSLOG_ERROR_INCORRECT_CONDITION_TYPE);
        return -1;
    }
	
	pattern_condition_t* cond = new_pattern_condition(prop, value, condition);

	if (cond == NULL) return -1;

    pattern->uncheck_prop_list = list_get_new_list_if_null(pattern->uncheck_prop_list);

    list_add_data(cond, pattern->uncheck_prop_list);
	
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_add_unproperty_to_pattern END: property is added to uncheck list\n", KPLIB_DEBUG_PATTERNS_PREFIX);

    return 0;
}

/**
 * @fn int sslog_add_property_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition)
 *
 * @brief Set check property to pattern.
 *
 * @param pattern_t *pattern knowledge pattern to set property for
 * @param property_t *prop property that would be set
 * @param void *value value that would be set
 * @param int condition condition type
 *
 * @return 0 on success, -1 otherwise
 */
SSLOG_EXTERN int sslog_add_property_to_pattern(pattern_t *pattern, property_t *prop, void *value, int condition)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_add_property_to_pattern START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_property_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	error_code = verify_property(prop);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_property_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	if (!condition_type_check(condition)) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_add_property_to_pattern END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, SSLOG_ERROR_INCORRECT_CONDITION_TYPE);
		set_error(SSLOG_ERROR_INCORRECT_CONDITION_TYPE);
        return -1;
    }

    pattern_condition_t* cond = new_pattern_condition(prop, value, condition);

	if (cond == NULL) return -1;

    pattern->check_prop_list = list_get_new_list_if_null(pattern->check_prop_list);

    list_add_data(cond, pattern->check_prop_list);
	
	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_add_property_to_pattern END: property is added to uncheck list\n", KPLIB_DEBUG_PATTERNS_PREFIX);

    return 0;
}

/**
* @fn list_t* sslog_ss_get_individual_by_pattern_all(pattern_t *individual_pattern)
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
SSLOG_EXTERN list_t* sslog_ss_get_individual_by_pattern_all(pattern_t *individual_pattern)
{
	list_t *node = NULL;
	list_head_t *pos = NULL;
	ss_triple_t * triples = NULL;
	ss_triple_t * first_triple = NULL;
	pattern_condition_t *cond = NULL;

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_get_individual_by_pattern_all START\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	reset_error();

	int error_code = verify_pattern(individual_pattern);
	
	if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: error_code = %i\n", KPLIB_DEBUG_PATTERNS_PREFIX, error_code);
		set_error(error_code);
        return NULL;
    }

	/* Transform pattern to triples */
	triples = pattern_to_triples(individual_pattern);

	/* If an error accures the error message would be already set */
	if (triples == NULL)
	{
		// If an error accures the error message would be already set
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: can't transform pattern to triples\n", KPLIB_DEBUG_PATTERNS_PREFIX);
		return NULL;
	}

	/* Find individuals by pattern without checking OBJECT properties */
	if(ss_query(sslog_get_ss_info(), triples, &first_triple) < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: ss_query returns -1\n", KPLIB_DEBUG_PATTERNS_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triples);
		return NULL;
	}
	
	/* Clean request triples */
	ss_delete_triples(triples);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: ss_query returns empty triples\n", KPLIB_DEBUG_PATTERNS_PREFIX);
		set_error_from_kpi();
		return NULL;
	}
	
	/* Transform triples to individuals. */
	list_t *inds = triples_to_individuals (first_triple);
	if (inds == NULL) {
		// If an error accures the error message would be already set
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: can't transform triples to individuals\n", KPLIB_DEBUG_PATTERNS_PREFIX);
		return NULL;
	}

	ss_delete_triples(first_triple);

	/* If np individual founede by class or by class and UUID return NULL */
	if ((inds==NULL) || list_empty(&inds->links)) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_pattern_all END: no individuals found\n", KPLIB_DEBUG_PATTERNS_PREFIX);
		return NULL;
	}

	/* Check OBJECT properties recursively */
	pos = NULL;
	if (individual_pattern->check_prop_list && !list_empty(&individual_pattern->check_prop_list->links)) {
		list_for_each (pos, &individual_pattern->check_prop_list->links) {
			node = list_entry(pos, list_t, links);
			cond = (pattern_condition_t *) node->data;

			/* Check all properties and delete from list where properties are not set or wrong */
			filter_list_for_property(inds, cond);
		}
	}

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_set_pattern_class END: individuals returned\n", KPLIB_DEBUG_PATTERNS_PREFIX);

	return inds;
}

static bool condition_type_check(int condition)
{
	if (condition <= PATTERN_COND_MIN_VALUE)
		return false;

	if (condition >= PATTERN_COND_MAX_VALUE)
		return false;

	return true;
}

static void filter_list_for_property(list_t* inds, pattern_condition_t *cond)
{
	bool founded = false;
	bool clean = false;
	list_t *node3 = NULL;
	list_head_t *pos2 = NULL;
	list_head_t *pos3 = NULL;
	list_head_t *tmp = NULL;
	pattern_t *object = NULL;
	individual_t *ind = NULL;
	list_t *inds2 = NULL;
	
	if ((inds == NULL) || (cond == NULL) || (cond->prop == NULL) || list_empty(&inds->links)) {
		return;
	}

	property_t *proptype = cond->prop;

	list_for_each_safe (pos2, tmp, &inds->links) {
		if (clean) {
			list_del(pos2);
			continue;
		}

		list_t *node = list_entry(pos2, list_t, links);
		if (node->data == NULL) {
			continue;
		}
		
		ind = (individual_t *)node->data;
		
		if (cond->value == NULL) {
			const prop_val_t *prop = sslog_get_property(ind, proptype);
			
			if (prop == NULL) {
				list_del(pos2);
			}
			continue;
		}
		
		if (proptype->type == DATATYPEPROPERTY) {
			char *prop_value = (char *) cond->value;
			
			if (!sslog_ss_exists_property_value(ind, proptype, (void *) prop_value)) {
				list_del(pos2);
			}
		}
		else if (proptype->type == OBJECTPROPERTY) {
			int rtti = sslog_get_rtti_type(cond->value);
			
			if (rtti == RTTI_INDIVIDUAL) {
				individual_t *ind_value = (individual_t *) cond->value;
				
				if (!sslog_ss_exists_property_value(ind, proptype, (void *) ind_value)) {
					list_del(pos2);
				}
			}
			else if (rtti == RTTI_PATTERN) {
				object = (pattern_t *) cond->value;
				
				if (inds2 == NULL)
					inds2 = sslog_ss_get_individual_by_pattern_all(object);
				
				if ((inds2 == NULL) || list_empty(&inds2->links)) {
					/* Clean list */
					list_del(pos2);
					clean = true;
					continue;
				}
				
				founded = false;
				list_for_each(pos3, &inds2->links) {
					node3 = list_entry(pos3, list_t, links);
					
					if (sslog_ss_exists_property_value(ind, proptype, (void *) node3->data)) {
						founded = true;
						break;
					}
				}
				if (!founded) {
					list_del(pos2);
				}
			}
		}
	}	
}

#endif	/* _PATTERNS_C */
