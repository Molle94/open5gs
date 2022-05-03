//
// Created by kinnarr on 16.04.22.
//

#include "statemanagement.h"

int __statemanagement_log_domain;

void init_statemanagement(void) {
//  ogs_log_install_domain(&__statemanagement_log_domain, "statemanagement", ogs_core()->log.level)

  ogs_info("starting data store connection");
  __statestore_socket = zsock_new_req("tcp://localhost:4242");
}

void finalize_statemanagement(void) {
  zsock_destroy(&__statestore_socket);
  ogs_info("data store connection destroyed");
}

StoreKeyValue *create_store_keyvalue(void) {
  StoreKeyValue *store_kv = malloc(sizeof(StoreKeyValue));
  store_key_value__init(store_kv);
//
//  store_kv->n_key = 0;
//  store_kv->key = NULL;

  return store_kv;
}

void add_store_key(StoreKeyValue *store_kv, char *key) {
    store_kv->n_key += 1;
    store_kv->key = realloc(store_kv->key, sizeof(char *) * store_kv->n_key);
    store_kv->key[store_kv->n_key - 1] = key;
}

void set_store_data(StoreKeyValue *store_kv, uint8_t *data, size_t data_size) {
  store_kv->data.data = data;
  store_kv->data.len = data_size;

}

void save_store_data(StoreKeyValue *store_kv) {
  ogs_info("store message ready to save");

  DatastoreRequest request = DATASTORE_REQUEST__INIT;

  request.storekeyvalue = store_kv;
  request.request_case = DATASTORE_REQUEST__REQUEST_STORE_KEY_VALUE;

  size_t request_size = datastore_request__get_packed_size(&request);
  uint8_t *data_out = malloc(request_size);

  datastore_request__pack(&request, data_out);
  ogs_info("store message encoded");

  zsock_send(__statestore_socket, "b", data_out, request_size);
  uint8_t *response_data;
  size_t response_size;
  zsock_recv(__statestore_socket, "b", &response_data, &response_size);

  ogs_info("store message response size: %zu", response_size);

  DatastoreResponse *response = datastore_response__unpack(NULL, response_size, response_data);

  if(response->status == DATASTORE_RESPONSE__RESPONSE_STATUS__SUCCESS) {
    ogs_info("store successful");
  } else {
    ogs_info("store failed");
  }

  datastore_response__free_unpacked(response, NULL);
  free(response_data);

  ogs_info("store message send");

  free(data_out);
}

void destroy_store_keyvalue(StoreKeyValue *store_kv) {
  free(store_kv->key);
  free(store_kv);
}

void get_store_data(char *key, uint8_t **data_out, size_t*data_size) {
  DatastoreRequest request = DATASTORE_REQUEST__INIT;
  GetValue innerRequest = GET_VALUE__INIT;

  innerRequest.key = key;

  request.getvalue = &innerRequest;
  request.request_case = DATASTORE_REQUEST__REQUEST_GET_VALUE;

  size_t request_size = datastore_request__get_packed_size(&request);
  uint8_t *request_data = malloc(request_size);

  datastore_request__pack(&request, request_data);
  ogs_info("get value request message encoded");

  zsock_send(__statestore_socket, "b", request_data, request_size);

  uint8_t *response_data;
  size_t response_size;
  zsock_recv(__statestore_socket, "b", &response_data, &response_size);

  free(request_data);

  ogs_info("get value response size: %zu", response_size);

  DatastoreResponse *response = datastore_response__unpack(NULL, response_size, response_data);

  if(response->status == DATASTORE_RESPONSE__RESPONSE_STATUS__SUCCESS) {
    ogs_info("get value successful");
    if(response->response_case == DATASTORE_RESPONSE__RESPONSE_GET_VALUE_RESPONSE) {
      ogs_info("get value found data");
      *data_size = response->getvalueresponse->data.len;
      *data_out = malloc(*data_size);
      memcpy(*data_out, response->getvalueresponse->data.data, *data_size);
    }

  } else {
    ogs_info("get value failed");
  }

  datastore_response__free_unpacked(response, NULL);
  free(response_data);
}

void delete_key_from_data_store(char *key) {
  DatastoreRequest request = DATASTORE_REQUEST__INIT;
  DeleteKey innerRequest = DELETE_KEY__INIT;

  innerRequest.key = key;
  innerRequest.deletevalue = false;

  request.deletekey = &innerRequest;
  request.request_case = DATASTORE_REQUEST__REQUEST_DELETE_KEY;

  size_t request_size = datastore_request__get_packed_size(&request);
  uint8_t *request_data = malloc(request_size);

  datastore_request__pack(&request, request_data);
  ogs_info("delete key request message encoded");

  zsock_send(__statestore_socket, "b", request_data, request_size);

  uint8_t *response_data;
  size_t response_size;
  zsock_recv(__statestore_socket, "b", &response_data, &response_size);

  free(request_data);

  ogs_info("delete key response size: %zu", response_size);

  DatastoreResponse *response = datastore_response__unpack(NULL, response_size, response_data);

  if(response->status == DATASTORE_RESPONSE__RESPONSE_STATUS__SUCCESS) {
    ogs_info("delete key successful");
  } else {
    ogs_info("delete key failed");
  }

  datastore_response__free_unpacked(response, NULL);
  free(response_data);
}

void add_key_from_data_store(char *existing_key, char *new_key) {
  DatastoreRequest request = DATASTORE_REQUEST__INIT;
  AddKey innerRequest = ADD_KEY__INIT;

  innerRequest.n_key = 2;
  innerRequest.key = malloc(sizeof(char *) * innerRequest.n_key);
  innerRequest.key[0] = existing_key;
  innerRequest.key[1] = new_key;

  request.addkey = &innerRequest;
  request.request_case = DATASTORE_REQUEST__REQUEST_ADD_KEY;

  size_t request_size = datastore_request__get_packed_size(&request);
  uint8_t *request_data = malloc(request_size);

  datastore_request__pack(&request, request_data);
  ogs_info("add key request message encoded");

  zsock_send(__statestore_socket, "b", request_data, request_size);

  uint8_t *response_data;
  size_t response_size;
  zsock_recv(__statestore_socket, "b", &response_data, &response_size);

  free(request_data);

  ogs_info("add key response size: %zu", response_size);

  DatastoreResponse *response = datastore_response__unpack(NULL, response_size, response_data);

  if(response->status == DATASTORE_RESPONSE__RESPONSE_STATUS__SUCCESS) {
    ogs_info("add key successful");
  } else {
    ogs_info("add key failed");
  }

  datastore_response__free_unpacked(response, NULL);
  free(response_data);
}