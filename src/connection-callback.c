/*
 * Punica - LwM2M server with REST API
 * Copyright (C) 2018 8devices
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "restserver.h"
#include "connection-secure.h"

#include <mbedtls/ssl.h>
#include <mbedtls/oid.h>

int psk_callback(void *p_cont, mbedtls_ssl_context *ssl, const unsigned char *name, size_t name_len)
{
    coap_settings_t *coap = (coap_settings_t *)p_cont;
    device_database_t *curr = (device_database_t *)(coap->security);

    while (curr != NULL)
    {
        if (memcmp(name, curr->psk_id, name_len) == 0)
        {
            return mbedtls_ssl_set_hs_psk(ssl, curr->psk, curr->psk_len);
        }
        curr = curr->next;
    }

    return -1;
}

uint8_t lwm2m_buffer_send(void *sessionH, uint8_t *buffer, size_t length, void *userData)
{
    connection_api_t *connApi = (connection_api_t *)userData;

    if (sessionH == NULL)
    {
        fprintf(stderr, "#> failed sending %lu bytes, missing connection\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    if (connApi->f_send(sessionH, buffer, length) < 0)
    {
        fprintf(stderr, "#> failed sending %lu bytes\r\n", length);
        return COAP_500_INTERNAL_SERVER_ERROR;
    }

    return COAP_NO_ERROR;
}

bool lwm2m_session_is_equal(void *session1, void *session2, void *userData)
{
    return (session1 == session2);
}

int lwm2m_client_validate(char *name, void *fromSessionH)
{
    mbedtls_connection_t * conn = (mbedtls_connection_t *)fromSessionH;
    const char * short_name;
    int ret;

    //check if using cipher with certificate
    if ((conn->ssl->session->ciphersuite != MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CCM_8) && (conn->ssl->session->ciphersuite != MBEDTLS_TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256))
    {
        return 0;
    }

    mbedtls_x509_name * subject = &conn->ssl->session->peer_cert->subject;
    while (subject != NULL)
    {
        ret = mbedtls_oid_get_attr_short_name(&subject->oid, &short_name);

        if (ret == 0)
        {
            if (strcmp(short_name, "CN") == 0)
            {
                if (strncmp((const char *)subject->val.p, name, subject->val.len) == 0)
                {
                    return 0;
                }

                return COAP_400_BAD_REQUEST;
            }
        }

        subject = subject->next;
    }

    return COAP_500_INTERNAL_SERVER_ERROR;
}
