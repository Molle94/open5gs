import zmq
from google.protobuf.json_format import MessageToJson
import defs_pb2
import uuid

DATASTORE = {}
KEYS_TO_ID = {}


def find_by_key(key) -> defs_pb2.StoreKeyValue:
    print(f"Searching for key {key}")
    if data_key := KEYS_TO_ID.get(key):
        print(f"Found data for key '{key}'")
        return DATASTORE[data_key]
    return None

def update_or_insert(store_request: defs_pb2.StoreKeyValue):
    for key in store_request.key:
        if data_key := KEYS_TO_ID.get(key):
            print(f"Found existing key. Updating data...")
            DATASTORE[data_key] = store_request
            data = store_request.data
            decoded = defs_pb2.ausf_ue_s_enc()
            decoded.ParseFromString(data)
            print(MessageToJson(decoded))
            print(f"Updating keys...")
            for nkey in store_request.key:
                KEYS_TO_ID[nkey] = data_key
            break

    if data_key is None:
        print(f"Object not found in data store. Inserting...")
        data_key = uuid.uuid4()
        DATASTORE[data_key] = store_request
        for key in store_request.key:
            KEYS_TO_ID[key] = data_key

    print(f"Store contains {len(DATASTORE)} elements")

def main():
    context = zmq.Context()
    socket = context.socket(zmq.REP)
    socket.bind("tcp://*:4242")

    while True:
        #  Wait for next request from client
        message = socket.recv()
        print(f"Received request of size {len(message)}")

        request = defs_pb2.DatastoreRequest()
        request.ParseFromString(message)

        request_type = request.WhichOneof('request')
        print(f"Request is {request_type}")

        response = defs_pb2.DatastoreResponse()

        if request_type == 'storeKeyValue':
            update_or_insert(request.storeKeyValue)
        else:
            if stored_data := find_by_key(request.getValue.key):
                data_response = defs_pb2.GetValueResponse()
                data_response.data = stored_data.data
                response.getValueResponse.CopyFrom(data_response)

        response.status = defs_pb2.DatastoreResponse.ResponseStatus.Value('SUCCESS')

        response_message = response.SerializeToString()

        #  Send reply back to client
        socket.send(response_message)


if __name__ == "__main__":
    main()