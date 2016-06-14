/**
 * @file classes.h
 * @Author Alexandr A. Lomov <lomov@cs.karelia.ru>
 * @date   05 December, 2009
 * @brief  Describe functions for work with classes.
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
 * Describe functions for work with classes.
 * This file is part of SmartSlog KP Library.
 *
 * Copyright (C) 2009 - SmartSlog. All rights reserved.
 *
 */

#include "structures.h"
#include "utils/kp_debug.h"

#ifndef _CLASSES_H
#define	_CLASSES_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

#ifdef	__cplusplus
extern "C" {
#endif
	

/* Functions for work with classes and properties */
/// @cond EXTERNAL_FUNCTIONS
SSLOG_EXTERN individual_t* sslog_new_individual(const class_t *classtype);
SSLOG_EXTERN list_t* sslog_get_superclass_all(const class_t *ont_class);
SSLOG_EXTERN int sslog_is_subclass_of(const class_t *ont_class, const class_t *super_class);
SSLOG_EXTERN int sslog_is_classtype_of(const individual_t *individual, const class_t *prent_class);
SSLOG_EXTERN int sslog_repo_count_individual_references(individual_t *individual);
SSLOG_EXTERN int sslog_set_individual_uuid(individual_t *ind, const char *uuid);
/// @endcond 

#ifdef	__cplusplus
}
#endif

#endif	/* _CLASSES_H */
