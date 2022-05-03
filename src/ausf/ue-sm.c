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

#include "sbi-path.h"
#include "nnrf-handler.h"
#include "nausf-handler.h"
#include "nudm-handler.h"

void ausf_ue_state_initial(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    OGS_FSM_TRAN(s, &ausf_ue_state_operational);
}

void ausf_ue_state_final(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);
}

void ausf_ue_state_operational(ogs_fsm_t *s, ausf_event_t *e)
{
    bool handled;
    ausf_ue_t *ausf_ue = NULL;

    ogs_sbi_stream_t *stream = NULL;
    ogs_sbi_message_t *message = NULL;

    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    case AUSF_EVT_SBI_SERVER:
        message = e->sbi.message;
        ogs_assert(message);
        stream = e->sbi.data;
        ogs_assert(stream);

        SWITCH(message->h.method)
        CASE(OGS_SBI_HTTP_METHOD_POST)
            handled = ausf_nausf_auth_handle_authenticate(
                    ausf_ue, stream, message);
            if (!handled) {
                ogs_error("[%s] Cannot handle SBI message",
                        ausf_ue->suci);
                OGS_FSM_TRAN(s, ausf_ue_state_exception);
            }
            break;
        CASE(OGS_SBI_HTTP_METHOD_PUT)
            handled = ausf_nausf_auth_handle_authenticate_confirmation(
                    ausf_ue, stream, message);
            if (!handled) {
                ogs_error("[%s] Cannot handle SBI message",
                        ausf_ue->suci);
                OGS_FSM_TRAN(s, ausf_ue_state_exception);
            }
            break;
        DEFAULT
            ogs_error("[%s] Invalid HTTP method [%s]",
                    ausf_ue->suci, message->h.method);
            ogs_assert(true ==
                ogs_sbi_server_send_error(stream,
                    OGS_SBI_HTTP_STATUS_FORBIDDEN, message,
                    "Invalid HTTP method", message->h.method));
        END
        update_ausf_ue(ausf_ue);
        break;

    case AUSF_EVT_SBI_CLIENT:
        message = e->sbi.message;
        ogs_assert(message);

        ausf_ue = e->ausf_ue;
        ogs_assert(ausf_ue);
        stream = e->sbi.data;
        ogs_assert(stream);

        SWITCH(message->h.service.name)
        CASE(OGS_SBI_SERVICE_NAME_NUDM_UEAU)
            if (message->res_status != OGS_SBI_HTTP_STATUS_OK &&
                message->res_status != OGS_SBI_HTTP_STATUS_CREATED) {
                if (message->res_status == OGS_SBI_HTTP_STATUS_NOT_FOUND) {
                    ogs_warn("[%s] Cannot find SUPI [%d]",
                        ausf_ue->suci, message->res_status);
                } else {
                    ogs_error("[%s] HTTP response error [%d]",
                        ausf_ue->suci, message->res_status);
                }
                ogs_assert(true ==
                    ogs_sbi_server_send_error(
                        stream, message->res_status,
                        NULL, "HTTP response error", ausf_ue->suci));
                break;
            }

            SWITCH(message->h.resource.component[1])
            CASE(OGS_SBI_RESOURCE_NAME_SECURITY_INFORMATION)
                ausf_nudm_ueau_handle_get(ausf_ue, stream, message);
                break;

            CASE(OGS_SBI_RESOURCE_NAME_AUTH_EVENTS)
                ausf_nudm_ueau_handle_result_confirmation_inform(
                        ausf_ue, stream, message);
                break;

            DEFAULT
                ogs_error("[%s] Invalid HTTP method [%s]",
                        ausf_ue->suci, message->h.method);
                ogs_assert_if_reached();
            END
            break;

        DEFAULT
            ogs_error("Invalid API name [%s]", message->h.service.name);
            ogs_assert_if_reached();
        END

        update_ausf_ue(ausf_ue);
        break;


    default:
        ogs_error("[%s] Unknown event %s",
                ausf_ue->suci, ausf_event_get_name(e));
        break;
    }
}

void ausf_ue_state_exception(ogs_fsm_t *s, ausf_event_t *e)
{
    ausf_ue_t *ausf_ue = NULL;
    ogs_assert(s);
    ogs_assert(e);

    ausf_sm_debug(e);

    ausf_ue = e->ausf_ue;
    ogs_assert(ausf_ue);

    switch (e->id) {
    case OGS_FSM_ENTRY_SIG:
        break;

    case OGS_FSM_EXIT_SIG:
        break;

    default:
        ogs_error("[%s] Unknown event %s", ausf_ue->suci, ausf_event_get_name(e));
        break;
    }
}
