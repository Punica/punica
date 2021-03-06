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

#include <string>

#include <punica/core.hpp>
#include <punica/plugin/plugin.hpp>
#include <punica/plugin/plugin_api.hpp>
#include <punica/rest/core.hpp>
#include <punica/version.h>

#include "counter.hpp"

class TestPluginWithCounters: public punica::plugin::Plugin
{
public:
    TestPluginWithCounters(punica::rest::Core &core);
    ~TestPluginWithCounters();

    bool createCounter(std::string name);
    bool destroyCounter(std::string name);

private:
    punica::rest::Core &mRestCore;
    Counter::map mCounters;
};

punica::rest::CallbackFunction createCounterCallback;
punica::rest::CallbackFunction destroyCounterCallback;

punica::plugin::PluginCreate newTestPlugin;
punica::plugin::PluginDestroy deleteTestPlugin;

extern "C" const punica::plugin::PluginApi PLUGIN_API =
{
    .version = PUNICA_VERSION,
    .create = &newTestPlugin,
    .destroy = &deleteTestPlugin,
};
