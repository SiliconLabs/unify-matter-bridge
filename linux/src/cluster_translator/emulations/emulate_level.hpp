/******************************************************************************
 * # License
 * <b>Copyright 2023 Silicon Laboratories Inc. www.silabs.com</b>
 ******************************************************************************
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 *****************************************************************************/

#ifndef EMULATE_LevelControl_HPP
#define EMULATE_LevelControl_HPP

// Emulator interface
#include "emulator.hpp"

#include "chip_types_to_json.hpp"
namespace unify::matter_bridge {

using namespace chip::app;
using namespace chip::app::Clusters;

class EmulateLevelControl : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "Level"; }

    chip::ClusterId emulated_cluster() const override { return LevelControl::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { LevelControl::Attributes::MinLevel::Id, LevelControl::Attributes::CurrentLevel::Id, LevelControl::Attributes::MaxLevel::Id };
    }

    std::vector<chip::CommandId> emulated_commands() const override
    {
        return { LevelControl::Commands::Move::Id };
    }

    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        // LevelControlType is a mandatory attribute we default it to None
        switch (aPath.mAttributeId)
        {
        case LevelControl::Attributes::StartUpCurrentLevel::Id: {
            LevelControl::Attributes::StartUpCurrentLevel::TypeInfo::Type startUpCurrentLevel;
            uint8_t start_up_level_value;

            if (cache.get(aPath, start_up_level_value))
            {
                if (start_up_level_value < 255)
                {
                    startUpCurrentLevel.SetNonNull(start_up_level_value);
                }
            }
            return aEncoder.Encode(startUpCurrentLevel);
        }
        case LevelControl::Attributes::MinLevel::Id: {
            LevelControl::Attributes::MinLevel::TypeInfo::Type min_level;
            if (cache.get(aPath, min_level))
            {
                if (min_level < 1)
                {
                    min_level = 1;
                }
                return aEncoder.Encode(min_level);
            }
            break;
        }
        case LevelControl::Attributes::MaxLevel::Id: {
            LevelControl::Attributes::MaxLevel::TypeInfo::Type max_level;
            if (cache.get(aPath, max_level))
            {
                if (max_level > 254)
                {
                    max_level = 254;
                }
                return aEncoder.Encode(max_level);
            }
            break;
        }
        case LevelControl::Attributes::CurrentLevel::Id: {
            LevelControl::Attributes::CurrentLevel::TypeInfo::Type current_level;
            uint8_t level_value;
            if (cache.get<uint8_t>(aPath, level_value))
            {
                if (level_value < 1)
                {
                    current_level.SetNonNull(1);
                }
                else if (level_value < 255)
                {
                    current_level.SetNonNull(level_value);
                } // 255 means not defined
            }
            return aEncoder.Encode(current_level);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };

    CHIP_ERROR command(CommandHandlerInterface::HandlerContext & handlerContext, emulated_cmd_payload & cdata) override
    {
        using namespace chip::app::Clusters::LevelControl;
        CHIP_ERROR err = CHIP_ERROR_NOT_IMPLEMENTED;

        cdata.cmd_emulation_completed = false;

        switch (handlerContext.mRequestPath.mCommandId)
        {
            case LevelControl::Commands::Move::Id:
            {
                Commands::Move::DecodableType data;
                if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR)
                {
                    if (data.rate.IsNull() || data.rate.Value() == 0) {
                         handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                         handlerContext.SetCommandHandled();
                         cdata.cmd_emulation_completed = true;
                         err = CHIP_ERROR_BAD_REQUEST;
                         break;
                    }
                    cdata.cmd = "Move"; // "Move"
                    try {
                        cdata.payload["MoveMode"] = to_json(data.moveMode);
                        cdata.payload["Rate"] = to_json(data.rate);
                        cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                        cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                    } catch (std::exception& ex) {
                        sl_log_warning("emulate_level", "Failed to add the command argument value to json format: %s", ex.what());
                    }
                }
                err = CHIP_NO_ERROR;
            }
            break;
        }
        return err;
    }
};

} // namespace unify::matter_bridge

#endif // EMULATE_LevelControl_HPP
