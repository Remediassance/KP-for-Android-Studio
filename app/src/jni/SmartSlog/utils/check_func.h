/**
 * @file check_func.h
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   11 January, 2010
 * @brief  Interfaces functions for different checks.
 *
 *
 * @section LICENSE
 *
 * PetrSU KP Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PetrSU KP Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PetrSU KP Library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Contains interfaces functions for different checks.
 * This file is part of PetrSU KP Library.
 */

#ifndef _CHECK_FUNC_H
#define _CHECK_FUNC_H

#include <stdio.h>
#include <string.h>

#include "kp_error.h"
#include "kp_bool.h"
#include "../structures.h"
#include "../subscription.h"
#include "../properties.h"
#include "../patterns.h"
#include "../subscription_changes.h"

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
SSLOG_EXTERN  bool is_str_null_or_empty(const char *string);
/// @endcond


/// @cond INTERNAL_FUNCTIONS
int verify_class(const class_t *ont_class);
int verify_individual(const individual_t *individual);
int verify_entity(const void *entity);
int verify_property(const property_t *prop);
int verify_individual_prop_list(const individual_t *individual, list_t *prop_list);
int verify_subscription(const subscription_t *container);
int verify_pattern(const pattern_t *pattern);
int verify_subscription_changes(const subscription_changes_data_t *changes);
/// @endcond

#ifdef	__cplusplus
}
#endif

#endif /* _CHECK_FUNC_H */
