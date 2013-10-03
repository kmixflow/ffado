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


MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent, unsigned long int key)
: Control::Discrete(&parent)
, m_parent(parent)
, m_key(key)
{
}

MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent, unsigned long int key,
		std::string name, std::string label, std::string descr)
: Control::Discrete(&parent)
, m_parent(parent)
, m_key(key)
{
    setName(name);
    setLabel(label);
    setDescription(descr);
}

MixDestMk3::MixDestMk3(MotuDevice &parent, unsigned long int key)
: MotuDiscreteCtrlMk3(parent, key)
{
}

MixDestMk3::MixDestMk3(MotuDevice &parent, unsigned long int key,
		std::string name, std::string label, std::string descr)
: MotuDiscreteCtrlMk3(parent, key, name, label, descr)
{
}

//Example: Assigning Mix1 out to none:  0x020169ff00000002
//FIRST Quadlet
#define MK3CTRL_SWITCH					0x02006900
#define MK3CTRL_SERIAL_NUMBER		    0x00020000

//SECOND Quadlet
#define MK3CTRL_MIX_OUTPUT_ASSIGN     	0x00000002
#define MK3CTRL_MIX1                    0x00000000
#define MK3CTRL_MIX2                    0x01000000

bool
MixDestMk3::setValue(int value)
{
    unsigned int dest;
	switch (value) {
		case 0:
			dest = MOTU_MK3CTRL_MIX_DEST_DISABLED;
			break;
		case 1:
			dest = MOTU_MK3CTRL_MIX_DEST_PHONES;
			break;
		case 2:
			dest = MOTU_MK3CTRL_MIX_DEST_MAIN_L_R; //Traveler's Analog 1-2
			break;
		case 3:
			dest = MOTU_MK3CTRL_MIX_DEST_ANALOG_1_2; //Traveler's Analog 3-4
			break;
		case 4:
			dest = MOTU_MK3CTRL_MIX_DEST_ANALOG_3_4; //Traveler's Analog 5-6
			break;
		case 5:
			dest = MOTU_MK3CTRL_MIX_DEST_ANALOG_5_6; //Traveler's Analog 7-8
			break;
		case 6:
			dest = MOTU_MK3CTRL_MIX_DEST_ANALOG_7_8; //Traveler's AES 1-2
			break;
		case 7:
			dest = MOTU_MK3CTRL_MIX_DEST_SPDIF; //Traveler's AES 1-2
			break;
		default:
			dest = MOTU_MK3CTRL_MIX_DEST_DISABLED;
			break;
	}
    debugOutput(DEBUG_LEVEL_WARNING, "setValue for switch %s (0x%X) to MK3 mixer register\n",
      getName().c_str(), MOTU_G3_REG_MIXER, value);

    //FIXME: This is a hack to skip the "heartbeat" counting by resetting the serial number
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, 0x00000000);
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, 0x00010000);

    quadlet_t data[2];
    data[0] = MK3CTRL_SWITCH | MK3CTRL_SERIAL_NUMBER | dest;
    data[1] = MK3CTRL_MIX_OUTPUT_ASSIGN | this->m_key;

    if(m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 2)){
    	debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) to MK3 mixer register\n", data[0], data[1], MOTU_G3_REG_MIXER);
    	return false;
    }
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
