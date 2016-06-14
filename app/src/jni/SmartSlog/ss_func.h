/*
 * ss_func.h - interface for functions for work with SS.
 * This file is part of PetrSU KP library.
 *
 * Copyright (C) 2009 - Alexandr A. Lomov. All rights reserved.
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
 */

#ifdef KPI_LOW
        #include <kpilow/kpi_low.h>
#else                 
        #include <ckpi/ckpi.h>
#endif        

#include "utils/check_func.h"
#include "ss_multisib.h"
#include <time.h>

#ifndef _SS_FUNC_H
#define	_SS_FUNC_H

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

/**
 * @brief UUID maximum lenght.
 */
#define KPLIB_UUID_MAX_LEN 8192


#ifdef	__cplusplus
extern "C" {
#endif

//ss_info_t* sslog_get_ss_info();
SSLOG_EXTERN void sslog_reset_errno();

SSLOG_EXTERN char* sslog_generate_hash();
SSLOG_EXTERN char* sslog_generate_uuid(const class_t *ont_class);

SSLOG_EXTERN int sslog_ss_init_session();
SSLOG_EXTERN int sslog_ss_init_session_with_parameters(const char *space_id, const char *ss_address, int ss_port);

#ifdef	__cplusplus
}
#endif

#endif	/* _SS_FUNC_H */

