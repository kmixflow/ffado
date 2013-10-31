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

class MotuDiscreteCtrlMk3
    : public Control::Discrete
{
public:
    MotuDiscreteCtrlMk3(MotuDevice &parent, unsigned long int bus,
    		std::string name, std::string label, std::string descr);

    virtual bool setValue(int value) = 0;
    virtual int getValue() = 0;

    // default implementations
	virtual bool setValue(int idx, int v)
		{return setValue(v);};
	virtual int getValue(int idx)
		{return getValue();};

	virtual int getMinimum() {return 0;};
	virtual int getMaximum() {return 0;};


protected:
    MotuDevice    &m_parent;
    unsigned long int  m_key;
    unsigned long int  m_bus;
};

class MixDestMk3
    : public MotuDiscreteCtrlMk3
{
public:
    MixDestMk3(MotuDevice &parent, unsigned long int bus,
            std::string name, std::string label, std::string descr);

    virtual bool setValue(int value);
    virtual int getValue();
};

class MotuContinuousCtrlMk3
    : public Control::Continuous
{
public:
    MotuContinuousCtrlMk3(MotuDevice &parent, unsigned long int bus,
            std::string name, std::string label, std::string descr);

    // default implementations
    virtual bool setValue(double v) = 0;
    virtual double getValue() = 0;

    virtual bool setValue(int idx, double v) {
        return setValue(v);
    }
    ;
    virtual double getValue(int idx) {
        return getValue();
    }
    ;

    virtual double getMinimum();
    virtual double getMaximum();

protected:
    MotuDevice    &m_parent;
    unsigned long int  m_key;
    unsigned long int  m_bus;
    unsigned long int  m_minimum;
    unsigned long int  m_maximum;
};

class MixFaderMk3
    : public MotuContinuousCtrlMk3
{
public:
    MixFaderMk3(MotuDevice &parent, unsigned long int bus,
          std::string name, std::string label, std::string descr);

    virtual bool setValue(double v);
    virtual double getValue();
};

class MotuMatrixMixerMk3 : public Control::MatrixMixer
{
public:
    MotuMatrixMixerMk3(MotuDevice &parent);
    MotuMatrixMixerMk3(MotuDevice &parent, std::string name);
    virtual ~MotuMatrixMixerMk3() {};

    void addRowInfo(std::string name, unsigned int flags, unsigned int address);
    void addColInfo(std::string name, unsigned int flags, unsigned int address);
    uint32_t getCellRegister(const unsigned int row, const unsigned int col);

    virtual void show();

    bool hasNames() const { return true; }
    bool canConnect() const { return false; }

    virtual std::string getRowName(const int row);
    virtual std::string getColName(const int col);
    virtual int canWrite( const int, const int ) { return true; }
    virtual int getRowCount();
    virtual int getColCount();

    // full map updates are unsupported
    virtual bool getCoefficientMap(int &) {return false;};
    virtual bool storeCoefficientMap(int &) {return false;};

protected:
     struct sSignalInfo {
         std::string name;
         unsigned int flags;
         unsigned int address;
     };

     std::vector<struct sSignalInfo> m_RowInfo;
     std::vector<struct sSignalInfo> m_ColInfo;

     MotuDevice& m_parent;
};


class ChannelFaderMatrixMixerMk3 : public MotuMatrixMixerMk3
{
public:
    ChannelFaderMatrixMixerMk3(MotuDevice &parent);
    ChannelFaderMatrixMixerMk3(MotuDevice &parent, std::string name);
    virtual double setValue(const int row, const int col, const double val);
    virtual double getValue(const int row, const int col);
};

class ChannelPanMatrixMixerMk3 : public MotuMatrixMixerMk3
{
public:
    ChannelPanMatrixMixerMk3(MotuDevice &parent);
    ChannelPanMatrixMixerMk3(MotuDevice &parent, std::string name);
    virtual double setValue(const int row, const int col, const double val);
    virtual double getValue(const int row, const int col);
};

class ChannelBinSwMatrixMixerMk3 : public MotuMatrixMixerMk3
{
public:
    ChannelBinSwMatrixMixerMk3(MotuDevice &parent);
    ChannelBinSwMatrixMixerMk3(MotuDevice &parent, std::string name,
      unsigned int val_mask, unsigned int setenable_mask);
    virtual double setValue(const int row, const int col, const double val);
    virtual double getValue(const int row, const int col);

protected:
    unsigned int m_value_mask;
    unsigned int m_setenable_mask;
};


/* A "register" value used to signify that a particular control in a matrix
 * mixer is not available on the current interface.
 */
#define MOTU_MK3CTRL_NONE          0xffffffff

/* Default key for all MK3 Controls. This should be set up properly by each
 * control constructor
 */
#define MOTU_MK3_KEY_NONE          0x00000000

// FIXME: What would be the best value for this?
#define MOTU_MK3_VALUE_NONE        0x00000000

/* The following quadlets needs to be sent to MOTU_G3_REG_MIXER to reset
 * the packet serial number and begin interaction with the mixer
 */
#define MOTU_MK3CTRL_MIXER_RESET0  0x00000000
#define MOTU_MK3CTRL_MIXER_RESET1  0x00010000

/*The following flags are sent for request device status???
 *  (aa = packet serial number)*/
#define MOTU_MK3CTRL_MIXER_GET1    0x01aa0000
#define MOTU_MK3CTRL_MIXER_GET2    0x02aa0000

/*Mask for packet serial number */
#define MOTU_MK3CTRL_SERIAL_NUMBER		   0x00020000
#define MOTU_MK3CTRL_SERIAL_NUMBER_MASK	   0x00ff0000


/*
 *
 * Discrete controls
 *
 */

/* Control key definitions */
#define MOTU_MK3_MIX_DEST_ASSIGN_CTRL     0x00000002
#define MOTU_MK3_DISCRETE_CTRL            0x02006900
#define MOTU_MK3_CONTINUOUS_CTRL          0x02006600

#define MOTU_MK3CTRL_MIX_DEST_DISABLED     0xff
#define MOTU_MK3CTRL_MIX_DEST_MAIN_L_R     0x00
#define MOTU_MK3CTRL_MIX_DEST_ANALOG_1_2   0x01
#define MOTU_MK3CTRL_MIX_DEST_ANALOG_3_4   0x02
#define MOTU_MK3CTRL_MIX_DEST_ANALOG_5_6   0x03
#define MOTU_MK3CTRL_MIX_DEST_ANALOG_7_8   0x04
#define MOTU_MK3CTRL_MIX_DEST_SPDIF        0x05
#define MOTU_MK3CTRL_MIX_DEST_PHONES       0x06
#define MOTU_MK3CTRL_MIX_DEST_ADAT_A_1_2   0x07
#define MOTU_MK3CTRL_MIX_DEST_ADAT_A_3_4   0x08
#define MOTU_MK3CTRL_MIX_DEST_ADAT_A_5_6   0x09
#define MOTU_MK3CTRL_MIX_DEST_ADAT_A_7_8   0x0a
#define MOTU_MK3CTRL_MIX_DEST_ADAT_B_1_2   0x0b
#define MOTU_MK3CTRL_MIX_DEST_ADAT_B_3_4   0x0c
#define MOTU_MK3CTRL_MIX_DEST_ADAT_B_5_6   0x0d
#define MOTU_MK3CTRL_MIX_DEST_ADAT_B_7_8   0x0e

#define MOTU_MK3CTRL_MIX1                  0x00000000
#define MOTU_MK3CTRL_MIX2                  0x02000000
#define MOTU_MK3CTRL_MIX3                  0x03000000
#define MOTU_MK3CTRL_MIX4                  0x05000000
#define MOTU_MK3CTRL_MIX5                  0x06000000
#define MOTU_MK3CTRL_MIX6                  0x07000000
#define MOTU_MK3CTRL_MIX7                  0x08000000
#define MOTU_MK3CTRL_MIX8                  0x09000000

/*
 *
 * DISCRETE CONTROLS KEYS
 *
 */
/*
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
#define MOTU_MK3CTRL_MIX_CHANNEL_SOLO             0x010002 //mm
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

#define MOTU_MK3CTRL_CHANNEL                      0x02 // 02->1c*/

/*
 *
 * CONTINUOUS CONTROLS KEYS
 *
 */


#define MOTU_MK3CTRL_BUS_REVERB_SEND       0x00010200
#define MOTU_MK3CTRL_INPUT_REVERB_SEND     0x00010200
#define MOTU_MK3CTRL_BUS_REVERB_RETURN     0x01010200
#define MOTU_MK3CTRL_INPUT_TRIM            0x02000100
#define MOTU_MK3CTRL_BUS_MASTER_FADER      0x02000200
#define MOTU_MK3CTRL_CHANNEL_PAN           0x02000200 //CAUTION
#define MOTU_MK3CTRL_CHANNEL_FADER         0x03000200
#define MOTU_MK3CTRL_CHANNEL_BALANCE       0x05000200
#define MOTU_MK3CTRL_CHANNEL_WIDTH         0x06000200
#define MOTU_MK3CTRL_FOCUS_SELECT          0x07000000

#define MOTU_MK3CTRL_INPUT_EQ_FREQ         0x02000100
#define MOTU_MK3CTRL_INPUT_EQ_GAIN         0x03000100
#define MOTU_MK3CTRL_INPUT_EQ_Q            0x04000100
#define MOTU_MK3CTRL_OUTPUT_EQ_FREQ        0x02000300
#define MOTU_MK3CTRL_OUTPUT_EQ_GAIN        0x03000300
#define MOTU_MK3CTRL_OUTPUT_EQ_Q           0x04000300

#define MOTU_MK3CTRL_EQ_A                  0x00030000
#define MOTU_MK3CTRL_EQ_B                  0x00070000
#define MOTU_MK3CTRL_EQ_C                  0x00040000
#define MOTU_MK3CTRL_EQ_D                  0x00050000
#define MOTU_MK3CTRL_EQ_E                  0x00060000
#define MOTU_MK3CTRL_EQ_F                  0x00020000
#define MOTU_MK3CTRL_EQ_G                  0x00800000

#define MOTU_MK3CTRL_INPUT_COMP_THRESHOLD  0x010a0100
#define MOTU_MK3CTRL_INPUT_COMP_RATIO      0x020a0100
#define MOTU_MK3CTRL_INPUT_COMP_ATTACK     0x030a0100
#define MOTU_MK3CTRL_INPUT_COMP_RELEASE    0x040a0100
#define MOTU_MK3CTRL_INPUT_COMP_TRIM       0x050a0100

#define MOTU_MK3CTRL_INPUT_LVLR_MAKEUP     0x020b0100
#define MOTU_MK3CTRL_INPUT_LVLR_REDUCTION  0x030b0100

#define MOTU_MK3CTRL_OUTPUT_COMP_THRESHOLD 0x01090300
#define MOTU_MK3CTRL_OUTPUT_COMP_RATIO     0x02090300
#define MOTU_MK3CTRL_OUTPUT_COMP_ATTACK    0x03090300
#define MOTU_MK3CTRL_OUTPUT_COMP_RELEASE   0x04090300
#define MOTU_MK3CTRL_OUTPUT_COMP_TRIM      0x05090300

#define MOTU_MK3CTRL_OUTPUT_LVLR_MAKEUP    0x020a0300
#define MOTU_MK3CTRL_OUTPUT_LVLR_REDUCTION 0x030a0300

#define MOTU_MK3CTRL_REVERB_PREDELAY       0x02000400
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ     0x03000400
#define MOTU_MK3CTRL_REVERB_SHELF_CUT      0x04000400
#define MOTU_MK3CTRL_REVERB_TIME           0x05000400
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_LOW 0x06000400
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MED 0x07000400
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_HIGH 0x08000400
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW 0x09000400
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH 0x0a000400
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH      0x0b000400
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE 0x0d000400
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL  0x0e000400




//The following belongs to "Discrete" section:
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

// Channel section limits
#define MOTU_MK3CTRL_FADER_MIN          0x00000000 // -inf dB
#define MOTU_MK3CTRL_FADER_MAX          0x3f800000 //    0 dB

#define MOTU_MK3CTRL_TRIM_MIC_MIN       0x00000000 //    0 dB
#define MOTU_MK3CTRL_TRIM_MIC_MAX       0x42540000 //  +53 dB
#define MOTU_MK3CTRL_TRIM_LINE_MIN      0xc2c00000 //  -96 dB
#define MOTU_MK3CTRL_TRIM_LINE_MAX      0x41b00000 //  +22 dB

#define MOTU_MK3CTRL_BALANCE_MIN        0xbf800000 // -1
#define MOTU_MK3CTRL_BALANCE_MAX        0x3f800000 // +1

#define MOTU_MK3CTRL_CH_WIDTH_MIN       0x00000000 // 0
#define MOTU_MK3CTRL_CH_WIDTH_MAX       0x3f800000 // 1

#define MOTU_MK3CTRL_PAN_LEFT           0xbf800000 // Pan 100% Left
#define MOTU_MK3CTRL_PAN_RGHT           0x3f800000 // Pan 100% Right

// Dynamics section limits
#define MOTU_MK3CTRL_COMP_TRIM_MIN      0xc0c00000 //  -6 dB
#define MOTU_MK3CTRL_COMP_TRIM_MAX      0x00000000 //   0 dB
#define MOTU_MK3CTRL_COMP_RATIO_MIN     0x3f800000 //   1:1
#define MOTU_MK3CTRL_COMP_RATIO_MAX     0x41200000 //  10:1
#define MOTU_MK3CTRL_COMP_ATTACK_MIN    0x41200000 //  10 ms
#define MOTU_MK3CTRL_COMP_ATTACK_MAX    0x42c80000 // 100 ms
#define MOTU_MK3CTRL_COMP_RELEASE_MIN   0x41200000 //  10 ms
#define MOTU_MK3CTRL_COMP_RELEASE_MAX   0x44fa1200 //   2 s
#define MOTU_MK3CTRL_COMP_THRESHOLD_MIN 0xc2400000 // -48 dB
#define MOTU_MK3CTRL_COMP_THRESHOLD_MAX 0x00000000 //   0 dB

#define MOTU_MK3CTRL_LVLR_MAKEUP_MIN    0x00000000 //   ? db
#define MOTU_MK3CTRL_LVLR_MAKEUP_MAX    0x42c80000 //   ? db
#define MOTU_MK3CTRL_LVLR_REDUCTION_MIN 0x00000000 //   ? db
#define MOTU_MK3CTRL_LVLR_REDUCTION_MAX 0x42c80000 //   ? db

// EQ section limits
#define MOTU_MK3CTRL_EQ_FREQ_MIN  0x41a00001 //   20 Hz
#define MOTU_MK3CTRL_EQ_FREQ_MAX  0x469c4004 //   20 kHz
#define MOTU_MK3CTRL_EQ_GAIN_MIN  0xc1a00000 //  -20 dB
#define MOTU_MK3CTRL_EQ_GAIN_MAX  0x41a00000 //  +20 dB
#define MOTU_MK3CTRL_EQ_Q_MIN     0x3c23d70a // 0.01
#define MOTU_MK3CTRL_EQ_Q_MAX     0x40400000 // 3.00

//Reverb section limits
#define MOTU_MK3CTRL_REVERB_MIN                    0x00000000 // -inf dB
#define MOTU_MK3CTRL_REVERB_MAX                    0x3f800000 //    0 dB
#define MOTU_MK3CTRL_REVERB_PREDELAY_MIN           0x00000000 //    0 ms
#define MOTU_MK3CTRL_REVERB_PREDELAY_MAX           0x42c80000 //  100 ms
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ_MIN         0x447a0000 //    1 kHz
#define MOTU_MK3CTRL_REVERB_SHELF_FREQ_MAX         0x469c4000 //   20 kHz
#define MOTU_MK3CTRL_REVERB_SHELF_CUT_MIN          0xc2200000 //  -40 dB
#define MOTU_MK3CTRL_REVERB_SHELF_CUT_MAX          0x00000000 //    0 dB
#define MOTU_MK3CTRL_REVERB_TIME_MIN               0x42c80000 //  100 ms
#define MOTU_MK3CTRL_REVERB_TIME_MAX               0x476a6000 //   60 s
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MIN        0x00000000 //    0 %
#define MOTU_MK3CTRL_REVERB_DESIGN_TIME_MAX        0x42c80000 //  100 %
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW_MIN   0x42c80000 //  100 Hz
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_LOW_MAX   0x469c4004 //   20 kHz
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH_MIN  0x447a0000 //    1 kHz
#define MOTU_MK3CTRL_REVERB_DESIGN_CROSS_HIGH_MAX  0x469c4004 //   20 kHz
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH_MIN       0x469c4004 // -100 %
#define MOTU_MK3CTRL_REVERB_DESIGN_WIDTH_MAX       0x3f800000 // +100 %
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE_MIN 0x42480000 //   50 %
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_SIZE_MAX 0x43c64000 //  400 %
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL_MIN  0x00000000 // -inf dB
#define MOTU_MK3CTRL_REVERB_EARLY_REFLECT_LVL_MAX  0x3f800000 //    0 dB



}
