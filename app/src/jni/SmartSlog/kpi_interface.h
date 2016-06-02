/**
* @file kpi_interface.h
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   05 April, 2010
* @brief  Contains describtion of functions for providing interface to kpi_low.
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
* Contains describtion of functions for providing interface to kpi_low.
* This file is part of SmartSlog KP library.
*
*/

#ifndef _KPI_INTERFACE_H
#define	_KPI_INTERFACE_H

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#include "utils/util_func.h"
#include "utils/kp_debug.h"
#include "utils/kp_bool.h"
#include "ss_func.h"
#include "classes.h"

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

/** @cond INTERNAL_FUNCTIONS */


/**
* @fn ss_triple_t *individual_to_triples (individual_t *ind)
*
* @brief Converts individual to triples.
*
* Converts individual properties with values and uuid with classtype to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *individual_to_triples (individual_t *ind);

/**
* @fn ss_triple_t *individual_to_triples_any (individual_t *ind)
*
* @brief Converts individual to triples.
*
* Converts individual properties without values and uuid with classtype to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *individual_to_triples_any (individual_t *ind);

/**
* @fn ss_triple_t *individual_to_triples_by_pattern (individual_t *ind, void *pattern)
*
* @brief Converts individual to triples.
*
* Converts individual properties by pattern with values and uuid with classtype to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @param[in] void *pattern. Entity of pettern (class or individual).
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *individual_to_triples_by_pattern (individual_t *ind, pattern_t *pattern);

/**
* @fn ss_triple_t *individual_to_triples_by_pattern_any (individual_t *ind, void *pattern)
*
* @brief Converts individual to triples.
*
* Converts individual properties by pattern without values and uuid with classtype to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @param[in] void *pattern. Entity of pettern (class or individual).
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *individual_to_triples_by_pattern_any (individual_t *ind, pattern_t *pattern);

/**
* @fn ss_triple_t *class_to_triples (class_t *ont_class)
*
* @brief Converts individual to triples.
*
* Converts class properties with values with classtype to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] class_t *ont_class. Pointer to the class struct.
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *class_to_triples (class_t *ont_class);

/**
* @fn ss_triple_t *pattern_to_triples (pattern_t *pattern)
*
* @brief Converts pattern to triples.
*
* Converts pattern structures to triples.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * individual. Pointer to the individual struct.
* @param[in] void *pattern. Entity of pettern (class or individual).
* @return ss_triple_t *. List of triples or NULL if error accures.
*/
ss_triple_t *pattern_to_triples (pattern_t *pattern);


/**
* @fn ss_triple_t *class_to_triples (class_t *ont_class)
*
* @brief Converts triples to individuals.
*
* Converts properties without values and uuid with classtype to individual struct.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] ss_triple_t *triples. Pointer to the list of triples.
* @return list_t *. List of all individuals.
*/
list_t *triples_to_individuals (ss_triple_t *triples);

/**
* @fn individual_t *triples_to_individual_first (ss_triple_t *triples)
*
* @brief Converts triples to individual.
*
* Converts properties without values and uuid with classtype to individual struct.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] ss_triple_t *triples. Pointer to the list of triples.
* @return individual_t . First individual only.
*/
individual_t *triples_to_individual_first (ss_triple_t *triples);

/**
* @brief Convert individual to triplets using given list properties.
*
* Checks properties, if they can be set for given individuals it convert it to
* triples, without set object (use SS_RDF_SIB_ANY - any value).
* It not convert object properties.
*
* @param ind individual.
* @param properties list of properties.
* It can be NULL - all properties that be set for individual are converted to triplets.
*
* @return 0 on success or not otherwise.
*/
ss_triple_t* individual_to_triples_by_properties_any(individual_t *ind, list_t *properties);

/** @endcond */

#ifdef	__cplusplus
}
#endif

#endif	/* _KPI_INTERFACE_H */
