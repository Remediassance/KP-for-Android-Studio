/**
* @file ss_populate.h
* @Author Vanag Pavel <vanag@cs.karelia.ru>
* @date   05 April, 2010
* @brief  Contains describtion of functions for work with populations from SS.
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
* Contains describtion of functions for work with populations from SS.
* This file is part of SmartSlog KP library.
*
*/

#ifndef _SS_POPULATE_H
#define	_SS_POPULATE_H

#include "kpi_interface.h"
#include "ss_func.h"
#include "utils/list.h"

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
SSLOG_EXTERN int sslog_ss_populate_individual(individual_t* individual);

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
SSLOG_EXTERN int sslog_ss_populate_individual_by_pattern(individual_t *individual, pattern_t *pattern);

/*int ss_populate_by_subscription(subscription_container_t *subscr_container);*/

/** @endcond */

#ifdef	__cplusplus
}
#endif

#endif	/* _SS_POPULATE_H */

