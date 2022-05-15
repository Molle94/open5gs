/*
 * Copyright (C) 2019,2020 by Sukchan Lee <acetcom@gmail.com>
 *
 * This file is part of Open5GS.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "../instrumentation/instrumentation.h"

#include <unistd.h>
#include "sbi-path.h"

static ausf_context_t self;

int __ausf_log_domain;

static OGS_POOL(ausf_ue_pool, ausf_ue_t);

static int context_initialized = 0;

void ausf_context_init(void)
{
    ogs_assert(context_initialized == 0);

    ogs_log_install_domain(&__ausf_log_domain, "ausf", ogs_core()->log.level);
    instr_start_timing();

    init_statemanagement();

    /* Initialize AUSF context */
    memset(&self, 0, sizeof(ausf_context_t));
    instr_state_logging("ausf_context_t", INSTR_MEM_ACTION_CLEAR, "");
//    ogs_info("[state] new context");

    ogs_pool_init(&ausf_ue_pool, ogs_app()->max.ue);
    instr_state_logging_f("ausf_ue_pool", INSTR_MEM_ACTION_INIT, "pool size: %lu", ogs_app()->max.ue);
//    ogs_info("[state] new ue pool");

//    ogs_list_init(&self.ausf_ue_list);
    instr_state_logging_child("ausf_context_t", "ausf_ue_list", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] init context ue list");
//    self.suci_hash = ogs_hash_make();
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] context set suci_hash");
//    ogs_assert(self.suci_hash);
//    self.supi_hash = ogs_hash_make();
    instr_state_logging_child("ausf_context_t", "supi_hash", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] context set supi_hash");
//    ogs_assert(self.supi_hash);

    context_initialized = 1;
    instr_stop_timing("ausf_context_init");
}

void ausf_context_final(void)
{
    instr_start_timing();
    ogs_assert(context_initialized == 1);

    ausf_ue_remove_all();

//    ogs_assert(self.suci_hash);
//    ogs_hash_destroy(self.suci_hash);
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_FREE, "");
//    ogs_assert(self.supi_hash);
//    ogs_hash_destroy(self.supi_hash);
    instr_state_logging_child("ausf_context_t", "supi_hash", INSTR_MEM_ACTION_FREE, "");

    ogs_pool_final(&ausf_ue_pool);
    instr_state_logging("ausf_ue_pool", INSTR_MEM_ACTION_FREE, "");

    finalize_statemanagement();
    context_initialized = 0;
    instr_stop_timing("ausf_context_final");
}

ausf_context_t *ausf_self(void)
{
    return &self;
}

static int ausf_context_prepare(void)
{
    instr_start_timing();
    self.nf_type = OpenAPI_nf_type_AUSF;
    instr_state_logging_child("ausf_context_t", "nf_type", INSTR_MEM_ACTION_WRITE, "");

    instr_stop_timing("ausf_context_prepare");
    return OGS_OK;
}

static int ausf_context_validation(void)
{
    return OGS_OK;
}

int ausf_context_parse_config(void)
{
    int rv;
    yaml_document_t *document = NULL;
    ogs_yaml_iter_t root_iter;

    document = ogs_app()->document;
    ogs_assert(document);

    rv = ausf_context_prepare();
    if (rv != OGS_OK) return rv;

    ogs_yaml_iter_init(&root_iter, document);
    while (ogs_yaml_iter_next(&root_iter)) {
        const char *root_key = ogs_yaml_iter_key(&root_iter);
        ogs_assert(root_key);
        if (!strcmp(root_key, "ausf")) {
            ogs_yaml_iter_t ausf_iter;
            ogs_yaml_iter_recurse(&root_iter, &ausf_iter);
            while (ogs_yaml_iter_next(&ausf_iter)) {
                const char *ausf_key = ogs_yaml_iter_key(&ausf_iter);
                ogs_assert(ausf_key);
                if (!strcmp(ausf_key, "sbi")) {
                    /* handle config in sbi library */
                } else
                    ogs_warn("unknown key `%s`", ausf_key);
            }
        }
    }

    rv = ausf_context_validation();
    if (rv != OGS_OK) return rv;

    return OGS_OK;
}

ausf_ue_t *ausf_ue_add(char *suci)
{
    instr_start_timing();
    ausf_event_t e;
    ausf_ue_t *ausf_ue = NULL;

    StoreKeyValue *store_kv = create_store_keyvalue();

    ogs_assert(suci);

    ogs_pool_alloc(&ausf_ue_pool, &ausf_ue);
    instr_state_logging("ausf_ue_pool", INSTR_MEM_ACTION_WRITE, "get free ue from pool");
//    ogs_info("[state] ue pool add ue");
    ogs_assert(ausf_ue);
    memset(ausf_ue, 0, sizeof *ausf_ue);
    instr_state_logging("ausf_ue_t", INSTR_MEM_ACTION_CLEAR, "");
//    ogs_info("[state] ue allocate");

    ausf_ue->ctx_id =
        ogs_msprintf("%d", (int)ogs_pool_index(&ausf_ue_pool, ausf_ue));
    ogs_assert(ausf_ue->ctx_id);
    instr_state_logging_child("ausf_ue_t", "ctx_id", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] ue set ctx_id");
    add_store_key(store_kv, ausf_ue->ctx_id);

    ausf_ue->suci = ogs_strdup(suci);
    add_store_key(store_kv, suci);
    ogs_assert(ausf_ue->suci);
    instr_state_logging_child("ausf_ue_t", "suci", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] ue set suci");
//    ogs_hash_set(self.suci_hash, ausf_ue->suci, strlen(ausf_ue->suci), ausf_ue);
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_NEW, "overwrite or create hash entry");
//    ogs_info("[state] context set suci_hash");

    ausf_ue->supi = ogs_supi_from_suci(ausf_ue->suci);
    add_store_key(store_kv, ausf_ue->supi);
    ogs_assert(ausf_ue->supi);
    instr_state_logging_child("ausf_ue_t", "supi", INSTR_MEM_ACTION_INIT, "");
//    ogs_info("[state] ue set supi");
//    ogs_hash_set(self.supi_hash, ausf_ue->supi, strlen(ausf_ue->supi), ausf_ue);
    instr_state_logging_child("ausf_context_t", "supi_hash", INSTR_MEM_ACTION_NEW, "overwrite or create hash entry");
//    ogs_info("[state] context set supi_hash");

    memset(&e, 0, sizeof(e));
    e.ausf_ue = ausf_ue;
    ogs_fsm_create(&ausf_ue->sm, ausf_ue_state_initial, ausf_ue_state_final);
    instr_state_logging_child("ausf_ue_t", "sm", INSTR_MEM_ACTION_WRITE, "fsm create");
    ogs_fsm_init(&ausf_ue->sm, &e);
    instr_state_logging_child("ausf_ue_t", "sm", INSTR_MEM_ACTION_WRITE, "fsm init");

//    ogs_list_add(&self.ausf_ue_list, ausf_ue);
    instr_state_logging_child("ausf_context_t", "ausf_ue_list", INSTR_MEM_ACTION_WRITE, "add ue to list");

//    ogs_info("[state] ue list add ue");
    encode_and_save_ue_data(store_kv, ausf_ue);
    destroy_store_keyvalue(store_kv);

    instr_stop_timing("ausf_ue_add");

    return ausf_ue;
}

void update_ausf_ue(ausf_ue_t *ausf_ue) {
  instr_start_timing();
  StoreKeyValue *store_kv = create_store_keyvalue();

  // one of the original keys is enough for a data update
  add_store_key(store_kv, ausf_ue->suci);

  encode_and_save_ue_data(store_kv, ausf_ue);
  destroy_store_keyvalue(store_kv);

//  ogs_hash_set(self.suci_hash, ausf_ue->suci, strlen(ausf_ue->suci), NULL);
//  ogs_hash_set(self.supi_hash, ausf_ue->supi, strlen(ausf_ue->supi), NULL);

  ogs_pool_free(&ausf_ue_pool, ausf_ue);

  instr_stop_timing_autofun();
}

void encode_and_save_ue_data(StoreKeyValue *store_kv, ausf_ue_t* ausf_ue) {
  AusfUeSEnc request = AUSF_UE_S_ENC__INIT;

  request.ctx_id = ausf_ue->ctx_id;
  request.suci = ausf_ue->suci;
  request.supi = ausf_ue->supi;
  request.serving_network_name = ausf_ue->serving_network_name;

  request.auth_type = ausf_ue->auth_type;
  request.auth_events_url = ausf_ue->auth_events_url;
  request.auth_result = ausf_ue->auth_result;

  request.rand.data = ausf_ue->rand;
  request.rand.len = sizeof(ausf_ue->rand);
  request.xres_star.data = ausf_ue->xres_star;
  request.xres_star.len = sizeof(ausf_ue->xres_star);
  request.hxres_star.data = ausf_ue->hxres_star;
  request.hxres_star.len = sizeof(ausf_ue->hxres_star);
  request.kausf.data = ausf_ue->kausf;
  request.kausf.len = sizeof(ausf_ue->kausf);
  request.kseaf.data = ausf_ue->kseaf;
  request.kseaf.len = sizeof(ausf_ue->kseaf);

  if (OGS_FSM_CHECK(&ausf_ue->sm, &ausf_ue_state_operational)) {
    ogs_info("&ausf_state_operational");
    request.fsm_state = (char *)"ausf_ue_state_operational";
  } else if (OGS_FSM_CHECK(&ausf_ue->sm, &ausf_ue_state_final)) {
    ogs_info("&ausf_state_final");
    request.fsm_state = (char *)"ausf_ue_state_final";
  } else if (OGS_FSM_CHECK(&ausf_ue->sm, &ausf_ue_state_initial)) {
    ogs_info("&ausf_state_initial");
    request.fsm_state = (char *)"ausf_ue_state_initial";
  } else {
    ogs_warn("Can't encode fsm state");
  }
  ogs_info("fsm state: %s", request.fsm_state);

  size_t request_size = ausf_ue_s_enc__get_packed_size(&request);
  uint8_t *data_out = malloc(request_size);
  ausf_ue_s_enc__pack(&request, data_out);

  set_store_data(store_kv, data_out, request_size);
  save_store_data(store_kv);

  free(data_out);
}

void decode_ue_data(uint8_t *store_data, size_t data_size, ausf_ue_t* ausf_ue) {
  instr_start_timing();
  AusfUeSEnc *response = ausf_ue_s_enc__unpack(NULL, data_size, store_data);

  ausf_ue->ctx_id = ogs_strdup(response->ctx_id);
  ausf_ue->suci = ogs_strdup(response->suci);
  ausf_ue->supi = ogs_strdup(response->supi);
  ausf_ue->serving_network_name = ogs_strdup(response->serving_network_name);

  ausf_ue->auth_type = response->auth_type;
  ausf_ue->auth_events_url = ogs_strdup(response->auth_events_url);
  ausf_ue->auth_result = response->auth_result;

  memcpy(ausf_ue->rand, response->rand.data, response->rand.len);
  memcpy(ausf_ue->xres_star, response->xres_star.data, response->xres_star.len);
  memcpy(ausf_ue->hxres_star, response->hxres_star.data, response->hxres_star.len);
  memcpy(ausf_ue->kausf, response->kausf.data, response->kausf.len);
  memcpy(ausf_ue->kseaf, response->kseaf.data, response->kseaf.len);

  ogs_fsm_create(&ausf_ue->sm, ausf_ue_state_initial, ausf_ue_state_final);
  ogs_info("fsm state: %s", response->fsm_state);
  if(strcmp(response->fsm_state, "ausf_ue_state_operational") == 0) {
    ogs_info("ausf_ue_state_operational");
    OGS_FSM_TRAN(&ausf_ue->sm, &ausf_ue_state_operational);
  } else if(strcmp(response->fsm_state, "ausf_ue_state_final") == 0) {
    ogs_info("ausf_ue_state_final");
    OGS_FSM_TRAN(&ausf_ue->sm, &ausf_ue_state_final);
  }

  ausf_ue_s_enc__free_unpacked(response, NULL);
  instr_stop_timing_autofun();
}

void ausf_ue_remove(ausf_ue_t *ausf_ue)
{
    instr_start_timing();
    ausf_event_t e;

    ogs_assert(ausf_ue);

//    ogs_list_remove(&self.ausf_ue_list, ausf_ue);
    instr_state_logging_child("ausf_context_t", "ausf_ue_list", INSTR_MEM_ACTION_WRITE, "remove ue from list");
//    ogs_info("[state] remove ue from list");

    memset(&e, 0, sizeof(e));
    e.ausf_ue = ausf_ue;
    ogs_fsm_fini(&ausf_ue->sm, &e);
    instr_state_logging_child("ausf_ue_t", "sm", INSTR_MEM_ACTION_WRITE, "fsm fini");
    ogs_fsm_delete(&ausf_ue->sm);
    instr_state_logging_child("ausf_ue_t", "sm", INSTR_MEM_ACTION_WRITE, "fsm delete");
//    ogs_info("[state] ue fini sm");

    /* Free SBI object memory */
    ogs_sbi_object_free(&ausf_ue->sbi);
    instr_state_logging_child("ausf_ue_t", "sbi", INSTR_MEM_ACTION_FREE, "");
//    ogs_info("[state] ue free sbi");

    ogs_assert(ausf_ue->ctx_id);
    ogs_free(ausf_ue->ctx_id);
    instr_state_logging_child("ausf_ue_t", "ctx_id", INSTR_MEM_ACTION_FREE, "");
//    ogs_info("[state] ue free ctx_id");

    ogs_assert(ausf_ue->suci);
//    ogs_hash_set(self.suci_hash, ausf_ue->suci, strlen(ausf_ue->suci), NULL);
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_WRITE, "remove entry from hash");
//    ogs_info("[state] context set suci_hash");
    ogs_free(ausf_ue->suci);
    instr_state_logging_child("ausf_ue_t", "suci", INSTR_MEM_ACTION_FREE, "");
//    ogs_info("[state] ue free suci");

    ogs_assert(ausf_ue->supi);
//    ogs_hash_set(self.supi_hash, ausf_ue->supi, strlen(ausf_ue->supi), NULL);
    instr_state_logging_child("ausf_context_t", "supi_hash", INSTR_MEM_ACTION_WRITE, "remove entry from hash");
//    ogs_info("[state] context set supi_hash");
    ogs_free(ausf_ue->supi);
    instr_state_logging_child("ausf_ue_t", "supi", INSTR_MEM_ACTION_FREE, "");
//    ogs_info("[state] ue free supi");

    if (ausf_ue->auth_events_url) {
      ogs_free(ausf_ue->auth_events_url);
      instr_state_logging_child("ausf_ue_t", "auth_events_url", INSTR_MEM_ACTION_FREE, "");
//      ogs_info("[state] ue free auth_events_url");
    }

    if (ausf_ue->serving_network_name) {
      ogs_free(ausf_ue->serving_network_name);
      instr_state_logging_child("ausf_ue_t", "serving_network_name", INSTR_MEM_ACTION_FREE, "");
//      ogs_info("[state] ue free serving_network_name");
    }
    
    ogs_pool_free(&ausf_ue_pool, ausf_ue);
    instr_state_logging("ausf_ue_pool", INSTR_MEM_ACTION_WRITE, "mark ue as free in pool");
//    ogs_info("[state] ue pool free ue");
    instr_stop_timing("ausf_ue_remove");
}

void ausf_ue_remove_all()
{
    ausf_ue_t *ausf_ue = NULL, *next = NULL;;

//    ogs_list_for_each_safe(&self.ausf_ue_list, next, ausf_ue)
//        ausf_ue_remove(ausf_ue);

}

ausf_ue_t *create_ausf_ue_from_data_store(uint8_t *store_data, size_t data_size) {
  instr_start_timing();
  ausf_ue_t *ausf_ue = NULL;

  ogs_pool_alloc(&ausf_ue_pool, &ausf_ue);
  ogs_assert(ausf_ue);
  memset(ausf_ue, 0, sizeof *ausf_ue);

  decode_ue_data(store_data, data_size, ausf_ue);

//  delete_key_from_data_store(ausf_ue->ctx_id);
//  ausf_ue->ctx_id =
//      ogs_msprintf("%d", (int)ogs_pool_index(&ausf_ue_pool, ausf_ue));
//  ogs_assert(ausf_ue->ctx_id);
//  add_key_from_data_store(ausf_ue->suci, ausf_ue->ctx_id);

  ogs_assert(ausf_ue->suci);
//  ogs_hash_set(self.suci_hash, ausf_ue->suci, strlen(ausf_ue->suci), ausf_ue);

  ogs_assert(ausf_ue->supi);
//  ogs_hash_set(self.supi_hash, ausf_ue->supi, strlen(ausf_ue->supi), ausf_ue);

//  ogs_list_add(&self.ausf_ue_list, ausf_ue);
  instr_stop_timing_autofun();

  return ausf_ue;
}

void get_from_data_store(char *key, ausf_ue_t **ausf_ue_p) {
  uint8_t *data_out = NULL;
  size_t data_size = 0;

  get_store_data(key, &data_out, &data_size);

  if(data_size > 0) {
    ogs_info("store size greater 0, we got data");
    if(!(*ausf_ue_p)) {
      (*ausf_ue_p) = create_ausf_ue_from_data_store(data_out, data_size);
    } else {
      decode_ue_data(data_out, data_size, (*ausf_ue_p));
    }
  }
  free(data_out);
}

ausf_ue_t *ausf_ue_find_by_suci(char *suci)
{
    instr_start_timing();
    ogs_assert(suci);
    ausf_ue_t *ausf_ue = NULL; // = (ausf_ue_t *)ogs_hash_get(self.suci_hash, suci, strlen(suci));
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_READ, "");
    instr_state_logging("ausf_ue_t", INSTR_MEM_ACTION_READ, "");

    get_from_data_store(suci, &ausf_ue);

    instr_stop_timing("ausf_ue_find_by_suci");
    return ausf_ue;
}

ausf_ue_t *ausf_ue_find_by_supi(char *supi)
{
    instr_start_timing();
    ogs_assert(supi);
    ausf_ue_t *ausf_ue = NULL; //= (ausf_ue_t *)ogs_hash_get(self.supi_hash, supi, strlen(supi));
    instr_state_logging_child("ausf_context_t", "suci_hash", INSTR_MEM_ACTION_READ, "");
    instr_state_logging("ausf_ue_t", INSTR_MEM_ACTION_READ, "");

    get_from_data_store(supi, &ausf_ue);

    instr_stop_timing("ausf_ue_find_by_supi");
    return ausf_ue;
}

ausf_ue_t *ausf_ue_find_by_suci_or_supi(char *suci_or_supi)
{
    ogs_assert(suci_or_supi);
    if (strncmp(suci_or_supi, "suci-", strlen("suci-")) == 0)
        return ausf_ue_find_by_suci(suci_or_supi);
    else
        return ausf_ue_find_by_supi(suci_or_supi);
}

ausf_ue_t *ausf_ue_find_by_ctx_id(char *ctx_id)
{
    instr_start_timing();
    ogs_assert(ctx_id);
    ausf_ue_t *ausf_ue = NULL; // = ogs_pool_find(&ausf_ue_pool, atoll(ctx_id));
    instr_state_logging("ausf_ue_pool", INSTR_MEM_ACTION_READ, "");
    instr_state_logging("ausf_ue_t", INSTR_MEM_ACTION_READ, "");

    get_from_data_store(ctx_id, &ausf_ue);

    instr_stop_timing("ausf_ue_find_by_ctx_id");
    return ausf_ue;
}

ausf_ue_t *ausf_ue_cycle(ausf_ue_t *ausf_ue)
{
    instr_start_timing();
    ausf_ue_t *ausf_ue_ret = ausf_ue_find_by_suci(ausf_ue->suci);
//    ausf_ue_t *ausf_ue_ret = ogs_pool_cycle(&ausf_ue_pool, ausf_ue);

    instr_stop_timing("ausf_ue_cycle");
    return ausf_ue_ret;
}

void ausf_ue_select_nf(ausf_ue_t *ausf_ue, OpenAPI_nf_type_e nf_type)
{
    ogs_assert(ausf_ue);
    ogs_assert(nf_type);

    if (nf_type == OpenAPI_nf_type_NRF) {
      ogs_sbi_select_nrf(&ausf_ue->sbi, ausf_nf_state_registered);
      instr_state_logging_child("ausf_ue_t", "sbi", INSTR_MEM_ACTION_WRITE, "");
//      ogs_info("[state] ue change sbi");
    } else {
      ogs_sbi_select_first_nf(&ausf_ue->sbi, nf_type, ausf_nf_state_registered);
      instr_state_logging_child("ausf_ue_t", "sbi", INSTR_MEM_ACTION_WRITE, "");
//      ogs_info("[state] ue change sbi");
    }
}
