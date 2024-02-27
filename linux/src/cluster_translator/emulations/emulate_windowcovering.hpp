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

#ifndef EMULATE_WINDOWCOVERING_HPP
#define EMULATE_WINDOWCOVERING_HPP

// Emulator interface
#include "emulator.hpp"

namespace unify::matter_bridge {

using namespace chip::app::Clusters;

class EmulateWindowCovering : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "WindowCovering"; }

    chip::ClusterId emulated_cluster() const override { return WindowCovering::Id; };

    std::vector<chip::AttributeId> emulated_attributes() const override
    {
        return { WindowCovering::Attributes::OperationalStatus::Id, WindowCovering::Attributes::EndProductType::Id,
                    WindowCovering::Attributes::Mode::Id };
    }

    /**     
     * @brief Emulate WindowCovering cluster to emulate commands and attributes
     *
     * @param unify_cluster
     * @param cluster_builder
     * @return CHIP_ERROR
     */
    CHIP_ERROR
    emulate(const node_state_monitor::cluster & unify_cluster, matter_cluster_builder & cluster_builder) override
    {
        // Add OperationalStatus and EndProductType attribute to the matter cluster
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         WindowCovering::Attributes::OperationalStatus::Id,
                                                                         1,
                                                                         ZAP_TYPE(BITMAP8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});
        cluster_builder.attributes.emplace_back(EmberAfAttributeMetadata{ZAP_EMPTY_DEFAULT(),
                                                                         WindowCovering::Attributes::EndProductType::Id,
                                                                         2,
                                                                         ZAP_TYPE(ENUM8),
                                                                         ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)});

        return CHIP_NO_ERROR;
    }

    /**
     * @brief Read an emulated attribute for WindowCovering cluster 
     *        
     * This read_attribute overides the default read for emulated attributes.
     *
     * @param aPath
     * @param aEncoder
     * @return CHIP_ERROR
     */
    CHIP_ERROR read_attribute(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        attribute_state_cache & cache = attribute_state_cache::get_instance();

        switch (aPath.mAttributeId)
        {
        case WindowCovering::Attributes::EndProductType::Id: {
            WindowCovering::Attributes::EndProductType::TypeInfo::Type EndProductType;

            if (!cache.get(aPath, EndProductType))
            {
                EndProductType = WindowCovering::EndProductType::kRollerShade; 
            }
            return aEncoder.Encode(EndProductType);
        }
        case WindowCovering::Attributes::OperationalStatus::Id: {
            WindowCovering::Attributes::OperationalStatus::TypeInfo::Type OperationalStatus;
       
            if (!cache.get(aPath, OperationalStatus))
            {
                OperationalStatus = static_cast<WindowCovering::Attributes::OperationalStatus::TypeInfo::Type>(0X00);
            }
            return aEncoder.Encode(OperationalStatus);
        }
        case WindowCovering::Attributes::Mode::Id: {
            WindowCovering::Attributes::Mode::TypeInfo::Type Mode;
       
            if (!cache.get(aPath, Mode))
            {
                Mode = static_cast<WindowCovering::Attributes::Mode::TypeInfo::Type>(0X00);
            }
            return aEncoder.Encode(Mode);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };
    CHIP_ERROR write_attribute(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder) override
    {     
        switch (aPath.mAttributeId)
        {
        case WindowCovering::Attributes::Mode::Id: {
            return CHIP_IM_GLOBAL_STATUS(ConstraintError);
        }
        }
        return CHIP_ERROR_INVALID_ARGUMENT;
    };
};

} // namespace unify::matter_bridge

#endif // EMULATE_WindowCovering_HPP