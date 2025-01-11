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

#ifndef EMULATE_THERMOSTAT_HPP
#define EMULATE_THERMOSTAT_HPP

// Emulator interface
#include "emulator.hpp"
#include "sl_log.h"
#include "chip_types_to_json.hpp"

// Default setpoint limits
const int16_t OCCUPIED_COOLING_SETPOINT = 2600;
const int16_t OCCUPIED_HEATING_SETPOINT = 2000;
const int16_t MIN_COOL_SETPOINT_LIMIT = 1600;
const int16_t MAX_COOL_SETPOINT_LIMIT = 3200;
const int16_t MIN_HEAT_SETPOINT_LIMIT = 700;
const int16_t MAX_HEAT_SETPOINT_LIMIT = 3000;

constexpr const char* LOG_TAG = "emulate_thermostat";

namespace unify::matter_bridge
{
    class EmulateThermostat : public EmulatorInterface
    {
    public:
        const char *emulated_cluster_name() const override { return "Thermostat"; }

        chip::ClusterId emulated_cluster() const override { return Thermostat::Id; };

        std::vector<chip::AttributeId> emulated_attributes() const override
        {
            return {Thermostat::Attributes::OccupiedCoolingSetpoint::Id, Thermostat::Attributes::OccupiedHeatingSetpoint::Id};
        }
        std::vector<chip::CommandId> emulated_commands() const override
        {
            return {Thermostat::Commands::SetpointRaiseLower::Id};
        }

        CHIP_ERROR read_attribute(const ConcreteReadAttributePath &aPath, AttributeValueEncoder &aEncoder) override
        {
            const attribute_state_cache &cache = attribute_state_cache::get_instance();

            switch (aPath.mAttributeId)
            {
            case Thermostat::Attributes::OccupiedCoolingSetpoint::Id:
            {
                Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo::Type occupiedCoolingSetpoint;
                if (!cache.get(aPath, occupiedCoolingSetpoint))
                {
                    return CHIP_ERROR_NOT_IMPLEMENTED;
                }
                return aEncoder.Encode(occupiedCoolingSetpoint);
            }
            case Thermostat::Attributes::OccupiedHeatingSetpoint::Id:
            {
                Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo::Type occupiedHeatingSetpoint;
                if (!cache.get(aPath, occupiedHeatingSetpoint))
                {
                    return CHIP_ERROR_NOT_IMPLEMENTED;
                }
                return aEncoder.Encode(occupiedHeatingSetpoint);
            }
            default:
                return CHIP_ERROR_NOT_IMPLEMENTED;
            }
            return CHIP_ERROR_INVALID_ARGUMENT;
        };

        CHIP_ERROR write_attribute(const ConcreteDataAttributePath &aPath, AttributeValueDecoder &aDecoder) override
        {
            const attribute_state_cache &cache = attribute_state_cache::get_instance();

            auto err = CHIP_ERROR_INVALID_ARGUMENT;
            switch (aPath.mAttributeId)
            {
            case Thermostat::Attributes::OccupiedCoolingSetpoint::Id:
            {
                Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo::Type occupiedCoolingSetpoint;
                Thermostat::Attributes::MinCoolSetpointLimit::TypeInfo::Type MinCoolSetpointLimit;
                Thermostat::Attributes::MaxCoolSetpointLimit::TypeInfo::Type MaxCoolSetpointLimit;

                ConcreteDataAttributePath minPath(aPath.mEndpointId, aPath.mClusterId, Thermostat::Attributes::MinCoolSetpointLimit::Id);
                ConcreteDataAttributePath maxPath(aPath.mEndpointId, aPath.mClusterId, Thermostat::Attributes::MaxCoolSetpointLimit::Id);

                err = aDecoder.Decode(occupiedCoolingSetpoint);
                if (err != CHIP_NO_ERROR)
                {
                    return err;
                }

                if (!cache.get(minPath, MinCoolSetpointLimit))
                {
                    MinCoolSetpointLimit = MIN_COOL_SETPOINT_LIMIT;
                }
                if (!cache.get(maxPath, MaxCoolSetpointLimit))
                {
                    MaxCoolSetpointLimit = MAX_COOL_SETPOINT_LIMIT;
                }

                if (occupiedCoolingSetpoint < MinCoolSetpointLimit || occupiedCoolingSetpoint > MaxCoolSetpointLimit)
                {
                    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
                }
                err = CHIP_ERROR_IN_PROGRESS;
                break;
            }
            case Thermostat::Attributes::OccupiedHeatingSetpoint::Id:
            {
                Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo::Type occupiedHeatingSetpoint;
                Thermostat::Attributes::MinHeatSetpointLimit::TypeInfo::Type MinHeatSetpointLimit;
                Thermostat::Attributes::MaxHeatSetpointLimit::TypeInfo::Type MaxHeatSetpointLimit;

                ConcreteDataAttributePath minPath(aPath.mEndpointId, aPath.mClusterId, Thermostat::Attributes::MinHeatSetpointLimit::Id);
                ConcreteDataAttributePath maxPath(aPath.mEndpointId, aPath.mClusterId, Thermostat::Attributes::MaxHeatSetpointLimit::Id);

                err = aDecoder.Decode(occupiedHeatingSetpoint);
                if (err != CHIP_NO_ERROR)
                {
                    return err;
                }

                if (!cache.get(minPath, MinHeatSetpointLimit))
                {
                    MinHeatSetpointLimit = MIN_HEAT_SETPOINT_LIMIT;
                }
                if (!cache.get(maxPath, MaxHeatSetpointLimit))
                {
                    MaxHeatSetpointLimit = MAX_HEAT_SETPOINT_LIMIT;
                }

                if (occupiedHeatingSetpoint < MinHeatSetpointLimit || occupiedHeatingSetpoint > MaxHeatSetpointLimit)
                {
                    return CHIP_IM_GLOBAL_STATUS(ConstraintError);
                }

                err = CHIP_ERROR_IN_PROGRESS;
                break;
            }
            default:
                return CHIP_ERROR_NOT_IMPLEMENTED;
            }
            return err;
        }

        CHIP_ERROR command(CommandHandlerInterface::HandlerContext &handlerContext, emulated_cmd_payload &cdata) override
        {
            const attribute_state_cache &cache = attribute_state_cache::get_instance();
            cdata.cmd_emulation_completed = false;
            using namespace chip::app::Clusters::Thermostat;

            auto err = CHIP_ERROR_NOT_IMPLEMENTED;

            switch (handlerContext.mRequestPath.mCommandId)
            {
            case Commands::SetpointRaiseLower::Id:
            {
                cdata.cmd = "SetpointRaiseLower";
                Commands::SetpointRaiseLower::DecodableType data;
                if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR)
                {
                    if (data.mode == SetpointRaiseLowerModeEnum::kHeat || data.mode == SetpointRaiseLowerModeEnum::kBoth)
                    {
                        err = UpdateHeatingSetpoint(cache, handlerContext, data.amount, data.mode);
                    }
                    else if (data.mode == SetpointRaiseLowerModeEnum::kCool || data.mode == SetpointRaiseLowerModeEnum::kBoth)
                    {
                        err = UpdateCoolingSetpoint(cache, handlerContext, data.amount, data.mode);
                    }
                    else
                    {
                        err = CHIP_ERROR_INVALID_ARGUMENT;
                    }

                    if (err != CHIP_NO_ERROR)
                    {
                        return err;
                    }

                    try
                    {
                        cdata.payload["Mode"] = to_json(data.mode);
                        cdata.payload["Amount"] = to_json(data.amount);
                    }
                    catch (const nlohmann::json::exception &ex)
                    {
                        sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                    }
                }
                break;
            }
            default:
                return CHIP_ERROR_NOT_IMPLEMENTED;
            }
            return CHIP_ERROR_NOT_IMPLEMENTED;
        }

        CHIP_ERROR UpdateHeatingSetpoint(const attribute_state_cache &cache, const CommandHandlerInterface::HandlerContext &handlerContext, int8_t &amount, Thermostat::SetpointRaiseLowerModeEnum mode) const
        {
            Thermostat::Attributes::MaxHeatSetpointLimit::TypeInfo::Type MaxHeatSetpointLimit;
            Thermostat::Attributes::MinHeatSetpointLimit::TypeInfo::Type MinHeatSetpointLimit;
            Thermostat::Attributes::OccupiedHeatingSetpoint::TypeInfo::Type occupiedHeatingSetpoint;

            ConcreteDataAttributePath heatPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::OccupiedHeatingSetpoint::Id);
            ConcreteDataAttributePath maxPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::MaxHeatSetpointLimit::Id);
            ConcreteDataAttributePath minPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::MinHeatSetpointLimit::Id);

            if (!cache.get(heatPath, occupiedHeatingSetpoint))
            {
                if (mode == Thermostat::SetpointRaiseLowerModeEnum::kBoth)
                {
                    return CHIP_NO_ERROR;
                }
                return CHIP_ERROR_INVALID_ARGUMENT;
            }

            if (!cache.get(minPath, MinHeatSetpointLimit))
            {
                MinHeatSetpointLimit = MIN_HEAT_SETPOINT_LIMIT;
            }
            if (!cache.get(maxPath, MaxHeatSetpointLimit))
            {
                MaxHeatSetpointLimit = MAX_HEAT_SETPOINT_LIMIT;
            }

            int16_t updatedOccupiedHeatingSetpoint = occupiedHeatingSetpoint + amount * 10;

            if (updatedOccupiedHeatingSetpoint < MinHeatSetpointLimit)
            {
                amount = static_cast<uint8_t>((MinHeatSetpointLimit - occupiedHeatingSetpoint) /10);
            }
            if (updatedOccupiedHeatingSetpoint > MaxHeatSetpointLimit)
            {
                amount = static_cast<uint8_t>((MaxHeatSetpointLimit - occupiedHeatingSetpoint) /10);
            }
            
            return CHIP_NO_ERROR;
        }

        CHIP_ERROR UpdateCoolingSetpoint(const attribute_state_cache &cache, const CommandHandlerInterface::HandlerContext &handlerContext, int8_t &amount, Thermostat::SetpointRaiseLowerModeEnum mode) const
        {
            Thermostat::Attributes::MaxCoolSetpointLimit::TypeInfo::Type MaxCoolSetpointLimit;
            Thermostat::Attributes::MinCoolSetpointLimit::TypeInfo::Type MinCoolSetpointLimit;
            Thermostat::Attributes::OccupiedCoolingSetpoint::TypeInfo::Type occupiedCoolingSetpoint;

            ConcreteDataAttributePath coolPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::OccupiedCoolingSetpoint::Id);
            ConcreteDataAttributePath maxPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::MaxCoolSetpointLimit::Id);
            ConcreteDataAttributePath minPath(handlerContext.mRequestPath.mEndpointId, handlerContext.mRequestPath.mClusterId, Thermostat::Attributes::MinCoolSetpointLimit::Id);

            if (!cache.get(coolPath, occupiedCoolingSetpoint))
            {
                if (mode == Thermostat::SetpointRaiseLowerModeEnum::kBoth)
                {
                    return CHIP_NO_ERROR;
                }
                return CHIP_ERROR_INVALID_ARGUMENT;
            }
            if (!cache.get(minPath, MinCoolSetpointLimit))
            {
                MinCoolSetpointLimit = MIN_COOL_SETPOINT_LIMIT;
            }
            if (!cache.get(maxPath, MaxCoolSetpointLimit))
            {
                MaxCoolSetpointLimit = MAX_COOL_SETPOINT_LIMIT;
            }
            int16_t updatedOccupiedCoolingSetpoint = occupiedCoolingSetpoint + amount * 10;

            if (updatedOccupiedCoolingSetpoint < MinCoolSetpointLimit)
            {
                amount = static_cast<uint8_t>((MinCoolSetpointLimit - occupiedCoolingSetpoint) / 10);
            }
            if (updatedOccupiedCoolingSetpoint > MaxCoolSetpointLimit)
            {
                amount = static_cast<uint8_t>((MaxCoolSetpointLimit - occupiedCoolingSetpoint) / 10);
            }

            return CHIP_NO_ERROR;
        }
    };

} // namespace unify::matter_bridge

#endif // EMULATE_THERMOSTAT_HPP
