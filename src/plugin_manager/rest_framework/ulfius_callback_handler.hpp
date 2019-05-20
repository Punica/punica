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

#ifndef PUNICA_PLUGIN_MANAGER_REST_ULFIUS_CALLBACK_HANDLER_HPP
#define PUNICA_PLUGIN_MANAGER_REST_ULFIUS_CALLBACK_HANDLER_HPP

#include <memory>

#include <punica/rest/core.hpp>

extern "C" {
#include <ulfius.h>
}

class UlfiusCallbackHandler
{
public:
    typedef std::unique_ptr<UlfiusCallbackHandler> ptr;
    typedef std::vector<UlfiusCallbackHandler::ptr> vector;

    UlfiusCallbackHandler(struct _u_instance *ulfius,
                          const std::string method,
                          const std::string prefix,
                          const std::string format,
                          unsigned int priority,
                          punica::rest::CallbackFunction *handlerFunction,
                          void *handlerContext);
    ~UlfiusCallbackHandler();

    int callFunction(punica::rest::Request &request,
                     punica::rest::Response &response);
    const std::string getMethod();
    const std::string getUrlPrefix();
    const std::string getUrlFormat();

private:
    struct _u_instance *mUlfius;
    std::string mMethod, mUrlPrefix, mUrlFormat;
    punica::rest::CallbackFunction *mFunction;
    void *mContext;
};

#endif // PUNICA_PLUGIN_MANAGER_REST_ULFIUS_CALLBACK_HANDLER_HPP
