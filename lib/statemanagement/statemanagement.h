//
// Created by kinnarr on 14.04.22.
//

#ifndef OPEN5GS_STATEMANAGEMENT_H
#define OPEN5GS_STATEMANAGEMENT_H

#include <czmq.h>

#include "ogs-core.h"

#include "defs.pb-c.h"


extern int __statemanagement_log_domain;

//#undef OGS_LOG_DOMAIN
//#define OGS_LOG_DOMAIN __statemanagement_log_domain

zsock_t *__statestore_socket;

void init_statemanagement(void);
void finalize_statemanagement(void);

StoreKeyValue *create_store_keyvalue(void);
void add_store_key(StoreKeyValue *, char *);
void set_store_data(StoreKeyValue *, uint8_t *, size_t);
void save_store_data(StoreKeyValue *);
void destroy_store_keyvalue(StoreKeyValue *);

void get_store_data(char *, uint8_t **, size_t*);

#endif // OPEN5GS_STATEMANAGEMENT_H
