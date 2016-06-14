/*
 * ss_multisib.h - interface for functions for work with SS.
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

#include "utils/list.h"

#ifndef _SS_MSIB_H
#define	_SS_MSIB_H

#if defined(WIN32) || defined(WINCE) || defined(SMARTSLOG_WIN_EXPORTS)
#define SSLOG_EXTERN __declspec(dllexport)
#else
#define SSLOG_EXTERN 
#endif

#ifdef	__cplusplus
extern "C" {
#endif

SSLOG_EXTERN ss_info_t* sslog_get_ss_info();

SSLOG_EXTERN int sslog_ss_init_new_session(const char *name_id);
SSLOG_EXTERN int sslog_ss_init_new_session_with_parameters(const char *space_id, const char *ss_address, int ss_port, const char *name_id);

SSLOG_EXTERN int sslog_switch_sib(const char *name_id);
SSLOG_EXTERN int sslog_ss_leave_session(ss_info_t * ss_info);
SSLOG_EXTERN int sslog_ss_leave_session_all();

#ifdef	__cplusplus
}
#endif

#endif	/* _SS_MSIB_H */

