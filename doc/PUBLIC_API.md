# Public API

## Table of contents
1. [Overview](#Overview)
2. [Minimal requirements](#Minimal-requirements)
3. [Structure of Punica](#Structure-of-Punica)
    1. [Punica core](#Punica-core)
        1. [Punica core interface](#Punica-core-interface)
    2. [REST API](#REST-API)
        1. [Callback function](#Callback-function)
        2. [REST core interface](#REST-core-interface)
        3. [REST request interface](#REST-request-interface)
        4. [REST response interface](#REST-response-interface)
    3. [LwM2M API](#LwM2M-API)
        1. [LwM2M core interface](#LwM2M-core-interface)
    4. [Plugin API](#Plugin-API)
        1. [Plugin API structure](#Plugin-API-structure)
        2. [Plugin interface](#Plugin-interface)
        3. [Valid `PluginApi` example](#Valid-PluginApi-example)

## Overview
[Punica Public API](#Public-API) allows user to use external projects together with Punica.

Punica extensions could be implemented by using [Plugin API](#Plugin-API) and other APIs,
which headers could be found in `include/punica` directory.

## Minimal requirements
There are minimal requirements for plugin to be compatible with punica, or else it won't be loaded:
1. Plugin API is written for C++ (writing wrappers should enable other languages).
2. Plugin should be built as as DLL.
3. Plugin should provide [valid `PluginApi`](#Valid-PluginApi-example), named as `PLUGIN_API`.

## Structure of Punica
This section should clarify, what to do with public headers (headers located in
`include/punica` directory) and how to create Plugins.

### Punica core

#### Punica core interface
`punica::Core` is described in `include/punica/core.hpp`.
It provides access to [REST](#REST-API) and [LwM2M](#LwM2M-API) APIs with its methods.

##### Getting access to [REST API](#REST-API)
```C++
punica::rest::Core &getRestCore()
```
Returns [REST core interface](#REST-core-interface)
for management of [REST API](#REST-API).

##### Getting access to [LwM2M](#LwM2M-API)
```C++
punica::lwm2m::Core &getLwm2mCore()
```
Returns [LwM2M core interface](#LwM2M-core-interface)
for management of [LwM2M](#LwM2M-API) interactions.

### REST API
As of current implementation, [REST API](#REST-API)
allows [adding](#Adding-callback-handler)
and [removing](#Removing-callback-handler)
[REST Callbacks](#Callback-function).

#### Callback function
`punica::rest::CallbackFunction` type is defined in `include/punica/rest/core.hpp`:
```C++
typedef int(CallbackFunction)(Request &, Response &, void *);                   
```
Callback function parameters:
    1. Reference to  [REST request interface](#REST-request-interface),
    2. Reference to [REST response interface](#REST-response-interface),
    3. Pointer to `void *` context ([callback addition method](#Adding-callback-handler) `handlerContext` parameter), 

Callback function declaration example:
```C++
punica::rest::CallbackFunction someCallbackFunction;
```

Callback function definition example:
```C++
int someCallbackFunction(punica::rest::Request &request,
                         punica::rest::Response &response,
                         void *context)
{
    /* don't do anything, just send response, that everything is ok. */
    return 200; /* HTTP Status code 200 - OK */
}
```

#### REST core interface
`punica::rest::Core` is described in `include/punica/rest/core.hpp`.

##### Adding callback handler
```C++
bool addCallbackHandler(const std::string method,
                        const std::string urlPrefix,
                        const std::string urlFormat,
                        unsigned int priority,
                        CallbackFunction *handlerFunction,
                        void *handlerContext)
```
Adds `method` callback with `urlPrefix` and `urlFormat` to REST API.
Returns `true` on success, `false` if it fails.

Lower callback handler `priority` number - higher callback priority.
`handlerContext` - pointer which will be passed to [`handlerFunction`](#Callback-function) as a context.

Example calls:
```C++
const char importantData = "Very important string...";

restCore.addCallbackHandler("*", "/some", "", 3, &someCallbackFunction, &importantData);
restCore.addCallbackHandler("GET", "/some/url/prefix", "/:name", 10, &someOtherCallbackFunction, NULL);
```
##### Removing callback handler
```C++
bool removeCallbackHandler(const std::string method,
                           const std::string urlPrefix,
                           const std::string urlFormat)
```
Removes `method` callback with `urlPrefix` and `urlFormat` from REST API.
Returns `true` on success, `false` if it fails.

Example calls:
```C++
restCore.removeCallbackHandler("*", "/some", "")
restCore.removeCallbackHandler("GET", "/some/url/prefix", "/:name")
```

#### REST request interface
`punica::rest::Request` interface is described in `include/punica/rest/request.hpp`.

##### Getting request path
```C++
std::string getPath()
```
Returns request path as `std::string`.

Example calls:
```C++
std::string path = request.getPath();
```

##### Getting request method
```C++
std::string getMethod()
```
Returns request method as `std::string`.

Example calls:
```C++
std::string method = request.getMethod();
```

##### Getting request headers 
```C++
std::string getHeader(const std::string header)
```
Returns request `header` named header value as `std::string`.

Example calls:
```C++
std::string contentType = request.getHeader("Content-Type");
```

##### Getting request URL format 
```C++
std::string getUrlFormat(const std::string name)
```
Returns request `name` named part of HTTP URL as `std::string`.

Example calls (if [added callback](#Adding-callback-handler) `urlFormat` was `"/:name"`):
```C++
std::string name = request.getUrlFormat("name");
```

##### Getting request body 
```C++
std::vector<uint8_t> getBody()
```
Returns request body as `std::vector<uint8_t>`.

Example calls:
```C++
std::vector<uint8_t> body = request.getBody();
```

#### REST response interface
`punica::rest::Response` interface is described in `include/punica/rest/response.hpp`.

##### Setting response body
```C++
void setBody(std::vector<uint8_t> binary_data)
```
Writes `binary_data` to response body.

Example calls:
```C++
std::string stringBody = "Some message";
response.setBody(std::vector<uint8_t>(stringBody.begin(), stringBody.end()));
```

##### Setting response HTTP status code
```C++
void setCode(const int code)
```
Sets response HTTP status code to `code`.

Example calls:
```C++
response.setCode(200);
```

##### Setting response headers
```C++
void setHeader(const std::string header, const std::string value)
```
Sets response `header` to `value`.

Example calls:
```C++
response.setHeader("Content-Type", "text/plain");
```

### LwM2M API

#### `punica::lwm2m::Core`
Punica LwM2M core interface is described in `include/punica/lwm2m/core.hpp`.

### Plugin API
To create a program, which could interact with Punica APIs,
one must use [plugin API structure](#Plugin-API-structure).
Also plugin class must implement [plugin interface](#Plugin-interface).

#### Plugin API structure
`PluginApi` structure is defined in `include/punica/plugin/plugin_api.hpp`:
```C
typedef struct                                                                  
{                                                                               
    const char *version;                                                        
    PluginCreate *create;                                                       
    PluginDestroy *destroy;                                                     
} PluginApi;
```

##### `PluginCreate`
```C
typedef Plugin *(PluginCreate)(Core &core);
```
Valid `PluginCreate` function should return
[Plugin interface](#Plugin-interface) type pointer to plugin
created with [Punica core interface](#Punica-core-interface).

##### `PluginDestroy`
```C
typedef void (PluginDestroy)(Plugin *plugin);
```
Function which should destroy plugin that was created by `create` function.
<br />_Note: this includes cleaning up memory, that was allocated by plugin. We don't want leaks, don't we?_

##### Plugin version
`version` - semver Punica version string, e.g. `v1.2.3`.
<br />_Note: version string should match `PUNICA_VERSION`, defined in `include/punica/version.h`._

#### Plugin interface
`punica::plugin::Plugin` interface is described in `include/punica/plugin/plugin_api.hpp`.

#### Valid `PluginApi` example
In order to be loaded, plugin must provide `PluginApi`, named as `PLUGIN_API`.
```C
#include <punica/plugin/plugin_api.hpp>
#include <punica/version.h>

 /* this should allocate Plugin and its memory and return pointer to it*/
punica::plugin::PluginCreate newPlugin;
 /* this should free memory used by Plugin*/
punica::plugin::PluginDestroy deletePlugin;

const punica::plugin::PluginApi PLUGIN_API = {
    .version = PUNICA_VERSION,
    .create = &newTestPlugin,
    .destroy = &deleteTestPlugin,
    }
```
