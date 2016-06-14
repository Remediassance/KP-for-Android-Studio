/**
* @file ss_populate.c
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   12 March, 2009
* @brief  Contains functions for work with populations from SS.
* @version 0.1alpha
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
* Contains functions for work with populations from SS.
* This file is part of SmartSlog KP Library.
*
*/

#ifndef _SS_POPULATE_C_
#define	_SS_POPULATE_C_

#include "ss_populate.h"


/** @cond EXTERNAL_FUNCTIONS */

/**
* @fn int sslog_ss_populate_individual(individual_t* individual)
*
* @brief Populate if given individual from SS
*
* Tries to ﬁnd individual by given individual data (uuid, properties) and populate it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_populate_individual(individual_t* individual)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_populate_individual START\n", KPLIB_DEBUG_POPULATE_PREFIX);

	int error_code = verify_individual(individual);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual END: error while verify_individual\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}

	ss_triple_t * first_triple = NULL;
	ss_triple_t * request = NULL;
	request = individual_to_triples_any (individual);

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual END: error while ss_query (returns -1)\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}
	ss_delete_triples(request);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual END: ss_query returns NULL\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}
	else {
		individual_t* ind = triples_to_individual_first(first_triple);
		individual = ind;
	
		ss_delete_triples(first_triple);
	}

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_populate_individual END: individual is populated\n", KPLIB_DEBUG_POPULATE_PREFIX);

	return 0;
}

/**
* @fn int sslog_ss_populate_individual_by_pattern(individual_t *individual, individual_t *pattern)
*
* @brief Populate if given individual from SS by pattern
*
* Tries to ﬁnd individual by given individual data (uuid, properties) by pattern and populate it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_populate_individual_by_pattern(individual_t* individual, pattern_t *pattern)
{
    KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_populate_individual_by_pattern START\n", KPLIB_DEBUG_POPULATE_PREFIX);

	int error_code = verify_individual(individual);

	if (error_code != SSLOG_ERROR_NO) {
		set_error(error_code);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual_by_pattern END: error while verify_individual\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}

	ss_triple_t * first_triple = NULL;
	ss_triple_t * request = NULL;
	request = individual_to_triples_by_pattern_any (individual, pattern);

	if(ss_query(sslog_get_ss_info(), request, &first_triple) < 0) {
		set_error(SSLOG_ERROR_UNKNOWN);
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual_by_pattern END: error while ss_query (returns -1)\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}

	ss_delete_triples(request);
	if (first_triple == NULL) {
		KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
			("\n%s sslog_ss_populate_individual_by_pattern END: ss_query returns NULL\n", KPLIB_DEBUG_POPULATE_PREFIX);
		return -1;
	}
	else {
		individual_t* ind = triples_to_individual_first(first_triple);
		if (ind == NULL) return -2;
		*individual = *ind;
		ss_delete_triples(first_triple);
	}

	KPLIB_DEBUG(KPLIB_DEBUG_LEVEL_MED)
		("\n%s sslog_ss_populate_individual_by_pattern END: individual is populated\n", KPLIB_DEBUG_POPULATE_PREFIX);

	return 0;
}

/** @endcond */

/*int ss_populate_by_subscription(subscription_container_t *subscr_container) {

}*/

#endif	/* _SS_POPULATE_C_ */

