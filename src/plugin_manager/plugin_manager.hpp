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

#ifndef PUNICA_PLUGIN_MANAGER_PLUGIN_MANAGER_HPP
#define PUNICA_PLUGIN_MANAGER_PLUGIN_MANAGER_HPP

#include <memory>
#include <string>

#include <punica/core.hpp>
#include <punica/plugin/plugin_api.hpp>
#include <punica/plugin/plugin.hpp>

typedef std::pair<punica::plugin::Plugin::ptr,
        std::shared_ptr<punica::plugin::PluginApi>> PluginPair;
typedef std::map<std::string, PluginPair> PluginMap;

class PluginManager
{
public:
    virtual ~PluginManager() { }

    virtual bool loadPlugin(std::string name, std::string path) = 0;
    virtual bool unloadPlugin(std::string name) = 0;
};

#endif // PUNICA_PLUGIN_MANAGER_PLUGIN_MANAGER_HPP 
