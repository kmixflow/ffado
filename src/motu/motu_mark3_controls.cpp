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




MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent)
: Control::Discrete(&parent)
, m_parent(parent)
{
}

MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent, std::string name, std::string label, std::string descr)
: Control::Discrete(&parent)
, m_parent(parent)
{
    setName(name);
    setLabel(label);
    setDescription(descr);
}

MixDestMk3::MixDestMk3(MotuDevice &parent)
: MotuDiscreteCtrlMk3(parent)
{
}

MixDestMk3::MixDestMk3(MotuDevice &parent, std::string name, std::string label, std::string descr)
: MotuDiscreteCtrlMk3(parent, name, label, descr)
{
}

//Example: Assigning Mix1 out to none:  0x020169ff00000002
//FIRST Quadlet
#define MK3CTRL_SWITCH					0x02006900
#define MK3CTRL_DISABLED				0x000000ff
#define MK3CTRL_MAGIC_NUMBER		    0x00010000

//SECOND Quadlet
#define MK3CTRL_BUS_OUTPUT_ASSIGN     	0x00000002

bool
MixDestMk3::setValue(int v)
{

    unsigned int val;
    debugOutput(DEBUG_LEVEL_VERBOSE, "setValue for switch %s (0x%X) to %d\n",
      getName().c_str(), MOTU_G3_REG_MIXER, v);

    //FIXME: This is a hack to skip the "heartbeat" counting by resetting the magic number
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, 0x00000000);

    quadlet_t data[2];
    data[0] = MK3CTRL_SWITCH | MK3CTRL_MAGIC_NUMBER | MK3CTRL_DISABLED;
    data[1] = MK3CTRL_BUS_OUTPUT_ASSIGN;

    if(m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 2)){
    	debugOutput(DEBUG_LEVEL_VERBOSE, "Error writing data[0]=(0x%X) data[1]=(0x%X) to register (0x%X)", data[0], data[1], MOTU_G3_REG_MIXER);
    	return false;
    }
    debugOutput(DEBUG_LEVEL_VERBOSE, "So far so good writing data[0]=(0x%X) data[1]=(0x%X) to register (0x%X)", data[0], data[1], MOTU_G3_REG_MIXER);
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
