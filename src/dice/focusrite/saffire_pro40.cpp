/*
 * Copyright (C) 2009 by Pieter Palmers
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

#include "saffire_pro40.h"

#include "focusrite_eap.h"

namespace Dice {
namespace Focusrite {

int SaffirePro40::SaffirePro40EAP::commandToFix(unsigned offset) {
    if (offset<0x14) return 2;
    if (offset<0x3C && offset>=0x14) return 1;
    if (offset<0x5C && offset>=0x54) return 1;
    if (offset<0x44 && offset>=0x3C) return 3;
    if (offset == 0x5C) return 4;
    return 0;
}
FocusriteEAP::Poti* SaffirePro40::SaffirePro40EAP::getMonitorPoti(std::string name) {
    return new FocusriteEAP::Poti(this, name, 0x54);
}
FocusriteEAP::Poti* SaffirePro40::SaffirePro40EAP::getDimPoti(std::string name) {
    return new FocusriteEAP::Poti(this, name, 0x58);
}

//
// Under 48kHz Saffire pro 40 has
//  - 8 analogic inputs (mic/line)
//  - 8 ADAT inputs
//  - 2 SPDIF inputs
//  - 20 ieee1394 inputs
//  - 18 mixer inputs
//
//  - 10 analogic outputs
//  - 8 ADAT outputs
//  - 2 SPDIF outputs
//  - 20 ieee1394 outputs
//  - 16 mixer outputs
//
void SaffirePro40::SaffirePro40EAP::setupSources_low() {
    addSource("SPDIF",  0,  2, eRS_AES);
    addSource("ADAT",   0,  8, eRS_ADAT);
    addSource("Analog", 16,  8, eRS_InS0);
    addSource("Mixer",  0, 16, eRS_Mixer);
    addSource("1394",   0, 12, eRS_ARX0);
    addSource("1394",   0, 8, eRS_ARX1, 12);
    addSource("Mute",   0,  1, eRS_Muted);
}

void SaffirePro40::SaffirePro40EAP::setupDestinations_low() {
    addDestination("SPDIF",  0,  2, eRD_AES);
    addDestination("ADAT",   0,  8, eRD_ADAT);
    addDestination("Analog", 0,  2, eRD_InS0);
    addDestination("Analog", 0,  8, eRD_InS1, 2);
    addDestination("Mixer",  0, 16, eRD_Mixer0);
    addDestination("Mixer",  0,  2, eRD_Mixer1, 16);
    addDestination("1394",   0, 10, eRD_ATX0);
    addDestination("1394",   0,  8, eRD_ATX1, 10);
    addDestination("Loop",   8,  2, eRD_ATX1);
    addDestination("Mute",   0,  1, eRD_Muted);
}

//
// Under 96kHz Saffire pro 40 has
//  - 8 analogic inputs (mic/line)
//  - 4 ADAT inputs
//  - 2 SPDIF inputs
//  - 16 ieee1394 inputs
//  - 18 mixer inputs
//
//  - 10 analogic outputs
//  - 4 ADAT outputs
//  - 2 SPDIF outputs
//  - 16 ieee1394 outputs
//  - 16 mixer outputs
//
void SaffirePro40::SaffirePro40EAP::setupSources_mid() {
      addSource("SPDIF",  0,  2, eRS_AES);
      addSource("ADAT",   0,  4, eRS_ADAT);
      addSource("Analog", 16,  8, eRS_InS0);
      addSource("Mixer",  0, 16, eRS_Mixer);
      addSource("1394",   0, 16, eRS_ARX0);
      addSource("Mute",   0,  1, eRS_Muted);
}

void SaffirePro40::SaffirePro40EAP::setupDestinations_mid() {
    addDestination("SPDIF",  0,  2, eRD_AES);
    addDestination("ADAT",   0,  4, eRD_ADAT);
    addDestination("Analog", 0,  2, eRD_InS0);
    addDestination("Analog", 0,  8, eRD_InS1, 2);
    addDestination("Mixer",  0, 16, eRD_Mixer0);
    addDestination("Mixer",  0,  2, eRD_Mixer1, 16);
    addDestination("1394",   0, 14, eRD_ATX0);
    addDestination("Loop",   14, 2, eRD_ATX0);
    addDestination("Mute",   0,  1, eRD_Muted);
}

//
// 192 kHz is not supported
//
void SaffirePro40::SaffirePro40EAP::setupSources_high() {
    printMessage("High (192 kHz) sample rate not supported by Saffire Pro 40\n");
}

void SaffirePro40::SaffirePro40EAP::setupDestinations_high() {
    printMessage("High (192 kHz) sample rate not supported by Saffire Pro 40\n");
}

SaffirePro40::SaffirePro40( DeviceManager& d,
                                        std::auto_ptr<ConfigRom>( configRom ))
    : Dice::Device( d , configRom)
{
    debugOutput( DEBUG_LEVEL_VERBOSE, "Created Dice::Focusrite::SaffirePro40 (NodeID %d)\n",
                 getConfigRom().getNodeId() );
}

SaffirePro40::~SaffirePro40()
{
    getEAP()->storeFlashConfig();
}

bool SaffirePro40::discover() {
    if (Dice::Device::discover()) {
        m_monitor = new FocusriteEAP::MonitorSection(dynamic_cast<FocusriteEAP*>(getEAP()), "Monitoring");
        getEAP()->addElement(m_monitor);
        return true;
    }
    return false;
}

void
SaffirePro40::showDevice()
{
    debugOutput(DEBUG_LEVEL_VERBOSE, "This is a Dice::Focusrite::SaffirePro40\n");
    Dice::Device::showDevice();
}

Dice::EAP* SaffirePro40::createEAP() {
    return new SaffirePro40EAP(*this);
}

bool SaffirePro40::setNickname(std::string name) {
    return getEAP()->writeRegBlock(Dice::EAP::eRT_Application, 0x44, (quadlet_t*)name.c_str(), name.size());
}
std::string SaffirePro40::getNickname() {
    char name[16];
    getEAP()->readRegBlock(Dice::EAP::eRT_Application, 0x44, (quadlet_t*)name, 16);
    return std::string(name);
}

}
}
// vim: et
