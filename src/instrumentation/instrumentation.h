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

#define instr_state_logging_child(obj, child, op, msg) ogs_info("[state]{%s,%s,%s,%s}", obj, child, op, msg)
#define instr_state_logging(obj, op, msg) instr_state_logging_child(obj, "", op, msg)

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


/*
 * start_timing() and stop_timing() must run in the same block.
 */
#define instr_start_timing() long start, end; \
  struct timeval timecheck; \
  gettimeofday(&timecheck, NULL); \
  start = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000

#define instr_stop_timing(fun) gettimeofday(&timecheck, NULL); \
  end = (long)timecheck.tv_sec * 1000 + (long)timecheck.tv_usec / 1000; \
  ogs_info("[time]{%s,%ld}", fun, (end - start))

#endif // OPEN5GS_INSTRUMENTATION_H
