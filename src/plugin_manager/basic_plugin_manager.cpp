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

#include <dlfcn.h>
#include <iostream>

#include "basic_core.hpp"
#include "basic_plugin_manager.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#include "basic_plugin_manager.h"

basic_plugin_manager_t *basic_plugin_manager_new(basic_punica_core_t *c_core)
{
    punica::Core::ptr core(reinterpret_cast<BasicCore *>(c_core));

    return reinterpret_cast<basic_plugin_manager_t *>(new BasicPluginManager(core));
}

void basic_plugin_manager_delete(basic_plugin_manager_t *c_manager)
{
    delete reinterpret_cast<BasicPluginManager *>(c_manager);
}

int basic_plugin_manager_load_plugin(basic_plugin_manager_t *c_manager,
                                     const char *c_path,
                                     const char *c_name)
{
    BasicPluginManager *manager = reinterpret_cast<BasicPluginManager *>(c_manager);
    const std::string path(c_path);
    const std::string name(c_name);

    return static_cast<int>(manager->loadPlugin(path, name));
}

int basic_plugin_manager_unload_plugin(basic_plugin_manager_t *c_manager,
                                       const char *c_name)
{
    BasicPluginManager *manager = reinterpret_cast<BasicPluginManager *>(c_manager);
    const std::string name(c_name);

    return static_cast<int>(manager->unloadPlugin(name));
}

#ifdef __cplusplus
} // extern "C"
#endif

BasicPluginManager::BasicPluginManager(punica::Core::ptr core):
    mCore(core),
    mPlugins()
{ }

BasicPluginManager::~BasicPluginManager()
{
    std::map<std::string, std::pair<punica::plugin::Plugin *, punica::plugin::PluginApi *> >::iterator
    pluginsIterator;

    for (pluginsIterator = mPlugins.begin(); pluginsIterator != mPlugins.end(); ++pluginsIterator)
    {
        unloadPlugin(pluginsIterator->first);
    }
}

bool BasicPluginManager::loadPlugin(std::string path, std::string name)
{
    if (mPlugins.find(name) != mPlugins.end())
    {
        if (unloadPlugin(name) != true)
        {
            return false;
        }
    }

    void *dll = dlopen(path.c_str(), RTLD_NOW | RTLD_LOCAL);
    if (dll == NULL)
    {
        std::cerr << "Failed to load plugin file from '" << path << "\"!" << std::endl;
        return false;
    }

    punica::plugin::PluginApi *pluginApi = static_cast<punica::plugin::PluginApi *>(dlsym(dll,
                                           PLUGIN_API_HANDLE_NAME));

    if (pluginApi == NULL)
    {
        std::cerr << "Failed to load plugin api from '" << path << "\"!" << std::endl;
        return false;
    }

    if (pluginApi->create == NULL)
    {
        std::cerr << "Failed to load plugin api from '" << path << "\"!" << std::endl;
        std::cerr << "Missing PLUGIN_API->create()" << std::endl;
        return false;
    }
    if (pluginApi->destroy == NULL)
    {
        std::cerr << "Failed to load plugin api from '" << path << "\"!" << std::endl;
        std::cerr << "Missing PLUGIN_API->destroy()" << std::endl;
        return false;
    }

    punica::plugin::PluginVersion pluginVersion = pluginApi->version;
    punica::plugin::Plugin *plugin = pluginApi->create(mCore);

    if (plugin == NULL)
    {
        std::cerr << "Failed to create plugin instance!" << std::endl;
        return false;
    }

    mPlugins[name] = std::make_pair(plugin, pluginApi);

    return true;
}

bool BasicPluginManager::unloadPlugin(std::string name)
{
    std::map<std::string, std::pair<punica::plugin::Plugin *, punica::plugin::PluginApi *> >::iterator
    pluginsIterator = mPlugins.find(name);
    punica::plugin::Plugin *plugin;
    punica::plugin::PluginApi *pluginApi;

    if (pluginsIterator == mPlugins.end())
    {
        return false;
    }

    plugin = pluginsIterator->second.first;
    pluginApi = pluginsIterator->second.second;

    pluginApi->destroy(plugin);

    mPlugins.erase(pluginsIterator);

    return true;
}
