/**
* @file ss_classes.h
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   05 April, 2010
* @brief  Contains describtion of functions for work with classes throw SS.
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
* Contains describtion of functions for work with classes throw SS.
* This file is part of SmartSlog KP library.
*
*/

#ifndef _SS_CLASSES_H
#define	_SS_CLASSES_H

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#include "kpi_interface.h"
#include "ss_properties.h"
#include "ss_func.h"
#include "utils/list.h"

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
	
	
/** @cond EXTERNAL_FUNCTIONS */

/**
 * @brief Initialize individual for SS.
 *
 * This function generate UUID and registry given individuals in SS.
 *
 * @param individual_t individual for initialize.
 *
 * @return 0 on success or not otherwise.
 */
SSLOG_EXTERN int sslog_ss_init_individual(individual_t *ind);

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
SSLOG_EXTERN int sslog_ss_init_individual_with_uuid(individual_t *ind, const char *uuid);

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
SSLOG_EXTERN int sslog_ss_insert_individual(individual_t *individual);

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
* @param[in] pattern_t *pattern. Entity contains pattern (individual or class).
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_insert_individual_by_pattern(individual_t *individual, pattern_t *pattern);

/**
* @fn int sslog_ss_update_individual(individual_t *individual)
*
* @brief Updates given individual in SS
*
* Checks individual correctness: cardinality, property values limitation, and
* converts the individual to triplets, which will be updated in
* SS, other properties (which has’t individual, but that are in
* SS) will be removed from SS.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_update_individual(individual_t *individual);

/**
 * @fn int ss_update_by_pattern(individual_t *individual, pattern_t *pattern)
 *
 * @brief Updates given individual in SS by pattern
 *
 * Checks individual correctness: cardinality, property values limitation, and
 * converts the individual to triplets by pattern, which will be updated in
 * SS, other properties (which has’t individual, but that are in
 * SS) will be removed from SS.
 * Function sets global SmartSlog KP Library's errno.
 *
 * @param[in] individual_t * individual. Pointer to the individual struct.
 * @return int. Status of the operation when completed (0 if successfull,
otherwise -1).
 */
SSLOG_EXTERN int sslog_ss_update_individual_by_pattern(individual_t *individual, pattern_t *pattern);


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
SSLOG_EXTERN int sslog_ss_remove_individual(individual_t *individual);

/**
* @fn int sslog_ss_exists_individual(individual_t *individual)
*
* @brief Checks if given individual exists in SS
*
* Tries to ﬁnd individual by given individual data (uuid, properties).
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return int. Status (1 if individual exists in SS, otherwise 0).
*/
SSLOG_EXTERN int sslog_ss_exists_individual(individual_t *individual);

/**
* @fn int sslog_ss_exists_class(class_t *ont_class)
*
* @brief Checks if any individual of given class exists in SS
*
* Tries to ﬁnd individual of given class.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] class_t *ont_class. Pointer to the class struct.
* @return int. Status (1 if individual exists in SS, otherwise 0).
*/
SSLOG_EXTERN int sslog_ss_exists_class(class_t *ont_class);

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
SSLOG_EXTERN int sslog_ss_exists_individual_by_uuid(individual_t *individual);

/**
* @fn int sslog_ss_exists_uuid(char  *uuid)
*
* @brief Checks if individual with given uuid exists in SS
*
* Tries to ﬁnd individual by uuid.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] char *uuid. Char array with uuid.
* @return int. Status (1 if individual exists in SS, otherwise 0).
*/
SSLOG_EXTERN int sslog_ss_exists_uuid(char  *uuid);

/**
* @fn int sslog_ss_exists_individual_by_pattern(pattern_t *pattern)
*
* @brief Checks if individual matches given pattern exists in SS
*
* Tries to ﬁnd individual by pattern.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] pattern_t *pattern. Pointer to the individual or class struct.
* @return int. Status (1 if individual exists in SS, otherwise 0).
*/
SSLOG_EXTERN int sslog_ss_exists_individual_by_pattern(pattern_t *pattern);

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
SSLOG_EXTERN list_t* sslog_ss_get_individual_by_class_all(class_t *ont_class);

/** @endcond */

#ifdef	__cplusplus
}
#endif

#endif	/* _SS_CLASSES_H */
