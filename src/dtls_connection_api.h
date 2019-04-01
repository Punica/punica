/*
 * Punica - LwM2M server with REST API
 * Copyright (C) 2019 8devices
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

#ifndef DTLS_CONNECTION_API_H_
#define DTLS_CONNECTION_API_H_

#include "punica.h"

/*
 * Initialize a DTLS connection context
 *
 * Parameters:
 *      api - API context pointer. Is set after return,
 *      port - UDP port to bind to,
 *      address_family - UDP socket family. Can be: AF_INET, AF_INET6 or AF_UNSPEC,
 *      cert_file - path to a ECDHE-ECDSA certificate in file system,
 *      key_file - path to a matching x509 private key file,
 *      data - pointer to a data structure for use in a PSK authentication callback,
 *      psk_cb - pointer to callback used during DTLS handshake with PSK key exchange
 *
 * Returns:
 *      0 on success,
 *      negative value on error
 */
connection_api_t *dtls_connection_api_init(int port, int address_family, const char *cert_file,
                                           const char *key_file, void *data, f_psk_cb_t psk_cb, f_handshake_done_cb_t handshake_done_cb);

/*
 * Deinitialize a DTLS connection context
 *
 * Parameters:
 *      context - API context poiner
 */
void dtls_connection_api_deinit(void *context);

#endif
