/**
* @file ss_properties.h
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   05 April, 2010
* @brief  Contains describtions of functions for work with properties throw SS.
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
* Contains describtions of functions for work with properties throw SS:
* This file is part of SmartSlog KP library.
*
*/

#include "structures.h"
#include "properties.h"
#include "classes.h"
#include "utils/list.h"
#include "utils/check_func.h"
#include "ss_func.h"

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#ifndef _SS_PROPERTIES_H
#define	_SS_PROPERTIES_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

#ifdef	__cplusplus
extern "C" {
#endif
	
/** @cond EXTERNAL_FUNCTIONS */

/**
* @fn int sslog_ss_update_property(individual_t *ind, char *propname, void *old_data, void *new_data)
*
* @brief Updates property of individual in SS and localy.
*
* Checks individual correctness: cardinality, prop-
* erty values limitation, and converts the individual’s given
* properties to triplets, which will be updated in SS, other
* properties not changed in SS. Same localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Char array with name of property.
* @param[in] void *old_data. Pointer to the old value (DATATYPE or OBJECTTYPE).
* @param[in] void *new_data. Pointer to the new value (DATATYPE or OBJECTTYPE).
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_update_property(individual_t *ind, char *propname, void *old_data, void *new_data);
/*int ss_update_properties(individual_t *individual, list_t *properties);*/

/**
* @fn int sslog_ss_remove_property_by_name_all(individual_t *ind, char *propname)
*
* @brief Removes all properties with given name for individual in SS and localy.
*
* Finds properties with given name for individual and removes it from SS.
* Then try find all local property with given name and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Char array with name of property.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_by_name_all(individual_t *ind, char *propname);

/**
* @fn int sslog_ss_remove_property_all(individual_t *ind, property_t *prop)
*
* @brief Removes all properties with given structure for individual in SS and localy.
*
* Finds properties with given structure for individual and removes it from SS.
* Then try find all local property with given structure and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Property structure with name of property.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_all(individual_t *ind, property_t *prop);

/**
* @fn int sslog_ss_remove_property_by_value_struct(individual_t *ind, prop_val_t *prop_val)
*
* @brief Removes all properties with given name and value for individual in SS and localy.
*
* Get name and value from value struct, finds properties with given
* name and value for individual and removes it from SS.
* Then try find all local property with given name and value and remove it.
* After all you have a struct without removed properties.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] prop_val_t *prop_val. Pointer to the value struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_remove_property_by_value_struct(individual_t *ind, prop_val_t *prop_val);

/**
* @fn int sslog_ss_add_property(void *entity, property_t *prop, void *data)
*
* @brief Sets property for entity in SS and localy by name.
*
* Checks either entity is individual or class
* Checks entity correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] void *entity. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_add_property(void *entity, property_t *prop, void *data);

/**
* @fn int sslog_ss_set_property_with_value_struct(void *entity, prop_val_t *prop_val)
*
* @brief Sets property for entity in SS and localy by name.
*
* Checks either entity is individual or class
* Checks entity correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] void *entity. Pointer to the individual struct.
* @param[in] prop_val_t *prop_val. Pointer to the value struct.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
SSLOG_EXTERN int sslog_ss_set_property_with_value_struct(void *entity, prop_val_t *prop_val);

/**
* @fn prop_val_t* sslog_ss_get_property(individual_t *ind, property_t *prop)
*
* @brief Gets property for individual from SS by name.
*
* Gets the property from the SS by name and returns it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property(individual_t *ind, property_t *prop);

/**
* @fn prop_val_t* sslog_ss_get_property_by_name((individual_t *ind, char *propname)
*
* @brief Gets property for individual from SS by name.
*
* Gets the property from the SS by name and returns it.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property_by_name(individual_t *ind, char *propname);

/**
* @fn prop_val_t* sslog_ss_get_property_by_value(individual_t *ind, property_t *prop, void *data)
*
* @brief Gets first founded property for individual from SS by name and value.
*
* Gets the property from the SS by name and value and returns first founded.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return prop_val_t*. NULL if properties with given name doesn't exists,
*                      pointer to value struct otherwise.
*/
SSLOG_EXTERN prop_val_t* sslog_ss_get_property_by_value(individual_t *ind, property_t *prop, void *data);

/**
* @fn list_t* sslog_ss_get_property_max(individual_t *ind, property_t *prop, int max)
*
* @brief Gets list of properties for individual from SS by name and value.
*
* Gets the property from the SS by name and value and returns list of properties.
* Returns all founded but not more then max.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @param[in] int max. Maximum returns properties.
* @return list_t*. NULL if properties with given name doesn't exists,
*                      pointer to list of value structs otherwise.
*/
SSLOG_EXTERN list_t* sslog_ss_get_property_max(individual_t *ind, property_t *prop, int max);

/**
* @fn bool sslog_ss_exists_property_value(individual_t* ind, property_t *prop, void *data)
*
* @brief Checks if individual has property with given value.
*
* Checks if individual has property with given name and value.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t * ind. Pointer to the individual struct.
* @param[in] property_t *prop. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. 0 if property doesn't exists, 1 otherwise.
*/
SSLOG_EXTERN bool sslog_ss_exists_property_value(individual_t* ind, property_t *prop, void *data);

/** @endcond */

/** @cond INTERNAL_FUNCTIONS */

/**
* @fn int ss_set_property_for_individual(individual_t *ind, char *propname, void *data)
*
* @brief Sets property for individual in SS and localy by name.
*
* Checks individual correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t *ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
int ss_add_property_for_individual(individual_t *ind, char *propname, void *data);

/**
* @fn int ss_set_property_for_class(class_t *ind, char *propname, void *data)
*
* @brief Sets property for class in SS and localy by name.
*
* Checks class correctness: cardinality, property values limitation
* and sets property for entity in SS and localy.
* Function sets global SmartSlog KP Library's errno.
*
* @param[in] individual_t *ind. Pointer to the individual struct.
* @param[in] char *propname. Pointer to the char array with name.
* @param[in] void *data. Pointer to the value.
* @return int. Status of the operation when completed (0 if successfull, otherwise -1).
*/
int ss_add_property_for_class(class_t *ind, char *propname, void *data);

/** @endcond */

#ifdef	__cplusplus
}
#endif

#endif	/* _SS_PROPERTIES_H */
