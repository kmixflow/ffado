/*
 * Copyright (C) 2007 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
 *
 * FFADO is based upon FreeBoB
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include "efc_cmd.h"
#include "efc_cmds_hardware_ctrl.h"

#include <netinet/in.h>
#include <iostream>

using namespace std;

namespace FireWorks {

EfcGetClockCmd::EfcGetClockCmd()
    : EfcCmd(EFC_CAT_HARDWARE_CONTROL, EFC_CMD_HWCTRL_GET_CLOCK)
    , m_clock ( EFC_CMD_HW_CLOCK_UNSPECIFIED )
    , m_samplerate ( EFC_CMD_HW_CLOCK_UNSPECIFIED )
    , m_index ( 0 )
{
}

bool
EfcGetClockCmd::serialize( Util::IOSSerialize& se )
{
    bool result=true;

    // the length should be specified before
    // the header is serialized
    m_length=EFC_HEADER_LENGTH_QUADLETS;

    result &= EfcCmd::serialize ( se );

    return result;
}

bool
EfcGetClockCmd::deserialize( Util::IISDeserialize& de )
{
    bool result=true;

    result &= EfcCmd::deserialize ( de );

    EFC_DESERIALIZE_AND_SWAP(de, &m_clock, result);
    EFC_DESERIALIZE_AND_SWAP(de, &m_samplerate, result);
    EFC_DESERIALIZE_AND_SWAP(de, &m_index, result);

    return result;
}

void
EfcGetClockCmd::showEfcCmd()
{
    EfcCmd::showEfcCmd();
    debugOutput(DEBUG_LEVEL_NORMAL, "EFC Get Clock:\n");
    debugOutput(DEBUG_LEVEL_NORMAL, " Clock       : %lu\n", m_clock);
    debugOutput(DEBUG_LEVEL_NORMAL, " Samplerate  : %lu\n", m_samplerate);
    debugOutput(DEBUG_LEVEL_NORMAL, " Index       : %lu\n", m_index);
}

// ----
EfcSetClockCmd::EfcSetClockCmd()
    : EfcCmd(EFC_CAT_HARDWARE_CONTROL, EFC_CMD_HWCTRL_SET_CLOCK)
    , m_clock ( EFC_CMD_HW_CLOCK_UNSPECIFIED )
    , m_samplerate ( EFC_CMD_HW_CLOCK_UNSPECIFIED )
    , m_index ( 0 )
{
}

bool
EfcSetClockCmd::serialize( Util::IOSSerialize& se )
{
    bool result=true;

    // the length should be specified before
    // the header is serialized
    m_length=EFC_HEADER_LENGTH_QUADLETS+3;

    result &= EfcCmd::serialize ( se );

    result &= se.write(htonl(m_clock), "Clock" );
    result &= se.write(htonl(m_samplerate), "Samplerate" );
    result &= se.write(htonl(m_index), "Index" );

    return result;
}

bool
EfcSetClockCmd::deserialize( Util::IISDeserialize& de )
{
    bool result=true;

    result &= EfcCmd::deserialize ( de );

    EFC_DESERIALIZE_AND_SWAP(de, &m_clock, result);
    EFC_DESERIALIZE_AND_SWAP(de, &m_samplerate, result);
    EFC_DESERIALIZE_AND_SWAP(de, &m_index, result);

    return result;
}

void
EfcSetClockCmd::showEfcCmd()
{
    EfcCmd::showEfcCmd();
    debugOutput(DEBUG_LEVEL_NORMAL, "EFC Set Clock:\n");
    debugOutput(DEBUG_LEVEL_NORMAL, " Clock       : %lu\n", m_clock);
    debugOutput(DEBUG_LEVEL_NORMAL, " Samplerate  : %lu\n", m_samplerate);
    debugOutput(DEBUG_LEVEL_NORMAL, " Index       : %lu\n", m_index);
}

// ----
EfcPhyReconnectCmd::EfcPhyReconnectCmd()
    : EfcCmd(EFC_CAT_HARDWARE_CONTROL, EFC_CMD_HWCTRL_RECONNECT_PHY)
{
}

bool
EfcPhyReconnectCmd::serialize( Util::IOSSerialize& se )
{
    bool result=true;

    // the length should be specified before
    // the header is serialized
    m_length=EFC_HEADER_LENGTH_QUADLETS;

    result &= EfcCmd::serialize ( se );

    return result;
}

bool
EfcPhyReconnectCmd::deserialize( Util::IISDeserialize& de )
{
    bool result=true;

    result &= EfcCmd::deserialize ( de );

    return result;
}

void
EfcPhyReconnectCmd::showEfcCmd()
{
    EfcCmd::showEfcCmd();
    debugOutput(DEBUG_LEVEL_NORMAL, "EFC Phy Reconnect\n");
}

} // namespace FireWorks