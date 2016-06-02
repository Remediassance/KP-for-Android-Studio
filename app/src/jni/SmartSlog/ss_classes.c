/**
* @file ss_classes.c
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   13 April, 2009
* @brief  Contains functions for work with classes throw SS.
* @version 0.2alpha
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
* @section DESCRIPTION
*
* Contains functions for work with classes throw SS.
* This file is part of SmartSlog KP Library.
*
*/

#ifndef _SS_CLASSES_C
#define	_SS_CLASSES_C

#include "ss_classes.h"
#include "utils/util_func.h"

/** @cond EXTERNAL_FUNCTIONS */


/**
 * @brief Initialize individual for SS.
 *
 * This function generate UUID and registry given individuals in SS.
 *
 * @param individual_t individual for initialize.
 *
 * @return 0 on success, -1 otherwise.
 */
SSLOG_EXTERN int sslog_ss_init_individual(individual_t *ind)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_init_individual START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();

	int error_code = verify_individual(ind);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_init_individual END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

    ind->uuid = sslog_generate_uuid(ind->parent_class);

    ss_triple_t *triple = NULL;

    /* Only class TYPE for initialization of individual inserted */
    if (ss_add_triple(&triple, ind->uuid, RDF_TYPE, ind->classtype, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_init_individual END: ss_add_triple returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		return -1;
	}

    /* insert sensor to the smart space */
    if (ss_insert(sslog_get_ss_info(), triple, NULL) != 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_init_individual END: ss_insert returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triple);
		return -1;
    }

    ss_delete_triples(triple);
    triple = NULL;

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_init_individual END: triples are sent\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

    return 0;
}

/**
 * @brief Initialize individual for SS by given uuid.
 *
 * This function registry given individuals in SS with given uuid.
 *
 * @param individual_t individual for initialize.
 * @param const char * uuid of individual.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_ss_init_individual_with_uuid(individual_t *ind, const char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_init_individual_with_uuid START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(ind);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_init_individual_with_uuid END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

    if (is_str_null_or_empty(uuid) == true) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_init_individual_with_uuid END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, SSLOG_ERROR_INCORRECT_UUID);
		set_error(SSLOG_ERROR_INCORRECT_UUID);
        return -1;
    }

    ind->uuid = sslog_strndup(uuid, KPLIB_UUID_MAX_LEN);

    ss_triple_t *triple = NULL;

    /* Only class TYPE for initialization of individual inserted */
    if (ss_add_triple(&triple, ind->uuid, RDF_TYPE, ind->classtype, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_init_individual_with_uuid END: ss_add_triple returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		return -1;
	}

    /* insert sensor to the smart space */
    if (ss_insert(sslog_get_ss_info(), triple, NULL) != 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_init_individual_with_uuid END: ss_insert returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triple);
		return -1;
    }

    ss_delete_triples(triple);
    triple = NULL;

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_init_individual_with_uuid END: triples are sent\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

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
SSLOG_EXTERN int sslog_ss_insert_individual(individual_t *individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_insert_individual START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_insert_individual END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	/* Transform individual to tripes */
	ss_triple_t * triples = NULL;
	triples = individual_to_triples (individual);

	// If an error accures the error message would be already set
	if (triples == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_insert_individual END: can't transform individual to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	/* Insert all triples */
	if (ss_insert(sslog_get_ss_info(), triples, NULL) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_insert_individual END: ss_insert returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triples);
		return -1;
	}

	ss_delete_triples(triples);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_insert_individual END: triples are sent\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	return 0;
}

/**
* @fn int sslog_ss_insert_individual_by_pattern(individual_t *individual, pattern_t *pattern)
*
* @brief Inserts given individual to SS by pattern
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets by pattern, which will be placed to SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @param[in] void *pattern. Entity contains pattern (individual or class).
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_insert_individual_by_pattern(individual_t *individual, pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_insert_individual_by_pattern START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_insert_individual_by_pattern END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	error_code = verify_pattern(pattern);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_insert_individual_by_pattern END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	/* Transform individual to tripes by pattern */
	ss_triple_t * triples = NULL;
	triples = individual_to_triples_by_pattern (individual, pattern);

	// If an error accures the error message would be already set
	if (triples == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_insert_individual_by_pattern END: can't transform individual to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	/* Insert all triples */
	if (ss_insert(sslog_get_ss_info(), triples, NULL) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_insert_individual_by_pattern END: ss_insert returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triples);
		return -1;
	}

	ss_delete_triples(triples);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_insert_individual_by_pattern END: triples are sent\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	return 0;
}

/**
* @fn int sslog_ss_update_individual(individual_t *individual)
*
* @brief Updates given individual in SS
*
* IMPORTANT: function replace individual (removes and inserts it)
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be updated in
* SS, other properties (which has’t individual, but that are in
* SS) will be removed from SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_update_individual(individual_t *individual)
{
	sslog_ss_remove_individual(individual);
	return sslog_ss_insert_individual(individual);
}

/**
 * @fn int sslog_ss_update_individual_by_pattern(individual_t *individual, pattern_t *pattern)
 *
 * @brief Updates given individual in SS by pattern
 *
 * IMPORTANT: function replace individual (removes and inserts it by pattern)
 *
 * Checks individual correctness: cardinality, property values limitation, and
 * converts the individual to triplets by pattern, which will be updated in
 * SS, other properties (which has’t individual, but that are in
 * SS) will be removed from SS.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] individual_t * individual. Pointer to the individual struct.
 * @param[in] void * pattern. Pointer to the individual struct that considered as pattern.
 * @return int. Status of the operation when completed (0 if successfull, otherwise -1).
 * */
SSLOG_EXTERN int sslog_ss_update_individual_by_pattern(individual_t *individual, pattern_t *pattern)
{
        sslog_ss_remove_individual(individual);
        return sslog_ss_insert_individual_by_pattern(individual, pattern);
}


/**
* @fn int sslog_ss_exists_individual(individual_t *individual)
*
* @brief Checks if given individual exists in SS
*
* Tries to ﬁnd individual by given individual data (uuid, properties).
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status (1 if individual exists in SS, otherwise 0), -1 on error.
*/
SSLOG_EXTERN int sslog_ss_exists_individual(individual_t *individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_exists_individual START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;

	/* Transform individual to tripes */
	request = individual_to_triples (individual);

	// If an error accures the error message would be already set
	if (request == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual END: can't transform individual to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_individual END: ss_query returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(request);
		return false;
	}
	ss_delete_triples(request);
	

	/* If individual was founded then it exists */
	if (first_triple == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual END: no such individual\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return false;
	}
	else
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual END: individual exists\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		ss_delete_triples(first_triple);
		return true;
	}
}

/**
* @fn int sslog_ss_exists_class(class_t *ont_class)
*
* @brief Checks if any individual of given class exists in SS
*
* Tries to ﬁnd individual of given class.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] class_t *ont_class. Pointer to the class struct.
* @return int. Status (1 if individual exists in SS, otherwise 0), -1 on error.
*/
SSLOG_EXTERN int sslog_ss_exists_class(class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_exists_class START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_class END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;

	/* Transform class to tripes */
	request = class_to_triples (ont_class);

	// If an error accures the error message would be already set
	if (request == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_class END: can't transform class to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_class END: ss_query returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(request);
		return false;
	}
	ss_delete_triples(request);
	
	/* If individual was founded then it exists */
	if (first_triple == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_class END: no such class\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return false;
	}
	else
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_class END: class exists\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		ss_delete_triples(first_triple);
		return true;
	}

}

/**
* @fn int sslog_ss_exists_individual_by_uuid(individual_t *individual)
*
* @brief Checks if uuid of given individual exists in SS
*
* Tries to ﬁnd individual by uuid.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status (1 if individual exists in SS, otherwise 0).
*/
SSLOG_EXTERN int sslog_ss_exists_individual_by_uuid(individual_t *individual)
{
	return sslog_ss_exists_uuid(individual->uuid);
}

/**
* @fn int sslog_ss_exists_uuid(char  *uuid)
*
* @brief Checks if individual with given uuid exists in SS
*
* Tries to ﬁnd individual by uuid.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] char *uuid. Char array with uuid.
* @return int. Status (1 if individual exists in SS, otherwise 0), -1 on error.
*/
SSLOG_EXTERN int sslog_ss_exists_uuid(char *uuid)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_exists_uuid START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();

    if (uuid == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_uuid END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, SSLOG_ERROR_INCORRECT_UUID);
		set_error(SSLOG_ERROR_INCORRECT_UUID);
        return -1;
    }

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;


	/* Only class TYPE for checking existence by given uuid */
	if (ss_add_triple(&request, uuid, RDF_TYPE, SS_RDF_SIB_ANY, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_uuid END: ss_add_triple returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		return -1;
	}

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_uuid END: ss_query returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(request);
		return false;
	}
	ss_delete_triples(request);
	
	/* If triples was founded then it exists */
	if (first_triple == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_uuid END: no such uuid\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return false;
	}
	else
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_uuid END: uuid exists\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		ss_delete_triples(first_triple);
		return true;
	}
}

/**
* @fn int sslog_ss_exists_individual_by_pattern(pattern_t *pattern)
*
* @brief Checks if individual matches given pattern exists in SS
*
* Tries to ﬁnd individual by pattern.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] void *pattern. Pointer to the individual or class struct.
* @return int. Status (1 if individual exists in SS, otherwise 0), -1 on error.
*/
SSLOG_EXTERN int sslog_ss_exists_individual_by_pattern(pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_exists_individual_by_pattern START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_pattern(pattern);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual_by_pattern END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	ss_triple_t * request = NULL;
	ss_triple_t * first_triple = NULL;

	/* Transform pattern to tripes */
	request = pattern_to_triples (pattern);

	// If an error accures the error message would be already set
	if (request == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_exists_individual_by_pattern END: can't transform pattern to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_individual_by_pattern END: ss_query returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(request);
		return false;
	}
	ss_delete_triples(request);

	/* If triples was founded then it exists */
	if (first_triple == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_individual_by_pattern END: no individuals found by pattern\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return false;
	}
	else
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_exists_individual_by_pattern END: some individuals are exists\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		ss_delete_triples(first_triple);
		return true;
	}

}

/**
* @fn list_t* sslog_ss_get_individual_by_class_all(class_t *ont_class)
*
* @brief Finds all individuals of given class in SS
*
* Tries to ﬁnd individual by class.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] class_t *ont_class. Pointer to the class struct.
* @return list_t*. Null if there is no individuals of such class in SS,
*                 list of founded individuals otherwise.
*/
SSLOG_EXTERN list_t* sslog_ss_get_individual_by_class_all(class_t *ont_class)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_get_individual_by_class_all START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_class(ont_class);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_class_all END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return NULL;
    }

	ss_triple_t *triple_rqst = NULL;
	ss_triple_t *triple_res = NULL;

	/* Only class TYPE for getting individuals by given uuid */
	if (ss_add_triple(&triple_rqst, SS_RDF_SIB_ANY, RDF_TYPE, ont_class->classtype, SS_RDF_TYPE_URI, SS_RDF_TYPE_URI) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_individual_by_class_all END: ss_add_triple returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		return NULL;
	}

	if(ss_query(sslog_get_ss_info(), triple_rqst, &triple_res) < 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_get_individual_by_class_all END: ss_query returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		ss_delete_triples(triple_rqst);
		set_error_from_kpi();
		return NULL;
	}

	ss_delete_triples(triple_rqst);

	if (triple_res == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_class_all END: no individuals found\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return NULL;
	}

	/* Transform triples to individuals */
	list_t *uuid_list = triples_to_individuals(triple_res);

	// If an error accures the error message would be already set
	if (uuid_list == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_get_individual_by_class_all END: can't transform triples to individuals\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return NULL;
	}
	ss_delete_triples(triple_res);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_get_individual_by_class_all END: individuals founded\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	return uuid_list;
}

/**
* @fn int sslog_ss_remove_individual(individual_t *individual)
*
* @brief Removes given individual from SS
*
* Tries to ﬁnd individuals description(triplets) in the SS, then removes
* all triplets from SS and from individual, UUID removed too.
* After work you have empty individual struct.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_individual(individual_t *individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_remove_individual START\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	reset_error();
	
	int error_code = verify_individual(individual);

    if (error_code != SSLOG_ERROR_NO) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_remove_individual END: error_code = %i\n", KPLIB_DEBUG_SS_CLASSES_PREFIX, error_code);
		set_error(error_code);
        return -1;
    }

	ss_triple_t * triples = NULL;

	triples = individual_to_triples (individual);

	// If an error accures the error message would be already set
	if (triples == NULL)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
			("\n%s sslog_ss_remove_individual END: can't transform individual to triples\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		return -1;
	}

	if (ss_remove(sslog_get_ss_info(), triples) != 0)
	{
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_remove_individual END: ss_remove returns -1\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);
		set_error_from_kpi();
		ss_delete_triples(triples);
		return -1;
	}

	ss_delete_triples(triples);

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_HIGH)
		("\n%s sslog_ss_remove_individual END: individuals removed\n", KPLIB_DEBUG_SS_CLASSES_PREFIX);

	return 0;
}

/** @endcond */

#endif	/* _SS_CLASSES_C */
