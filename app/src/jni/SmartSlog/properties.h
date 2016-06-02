/**
 * @file properties.h
 * @Author Alaksandr A. Lomov <lomov@cs.karelia.ru>
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
 * Described functions for work with properties.
 * This file is part of SmartSlog KP Library.
 */

#include "structures.h"
#include "utils/list.h"
#include "utils/check_func.h"
#include "utils/kp_debug.h"

#ifndef _PROPERTIES_H
#define	_PROPERTIES_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

#ifdef	__cplusplus
extern "C" {
#endif	

/******************************************************************************/
/****************************** Functions list ********************************/
/// @cond EXTERNAL_FUNCTIONS

SSLOG_EXTERN const property_t *sslog_get_property_type(const class_t *ont_class, const char *propname);


SSLOG_EXTERN int sslog_add_property(void *entity, property_t *property, const void *value);
SSLOG_EXTERN int sslog_add_property_with_value_struct(void *entity, const prop_val_t *prop_val);

SSLOG_EXTERN const prop_val_t* sslog_get_property(individual_t *ind, property_t *prop);
SSLOG_EXTERN const prop_val_t* sslog_get_property_by_value(individual_t *ind, property_t *prop, void *data);
SSLOG_EXTERN list_t* sslog_get_property_all(individual_t *ind, property_t *prop);

SSLOG_EXTERN int sslog_update_property(void *entity, const char *propname, void *new_data);
SSLOG_EXTERN int sslog_update_property_with_value(void *entity, const char *propname,
        const void *old_data, void *new_data);
SSLOG_EXTERN int sslog_update_value_struct(prop_val_t* prop_value, void* new_data);

SSLOG_EXTERN int sslog_remove_property(individual_t *ind, property_t *prop);
SSLOG_EXTERN int sslog_remove_property_with_value(individual_t *ind, property_t *prop, void *data);
SSLOG_EXTERN bool sslog_is_object_property(property_t *prop);
/// @endcond


/// @cond INTERNAL_FUNCTIONS

bool is_prop_values_equal(prop_val_t *a, prop_val_t *b);
bool is_prop_names_equal(prop_val_t *a, prop_val_t *b);

/// @endcond

#ifdef	__cplusplus
}
#endif


#endif	/* _PROPERTIES_H */
