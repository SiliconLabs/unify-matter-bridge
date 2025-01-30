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

#ifndef EMULATE_COLORCONTROL_HPP
#define EMULATE_COLORCONTROL_HPP

// Emulator interface
#include "emulator.hpp"
#include "cluster_emulator.hpp"
#include "sl_log.h"
#include "attribute_callback_registry.hpp"

namespace unify::matter_bridge {

using namespace chip::app::Clusters;

class EmulateColorControl : public EmulatorInterface
{
public:

    const char * emulated_cluster_name() const override { return "ColorControl"; }

    chip::ClusterId emulated_cluster() const override { return ColorControl::Id; };

    std::vector<chip::CommandId> emulated_commands() const override
    {
        return { ColorControl::Commands::MoveHue::Id, 
                ColorControl::Commands::StepHue::Id,
                ColorControl::Commands::MoveSaturation::Id,
                ColorControl::Commands::StepSaturation::Id,
                ColorControl::Commands::StepColor::Id,
                ColorControl::Commands::MoveColorTemperature::Id,
                ColorControl::Commands::StepColorTemperature::Id,
                ColorControl::Commands::EnhancedMoveHue::Id,
                ColorControl::Commands::EnhancedStepHue::Id
                };
    }
    
    /**
     * @brief Handle an emulated command for ColorControl cluster
     *
     * @param handlerContext
     * @param cdata Holder for the emulated command data and payload
     * @return CHIP_ERROR
     */
    CHIP_ERROR command(CommandHandlerInterface::HandlerContext & handlerContext, emulated_cmd_payload & cdata) override
    {
        // ColorControl cluster emulated command need further handling in command_translotor.
        cdata.cmd_emulation_completed = false;
        auto err = CHIP_ERROR_NOT_IMPLEMENTED;
    
        switch (handlerContext.mRequestPath.mCommandId)
        {
        case ColorControl::Commands::MoveHue::Id: {
            ColorControl::Commands::MoveHue::DecodableType data;
            cdata.cmd = "MoveHue"; // "MoveHue";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(((data.moveMode == ColorControl::MoveModeEnum::kUp ) || (data.moveMode == ColorControl::MoveModeEnum::kDown )) 
                    && (data.rate == 0)) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["MoveMode"] = to_json(data.moveMode);
                    cdata.payload["Rate"] = to_json(data.rate); 
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::StepHue::Id: {
            ColorControl::Commands::StepHue::DecodableType data;
            cdata.cmd = "StepHue"; // "StepHue";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(data.stepSize == 0) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["StepMode"] = to_json(data.stepMode);
                    cdata.payload["StepSize"] = to_json(data.stepSize); 
                    cdata.payload["TransitionTime"] = to_json(data.transitionTime);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::MoveSaturation::Id: {
            ColorControl::Commands::MoveSaturation::DecodableType data;
            cdata.cmd = "MoveSaturation"; // "MoveSaturation";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(((data.moveMode == ColorControl::MoveModeEnum::kUp ) || (data.moveMode == ColorControl::MoveModeEnum::kDown )) 
                    && (data.rate == 0)) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["MoveMode"] = to_json(data.moveMode);
                    cdata.payload["Rate"] = to_json(data.rate); 
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::StepSaturation::Id: {
            ColorControl::Commands::StepSaturation::DecodableType data;
            cdata.cmd = "StepSaturation"; // "StepSaturation";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(data.stepSize == 0) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["StepMode"] = to_json(data.stepMode);
                    cdata.payload["StepSize"] = to_json(data.stepSize); 
                    cdata.payload["TransitionTime"] = to_json(data.transitionTime);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::StepColor::Id: {
            ColorControl::Commands::StepColor::DecodableType data;
            cdata.cmd = "StepColor"; // "StepColor";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if((data.stepX == 0) && (data.stepY == 0)) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["StepX"] = to_json(data.stepX);
                    cdata.payload["StepY"] = to_json(data.stepY);
                    cdata.payload["TransitionTime"] = to_json(data.transitionTime);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::MoveColorTemperature::Id: {
            ColorControl::Commands::MoveColorTemperature::DecodableType data;
            cdata.cmd = "MoveColorTemperature"; // "MoveColorTemperature";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(((data.moveMode == ColorControl::MoveModeEnum::kUp ) || (data.moveMode == ColorControl::MoveModeEnum::kDown )) 
                    && (data.rate == 0)) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["MoveMode"] = to_json(data.moveMode);
                    cdata.payload["Rate"] = to_json(data.rate);
                    cdata.payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
                    cdata.payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::StepColorTemperature::Id: {
            ColorControl::Commands::StepColorTemperature::DecodableType data;
            cdata.cmd = "StepColorTemperature"; // "StepColorTemperature";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(data.stepSize == 0) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["StepMode"] = to_json(data.stepMode);
                    cdata.payload["StepSize"] = to_json(data.stepSize); 
                    cdata.payload["TransitionTime"] = to_json(data.transitionTime);
                    cdata.payload["ColorTemperatureMinimumMireds"] = to_json(data.colorTemperatureMinimumMireds);
                    cdata.payload["ColorTemperatureMaximumMireds"] = to_json(data.colorTemperatureMaximumMireds);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::EnhancedMoveHue::Id: {
            ColorControl::Commands::EnhancedMoveHue::DecodableType data;
            cdata.cmd = "EnhancedMoveHue"; // "EnhancedMoveHue";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(((data.moveMode == ColorControl::MoveModeEnum::kUp ) || (data.moveMode == ColorControl::MoveModeEnum::kDown )) 
                    && (data.rate == 0)) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["MoveMode"] = to_json(data.moveMode);
                    cdata.payload["Rate"] = to_json(data.rate); 
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        case ColorControl::Commands::EnhancedStepHue::Id: {
            ColorControl::Commands::EnhancedStepHue::DecodableType data;
            cdata.cmd = "EnhancedStepHue"; // "EnhancedStepHue";
            if (DataModel::Decode(handlerContext.GetReader(), data) == CHIP_NO_ERROR) {
                if(data.stepSize == 0) {
                    cdata.cmd_emulation_completed = true;
                    handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, chip::Protocols::InteractionModel::Status::InvalidCommand);
                    handlerContext.SetCommandHandled();
                    return CHIP_NO_ERROR;
                }
                
                try {
                    cdata.payload["StepMode"] = to_json(data.stepMode);
                    cdata.payload["StepSize"] = to_json(data.stepSize); 
                    cdata.payload["TransitionTime"] = to_json(data.transitionTime);
                    cdata.payload["OptionsMask"] = to_json(data.optionsMask);
                    cdata.payload["OptionsOverride"] = to_json(data.optionsOverride);
                }
                catch (const nlohmann::json::exception &ex)
                {
                    sl_log_warning(LOG_TAG, "Failed to add the command argument value to JSON format: %s", ex.what());
                }
            }
            return CHIP_NO_ERROR;
        }
        break;
        default:
            break;
        }
        return err;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_ColorControl_HPP