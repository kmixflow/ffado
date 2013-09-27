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


/* This function does the magic to set a control in MK3 mixers */
bool setMixerControl(int channel, int key, int value)
{
	/* See motu_firewire_protocol-mk3.txt for details */

	/*PLAN:
	 *
	 * 1.- Check channel value
	 * 2.- Depending on the channel, check the key
	 * 3.- Depending on the above, check if value is within parameters; if not set to min or max
	 * 4.- Format the 3qualet to 02 aa 66 bb - cc dd ee v1 - v2 v3 v4 00
	 * 5.- Set the control value
	 */


	return false;
}

}
