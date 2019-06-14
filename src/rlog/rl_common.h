/*******************************************************************************
################################################################################
#   Copyright (c) [2017-2019] [Radisys]                                        #
#                                                                              #
#   Licensed under the Apache License, Version 2.0 (the "License");            #
#   you may not use this file except in compliance with the License.           #
#   You may obtain a copy of the License at                                    #
#                                                                              #
#       http://www.apache.org/licenses/LICENSE-2.0                             #
#                                                                              #
#   Unless required by applicable law or agreed to in writing, software        #
#   distributed under the License is distributed on an "AS IS" BASIS,          #
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   #
#   See the License for the specific language governing permissions and        #
#   limitations under the License.                                             #
################################################################################
*******************************************************************************/

/********************************************************************20**

     Name:     Radisys Logging Framework
     Type:     C include file
     Desc:     This file contains logging framework include file for library.
     File:     rl_common.h

*********************************************************************21*/
/*************************************************************************
@ description: This is header file is used by logging framework module. This
file should not be cirectly included by any other application although it is
common file to logging framework and LOG MACROs used by any applicatoin.
***************************************************************************/

#ifndef __RL_COMMON_H__
#define __RL_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	L_ALWAYS=0,
	L_FATAL,
	L_ERROR,
	L_WARNING,
	L_EVENT,
	L_INFO,
	L_DEBUG,
	L_UNUSED,
	L_MAX_LOG_LEVEL
} R_LOG_LEVEL;

typedef enum {
	DBG_CELLID,
   DBG_PEERID,
   DBG_ENBID,
 	DBG_MMEID,
 	DBG_CRNTI,
   DBG_UEIDX,
 	DBG_UEID,
 	DBG_RBID,
 	DBG_LCID,
 	DBG_LCGID,
	DBG_TRNSID,
   DBG_INSTID,
	DBG_MAX_IDs
} R_SPL_ARG;

#ifdef USE_RLOG_DATA_TYPES
typedef const char* PSTR;
typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef int S32;
typedef signed short S16;
#else
#include "envdep.h"
typedef const char* PSTR;
#endif

typedef U32 LOGID;

#include <stdio.h>
extern FILE* g_fp;
void logLev0(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, ...);
void logLev1(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, U32 arg1, ...);
void logLev2(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, U32 arg1, U32 arg2, ...);
void logLev3(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, U32, U32, U32, ...);
void logLev4(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, U32, U32, U32, U32, ...);
void logLevN(int logLevel, const char* modName, const char* file, int lineno, const char* fmtStr, ...);
void logLevE(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, R_SPL_ARG splType,
   U32 splVal, U32 arg1, U32 arg2, U32 arg3, U32 arg4, ...);
void logLevH(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, PSTR hexdump, int hexlen, ...);
void logLevS(PSTR strLogLevel, PSTR modName, PSTR file, int lineno, PSTR fmtStr, PSTR str, ...);

void hextostr(char* p, PSTR h, int hexlen);

extern int g_logLevel;
extern U32 g_modMask;
extern const char* g_logStr[L_MAX_LOG_LEVEL]; 
extern const char* g_splStr[DBG_MAX_IDs];

#define RLOG_SEGFAULT_STR "Segmentation Fault Occurred\n%s"

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __RLOG_COMMON_H__ */
