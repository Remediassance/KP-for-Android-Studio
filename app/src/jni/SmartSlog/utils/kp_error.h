/**
 * @file kp_error.h
 * @author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   20 December, 2009
 * @brief  Interface for error handling.
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
 * Interface for error handling.
 * This file is part of PetrSU KP Library.
 *
 * Copyright (C) 2009 - Alexandr A. Lomov.
 */



#include "kp_bool.h"

 #ifndef _KP_ERROR_H
 #define _KP_ERROR_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

/// @cond INTERNAL_STRUCTURES

/** @brief Text for unknown error. */
#define ERROR_UNKNOWN_TEXT "Unknown error."
/// @endcond


#ifdef	__cplusplus
extern "C" {
#endif
	
/******************************************************************************/
/******************************** Enums list **********************************/
/// @cond INTERNAL_STRUCTURES
/**
 * @brief Enum with error codes.
 */
enum kp_errors {

#define KP_ERROR_DECLARATION( enum_code_decl, code_value, error_text) enum_code_decl code_value,
#define KP_ERROR_DECLARATION_LAST( enum_code_decl, code_value, error_text) enum_code_decl code_value
#include "kp_errors_define_decl.txt"
#undef KP_ERROR_DECLARATION
#undef KP_ERROR_DECLARATION_LAST

};
/// @endcond


/******************************************************************************/
/****************************** Functions list ********************************/
/// @cond INTERNAL_FUNCTIONS

int set_error(int error_code);
int set_error_from_kpi();
int reset_error();

/// @endcond


/// @cond EXTERNAL_FUNCTIONS

SSLOG_EXTERN int sslog_get_error_code();
SSLOG_EXTERN int sslog_get_error_code_and_reset();

SSLOG_EXTERN const char* sslog_get_error_text();
SSLOG_EXTERN char* sslog_get_error_text_and_reset();

SSLOG_EXTERN const char* sslog_get_error_text_by_code(int error_code);

/// @endcond

#ifdef	__cplusplus
}
#endif

 #endif /* _KP_ERROR_H */