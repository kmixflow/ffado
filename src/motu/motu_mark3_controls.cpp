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
/* Mk3 protocol description at ../../doc/motu_firewire_protocol-mk3.txt */

// This also includes motu_mark3_controls.h
#include "motu_avdevice.h"
#include "libutil/ByteSwap.h"

namespace Motu {

MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent,
        unsigned long int bus, std::string name, std::string label,
        std::string descr) :
        Control::Discrete(&parent), m_parent(parent) {
    setName(name);
    setLabel(label);
    setDescription(descr);
    m_key = MOTU_MK3CTRL_NONE;
    //TODO: Check if bus is valid
    m_bus = bus;
}

bool MotuDiscreteCtrlMk3::setValue(int value) {
    if (this->m_key == MOTU_MK3CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a discrete control with unintialized key\n");
        return true;
    }
    quadlet_t data[2];
    unsigned int serial = m_parent.getMk3MixerSerial();

    //Prepare data:
    data[0] = MOTU_MK3CTRL_DISCRETE_CTRL | (serial << 16) | value;
    data[1] = (this->m_bus << 24) | this->m_key;

    //Write data:
    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 2)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) to mixer register\n", data[0], data[1]);
        return false;
    }
    //Everything is OK, let's update serial number for next communication
    m_parent.updateMk3MixerSerial();
    return true;
}

int MotuDiscreteCtrlMk3::getValue() {
    //TODO: Try to figure out how to request control values from Mk3 Devices
    return 0;
}

MixDestMk3::MixDestMk3(MotuDevice &parent, unsigned long int bus,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, bus, name, label, descr) {
    this->m_key = MOTU_MK3CTRL_MIX_DEST_ASSIGN;
}

bool MixDestMk3::setValue(int value) {
    unsigned int val = (unsigned int) value;
    //Check if there is a mix destination description for this device (motu_mark3_mixerdefs.cpp):
    if(DevicesProperty[this->m_parent.m_motu_model-1].mk3mixer->mix_destinations == NULL){
        debugOutput(DEBUG_LEVEL_WARNING, "No mix destinations are defined for this model\n"); //FIXME: Which model?
        return false;
    }
    //Check if there is a mix destination description for selected value:
    if((0 > value) || (DevicesProperty[this->m_parent.m_motu_model-1].mk3mixer->n_mix_destinations <= val))
    {
        debugOutput(DEBUG_LEVEL_WARNING, "Mix destination number %d is not defined for this model\n", val); //FIXME: Which model?
        return false;
    }
    //Ok, there is a destination description for selected value. Let's send corresponding key:
    return MotuDiscreteCtrlMk3::setValue(DevicesProperty[this->m_parent.m_motu_model-1].mk3mixer->mix_destinations[val].key);
}

int MixDestMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

MixMuteMk3::MixMuteMk3(MotuDevice &parent, unsigned long int bus,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, bus, name, label, descr) {
    this->m_key = MOTU_MK3CTRL_BUS_MUTE;
}

bool MixMuteMk3::setValue(int value) {
    unsigned int val = (unsigned int) value;
    if ((0 > val) || (1 < val))
    {
        debugOutput(DEBUG_LEVEL_WARNING, "Value %d is not valid for MixMute control\n", val);
        return false;
    }
    return MotuDiscreteCtrlMk3::setValue(val);
}

int MixMuteMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

InputPadMk3::InputPadMk3(MotuDevice &parent, unsigned long int channel,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, channel, name, label, descr) {
    if((0 > channel) || (MOTU_CTRL_TRIMGAINPAD_MAX_CHANNEL < channel))
    {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Invalid channel %d: max supported is %d\n",
                    channel, MOTU_CTRL_TRIMGAINPAD_MAX_CHANNEL);
        this->m_key = MOTU_MK3CTRL_NONE;
    }
    this->m_key = MOTU_MK3CTRL_INPUT_PAD;
}

bool InputPadMk3::setValue(int value) {
    unsigned int val = (unsigned int) value;
    if ((0 > val) || (1 < val))
    {
        debugOutput(DEBUG_LEVEL_WARNING, "Value %d is not valid for InputGainPadInv control\n", val);
        return false;
    }
    return MotuDiscreteCtrlMk3::setValue(val);
}

int InputPadMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

InputPhaseMk3::InputPhaseMk3(MotuDevice &parent, unsigned long int channel,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, channel, name, label, descr) {
    if((0 > channel) || (MOTU_CTRL_GAINPHASEINV_MAX_CHANNEL < channel))
    {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Invalid channel %d: max supported is %d\n",
                    channel, MOTU_CTRL_GAINPHASEINV_MAX_CHANNEL);
        this->m_key = MOTU_MK3CTRL_NONE;
    }
    this->m_key = MOTU_MK3CTRL_INPUT_PHASE;
}

bool InputPhaseMk3::setValue(int value) {
    unsigned int val = (unsigned int) value;
    if ((0 > val) || (1 < val))
    {
        debugOutput(DEBUG_LEVEL_WARNING, "Value %d is not valid for InputPhaseInv control\n", val);
        return false;
    }
    return MotuDiscreteCtrlMk3::setValue(val);
}

int InputPhaseMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

InputLevelMk3::InputLevelMk3(MotuDevice &parent, unsigned long int channel,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, channel, name, label, descr) {
    //FIXME: Check if channel is valid
    this->m_key = MOTU_MK3CTRL_INPUT_LEVEL;
}

bool InputLevelMk3::setValue(int value) {
    unsigned int val = (unsigned int) value;
    if ((0 > val) || (1 < val))
    {
        debugOutput(DEBUG_LEVEL_WARNING, "Value %d is not valid for InputLevel control\n", val);
        return false;
    }
    return MotuDiscreteCtrlMk3::setValue(val);
}

int InputLevelMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

MotuContinuousCtrlMk3::MotuContinuousCtrlMk3(MotuDevice &parent,
        unsigned long int bus, std::string name, std::string label,
        std::string descr) :
        Control::Continuous(&parent), m_parent(parent) {
    setName(name);
    setLabel(label);
    setDescription(descr);
    m_key = MOTU_MK3CTRL_NONE;
    m_minimum = MOTU_MK3VALUE_NONE;
    m_maximum = MOTU_MK3VALUE_NONE;
    //FIXME: Check if bus is valid
    m_bus = bus;
}

MotuContinuousCtrlMk3::MotuContinuousCtrlMk3(MotuDevice &parent) :
        Control::Continuous(&parent), m_parent(parent) {
    m_key = MOTU_MK3CTRL_NONE;
    m_minimum = MOTU_MK3VALUE_NONE;
    m_maximum = MOTU_MK3VALUE_NONE;
    //FIXME: Check if bus is valid
    m_bus = NULL;
}

bool MotuContinuousCtrlMk3::setValue(double value) {
    if (this->m_key == MOTU_MK3CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a continuous control with uninitialized control key\n");
        return false;
    }

    unsigned int val, serial;
    quadlet_t data[3];

    val = (unsigned int)value;
    serial = m_parent.getMk3MixerSerial();

    if (val > this->m_maximum) {
        val = m_maximum;
        debugOutput(DEBUG_LEVEL_WARNING, "Trying to set a continuous control with value=%x, higher than control maximum=%lu\n", val, this->m_maximum);
    } else if (val < this->m_minimum) {
        val = m_minimum;
        debugOutput(DEBUG_LEVEL_WARNING, "Trying to set a continuous control with value=%x, lower than control minimum=%lu\n", val, this->m_minimum);
    }

    //Values sent to MOTU must be big-endian:
    val=CondSwapToBus32(val);

    //Prepare data:
    data[0] = MOTU_MK3CTRL_CONTINUOUS_CTRL | (serial << 16) | this->m_bus;
    data[1] = this->m_key | (val >> 24);
    data[2] = (val << 8);

    //Write data:
    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 3)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) data[2]=(0x%08x) to mixer register\n", data[0], data[1], data[2]);
        return false;
    }
    //Everything is OK, let's update serial number for next communication
    m_parent.updateMk3MixerSerial();
    return true;
}

double MotuContinuousCtrlMk3::getMinimum() {
    return m_minimum;
}

double MotuContinuousCtrlMk3::getMaximum() {
    return m_maximum;
}

MixFaderMk3::MixFaderMk3(MotuDevice &parent, unsigned long int bus,
        std::string name, std::string label, std::string descr) :
        MotuContinuousCtrlMk3(parent, bus, name, label, descr) {
    this->m_key = MOTU_MK3CTRL_BUS_MASTER_FADER;
    this->m_minimum = MOTU_MK3CTRL_FADER_MIN;
    this->m_maximum = MOTU_MK3CTRL_FADER_MAX;
}

bool MixFaderMk3::setValue(double value) {
    /* FIXME: Transform dbus values to motu scale in a nice way.
     * Currently: 0(dbus)=0x0(motu) -> 128(dbus)=0x3f800000(motu),
     */
    return MotuContinuousCtrlMk3::setValue(value*MOTU_MK3CTRL_FADER_MAX/128);
}

double MixFaderMk3::getValue()
{
    return 0;
}

InputTrimMk3::InputTrimMk3(MotuDevice &parent, unsigned long int channel, unsigned long int mode,
        std::string name, std::string label, std::string descr) :
        MotuContinuousCtrlMk3(parent, channel, name, label, descr) {
    this->m_key = MOTU_MK3CTRL_INPUT_TRIM;
    if(mode == MOTU_CTRL_INPUT_TRIMGAIN)
    {
        this->m_minimum = MOTU_MK3CTRL_INPUT_TRIM_MIC_MIN;
        this->m_maximum = MOTU_MK3CTRL_INPUT_TRIM_MIC_MAX;
    }
    else if(mode == MOTU_CTRL_INPUT_BOOST)
    {
        this->m_minimum = MOTU_MK3CTRL_INPUT_TRIM_LINE_MIN;
        this->m_maximum = MOTU_MK3CTRL_INPUT_TRIM_LINE_MAX;
    }
    else
    {
        this->m_key = MOTU_MK3CTRL_NONE;
        this->m_minimum = MOTU_MK3VALUE_NONE;
        this->m_maximum = MOTU_MK3VALUE_NONE;
        debugOutput(DEBUG_LEVEL_WARNING, "Invalid mode %d for InputTrim control\n", mode);
    }
}

bool InputTrimMk3::setValue(double value) {
    // FIXME: Transform dbus values to motu scale in a nice way.
    return MotuContinuousCtrlMk3::setValue(value*this->m_maximum/53);
}

double InputTrimMk3::getValue()
{
    return 0;
}

ChannelCtrlMk3::ChannelCtrlMk3(MotuDevice &parent) :
        MotuContinuousCtrlMk3(parent) {
}

bool ChannelCtrlMk3::setValue(double value, int bus, int channel){
    if (this->m_key == MOTU_MK3CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a continuous control with uninitialized control key\n");
        return false;
    }

    channel += 2; // TravelerMk3 channels have offset of 2... Need to check other models.
    unsigned int val, serial;
    quadlet_t data[3];

    val = (unsigned int)value;
    serial = m_parent.getMk3MixerSerial();

    if (val > this->m_maximum) {
        val = m_maximum;
        debugOutput(DEBUG_LEVEL_WARNING, "Trying to set a continuous control with value=%x, higher than control maximum=%x\n", val, this->m_maximum);
    } else if (val < this->m_minimum) {
        val = m_minimum;
        debugOutput(DEBUG_LEVEL_WARNING, "Trying to set a continuous control with value=%x, lower than control minimum=%x\n", val, this->m_minimum);
    }

    //Values sent to MOTU must be big-endian:
    val=CondSwapToBus32(val);

    //Prepare data:
    data[0] = MOTU_MK3CTRL_CONTINUOUS_CTRL | (serial << 16) | bus;
    data[1] = this->m_key | (val >> 24) | (channel << 16);
    data[2] = (val << 8);

    //Write data:
    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 3)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) data[2]=(0x%08x) to mixer register\n", data[0], data[1], data[2]);
        return false;
    }
    //Everything is OK, let's update serial number for next communication
    m_parent.updateMk3MixerSerial();
    return true;
}


ChannelFaderMk3::ChannelFaderMk3(MotuDevice &parent):
        ChannelCtrlMk3(parent){
    this->m_key = MOTU_MK3CTRL_CHANNEL_FADER;
    this->m_maximum = MOTU_MK3CTRL_FADER_MAX;
    this->m_minimum = MOTU_MK3CTRL_FADER_MIN;
}

bool ChannelFaderMk3::setValue(double value, int bus, int channel)
{
    return ChannelCtrlMk3::setValue(value*MOTU_MK3CTRL_FADER_MAX/128, bus, channel);
}

bool ChannelFaderMk3::setValue(double value)
{
    //FIXME: This shouldn't exist
    return false;
}

double ChannelFaderMk3::getValue()
{
    return 0;
}

ChannelPanMk3::ChannelPanMk3(MotuDevice &parent):
        ChannelCtrlMk3(parent){
    this->m_key = MOTU_MK3CTRL_CHANNEL_PAN;
    this->m_maximum = MOTU_MK3CTRL_PAN_LEFT;
    this->m_minimum = MOTU_MK3CTRL_PAN_RGHT;
}

bool ChannelPanMk3::setValue(double value, int bus, int channel)
{
    // FIXME: Values as described in motu_firewire_protocol-mk3.txt don't work
    //return ChannelCtrlMk3::setValue((this->m_maximum-this->m_minimum)/128*(value+64)+this->m_minimum, bus, channel);
    return true;
}

bool ChannelPanMk3::setValue(double value)
{
    //FIXME: This shouldn't exist
    return false;
}

double ChannelPanMk3::getValue()
{
    return 0;
}

/*
 *
 *
 *
 *
 * MATRIX MIXER
 *
 *
 *
 *
 */

MotuMatrixMixerMk3::MotuMatrixMixerMk3(MotuDevice &parent)
: Control::MatrixMixer(&parent, "MatrixMixerMk3")
, m_parent(parent)
{
}

MotuMatrixMixerMk3::MotuMatrixMixerMk3(MotuDevice &parent, std::string name)
: Control::MatrixMixer(&parent, name)
, m_parent(parent)
{
}

void MotuMatrixMixerMk3::addRowInfo(std::string name, unsigned int flags,
  unsigned int address)
{
    struct sSignalInfo s;
    s.name = name;
    s.flags = flags;
    s.address = address;
    m_RowInfo.push_back(s);
}

void MotuMatrixMixerMk3::addColInfo(std::string name, unsigned int flags,
  unsigned int address)
{
    struct sSignalInfo s;
    s.name = name;
    s.flags = flags;
    s.address = address;
    m_ColInfo.push_back(s);
}

void MotuMatrixMixerMk3::show()
{
    debugOutput(DEBUG_LEVEL_NORMAL, "MOTU mk3 matrix mixer\n");
}

std::string MotuMatrixMixerMk3::getRowName(const int row)
{
    return m_RowInfo.at(row).name;
}

std::string MotuMatrixMixerMk3::getColName(const int col)
{
    return m_ColInfo.at(col).name;
}

int MotuMatrixMixerMk3::getRowCount()
{
    return m_RowInfo.size();
}

int MotuMatrixMixerMk3::getColCount()
{
    return m_ColInfo.size();
}

ChannelFaderMatrixMixerMk3::ChannelFaderMatrixMixerMk3(MotuDevice &parent, std::string name)
: MotuMatrixMixerMk3(parent, name),
  virtual_fader(parent)
{
}

double ChannelFaderMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    //FIXME: Convert values to a nice scale
    return (this->virtual_fader.setValue(val, row, col));
}

double ChannelFaderMatrixMixerMk3::getValue(const int row, const int col)
{
    //FIXME
    return 0;
}

ChannelPanMatrixMixerMk3::ChannelPanMatrixMixerMk3(MotuDevice &parent, std::string name)
: MotuMatrixMixerMk3(parent, name),
  virtual_fader(parent)
{
}

double ChannelPanMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    return (this->virtual_fader.setValue(val, row, col));
}

double ChannelPanMatrixMixerMk3::getValue(const int row, const int col)
{
    //FIXME
    return 0;
}

/* If no "write enable" is implemented for a given switch it's safe to
 * pass zero in to setenable_mask.
 */
ChannelBinSwMatrixMixerMk3::ChannelBinSwMatrixMixerMk3(MotuDevice &parent, std::string name,
  unsigned int key)
: MotuMatrixMixerMk3(parent, name)
, m_key(key)
{
}

double ChannelBinSwMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    unsigned int m_bus = row;
    unsigned int channel = col + 2; // TravelerMk3 channels have offset of 2... Need to check other models.
    unsigned int value = (unsigned int)val;
    if (this->m_key == MOTU_MK3CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a discrete control with unintialized key\n");
        return true;
    }
    quadlet_t data[2];
    unsigned int serial = m_parent.getMk3MixerSerial();

    //Prepare data:
    data[0] = MOTU_MK3CTRL_DISCRETE_CTRL | (serial << 16) | value;
    data[1] = (m_bus << 24) | this->m_key | (channel << 8);

    //Write data:
    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 2)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) to mixer register\n", data[0], data[1]);
        return false;
    }
    //Everything is OK, let's update serial number for next communication
    m_parent.updateMk3MixerSerial();
    return true;
}

double ChannelBinSwMatrixMixerMk3::getValue(const int row, const int col)
{
    //FIXME
    return 0;
}

}
