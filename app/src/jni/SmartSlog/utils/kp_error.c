/**
 * @file kp_error.c
 * @author Alaxandr A. Lomov <lomov@cs.karelia.ru>
 * @date   20 December, 2009
 * @brief  Functions for work with kp errors.
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
 * Contains functions for work with kp errors.
 * This file is part of PetrSU KP Library.
 *
 * Copyright (C) 2009 - Alexandr A. Lomov.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "kp_error.h"
#include "ss_multisib.h"

#ifdef KPI_LOW
        #include <kpilow/sskp_errno.h>
#else                 
        #include <ckpi/sskp_errno.h>
#endif        




/// @cond INTERNAL_STRUCTURES
/** @brief Using with  kp_errors_list (@see kp_errors_list) as last element in the array. */
#define KP_ERRORS_LIST_END_CODE -666
/// @endcond


/******************************************************************************/
/****************************** Structures list *******************************/
/// @cond INTERNAL_STRUCTURES
/**
 * @brief KP error structure.
 */
typedef struct kp_error_s {
    int code;   /**< Error code */
    char *text; /**< Error text, describes the error */
} kp_error_t;
/// @endcond


/******************************************************************************/
/************************ Static and global entities **************************/
/**
 * @brief Array with kp error structures (@see kp_error_t).
 *
 * Array contains all errors from file with errors declarations.
 * Last elemet has code = KP_ERRORS_LIST_END_CODE (@see KP_ERRORS_LIST_END_CODE).
 */
static const kp_error_t kp_errors_list[] = {

#define KP_ERROR_DECLARATION(enum_code_decl, code_value, error_text) { enum_code_decl, error_text },
#define KP_ERROR_DECLARATION_LAST(enum_code_decl, code_value, error_text) { enum_code_decl, error_text },
#include "kp_errors_define_decl.txt"
#undef KP_ERROR_DECLARATION
#undef KP_ERROR_DECLARATION_LAST
    { KP_ERRORS_LIST_END_CODE, "" }
};


/** @brief Contains information about current error. */
static kp_error_t kp_error = { 0, NULL };



/******************************************************************************/
/*************************** Static functions list ****************************/
static const kp_error_t* get_kp_error_list();
static bool is_error_list_end(int index);



/******************************************************************************/
/***************************** External functions *****************************/
/// @cond INTERNAL_FUNCTIONS

/**
 * @brief Sets code of error.
 *
 * @param error_code code of error.
 *
 * @return given error code(usefull for end of function: return set_error(code))
 */
int set_error(int error_code)
{
    kp_error.code = error_code;
    kp_error.text = (char *) sslog_get_error_text_by_code(error_code);

    return error_code;
}

/**
 * @brief Sets code of error from KPI library
 *
 * @return given error code(usefull for end of function: return set_error(code))
 */
int set_error_from_kpi()
{
	int error_code = SSLOG_ERROR_NO;

	switch(sslog_get_ss_info()->ss_errno)
	{
		case SS_OK:
			error_code = SSLOG_ERROR_NO;
			break;
		case SS_ERROR_SSAP_MSG_FORMAT:
			error_code = SSLOG_ERROR_KPI_SSAP_MSG_FORMAT;
			break;
		case SS_ERROR_TRANSACTION_TYPE:
			error_code = SSLOG_ERROR_KPI_TRANSACTION_TYPE;
			break;
		case SS_ERROR_MESSAGE_TYPE:
			error_code = SSLOG_ERROR_KPI_SS_ERROR_MESSAGE_TYPE;
			break;
		case SS_ERROR_UNKNOWN_SS:
			error_code = SSLOG_ERROR_KPI_UNKNOWN_SS;
			break;
		case SS_ERROR_UNKNOWN_NODE:
			error_code = SSLOG_ERROR_KPI_UNKNOWN_NODE;
			break;
		case SS_ERROR_TRANSACTION_FAILED:
			error_code = SSLOG_ERROR_KPI_TRANSACTION_FAILED;
			break;
		case SS_ERROR_NO_SUBSCRIBE_ID:
			error_code = SSLOG_ERROR_KPI_NO_SUBSCRIBE_ID;
			break;
		case SS_ERROR_WRONG_SUBSCRIBE_ID:
			error_code = SSLOG_ERROR_KPI_WRONG_SUBSCRIBE_ID;
			break;
		case SS_ERROR_SOCKET_OPEN:
			error_code = SSLOG_ERROR_SOCKET_OPEN;
			break;
		case SS_ERROR_SOCKET_SEND:
			error_code = SSLOG_ERROR_SOCKET_SEND;
			break;
		case SS_ERROR_SOCKET_RECV:
			error_code = SSLOG_ERROR_SOCKET_RECV;
			break;
		case SS_ERROR_RECV_TIMEOUT:
			error_code = SSLOG_ERROR_RECV_TIMEOUT;
			break;
		case SS_ERROR_SOCKET_CLOSE:
			error_code = SSLOG_ERROR_SOCKET_CLOSE;
			break;
		default:
			error_code = SSLOG_ERROR_KPI_UNKNOWN;
			break;
	}

    return set_error(error_code);
}


/**
 * @brief Sets code of error equals 0 (no errors) and message equals empty string.
 *
 * @return SSLOG_ERROR_NO code (equals 0), usefull for end function, if no errors (return reset_error()).
 */
int reset_error()
{
    kp_error.code = 0;
    kp_error.text = NULL;

    return SSLOG_ERROR_NO;
}

/// @endcond


/// @cond EXTERNAL_FUNCTIONS

/**
 * @brief Return code of error.
 *
 * @return code of error.
 */
SSLOG_EXTERN int sslog_get_error_code()
{
    return kp_error.code; //get_kp_error().code;
}


/**
 * @brief Return code of error and reset error.
 *
 * @return code of error.
 */
SSLOG_EXTERN int sslog_get_error_code_and_reset()
{
    int code = kp_error.code;
    reset_error();
    return code;
}


/**
 * @brief Return error message.
 *
 * @return error description.
 */
SSLOG_EXTERN const char* sslog_get_error_text()
{
    return kp_error.text;
}


/**
 * @brief Return error message and reset error.
 *
 * Return error message and reset error.
 * You need free message text after using.
 *
 * @return error description.
 */
SSLOG_EXTERN char* sslog_get_error_text_and_reset()
{
    size_t len = strlen(kp_error.text);
    char* text = (char *) calloc(len, sizeof(char));
    reset_error();
    return text;
}


/**
 * @brief Gets error message by error code.
 *
 * @param error_code code of error.
 *
 * @return  error description.
 */
SSLOG_EXTERN const char* sslog_get_error_text_by_code(int error_code)
{
    int index = 0;

    const kp_error_t *list = get_kp_error_list();

    while (is_error_list_end(index) == false) {

        if (list[index].code == error_code) {
            return list[index].text;
        }
        ++index;
    }
    return ERROR_UNKNOWN_TEXT;
}

/// @endcond


/******************************************************************************/
/****************************** Static functions ******************************/
/**
 * @brief Return global KP errors list.
 *
 * @return list of kp errors.
 */
static const kp_error_t* get_kp_error_list()
{
    return kp_errors_list;
}

/**
 * @brief Checks if given index equals last element in error list.
 *
 * Gets from errors list element with given index and check error code.
 * If it equals KP_ERRORS_LIST_END_CODE, then it last element, no more elements in the list.
 *
 * @return true(1) if index equals last elemet in the list or it less than 0, otherwise false(0).
 */
static bool is_error_list_end(int index)
{
    if (index < 0) {
        return true;
    }

    if (get_kp_error_list()[index].code == KP_ERRORS_LIST_END_CODE) {
        return true;
    }
    return false;
}

