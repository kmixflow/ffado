/*
 * Copyright (C) 2005-2008 by Daniel Wagner
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

#include "debugmodule/debugmodule.h"

#include "libcontrol/BasicElements.h"
#include "libcontrol/MatrixMixer.h"

namespace Motu {

class MotuDevice;

/* A "register" value used to signify that a particular control in a matrix
 * mixer is not available on the current interface.
 */
#define MOTU_MK3CTRL_NONE                  0xffffffff



/*The following flags needs to be sent to MOTU_G3_REG_MIXER to reset
the packet serial number and start operating the mixer*/
#define MOTU_MK3CTRL_MIXER_RESET1  0x00000000
#define MOTU_MK3CTRL_MIXER_RESET2  0x00010000

/*The following flags are sent for request device status???
 *  (aa = packet serial number)*/
#define MOTU_MK3CTRL_MIXER_GET1    0x01aa0000
#define MOTU_MK3CTRL_MIXER_GET2    0x02aa0000

/*Mask for packet serial number */
#define MOTU_MK3CTRL_MIXER_SERIAL_NUMBER_MASK	   0x00ff0000

/*
 *
 * CONTINUOUS CONTROLS
 *
 */

/* Control key definitions */
#define MOTU_MK3CTRL_BUS_REVERB_SEND       0x000102
#define MOTU_MK3CTRL_INPUT_REVERB_SEND     0x000102
#define MOTU_MK3CTRL_BUS_REVERB_RETURN     0x010102
#define MOTU_MK3CTRL_INPUT_TRIM            0x020001
#define MOTU_MK3CTRL_BUS_MASTER_FADER      0x020002
#define MOTU_MK3CTRL_CHANNEL_PAN           0x020002 /*CAUTION*/
#define MOTU_MK3CTRL_CHANNEL_FADER         0x030002
#define MOTU_MK3CTRL_CHANNEL_BALANCE       0x050002
#define MOTU_MK3CTRL_CHANNEL_WIDTH         0x060002
#define MOTU_MK3CTRL_FOCUS_SELECT          0x070000

#define MOTU_MK3CTRL_INPUT_EQ_FREQ         0x020001
#define MOTU_MK3CTRL_INPUT_EQ_GAIN         0x030001
#define MOTU_MK3CTRL_INPUT_EQ_Q            0x040001
#define MOTU_MK3CTRL_OUTPUT_EQ_FREQ        0x020003
#define MOTU_MK3CTRL_OUTPUT_EQ_GAIN        0x030003
#define MOTU_MK3CTRL_OUTPUT_EQ_Q           0x040003

#define MOTU_MK3CTRL_EQ_A                  0x000300
#define MOTU_MK3CTRL_EQ_B                  0x000700
#define MOTU_MK3CTRL_EQ_C                  0x000400
#define MOTU_MK3CTRL_EQ_D                  0x000500
#define MOTU_MK3CTRL_EQ_E                  0x000600
#define MOTU_MK3CTRL_EQ_F                  0x000200
#define MOTU_MK3CTRL_EQ_G                  0x008000

#define MOTU_MK3CTRL_INPUT_COMP_THRESHOLD  0x010a01
#define MOTU_MK3CTRL_INPUT_COMP_RATIO      0x020a01
#define MOTU_MK3CTRL_INPUT_COMP_ATTACK     0x030a01
#define MOTU_MK3CTRL_INPUT_COMP_RELEASE    0x040a01
#define MOTU_MK3CTRL_INPUT_COMP_TRIM       0x050a01

#define MOTU_MK3CTRL_INPUT_LVLR_MAKEUP     0x020b01
#define MOTU_MK3CTRL_INPUT_LVLR_REDUCTION  0x030b01

#define MOTU_MK3CTRL_OUTPUT_COMP_THRESHOLD 0x010903
#define MOTU_MK3CTRL_OUTPUT_COMP_RATIO     0x020903
#define MOTU_MK3CTRL_OUTPUT_COMP_ATTACK    0x030903
#define MOTU_MK3CTRL_OUTPUT_COMP_RELEASE   0x040903
#define MOTU_MK3CTRL_OUTPUT_COMP_TRIM      0x050903

#define MOTU_MK3CTRL_OUTPUT_LVLR_MAKEUP    0x020a03
#define MOTU_MK3CTRL_OUTPUT_LVLR_REDUCTION 0x030a03

#define MOTU_MK3CTRL_REVERB_PREDELAY       0x020004
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ     0x030004
#define MOTU_MK3CTRL_REVERB_SHELF_CUT      0x040004
#define MOTU_MK3CTRL_REVERB_TIME           0x050004
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_LOW 0x060004
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MED 0x070004
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_HIGH 0x080004
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW 0x090004
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH 0x0a0004
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH      0x0b0004
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE 0x0d0004
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL  0x0e0004
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_MODEL_A  0x00
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_MODEL_B  0x01
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_MODEL_C  0x02
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_MODEL_D  0x03
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_MODEL_E  0x04

/*
 *
 * LIMITS
 *
 */

/* Channel section limits */
#define MOTU_MK3CTRL_FADER_MIN          0x00000000 /* -inf dB  */
#define MOTU_MK3CTRL_FADER_MAX          0x3f800000 /*    0 dB  */

#define MOTU_MK3CTRL_TRIM_MIC_MIN       0x00000000 /*    0 dB  */
#define MOTU_MK3CTRL_TRIM_MIC_MAX       0x42540000 /*  +53 dB  */
#define MOTU_MK3CTRL_TRIM_LINE_MAX      0xc2c00000 /*  -96 dB  */
#define MOTU_MK3CTRL_TRIM_LINE_MAX      0x41b00000 /*  +22 dB  */

#define MOTU_MK3CTRL_BALANCE_MIN        0xbf800000 /* -1 */
#define MOTU_MK3CTRL_BALANCE_MAX        0x3f800000 /* +1 */

#define MOTU_MK3CTRL_CH_WIDTH_MIN       0x00000000 /* 0 */
#define MOTU_MK3CTRL_CH_WIDTH_MAX       0x3f800000 /* 1 */

#define MOTU_MK3CTRL_PAN_LEFT           0xbf800000 /* Pan 100% Left  */
#define MOTU_MK3CTRL_PAN_RGHT           0x3f800000 /* Pan 100% Right */

/* Dynamics section limits */
#define MOTU_MK3CTRL_COMP_TRIM_MIN      0xc0c00000 /*  -6 dB  */
#define MOTU_MK3CTRL_COMP_TRIM_MAX      0x00000000 /*   0 dB  */
#define MOTU_MK3CTRL_COMP_RATIO_MIN     0x3f800000 /*   1:1   */
#define MOTU_MK3CTRL_COMP_RATIO_MAX     0x41200000 /*  10:1   */
#define MOTU_MK3CTRL_COMP_ATTACK_MIN    0x41200000 /*  10 ms  */
#define MOTU_MK3CTRL_COMP_ATTACK_MAX    0x42c80000 /* 100 ms  */
#define MOTU_MK3CTRL_COMP_RELEASE_MIN   0x41200000 /*  10 ms  */
#define MOTU_MK3CTRL_COMP_RELEASE_MAX   0x44fa1200 /*   2 s   */
#define MOTU_MK3CTRL_COMP_THRESHOLD_MIN 0xc2400000 /* -48 dB  */
#define MOTU_MK3CTRL_COMP_THRESHOLD_MAX 0x00000000 /*   0 dB  */

#define MOTU_MK3CTRL_LVLR_MAKEUP_MIN    0x00000000 /*   ? db  */
#define MOTU_MK3CTRL_LVLR_MAKEUP_MAX    0x42c80000 /*   ? db  */
#define MOTU_MK3CTRL_LVLR_REDUCTION_MIN 0x00000000 /*   ? db  */
#define MOTU_MK3CTRL_LVLR_REDUCTION_MAX 0x42c80000 /*   ? db  */

/* EQ section limits */
#define MOTU_MK3CTRL_EQ_FREQ_MIN  0x41a00001 /*   20 Hz  */
#define MOTU_MK3CTRL_EQ_FREQ_MAX  0x469c4004 /*   20 kHz */
#define MOTU_MK3CTRL_EQ_GAIN_MIN  0xc1a00000 /*  -20 dB  */
#define MOTU_MK3CTRL_EQ_GAIN_MAX  0x41a00000 /*  +20 dB  */
#define MOTU_MK3CTRL_EQ_Q_MIN     0x3c23d70a /* 0.01     */
#define MOTU_MK3CTRL_EQ_Q_MAX     0x40400000 /* 3.00     */

/*Reverb section limits */
#define MOTU_MK3CTRL_REVERB_MIN                    0x00000000 /* -inf dB */
#define MOTU_MK3CTRL_REVERB_MAX                    0x3f800000 /*    0 dB */
#define MOTU_MK3CTRL_REVERB_PREDELAY_MIN           0x00000000 /*    0 ms  */
#define MOTU_MK3CTRL_REVERB_PREDELAY_MAX           0x42c80000 /*  100 ms  */
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ_MIN         0x447a0000 /*    1 kHz */
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ_MAX         0x469c4000 /*   20 kHz */
#define MOTU_MK3CTRL_REVERB_SHELF_CUT_MIN          0xc2200000 /*  -40 dB  */
#define MOTU_MK3CTRL_REVERB_SHELF_CUT_MAX          0x00000000 /*    0 dB  */
#define MOTU_MK3CTRL_REVERB_TIME_MIN               0x42c80000 /*  100 ms  */
#define MOTU_MK3CTRL_REVERB_TIME_MAX               0x476a6000 /*   60 s   */
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MIN        0x00000000 /*    0 %   */
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MAX        0x42c80000 /*  100 %   */
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW_MIN   0x42c80000 /*  100 Hz  */
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW_MAX   0x469c4004 /*   20 kHz */
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH_MIN  0x447a0000 /*    1 kHz */
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH_MAX  0x469c4004 /*   20 kHz */
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH_MIN       0x469c4004 /* -100 %   */
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH_MAX       0x3f800000 /* +100 %   */
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE_MIN 0x42480000 /*   50 %   */
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE_MAX 0x43c64000 /*  400 %   */
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL_MIN  0x00000000 /* -inf dB  */
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL_MAX  0x3f800000 /*    0 dB  */

/*
 *
 * DISCRETE CONTROLS KEYS
 *
 */

#define MOTU_MK3CTRL_INPUT_CHANNEL_DYNAMICS       0x000001
#define MOTU_MK3CTRL_BUS_OUTPUT_ASSIGN            0x000002
#define MOTU_MK3CTRL_OUTPUT_CHANNEL_DYNAMICS      0x000003
#define MOTU_MK3CTRL_TALKBACK_CHANNEL             0x000300
#define MOTU_MK3CTRL_TALKBACK_LISTEN              0x000400
#define MOTU_MK3CTRL_MIX_CHANNEL_MUTE             0x000002
#define MOTU_MK3CTRL_BUS_MUTE                     0x000102
#define MOTU_MK3CTRL_OUTPUT_MONITOR               0x000c03
#define MOTU_MK3CTRL_INPUT_CHANNEL_MODE           0x010001
#define MOTU_MK3CTRL_REVERB_SPLIT_POINT           0x010004
#define MOTU_MK3CTRL_MIX_CHANNEL_SOLO             0x010002 /*mm*/
#define MOTU_MK3CTRL_LVLR_OUTPUT_CHANNEL_MODE     0x010a03
#define MOTU_MK3CTRL_TALKBACK_OUTPUT_CHANNEL_TALK 0x010c03
#define MOTU_MK3CTRL_TALKBACK_OUTPUT_CHANNEL_LSTN 0x020c03
#define MOTU_MK3CTRL_INPUT_CHANNEL_SWAP           0x030001
#define MOTU_MK3CTRL_INPUT_CHANNEL_STEREO_MODE    0x040001
#define MOTU_MK3CTRL_INPUT_CHANNEL_VLIMIT         0x060001
#define MOTU_MK3CTRL_INPUT_CHANNEL_COMP_MODE      0x060a01
#define MOTU_MK3CTRL_INPUT_CHANNEL_LVLR_MODE      0x060b01
#define MOTU_MK3CTRL_OUTPUT_CHANNEL_COMP_MODE     0x060903
#define MOTU_MK3CTRL_INPUT_CHANNEL_VLIMIT_LOOKAH  0x070001
#define MOTU_MK3CTRL_INPUT_CHANEL_SOFT_CLIP       0x080801
#define MOTU_MK3CTRL_INPUT_CHANEL_COMP_MODE       0x0c0004

#define MOTU_MK3CTRL_ON                           0x01
#define MOTU_MK3CTRL_OFF                          0x00

#define MOTU_MK3CTRL_CHANNEL                      0x02 /* 02->1c */


}
