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

class EmulateWindowCovering : public EmulatorInterface
{
public:
    const char * emulated_cluster_name() const override { return "WindowCovering"; }

    chip::ClusterId emulated_cluster() const override { return WindowCovering::Id; };

};

} // namespace unify::matter_bridge

#endif // EMULATE_WindowCovering_HPP