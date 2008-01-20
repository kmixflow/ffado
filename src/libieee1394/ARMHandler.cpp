/*
 * Copyright (C) 2005-2008 by Pieter Palmers
 *
 * This file is part of FFADO
 * FFADO = Free Firewire (pro-)audio drivers for linux
 *
 * FFADO is based upon FreeBoB
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

#include "ARMHandler.h"

IMPL_DEBUG_MODULE( ARMHandler, ARMHandler, DEBUG_LEVEL_VERBOSE);
/**
 * @param start          identifies addressrange
 * @param length         identifies addressrange length (in bytes)
 * @param initial_value  pointer to buffer containing (if necessary) initial value
 *                    NULL means undefined
 * @param access_rights access-rights for registered addressrange handled
 *                    by kernel-part. Value is one or more binary or of the
 *                    following flags - ARM_READ, ARM_WRITE, ARM_LOCK
 * @param notification_options identifies for which type of request you want
 *                    to be notified. Value is one or more binary or of the
 *                    following flags - ARM_READ, ARM_WRITE, ARM_LOCK
 * @param client_transactions identifies for which type of request you want
 *                    to handle the request by the client application.
 *                    for those requests no response will be generated, but
 *                    has to be generated by the application.
 *                    Value is one or more binary or of the
 *                    following flags - ARM_READ, ARM_WRITE, ARM_LOCK
 *                    For each bit set here, notification_options and
 *                    access_rights will be ignored.
 *
 */
ARMHandler::ARMHandler(nodeaddr_t start, size_t length,
               unsigned int access_rights,
               unsigned int notification_options,
               unsigned int client_transactions
              )
        : m_start(start),
        m_length(length),
        m_access_rights(access_rights),
        m_notification_options(notification_options),
        m_client_transactions(client_transactions),
        m_buffer(0)
{
    m_buffer=(byte_t*)calloc(length, sizeof(byte_t));
    memset(&m_response,0,sizeof(m_response));
}

ARMHandler::~ARMHandler() {
    if(m_buffer)
        delete m_buffer;
}

bool ARMHandler::handleRead(struct raw1394_arm_request *req) {
    debugOutput(DEBUG_LEVEL_VERBOSE,"Read\n");
    printRequest(req);
    return true;
}

bool ARMHandler::handleWrite(struct raw1394_arm_request *req) {
    debugOutput(DEBUG_LEVEL_VERBOSE,"Write\n");
    printRequest(req);
    return true;
}

bool ARMHandler::handleLock(struct raw1394_arm_request *req) {
    debugOutput(DEBUG_LEVEL_VERBOSE,"Lock\n");
    printRequest(req);
    return true;
}

// typedef struct raw1394_arm_request {
//         nodeid_t        destination_nodeid;
//         nodeid_t        source_nodeid;
//         nodeaddr_t      destination_offset;
//         u_int8_t        tlabel;
//         u_int8_t        tcode;
//         u_int8_t        extended_transaction_code;
//         u_int32_t       generation;
//         arm_length_t    buffer_length;
//         byte_t          *buffer;
// } *raw1394_arm_request_t;
//
// typedef struct raw1394_arm_response {
//         int             response_code;
//         arm_length_t    buffer_length;
//         byte_t          *buffer;
// } *raw1394_arm_response_t;
//
// typedef struct raw1394_arm_request_response {
//         struct raw1394_arm_request  *request;
//         struct raw1394_arm_response *response;
// } *raw1394_arm_request_response_t;

void ARMHandler::printRequest(struct raw1394_arm_request *arm_req) {
    debugOutput(DEBUG_LEVEL_VERBOSE," request info: \n");
    debugOutput(DEBUG_LEVEL_VERBOSE,"  from node 0x%04X to node 0x%04X\n",
        arm_req->source_nodeid, arm_req->destination_nodeid);
    debugOutput(DEBUG_LEVEL_VERBOSE,"  tlabel: 0x%02X, tcode: 0x%02X, extended tcode: 0x%02X\n",
        arm_req->tlabel, arm_req->tcode, arm_req->extended_transaction_code);
    debugOutput(DEBUG_LEVEL_VERBOSE,"  generation: %lu\n",
        arm_req->generation);
    debugOutput(DEBUG_LEVEL_VERBOSE,"  buffer length: %lu\n",
        arm_req->buffer_length);
    printBufferBytes(DEBUG_LEVEL_VERBOSE, arm_req->buffer_length, arm_req->buffer);
}

void
ARMHandler::printBufferBytes( unsigned int level, size_t length, byte_t* buffer ) const
{

    for ( unsigned int i=0; i < length; ++i ) {
        if ( ( i % 16 ) == 0 ) {
            if ( i > 0 ) {
                debugOutputShort(level,"\n");
            }
            debugOutputShort(level," %4d: ",i*16);
        }
        debugOutputShort(level,"%02X ",buffer[i]);
    }
    debugOutputShort(level,"\n");
}
