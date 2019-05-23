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

#include <cstring>

#include <punica/rest/http_codes.h>

#include "ulfius_callback_handler.hpp"

#include "ulfius_request.hpp"
#include "ulfius_response.hpp"

static int ulfiusCallback(const struct _u_request *ulfiusRequest,
                          struct _u_response *ulfiusResponse,
                          void *handlerContext)
{
    UlfiusCallbackHandler *handler =
        static_cast<UlfiusCallbackHandler *>(handlerContext);
    UlfiusRequest request(ulfiusRequest);
    UlfiusResponse response(ulfiusResponse);
    int statusCode;

    statusCode = handler->callFunction(request, response);
    response.setCode(statusCode);

    return U_CALLBACK_COMPLETE;

    switch (statusCode)
    {
    case HTTP_100_CONTINUE:
        return U_CALLBACK_CONTINUE;

    case HTTP_401_UNAUTHORIZED:
        return U_CALLBACK_UNAUTHORIZED;

    case HTTP_500_INTERNAL_ERROR:
        return U_CALLBACK_ERROR;

    default:
        return U_CALLBACK_COMPLETE;
    }
}

UlfiusCallbackHandler::UlfiusCallbackHandler(struct _u_instance *ulfius,
                                             const std::string method,
                                             const std::string prefix,
                                             const std::string format,
                                             unsigned int priority,
                                             punica::rest::CallbackFunction *function,
                                             void *context):
    mUlfius(ulfius),
    mMethod(method),
    mUrlPrefix(prefix),
    mUrlFormat(format),
    mFunction(function),
    mContext(context)
{
    ulfius_add_endpoint_by_val(mUlfius, mMethod.c_str(),
                               mUrlPrefix.c_str(), mUrlFormat.c_str(),
                               priority, &ulfiusCallback, this);
}

UlfiusCallbackHandler::~UlfiusCallbackHandler()
{
    ulfius_remove_endpoint_by_val(mUlfius, mMethod.c_str(),
                                  mUrlPrefix.c_str(), mUrlFormat.c_str());
}

int UlfiusCallbackHandler::callFunction(punica::rest::Request &request,
                                        punica::rest::Response &response)
{
    return mFunction(request, response, mContext);
}

const std::string UlfiusCallbackHandler::getMethod()
{
    return mMethod;
}

const std::string UlfiusCallbackHandler::getUrlPrefix()
{
    return mUrlPrefix;
}

const std::string UlfiusCallbackHandler::getUrlFormat()
{
    return mUrlFormat;
}
