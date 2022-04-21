//
// Created by kinnarr on 04.02.22.
//

#ifndef OPEN5GS_INSTRUMENTATION_H
#define OPEN5GS_INSTRUMENTATION_H

#define _GNU_SOURCE
#include <stdio.h>

#define INSTR_MEM_ACTION_NEW "new"
#define INSTR_MEM_ACTION_CLEAR "clear"
#define INSTR_MEM_ACTION_FREE "free"
#define INSTR_MEM_ACTION_WRITE "write"
#define INSTR_MEM_ACTION_INIT "init"
#define INSTR_MEM_ACTION_READ "read"

#define instr_state_logging_child(obj, child, op, msg) ogs_info("[state]{%s,%s,%s,%s,%s}(1)", obj, child, op, msg, OGS_FUNC)
#define instr_state_logging_child_v2(obj, child, op, msg) instr_state_logging_child(#obj, #child, op, msg)
#define instr_state_logging(obj, op, msg) instr_state_logging_child(obj, "", op, msg)
#define instr_state_logging_v2(obj, op, msg) instr_state_logging_child(#obj, "", op, msg)

/*
 * To use this macro 'instrumentation.h' must be the first included header
 */
//#define instr_state_logging_f(obj, op, msg, ...) { \
//      instr_state_logging(obj, op, ogs_msprintf(msg, __VA_ARGS__)); \
//  }

#define instr_state_logging_f(obj, op, msg, ...) { \
    char *msgStr; \
    int formattedStrResult = asprintf(&msgStr, msg, __VA_ARGS__); \
    if (formattedStrResult > 0) { \
      instr_state_logging(obj, op, msgStr); \
      free(msgStr); \
    }                                              \
  }

#define instr_state_logging_f_v2(obj, op, msg, ...) instr_state_logging_f(#obj, op, msg, __VA_ARGS__)


/*
 * start_timing() and stop_timing() must run in the same block.
 */
#define instr_start_timing_fun(fun) long start, end; \
  struct timeval timecheck; \
  gettimeofday(&timecheck, NULL); \
  start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000; \
  ogs_info("[timemarker]{%s,start}", fun)

#define instr_start_timing() instr_start_timing_fun(OGS_FUNC)

#define instr_stop_timing_fun(fun) gettimeofday(&timecheck, NULL); \
  end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000; \
  ogs_info("[timemarker]{%s,stop}", fun); \
  ogs_info("[time]{%s,%ld}", fun, (end - start))

#define instr_stop_timing(fun) instr_stop_timing_fun(fun)

#define instr_stop_timing_autofun() instr_stop_timing_fun(OGS_FUNC)

#endif // OPEN5GS_INSTRUMENTATION_H
