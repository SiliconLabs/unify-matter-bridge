/******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef ATTRIBUTE_CALLBACK_REGISTRY_HPP
#define ATTRIBUTE_CALLBACK_REGISTRY_HPP

#include <unordered_map>
#include <functional>
#include <string>
#include <nlohmann/json.hpp>

namespace unify::matter_bridge {

using AttributeCallback = std::function<void(const nlohmann::json& value, const short unsigned int& endpoint)>;

class AttributeCallbackRegistry
{
private:
    std::unordered_map<std::string, AttributeCallback> attribute_callbacks;

public:
    /**
     * @brief Register a callback for an attribute
     *
     * @param attributeName A representation of ClusterName::AttributeName
     * @param callback The callback function to register
     */
    void register_callback(const std::string& attributeName, AttributeCallback callback)
    {
        attribute_callbacks[attributeName] = std::move(callback);
    }

    /**
     * @brief Get the registered callbacks
     *
     * @param attributeName A representation of ClusterName::AttributeName
     * @return returns callback function if registed with attributeName provided else returns nullptr
     */
    const AttributeCallback* get_registered_callback(const std::string& attributeName) const
    {
        auto it = attribute_callbacks.find(attributeName);
        return (it != attribute_callbacks.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Static instance getter for Singleton pattern
     * 
     * @return The instance of AttributeCallbackRegistry
     */
    static AttributeCallbackRegistry& getInstance()
    {
        static AttributeCallbackRegistry instance;
        return instance;
    }
};

} // namespace unify::matter_bridge

#endif // ATTRIBUTE_CALLBACK_REGISTRY_HPP
