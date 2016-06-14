/**
 * @file kp_debug.h
 * @Author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   11 February, 2010
 * @brief  Debug macroses and defines.
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
 * along with PetrSU KP library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 *
 *
 * @section DESCRIPTION
 *
 * Contains macroses and defines for manipulation of debug for kp library.
 * This file is part of PetrSU KP library.
 *
 */



#ifndef _KP_DEBUG_H
#define	_KP_DEBUG_H

#ifdef	__cplusplus
extern "C" {
#endif

/******************************************************************************/
/********************************** Defines ***********************************/
/// @cond INTERNAL_STRUCTURES

/** @defgroup DebugLevels Debug levels */
/*@{*/
#define KPLIB_DEBUG_LEVEL 10        /**< Debug level of library */
#define KPLIB_DEBUG_LEVEL_HIGH 1    /**< Highest debug level */
#define KPLIB_DEBUG_LEVEL_AMED 3    /**< Above medium debug level */
#define KPLIB_DEBUG_LEVEL_BMED 7    /**< Below medium debug level */
#define KPLIB_DEBUG_LEVEL_MED 5     /**< Medium debug level */
#define KPLIB_DEBUG_LEVEL_LOW 10    /**< Lowes debug level */
/*@}*/


/** @brief Debug flag */
/** Uncomment below string with #define to turn on debug mode */
//#define KPLIB_DEBUG_ON

// TODO: compiler gives warnings when use: 
// #define KPLIB_DEBUG(level) //
// Now "(level <= -1) printf" just to make compiler happy, but it's 
// give extra if in the code.
#ifdef KPLIB_DEBUG_ON
/** @brief Debug macros */
#define KPLIB_DEBUG(level) if (level <= KPLIB_DEBUG_LEVEL) printf
#define KPLIB_DEBUG_PRINT printf
#else
#define KPLIB_DEBUG(level) if (level <= -1) printf
#define KPLIB_DEBUG_PRINT if (0) printf
//#define KPLIB_DEBUG(level) //
#endif 


/** @defgroup DebugPrefixes Defines of debug prefixes for output */
/*@{*/
/** @brief Debug prefix for output (classes.[c|h]) */
#define KPLIB_DEBUG_CLASSES_PREFIX    "-- [SmartSlog KPLIB] CLASSES:"

/** @brief Debug prefix for output (ss_classes.[c|h]) */
#define KPLIB_DEBUG_SS_CLASSES_PREFIX "-- [SmartSlog KPLIB] SS_CLASSES:"

/** @brief Debug prefix for output (patterns.[c|h]) */
#define KPLIB_DEBUG_PATTERNS_PREFIX    "-- [SmartSlog KPLIB] PATTERNS:"

/** @brief Debug prefix for output (repository.[c|h]) */
#define KPLIB_DEBUG_REPO_PREFIX       "-- [SmartSlog KPLIB] REPOSITORY:"

/** @brief Debug prefix for output (kpi_interface.[c|h]) */
#define KPLIB_DEBUG_KPIINTER_PREFIX   "-- [SmartSlog KPLIB] KPI_INTERFACE:"

/** @brief Debug prefix for output (subscribtion.[c|h]) */
#define KPLIB_DEBUG_SS_SBCR_PREFIX    "-- [SmartSlog KPLIB] SS_SBCR:"

/** @brief Debug prefix for output (properties.[c|h]) */
#define KPLIB_DEBUG_PROP_PREFIX       "-- [SmartSlog KPLIB] PROPS:"

/** @brief Debug prefix for output (ss_properties.[c|h]) */
#define KPLIB_DEBUG_SS_PROP_PREFIX    "-- [SmartSlog KPLIB] SS_PROPS:"

/** @brief Debug prefix for output (property_changes.[c|h]) */
#define KPLIB_DEBUG_PROP_CH_PREFIX    "-- [SmartSlog KPLIB] PROP_CHANGES:"

/** @brief Debug prefix for output (subscribtion_changes.[c|h]) */
#define KPLIB_DEBUG_SBCR_CH_PREFIX    "-- [SmartSlog KPLIB] SBCR_CHANGES:"

/** @brief Debug prefix for output (utils/check_func.[c|h]) */
#define KPLIB_DEBUG_CHECKS_PREFIX     "-- [SmartSlog KPLIB] CHECKS:"
	
/** @brief Debug prefix for output (ss_populate.[c|h]) */
#define KPLIB_DEBUG_POPULATE_PREFIX     "-- [SmartSlog KPLIB] POPULATE:"
/*@}*/



/// @endcond

#ifdef	__cplusplus
}
#endif

#endif	/* _KP_DEBUG_H */

