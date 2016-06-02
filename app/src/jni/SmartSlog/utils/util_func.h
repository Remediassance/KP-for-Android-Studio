/**
 * @file util_func.h
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>, Vanag Pavel <vanag@cs.karelia.ru>
 * @date   26 December, 2009
 * @brief  Interfaces for util functions.
 *
 *
 * @section LICENSE
 *
 * PetrSU KP library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * PetrSU KP library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Interfaces for util functions.
 * This file is part of PetrSU KP library.
 *
 * Copyright (C) 2009 - Alexander A. Lomov. All rights reserved.
 */

#ifndef _UTIL_FUNC_H_
#define _UTIL_FUNC_H_

#include <stdio.h>
#include <string.h>
#include "kp_error.h"
#include "kp_bool.h"
#include "../structures.h"
#include "../properties.h"
#include <sys/types.h>

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif


#ifdef	__cplusplus
extern "C" {
#endif

/******************************************************************************/
/**************************** External functions ******************************/
/** @cond EXTERNAL_FUNCTIONS */

/**
 *
 * @brief Duplicate string: allocates memory of defined size and copies string at most size characters.
 *        If string is longer than size, only size characters would be copied,
 *                                     terminating null byte ('\0') would be added.
 *        Internal function "strndup" not in ANSI C c99 standart, so SmartSlog has own implementation.
 *
 *
 * @param const char *str - string to copy
 * @param size_t len -  size of string to copy.
 *
 * @return returns duplicated string or NULL if error accures.
 */
SSLOG_EXTERN char *sslog_strndup(const char *str, size_t n);

/// @endcond


/******************************************************************************/
/****************************** Functions list ********************************/
/// @cond INTERNAL_FUNCTIONS
bool is_in_property_list(property_t *item, list_t *property_list);
bool is_in_property_values_list(prop_val_t *item, list_t *property_list);
ss_triple_t *concat_triplets(ss_triple_t *a, ss_triple_t *b);
/// @endcond

#ifdef	__cplusplus
}
#endif

#endif /* _UTIL_FUNC_H_ */
