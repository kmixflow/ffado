/*
 * Copyright (C) 2005-2008 by Pieter Palmers
 * Copyright (C) 2008-2009 by Jonathan Woithe
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
 *
 * FFADO is based upon FreeBoB.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* DBus controls associated with Mark3 mixer controls */

// This also includes motu_mark3_controls.h
#include "motu_avdevice.h"

namespace Motu {


#define MK3CTRL_SWITCH					0x0200690000000000
#define MK3CTRL_BUS_OUTPUT_ASSIGN     	0x0000000000000002
#define MK3CTRL_DISABLED				0x000000ff00000000
#define MK3CTRL_MAGIC_NUMBER		    0x0002000000000000


MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent)
: Control::Discrete(&parent)
, m_parent(parent)
{
}

MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent, std::string name, std::string label, std::string descr)
: Control::Discrete(&parent)
, m_parent(parent)
{
    bool setName(name);
    bool setLabel(label);
    bool setDescription(descr);
}

MixDestMk3::MixDestMk3(MotuDevice &parent)
: MotuDiscreteCtrlMk3(parent)
{
}

MixDestMk3::MixDestMk3(MotuDevice &parent, std::string name, std::string label, std::string descr)
: MotuDiscreteCtrlMk3(parent, name, label, descr)
{
}

bool
MixDestMk3::setValue(int v)
{



    unsigned int val;
    debugOutput(DEBUG_LEVEL_VERBOSE, "setValue for switch %s (0x%04x) to %d\n",
      getName().c_str(), MOTU_G3_REG_MIXER, v);

    //FIXME: This is a hack to skip the "heartbeat" counting by resetting the magic number
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, 0x00000000);
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, 0x00010000);


    val = MK3CTRL_SWITCH | MK3CTRL_BUS_OUTPUT_ASSIGN | MK3CTRL_MAGIC_NUMBER | MK3CTRL_DISABLED;

    m_parent.WriteRegisterMk3(MOTU_G3_REG_MIXER, val);

    //FIXME: Return an error if was not possible to write to the register
    return true;
}

int
MixDestMk3::getValue()
{
	//TODO: Try to figure out how to request control values
    /*unsigned int val;
    debugOutput(DEBUG_LEVEL_VERBOSE, "getValue for mix destination 0x%04x\n", m_register);

    // Silently swallow attempts to read non-existent controls for now
    if (m_register == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_WARNING, "use of MOTU_CTRL_NONE in non-matrix control\n");
        return true;
    }
    // FIXME: we could just read the appropriate mixer status field from the
    // receive stream processor once we work out an efficient way to do this.
    val = m_parent.ReadRegister(m_register);
    return (val >> 8) & 0x0f;*/
	return 0;
}


}
