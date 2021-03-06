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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <punica/version.h>
#include "linked_list.h"
#include "settings.h"
#include "security.h"
#include "rest/rest_core_types.h"

#define DATABASE_UUID_KEY_BIT       0x1
#define DATABASE_PSK_KEY_BIT        0x2
#define DATABASE_PSK_ID_KEY_BIT     0x4
#define DATABASE_ALL_KEYS_SET       0x7

const char *argp_program_version = PUNICA_FULL_VERSION;
static const char *logging_section = "[SETTINGS]";

static char doc[] = "Punica - REST interface to LwM2M server and all clients connected to it";

static struct argp_option options[] =
{
    {"log",   'l', "LOGGING_LEVEL", 0, "Specify logging level (0-5)" },
    {"config",   'c', "FILE", 0, "Specify parameters configuration file" },
    {"database",   'd', "FILE", 0, "Specify device database file" },
    {"private_key",   'k', "PRIVATE_KEY", 0, "Specify TLS security private key file" },
    {"certificate",   'C', "CERTIFICATE", 0, "Specify TLS security certificate file" },
    { 0 }
};

static void set_coap_settings(json_t *j_section, coap_settings_t *settings)
{
    const char *key;
    const char *section_name = "coap";
    const char *string_value;
    json_t *j_value;

    json_object_foreach(j_section, key, j_value)
    {
        if (strcasecmp(key, "mode") == 0)
        {
            if (json_is_integer(j_value))
            {
                settings->security_mode = (uint16_t) json_integer_value(j_value);
            }
            else
            {
                fprintf(stdout, "value at key %s:%s must be an integer",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "port") == 0)
        {
            if (json_is_integer(j_value))
            {
                settings->port = (uint16_t) json_integer_value(j_value);
            }
            else
            {
                fprintf(stdout, "value at key %s:%s must be an integer",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "private_key_file") == 0)
        {
            if (json_is_string(j_value))
            {
                string_value = json_string_value(j_value);

                settings->private_key_file = strdup(string_value);
                if (settings->private_key_file == NULL)
                {
                    fprintf(stderr, "fatal error while parsing value at key %s:%s",
                            section_name, key);
                }
            }
            else
            {
                fprintf(stdout, "value at key %s:%s must be a string",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "certificate_file") == 0)
        {
            if (json_is_string(j_value))
            {
                string_value = json_string_value(j_value);

                settings->certificate_file = strdup(string_value);
                if (settings->certificate_file == NULL)
                {
                    fprintf(stderr, "fatal error while parsing value at key %s:%s",
                            section_name, key);
                }
            }
            else
            {
                fprintf(stdout, "value at key %s:%s must be a string",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "database_file") == 0)
        {
            if (json_is_string(j_value))
            {
                string_value = json_string_value(j_value);

                settings->database_file = strdup(string_value);
                if (settings->database_file == NULL)
                {
                    fprintf(stderr, "fatal error while parsing value at key %s:%s",
                            section_name, key);
                }
            }
            else
            {
                fprintf(stdout, "value at key %s:%s must be a string",
                        section_name, key);
            }
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file key: %s.%s\n",
                    section_name, key);
        }
    }
}

static int validate_user_settings_scope(json_t *j_scope)
{
    json_t *j_scope_value;
    char *scope_value;
    size_t scope_length, scope_index;

    if (!json_is_array(j_scope))
    {
        return -1;
    }

    json_array_foreach(j_scope, scope_index, j_scope_value)
    {
        if (!json_is_string(j_scope_value))
        {
            fprintf(stdout, "User scope must be an array!\n");
            return -1;
        }

        scope_value = (char *) json_string_value(j_scope_value);
        scope_length = strnlen(scope_value,
                               J_MAX_LENGTH_METHOD + 1 + J_MAX_LENGTH_URL);
        if ((scope_length == 0)
            || (scope_length == J_MAX_LENGTH_METHOD + 1 + J_MAX_LENGTH_URL))
        {
            fprintf(stdout, "User scope array must contain regex strings!\n");
            return -1;
        }
    }

    return 0;
}

static int set_user_settings(json_t *user_settings, linked_list_t *users_list)
{
    user_t *user, *user_entry;
    linked_list_entry_t *entry;
    json_t *j_name, *j_secret, *j_scope;
    const char *user_name, *user_secret;
    size_t user_name_length, user_secret_length;

    j_name = json_object_get(user_settings, "name");
    j_secret = json_object_get(user_settings, "secret");
    j_scope = json_object_get(user_settings, "scope");

    if (!json_is_string(j_name) || strlen(json_string_value(j_name)) < 1)
    {
        fprintf(stdout, "User configured without name.\n");
        return 1;
    }

    user_name = json_string_value(j_name);
    user_name_length = strnlen(user_name, J_MAX_LENGTH_USER_NAME);
    if (user_name_length == 0 || user_name_length == J_MAX_LENGTH_USER_NAME)
    {
        fprintf(stdout, "User name length is invalid\n");
        return 1;
    }

    for (entry = users_list->head; entry != NULL; entry = entry->next)
    {
        user_entry = entry->data;

        if (strncmp(user_entry->name, user_name, J_MAX_LENGTH_USER_NAME) == 0)
        {
            fprintf(stdout, "Found duplicate \"%s\" user name in config\n", user_name);
            return 1;
        }
    }

    if (!json_is_string(j_secret))
    {
        fprintf(stdout, "User \"%s\" configured without valid secret key.\n", user_name);
        return 1;
    }

    user_secret = json_string_value(j_secret);
    user_secret_length = strnlen(user_secret, J_MAX_LENGTH_USER_SECRET);
    if (user_secret_length == J_MAX_LENGTH_USER_NAME)
    {
        fprintf(stdout, "User secret length is invalid\n");
        return 1;
    }

    if (validate_user_settings_scope(j_scope) != 0)
    {
        fprintf(stdout,
                "User \"%s\" configured without valid scope.\n",
                user_name);
        return 1;
    }

    user = security_user_new();

    security_user_set(user, user_name, user_secret, j_scope);

    linked_list_add(users_list, user);

    return 0;
}

static void set_jwt_settings(json_t *j_section, jwt_settings_t *settings)
{
    size_t user_index, value_length;
    const char *key, *string_value;
    const char *section_name = "http.security.jwt";
    json_t *j_value, *j_user_settings;
    jwt_init(settings);

    json_object_foreach(j_section, key, j_value)
    {
        if (strcasecmp(key, "algorithm") == 0)
        {
            settings->algorithm = jwt_str_alg(json_string_value(j_value));
        }
        else if (strcasecmp(key, "expiration_time") == 0)
        {
            if (json_is_integer(j_value))
            {
                settings->expiration_time = json_integer_value(j_value);
            }
            else
            {
                fprintf(stdout, "Token %s must be an integer\n", key);
            }
        }
        else if (strcasecmp(key, "secret_key") == 0)
        {
            if (!json_is_string(j_value))
            {
                fprintf(stdout, "Token %s must be a string\n", key);
                continue;
            }

            string_value = json_string_value(j_value);
            value_length = strnlen(string_value, J_MAX_LENGTH_SECRET_KEY);
            if (value_length == 0 || value_length == J_MAX_LENGTH_SECRET_KEY)
            {
                fprintf(stdout, "Token %s length is invalid\n", key);
                continue;
            }

            if (settings->secret_key != NULL)
            {
                free(settings->secret_key);
            }

            settings->secret_key_length = value_length;
            settings->secret_key = (unsigned char *) malloc(settings->secret_key_length * sizeof(
                                                                unsigned char));
            if (settings->secret_key == NULL)
            {
                fprintf(stderr, "Failed to allocate %s!\n", key);
                settings->secret_key_length = 0;
                continue;
            }
            memcpy(settings->secret_key, string_value, value_length);
        }
        else if (strcasecmp(key, "users") == 0)
        {
            if (json_is_array(j_value))
            {
                json_array_foreach(j_value, user_index, j_user_settings)
                {
                    if (json_is_object(j_user_settings))
                    {
                        set_user_settings(j_user_settings, settings->users_list);
                    }
                    else
                    {
                        fprintf(stdout, "User settings must be stored in an object\n");
                    }
                }
            }
            else
            {
                fprintf(stdout, "Users settings must be stored in objects list\n");
            }
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file key: %s.%s\n",
                    section_name, key);
        }
    }
}

static void set_http_security_settings(json_t *j_section, http_security_settings_t *settings)
{
    const char *key;
    const char *section_name = "http.security";
    const char *string_value;
    json_t *j_value;

    json_object_foreach(j_section, key, j_value)
    {
        if (strcasecmp(key, "private_key") == 0)
        {
            string_value = json_string_value(j_value);

            settings->private_key = strdup(string_value);
            if (settings->private_key == NULL)
            {
                fprintf(stderr, "fatal error while parsing value at key %s:%s",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "certificate") == 0)
        {
            string_value = json_string_value(j_value);

            settings->certificate = strdup(string_value);
            if (settings->certificate == NULL)
            {
                fprintf(stderr, "fatal error while parsing value at key %s:%s",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "jwt") == 0)
        {
            set_jwt_settings(j_value, &settings->jwt);
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file key: %s.%s\n",
                    section_name, key);
        }
    }
}

static void set_http_settings(json_t *j_section, http_settings_t *settings)
{
    const char *key, *section_name = "http";
    json_t *j_value;

    json_object_foreach(j_section, key, j_value)
    {
        if (strcasecmp(key, "port") == 0)
        {
            settings->port = (uint16_t) json_integer_value(j_value);
        }
        else if (strcasecmp(key, "security") == 0)
        {
            set_http_security_settings(j_value, &settings->security);
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file key: %s.%s\n",
                    section_name, key);
        }
    }
}

static void set_logging_settings(json_t *j_section, logging_settings_t *settings)
{
    const char *key;
    const char *section_name = "logging";
    json_t *j_value;

    json_object_foreach(j_section, key, j_value)
    {
        if (strcasecmp(key, "level") == 0)
        {
            settings->level = (logging_level_t) json_integer_value(j_value);
        }
        else if (strcasecmp(key, "timestamp") == 0)
        {
            if (json_is_boolean(j_value))
            {
                settings->timestamp = json_is_true(j_value) ? true : false;
            }
            else
            {
                fprintf(stdout, "%s.%s must be set to a boolean value!\n",
                        section_name, key);
            }
        }
        else if (strcasecmp(key, "human_readable_timestamp") == 0)
        {
            if (json_is_boolean(j_value))
            {
                settings->human_readable_timestamp = json_is_true(j_value) ? true : false;
            }
            else
            {
                fprintf(stdout, "%s.%s must be set to a boolean value!\n",
                        section_name, key);
            }
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file key: %s.%s\n",
                    section_name, key);
        }
    }
}

static int set_plugin_entry_settings(json_t *j_plugin_settings,
                                     linked_list_t *plugins_list)
{
    plugin_settings_t *plugin;
    size_t plugin_name_length, plugin_path_length;
    json_t *j_name = json_object_get(j_plugin_settings, "name");
    json_t *j_path = json_object_get(j_plugin_settings, "path");

    if (!json_is_string(j_name))
    {
        fprintf(stdout,
                "%s Plugin configured without name.\n",
                logging_section);
        return 1;
    }
    plugin_name_length = strnlen(json_string_value(j_name),
                                 J_MAX_LENGTH_PLUGIN_NAME);
    if (plugin_name_length == 0
        || plugin_name_length == J_MAX_LENGTH_PLUGIN_NAME)
    {
        fprintf(stdout,
                "%s Plugin name length is invalid.\n",
                logging_section);
        return 1;
    }

    if (!json_is_string(j_path))
    {
        fprintf(stdout,
                "%s Plugin \"%s\" configured without path.\n",
                logging_section, json_string_value(j_name));
        return 1;
    }
    plugin_path_length = strnlen(json_string_value(j_path),
                                 J_MAX_LENGTH_PLUGIN_NAME);
    if (plugin_path_length == 0
        || plugin_path_length == J_MAX_LENGTH_PLUGIN_PATH)
    {
        fprintf(stdout,
                "%s Plugin \"%s\" path length is invalid.\n",
                logging_section, json_string_value(j_name));
        return 1;
    }

    plugin = malloc(sizeof(plugin_settings_t));

    plugin->name = strdup(json_string_value(j_name));
    plugin->path = strdup(json_string_value(j_path));

    linked_list_add(plugins_list, plugin);

    return 0;
}

static int set_plugins_settings(json_t *j_section,
                                plugins_settings_t *plugins_settings)
{
    int plugin_status, plugins_status = 0;
    size_t plugin_index;
    const char *section_name = "plugins";
    json_t *j_plugin_settings;

    json_array_foreach(j_section, plugin_index, j_plugin_settings)
    {
        if (!json_is_object(j_plugin_settings))
        {
            fprintf(stdout,
                    "%s \"%s\" section contains invalid type value\n",
                    logging_section, section_name);
            plugins_status = 1;
        }

        plugin_status =
            set_plugin_entry_settings(j_plugin_settings,
                                      plugins_settings->plugins_list);
        if (plugin_status != 0)
        {
            plugins_status = plugin_status;
        }
    }

    if (plugins_status != 0)
    {
        fprintf(stdout,
                "%s Some entries in \"%s\" section aren't configured corretly!\n",
                logging_section, section_name);
    }

    return plugins_status;
}

static int read_config(char *config_name, settings_t *settings)
{
    json_error_t error;
    const char *section;
    json_t *j_value;

    json_t *settings_json = json_load_file(config_name, 0, &error);

    if (settings_json == NULL)
    {
        fprintf(stderr, "%s:%d:%d error:%s \n",
                config_name, error.line, error.column, error.text);
        return 1;
    }

    json_object_foreach(settings_json, section, j_value)
    {
        if (strcasecmp(section, "coap") == 0)
        {
            set_coap_settings(j_value, &settings->coap);
        }
        else if (strcasecmp(section, "http") == 0)
        {
            set_http_settings(j_value, &settings->http);
        }
        else if (strcasecmp(section, "logging") == 0)
        {
            set_logging_settings(j_value, &settings->logging);
        }
        else if (strcasecmp(section, "plugins") == 0)
        {
            set_plugins_settings(j_value, &settings->plugins);
        }
        else
        {
            fprintf(stdout, "Unrecognised configuration file section: %s\n", section);
        }
    }

    json_decref(settings_json);
    return 0;
}

error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    settings_t *settings = state->input;

    switch (key)
    {
    case 'l':
        settings->logging.level = atoi(arg);
        break;

    case 'c':
        if (read_config(arg, settings) != 0)
        {
            argp_usage(state);
            return 1;
        }
        break;

    case 'd':
        settings->coap.database_file = arg;
        break;

    case 'C':
        settings->http.security.certificate = arg;
        break;

    case 'k':
        settings->http.security.private_key = arg;
        break;


    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, 0, doc };

int settings_init(int argc, char *argv[], settings_t *settings)
{
    return argp_parse(&argp, argc, argv, 0, 0, settings);
}
