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

#ifndef REST_UTILS_H
#define REST_UTILS_H

#include <stdlib.h>
#include <stdint.h>

typedef struct
{
    char *uuid;
    uint8_t *psk;
    size_t psk_len;
    uint8_t *psk_id;
    size_t psk_id_len;
} database_entry_t;

int coap_to_http_status(int status);

void free_database_entry(database_entry_t *device);

#endif // REST_UTILS_H

