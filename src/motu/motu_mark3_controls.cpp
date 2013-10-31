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
#include "libutil/ByteSwap.h"

namespace Motu {

MotuDiscreteCtrlMk3::MotuDiscreteCtrlMk3(MotuDevice &parent,
        unsigned long int bus, std::string name, std::string label,
        std::string descr) :
        Control::Discrete(&parent), m_parent(parent) {
    setName(name);
    setLabel(label);
    setDescription(descr);
    m_key = MOTU_MK3_KEY_NONE;
    //FIXME: Check if bus is valid
    m_bus = bus;
}

bool MotuDiscreteCtrlMk3::setValue(int value) {
    if (this->m_key == MOTU_MK3_KEY_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a discrete control with unintialized key\n");
        return true;
    }
    //FIXME: This is a hack to avoid the "heartbeat" counting by resetting the serial number
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, MOTU_MK3CTRL_MIXER_RESET0);
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, MOTU_MK3CTRL_MIXER_RESET1);

    quadlet_t data[2];

    //First quadlet:
    data[0] = MOTU_MK3_DISCRETE_CTRL | MOTU_MK3CTRL_SERIAL_NUMBER | value;

    //Second quadlet:
    data[1] = this->m_bus | this->m_key;

    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 2)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) to Mark3 mixer register\n", data[0], data[1], MOTU_G3_REG_MIXER);
        return false;
    }
    return true;
}

int MotuDiscreteCtrlMk3::getValue() {
    //TODO: Try to figure out how to request control values from Mk3 Devices
    return 0;
}

MixDestMk3::MixDestMk3(MotuDevice &parent, unsigned long int bus,
        std::string name, std::string label, std::string descr) :
        MotuDiscreteCtrlMk3(parent, bus, name, label, descr) {
    this->m_key = MOTU_MK3_MIX_DEST_ASSIGN_CTRL;
}

bool MixDestMk3::setValue(int value) {
    //FIXME: Implement mixer parsing from motu_mark3_mixerdefs.cpp
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
        debugOutput(DEBUG_LEVEL_WARNING, "MixDestMk3 value %d not implemented\n", value);
        break;
    }
    return MotuDiscreteCtrlMk3::setValue(dest);
}

int MixDestMk3::getValue() {
    return MotuDiscreteCtrlMk3::getValue();
}

MotuContinuousCtrlMk3::MotuContinuousCtrlMk3(MotuDevice &parent,
        unsigned long int bus, std::string name, std::string label,
        std::string descr) :
        Control::Continuous(&parent), m_parent(parent) {
    setName(name);
    setLabel(label);
    setDescription(descr);
    m_key = MOTU_MK3_KEY_NONE;
    m_minimum = MOTU_MK3_VALUE_NONE;
    m_maximum = MOTU_MK3_VALUE_NONE;
    //FIXME: Check if bus is valid
    m_bus = bus;
}

bool MotuContinuousCtrlMk3::setValue(double value) {
    unsigned int val;
    quadlet_t data[3];

    val = (unsigned int)value;

    if (this->m_key == MOTU_MK3_KEY_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "Trying to set a continuous control with unintialized control key\n");
        return true;
    }
    if (val > this->m_maximum) {
        val = m_maximum;
    } else if (val < this->m_minimum) {
        val = m_minimum;
    }
    //FIXME: This is a hack to avoid the "heartbeat" counting by resetting the serial number
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, MOTU_MK3CTRL_MIXER_RESET0);
    m_parent.WriteRegister(MOTU_G3_REG_MIXER, MOTU_MK3CTRL_MIXER_RESET1);

    //Continuous values sent must be big-endian:
    val=CondSwapFromBus32(val);

    //First quadlet:
    data[0] = MOTU_MK3_CONTINUOUS_CTRL | MOTU_MK3CTRL_SERIAL_NUMBER | this->m_bus;

    //Second quadlet:
    data[1] = this->m_key | ((val >> 24) & 0xff);

    //Third quadlet:
    data[2] = (val << 8);

    if (m_parent.writeBlock(MOTU_G3_REG_MIXER, data, 3)) {
        debugOutput(DEBUG_LEVEL_WARNING, "Error writing data[0]=(0x%08x) data[1]=(0x%08x) data[2]=(0x%08x) to Mark3 mixer register\n", data[0], data[1], data[2], MOTU_G3_REG_MIXER);
        return false;
    }
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
    //FIXME: It is necessary to do
    return MotuContinuousCtrlMk3::setValue(value);
}

double MixFaderMk3::getValue()
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

uint32_t MotuMatrixMixerMk3::getCellRegister(const unsigned int row, const unsigned int col)
{
    if (m_RowInfo.at(row).address==MOTU_CTRL_NONE ||
        m_ColInfo.at(col).address==MOTU_CTRL_NONE)
        return MOTU_CTRL_NONE;
    return m_RowInfo.at(row).address + m_ColInfo.at(col).address;
}

void MotuMatrixMixerMk3::show()
{
    debugOutput(DEBUG_LEVEL_NORMAL, "MOTU matrix mixer\n");
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

ChannelFaderMatrixMixerMk3::ChannelFaderMatrixMixerMk3(MotuDevice &parent)
: MotuMatrixMixerMk3(parent, "ChannelFaderMatrixMixerMk3")
{
}

ChannelFaderMatrixMixerMk3::ChannelFaderMatrixMixerMk3(MotuDevice &parent, std::string name)
: MotuMatrixMixerMk3(parent, name)
{
}

double ChannelFaderMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    uint32_t v, reg;
    v = val<0?0:(uint32_t)val;
    if (v > 0x80)
      v = 0x80;
    debugOutput(DEBUG_LEVEL_VERBOSE, "ChannelFader setValue for row %d col %d to %lf (%d)\n",
      row, col, val, v);
    reg = getCellRegister(row,col);

    // Silently swallow attempts to set non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return true;
    }
    // Bit 30 indicates that the channel fader is being set
    v |= 0x40000000;
    m_parent.WriteRegister(reg, v);

    return true;
}

double ChannelFaderMatrixMixerMk3::getValue(const int row, const int col)
{
    uint32_t val, reg;
    reg = getCellRegister(row,col);

    // Silently swallow attempts to read non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return 0;
    }
    // FIXME: we could just read the appropriate mixer status field from the
    // receive stream processor once we work out an efficient way to do this.
    val = m_parent.ReadRegister(reg) & 0xff;

    debugOutput(DEBUG_LEVEL_VERBOSE, "ChannelFader getValue for row %d col %d = %u\n",
      row, col, val);
    return val;
}

ChannelPanMatrixMixerMk3::ChannelPanMatrixMixerMk3(MotuDevice &parent)
: MotuMatrixMixerMk3(parent, "ChannelPanMatrixMixerMk3")
{
}

ChannelPanMatrixMixerMk3::ChannelPanMatrixMixerMk3(MotuDevice &parent, std::string name)
: MotuMatrixMixerMk3(parent, name)
{
}

double ChannelPanMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    uint32_t v, reg;
    v = ((val<-64?-64:(int32_t)val)+64) & 0xff;
    if (v > 0x80)
      v = 0x80;

    debugOutput(DEBUG_LEVEL_VERBOSE, "ChannelPan setValue for row %d col %d to %lf (%d)\n",
      row, col, val, v);
    reg = getCellRegister(row,col);

    // Silently swallow attempts to set non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return true;
    }

    // Bit 31 indicates that pan is being set
    v = (v << 8) | 0x80000000;
    m_parent.WriteRegister(reg, v);

    return true;
}

double ChannelPanMatrixMixerMk3::getValue(const int row, const int col)
{
    int32_t val;
    uint32_t reg;
    reg = getCellRegister(row,col);

    // Silently swallow attempts to read non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return 0;
    }

    // FIXME: we could just read the appropriate mixer status field from the
    // receive stream processor once we work out an efficient way to do this.
    val = m_parent.ReadRegister(reg);
    val = ((val >> 8) & 0xff) - 0x40;

    debugOutput(DEBUG_LEVEL_VERBOSE, "ChannelPan getValue for row %d col %d = %u\n",
      row, col, val);
    return val;
}

ChannelBinSwMatrixMixerMk3::ChannelBinSwMatrixMixerMk3(MotuDevice &parent)
: MotuMatrixMixerMk3(parent, "ChannelPanMatrixMixerMk3")
, m_value_mask(0)
, m_setenable_mask(0)
{
}

/* If no "write enable" is implemented for a given switch it's safe to
 * pass zero in to setenable_mask.
 */
ChannelBinSwMatrixMixerMk3::ChannelBinSwMatrixMixerMk3(MotuDevice &parent, std::string name,
  unsigned int val_mask, unsigned int setenable_mask)
: MotuMatrixMixerMk3(parent, name)
, m_value_mask(val_mask)
, m_setenable_mask(setenable_mask)
{
}

double ChannelBinSwMatrixMixerMk3::setValue(const int row, const int col, const double val)
{
    uint32_t v, reg;

    debugOutput(DEBUG_LEVEL_VERBOSE, "BinSw setValue for row %d col %d to %lf (%d)\n",
      row, col, val, val==0?0:1);
    reg = getCellRegister(row,col);

    // Silently swallow attempts to set non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return true;
    }

    // Set the value
    if (m_setenable_mask) {
      v = (val==0)?0:m_value_mask;
      // Set the "write enable" bit for the value being set
      v |= m_setenable_mask;
    } else {
      // It would be good to utilise the cached value from the receive
      // processor (if running) later on.  For now we'll just fetch the
      // current register value directly when needed.
      v = m_parent.ReadRegister(reg);
      if (v==0)
        v &= ~m_value_mask;
      else
        v |= m_value_mask;
    }
    m_parent.WriteRegister(reg, v);

    return true;
}

double ChannelBinSwMatrixMixerMk3::getValue(const int row, const int col)
{
    uint32_t val, reg;
    reg = getCellRegister(row,col);

    // Silently swallow attempts to read non-existent controls for now
    if (reg == MOTU_CTRL_NONE) {
        debugOutput(DEBUG_LEVEL_VERBOSE, "ignoring control marked as non-existent\n");
        return 0;
    }

    // FIXME: we could just read the appropriate mixer status field from the
    // receive stream processor once we work out an efficient way to do this.
    val = m_parent.ReadRegister(reg);
    val = (val & m_value_mask) != 0;

    debugOutput(DEBUG_LEVEL_VERBOSE, "BinSw getValue for row %d col %d = %u\n",
      row, col, val);
    return val;
}

}
