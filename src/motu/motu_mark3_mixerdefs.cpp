/*
 * Copyright (C) 2005-2008 by Pieter Palmers
 * Copyright (C) 2005-2009 by Jonathan Woithe
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

/* This module contains definitions of mixers on devices which utilise the
 * "Mark3" mixer control protocol.
 */

#include "motu/motu_avdevice.h"
#include "motu/motu_mark3_mixerdefs.h"

namespace Motu {

const MatrixMixBusMk3 MixerBuses_TravelerMk3[] = {
    {"Mix 1 ", 0x00, },
    {"Mix 2 ", 0x01, },
    {"Mix 3 ", 0x02, },
    {"Mix 4 ", 0x03, },
    {"Mix 5 ", 0x04, },
    {"Mix 6 ", 0x05, },
    {"Mix 7 ", 0x06, },
    {"Mix 8 ", 0x08, },
};

const MatrixMixChannelMk3 MixerChannels_TravelerMk3[] = {
    {"Analog 1", MOTU_CTRL_STD_CHANNEL, 0x0000, },
    {"Analog 2", MOTU_CTRL_STD_CHANNEL, 0x0004, },
    {"Analog 3", MOTU_CTRL_STD_CHANNEL, 0x0008, },
    {"Analog 4", MOTU_CTRL_STD_CHANNEL, 0x000c, },
    {"Analog 5", MOTU_CTRL_STD_CHANNEL, 0x0010, },
    {"Analog 6", MOTU_CTRL_STD_CHANNEL, 0x0014, },
    {"Analog 7", MOTU_CTRL_STD_CHANNEL, 0x0018, },
    {"Analog 8", MOTU_CTRL_STD_CHANNEL, 0x001c, },
    {"AES/EBU 1", MOTU_CTRL_STD_CHANNEL, 0x0020, },
    {"AES/EBU 2", MOTU_CTRL_STD_CHANNEL, 0x0024, },
    {"SPDIF 1", MOTU_CTRL_STD_CHANNEL, 0x0028, },
    {"SPDIF 2", MOTU_CTRL_STD_CHANNEL, 0x002c, },
    {"ADAT 1", MOTU_CTRL_STD_CHANNEL, 0x0030, },
    {"ADAT 2", MOTU_CTRL_STD_CHANNEL, 0x0034, },
    {"ADAT 3", MOTU_CTRL_STD_CHANNEL, 0x0038, },
    {"ADAT 4", MOTU_CTRL_STD_CHANNEL, 0x003c, },
    {"ADAT 5", MOTU_CTRL_STD_CHANNEL, 0x0040, },
    {"ADAT 6", MOTU_CTRL_STD_CHANNEL, 0x0044, },
    {"ADAT 7", MOTU_CTRL_STD_CHANNEL, 0x0048, },
    {"ADAT 8", MOTU_CTRL_STD_CHANNEL, 0x004c, },
};

const MatrixMixDestMk3 MixDests_TravelerMk3[] = {
    /* The 0-based position of the following elements represents
     * the d-bus index of the corresponding destination.
     * i.e: dbus mixDest 0 is the first destination defined
     * on this array*/
    {"Disabled", 0xff, },
    {"Phones", 0x06, },
    {"Analog 1-2", 0x00, },
    {"Analog 3-4", 0x01, },
    {"Analog 5-6", 0x02, },
    {"Analog 7-8", 0x03, },
    {"AES 1-2", 0x04, },
    {"SPDIF", 0x05, },
    {"ADAT A 1-2", 0x07, },
    {"ADAT A 3-4", 0x08, },
    {"ADAT A 5-6", 0x09, },
    {"ADAT A 7-8", 0x0a, },
    //TODO: Check if the following values are correct:
    {"ADAT B 1-2", 0x0b, },
    {"ADAT B 3-4", 0x0c, },
    {"ADAT B 5-6", 0x0d, },
    {"ADAT B 7-8", 0x0e, },
};

const MixerCtrlMk3 MixerCtrls_TravelerMk3[] = {
    {"Mix1/Mix_", "Mix 1 ", "", MOTU_CTRL_STD_MIX, 0x00, },
    {"Mix2/Mix_", "Mix 2 ", "", MOTU_CTRL_STD_MIX, 0x01, },
    {"Mix3/Mix_", "Mix 3 ", "", MOTU_CTRL_STD_MIX, 0x02, },
    {"Mix4/Mix_", "Mix 4 ", "", MOTU_CTRL_STD_MIX, 0x03, },
    {"Mix5/Mix_", "Mix 5 ", "", MOTU_CTRL_STD_MIX, 0x04, },
    {"Mix6/Mix_", "Mix 6 ", "", MOTU_CTRL_STD_MIX, 0x05, },
    {"Mix7/Mix_", "Mix 7 ", "", MOTU_CTRL_STD_MIX, 0x06, },
    {"Mix8/Mix_", "Mix 8 ", "", MOTU_CTRL_STD_MIX, 0x07, },
    //{"Mainout_",  "MainOut ", "", MOTU_CTRL_MIX_FADER, 0x0c0c, }, //TODO: Implement as old MOTU control
    //{"Phones_",   "Phones ",  "", MOTU_CTRL_MIX_FADER, 0x0c10, }, //TODO: Implement as old MOTU control

    /* For mic/line input controls, the "register" is the zero-based channel number */
    {"Control/Ana1_", "Analog 1 input ", "", MOTU_CTRL_TRAVELER_MIC_INPUT_CTRLS, 0},
    {"Control/Ana2_", "Analog 2 input ", "", MOTU_CTRL_TRAVELER_MIC_INPUT_CTRLS, 1},
    {"Control/Ana3_", "Analog 3 input ", "", MOTU_CTRL_TRAVELER_MIC_INPUT_CTRLS, 2},
    {"Control/Ana4_", "Analog 4 input ", "", MOTU_CTRL_TRAVELER_MIC_INPUT_CTRLS, 3},
    {"Control/Ana5_", "Analog 5 input ", "", MOTU_CTRL_TRAVELER_LINE_INPUT_CTRLS, 4},
    {"Control/Ana6_", "Analog 6 input ", "", MOTU_CTRL_TRAVELER_LINE_INPUT_CTRLS, 5},
    {"Control/Ana7_", "Analog 7 input ", "", MOTU_CTRL_TRAVELER_LINE_INPUT_CTRLS, 6},
    {"Control/Ana8_", "Analog 8 input ", "", MOTU_CTRL_TRAVELER_LINE_INPUT_CTRLS, 7},

    /* For phones source control, "register" is currently unused */
    {"Control/Phones_", "Phones source", "", MOTU_CTRL_PHONES_SRC, 0},

    /* For optical mode controls, the "register" is used to indicate direction */
    {"Control/OpticalIn_mode", "Optical input mode ", "", MOTU_CTRL_OPTICAL_MODE, MOTU_CTRL_DIR_IN},
    {"Control/OpticalOut_mode", "Optical output mode ", "", MOTU_CTRL_OPTICAL_MODE, MOTU_CTRL_DIR_OUT},
};

const MotuMixerMk3 Mixer_TravelerMk3 = MOTUMIXERMK3(
    MixerCtrls_TravelerMk3, MixerBuses_TravelerMk3, MixerChannels_TravelerMk3, MixDests_TravelerMk3);

}
