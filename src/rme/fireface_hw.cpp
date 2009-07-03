/*
 * Copyright (C) 2009 by Jonathan Woithe
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

/* This file implements miscellaneous lower-level hardware functions for the Fireface */

#include "rme/rme_avdevice.h"
#include "rme/fireface_def.h"

#include "debugmodule/debugmodule.h"

namespace Rme {

signed int
Device::init_hardware(void)
{
    // Initialises the device's settings structure to a known state and then
    // sets the hardware to reflect this state.
    //
    // In time this function may read a cached device setup and initialise
    // based on that.  It may also read the device configuration from the
    // device flash and adopt that.  For now (for initial testing purposes)
    // we'll go with a static state.
    memset(&settings, 0, sizeof(settings));
    settings.spdif_input_mode = FF_SWPARAM_SPDIF_INPUT_COAX;
    settings.spdif_output_mode = FF_SWPARAM_SPDIF_OUTPUT_COAX;
    settings.clock_mode = FF_SWPARAM_SPDIF_CLOCK_MODE_MASTER;
    settings.sync_ref = FF_SWPARAM_SYNCREF_WORDCLOCK;
    settings.input_level = FF_SWPARAM_ILEVEL_LOGAIN;
    settings.output_level = FF_SWPARAM_OLEVEL_HIGAIN;

    return set_hardware_params(&settings);
}

signed int 
Device::set_hardware_params(FF_software_settings_t *sw_settings)
{
    // Initialises the hardware to the state defined by the supplied
    // software settings structure (which will usually be the device's
    // "settings" structure).  This has the side effect of extinguishing the
    // "Host" LED on the FF400 when done for the first time after the
    // interface has been powered up.

    quadlet_t data[3] = {0, 0, 0};
    unsigned int conf_reg;

    if (sw_settings->mic_phantom[0])
      data[0] |= CR0_PHANTOM_MIC0;
    if (sw_settings->mic_phantom[1])
      data[0] |= CR0_PHANTOM_MIC1;
    if (sw_settings->mic_phantom[2])
      data[0] |= CR0_PHANTOM_MIC2;
    if (sw_settings->mic_phantom[3])
      data[0] |= CR0_PHANTOM_MIC3;

    /* Phones level */
    switch (sw_settings->phones_level) {
        case FF_SWPARAM_PHONESLEVEL_HIGAIN:
            data[0] |= CRO_PHLEVEL_HIGAIN;
            break;
        case FF_SWPARAM_PHONESLEVEL_4dBU:
            data[0] |= CR0_PHLEVEL_4dBU;
            break;
        case FF_SWPARAM_PHONESLEVEL_m10dBV:
            data[0] |= CRO_PHLEVEL_m10dBV;
            break;
    }

    /* Input level */
    switch (sw_settings->input_level) {
        case FF_SWPARAM_ILEVEL_LOGAIN: // Low gain
            data[1] |= CR1_ILEVEL_CPLD_LOGAIN;    // CPLD
            data[0] |= CR0_ILEVEL_FPGA_LOGAIN;    // LED control (used on FF800 only)
            break;
        case FF_SWPARAM_ILEVEL_4dBU:   // +4 dBu
            data[1] |= CR1_ILEVEL_CPLD_4dBU;
            data[0] |= CR0_ILEVEL_FPGA_4dBU;
            break;
        case FF_SWPARAM_ILEVEL_m10dBV: // -10 dBV
            data[1] |= CR1_ILEVEL_CPLD_m10dBV;
            data[0] |= CR0_ILEVEL_FPGA_m10dBV;
            break;
    }

    /* Output level */
    switch (sw_settings->output_level) {
        case FF_SWPARAM_OLEVEL_HIGAIN: // High gain
            data[1] |= CR1_OLEVEL_CPLD_HIGAIN;   // CPLD
            data[0] |= CR0_OLEVEL_FPGA_HIGAIN;   // LED control (used on FF800 only)
            break;
        case FF_SWPARAM_OLEVEL_4dBU:   // +4 dBu
            data[1] |= CR1_OLEVEL_CPLD_4dBU;
            data[0] |= CR0_OLEVEL_FPGA_4dBU;
            break;
        case FF_SWPARAM_OLEVEL_m10dBV: // -10 dBV
            data[1] |= CR1_OLEVEL_CPLD_m10dBV;
            data[0] |= CR0_OLEVEL_FPGA_m10dBV;
            break;
    }

    /* Set input options.  The meaning of the options differs between
     * devices, so we use the generic identifiers here.
     */
    data[1] |= (sw_settings->input_opt[1] & FF_SWPARAM_INPUT_OPT_A) ? CR1_INPUT_OPT1_A : 0;
    data[1] |= (sw_settings->input_opt[1] & FF_SWPARAM_INPUT_OPT_B) ? CR1_INPUT_OPT1_B : 0;
    data[1] |= (sw_settings->input_opt[2] & FF_SWPARAM_INPUT_OPT_A) ? CR1_INPUT_OPT2_A : 0;
    data[1] |= (sw_settings->input_opt[2] & FF_SWPARAM_INPUT_OPT_B) ? CR1_INPUT_OPT2_B : 0;

    // Drive the speaker emulation / filter LED via FPGA
    data[0] |= (sw_settings->filter) ? CR0_FILTER_FPGA : 0;

    // Set the "rear" option for input 0 if selected
    data[1] |= (sw_settings->input_opt[0] & FF_SWPARAM_FF800_INPUT_OPT_REAR) ? CR1_FF800_INPUT1_REAR : 0;

    // The input 0 "front" option is activated using one of two bits
    // depending on whether the filter (aka "speaker emulation") setting is
    // active.
    if (sw_settings->input_opt[0] & FF_SWPARAM_FF800_INPUT_OPT_FRONT) {
        data[1] |= (sw_settings->filter) ? CR1_FF800_INPUT1_FRONT_WITH_FILTER : CR1_FF800_INPUT1_FRONT;
    }

    data[2] |= (sw_settings->spdif_output_emphasis==FF_SWPARAM_SPDIF_OUTPUT_EMPHASIS_ON) ? CR2_SPDIF_OUT_EMP : 0;
    data[2] |= (sw_settings->spdif_output_pro==FF_SWPARAM_SPDIF_OUTPUT_PRO_ON) ? CR2_SPDIF_OUT_PRO : 0;
    data[2] |= (sw_settings->spdif_output_nonaudio==FF_SWPARAM_SPDIF_OUTPUT_NONAUDIO_ON) ? CR2_SPDIF_OUT_NONAUDIO : 0;
    data[2] |= (sw_settings->spdif_output_mode==FF_SWPARAM_SPDIF_OUTPUT_OPTICAL) ? CR2_SPDIF_OUT_ADAT2 : 0;
    data[2] |= (sw_settings->clock_mode==FF_SWPARAM_SPDIF_CLOCK_MODE_AUTOSYNC) ? CR2_CLOCKMODE_AUTOSYNC : CR2_CLOCKMODE_MASTER;
    data[2] |= (sw_settings->spdif_input_mode==FF_SWPARAM_SPDIF_INPUT_COAX) ? CR2_SPDIF_IN_COAX : CR2_SPDIF_IN_ADAT2;
    data[2] |= (sw_settings->word_clock_single_speed=FF_SWPARAM_WORD_CLOCK_1x) ? CR2_WORD_CLOCK_1x : 0;

    /* TMS / TCO toggle bits in CR2 are not set by other drivers */

    /* Drive / fuzz */
    if (sw_settings->fuzz)
      data[0] |= CR0_INSTR_DRIVE_FPGA; // FPGA LED control
    else
      data[1] |= CR1_INSTR_DRIVE;      // CPLD

    /* Drop-and-stop is hardwired on in other drivers */
    data[2] |= CR2_DROP_AND_STOP;

    if (m_rme_model == RME_MODEL_FIREFACE400) {
        data[2] |= CR2_FF400_BIT;
    }

    switch (sw_settings->sync_ref) {
        case FF_SWPARAM_SYNCREF_WORDCLOCK:
            data[2] |= CR2_SYNC_WORDCLOCK;
            break;
        case FF_SWPARAM_SYNCREF_ADAT1:
            data[2] |= CR2_SYNC_ADAT1;
            break;
        case FF_SWPARAM_SYNCREF_ADAT2:
            data[2] |= CR2_SYNC_ADAT2;
            break;
        case FF_SWPARAM_SYNCREF_SPDIF:
            data[2] |= CR2_SYNC_SPDIF;
            break;
        case FF_SWPARAM_SYNCREC_TCO:
            data[2] |= CR2_SYNC_TCO;
            break;
    }

    // This is hardwired in other drivers
    data[2] |= (CR2_FREQ0 + CR2_FREQ1 + CR2_DSPEED + CR2_QSSPEED);

    // The FF800 limiter applies to the front panel instrument input, so it
    // only makes sense that it is disabled when that input is in use.
    data[2] |= (sw_settings->limiter_disable && 
                (sw_settings->input_opt[0] & FF_SWPARAM_FF800_INPUT_OPT_FRONT)) ? 
                CR2_DISABLE_LIMITER : 0;

//This is just for testing - it's a known consistent configuration
//data[0] = 0x00020811;      // Phantom off
data[0] = 0x00020811;      // Phantom on
data[1] = 0x0000031e;
data[2] = 0xc400101f;

    conf_reg = (m_rme_model==RME_MODEL_FIREFACE800)?RME_FF800_CONF_REG:RME_FF400_CONF_REG;
    if (writeBlock(conf_reg, data, 3) != 0)
        return -1;

    return -0;
}

signed int 
Device::read_tco(quadlet_t *tco_data, signed int size)
{
    // Read the TCO registers and return the respective values in *tco_data. 
    // Return value is 0 on success, or -1 if there is no TCO present. 
    // "size" is the size (in quadlets) of the array pointed to by tco_data. 
    // To obtain all TCO data "size" should be at least 4.  If the caller
    // doesn't care about the data returned by the TCO, tco_data can be
    // NULL.
    quadlet_t buf[4];
    signed int i;

    // The Fireface 400 can't have the TCO fitted
    if (m_rme_model==RME_MODEL_FIREFACE400)
        return -1;

    if (readBlock(RME_FF_TCO_READ_REG, buf, 4) != 0)
        return -1;

    if (tco_data != NULL) {
        for (i=0; i<(size<4)?size:4; i++)
            tco_data[i] = buf[i];
    }

    if ( (buf[0] & 0x80808080) == 0x80808080 &&
         (buf[1] & 0x80808080) == 0x80808080 &&
         (buf[2] & 0x80808080) == 0x80808080 &&
         (buf[3] & 0x8000FFFF) == 0x80008000) {
        // A TCO is present
        return 0;
    }

    return -1;
}

signed int 
Device::write_tco(quadlet_t *tco_data, signed int size)
{
    // Writes data to the TCO.  No check is made as to whether a TCO is
    // present in the current device.  Return value is 0 on success or -1 on
    // error.  "size" is the size (in quadlets) of the data pointed to by
    // "tco_data".  The first 4 quadlets of tco_data are significant; all
    // others are ignored.  If fewer than 4 quadlets are supplied (as
    // indicated by the "size" parameter, -1 will be returned.
    if (size < 4)
        return -1;

    // Don't bother trying to write if the device is a FF400 since the TCO
    // can't be fitted to this device.
    if (m_rme_model==RME_MODEL_FIREFACE400)
        return -1;

    if (writeBlock(RME_FF_TCO_WRITE_REG, tco_data, 4) != 0)
        return -1;

    return 0;
}

signed int 
Device::read_tco_state(FF_TCO_state_t *tco_state)
{
    // Reads the current TCO state into the supplied state structure

    quadlet_t tc[4];
    unsigned int PLL_phase;

    if (read_tco(tc, 4) != 0)
      return -1;

    // The timecode is stored in BCD (binary coded decimal) in register 0.
    tco_state->frames = (tc[0] & 0xf) + ((tc[0] & 0x30) >> 4)*10;
    tco_state->seconds = ((tc[0] & 0xf00) >> 8) + ((tc[0] & 0x7000) >> 12)*10;
    tco_state->minutes = ((tc[0] & 0xf0000) >> 16) + ((tc[0] & 0x700000) >> 20)*10;
    tco_state->hours = ((tc[0] & 0xf000000) >> 24) + ((tc[0] & 0x30000000) >> 28)*10;

    tco_state->locked = (tc[1] & FF_TCO1_TCO_lock) != 0;
    tco_state->ltc_valid = (tc[1] & FF_TCO1_LTC_INPUT_VALID) != 0;

    switch (tc[1] & FF_TCO1_LTC_FORMAT_MASK) {
        case FF_TC01_LTC_FORMAT_24fps: 
          tco_state->frame_rate = FF_TCOSTATE_FRAMERATE_24fps; break;
        case FF_TCO1_LTC_FORMAT_25fps: 
          tco_state->frame_rate = FF_TCOSTATE_FRAMERATE_25fps; break;
        case FF_TC01_LTC_FORMAT_29_97fps: 
          tco_state->frame_rate = FF_TCOSTATE_FRAMERATE_29_97fps; break;
        case FF_TCO1_LTC_FORMAT_30fps: 
          tco_state->frame_rate = FF_TCOSTATE_FRAMERATE_30fps; break;
    }

    tco_state->drop_frame = (tc[1] & FF_TCO1_SET_DROPFRAME) != 0;

    switch (tc[1] & FF_TCO1_VIDEO_INPUT_MASK) {
        case FF_TCO1_VIDEO_INPUT_NTSC:
            tco_state->video_input = FF_TCOSTATE_VIDEO_NTSC; break;
        case FF_TCO1_VIDEO_INPUT_PAL:
            tco_state->video_input = FF_TCOSTATE_VIDEO_PAL; break;
        default:
            tco_state->video_input = FF_TCOSTATE_VIDEO_NONE;
    }

    if ((tc[1] & FF_TCO1_WORD_CLOCK_INPUT_VALID) == 0) {
        tco_state->word_clock_state = FF_TCOSTATE_WORDCLOCK_NONE;
    } else {
        switch (tc[1] & FF_TCO1_WORD_CLOCK_INPUT_MASK) {
            case FF_TCO1_WORD_CLOCK_INPUT_1x:
                tco_state->word_clock_state = FF_TCOSTATE_WORDCLOCK_1x; break;
            case FF_TCO1_WORD_CLOCK_INPUT_2x:
                tco_state->word_clock_state = FF_TCOSTATE_WORDCLOCK_2x; break;
            case FF_TCO1_WORD_CLOCK_INPUT_4x:
                tco_state->word_clock_state = FF_TCOSTATE_WORDCLOCK_4x; break;
        }
    }

    PLL_phase = (tc[2] & 0x7f) + ((tc[2] & 0x7f00) >> 1);
    tco_state->sample_rate = (25000000.0 * 16.0)/PLL_phase;

    return 0;
}

signed int 
Device::write_tco_settings(FF_TCO_settings_t *tco_settings)
{
    // Writes the supplied application-level settings to the device's TCO
    // (Time Code Option).  Don't bother doing anything if the device doesn't
    // have a TCO fitted.  Returns 0 on success, -1 on error.

    quadlet_t tc[4] = {0, 0, 0, 0};

    if (!tco_present) {
        return -1;
    }

    if (tco_settings->MTC)
        tc[0] |= FF_TCO0_MTC;

    switch (tco_settings->input) {
        case FF_TCOPARAM_INPUT_LTC:
            tc[2] |= FF_TCO2_INPUT_LTC; break;
        case FF_TCOPARAM_INPUT_VIDEO:
            tc[2] |= FF_TCO2_INPUT_VIDEO; break;
        case FF_TCOPARAM_INPUT_WCK:
            tc[2] |= FF_TCO2_INPUT_WORD_CLOCK; break;
    }

    switch (tco_settings->frame_rate) {
        case FF_TCOPARAM_FRAMERATE_24fps:
            tc[1] |= FF_TC01_LTC_FORMAT_24fps; break;
        case FF_TCOPARAM_FRAMERATE_25fps:
            tc[1] |= FF_TCO1_LTC_FORMAT_25fps; break;
        case FF_TCOPARAM_FRAMERATE_29_97fps:
            tc[1] |= FF_TC01_LTC_FORMAT_29_97fps; break;
        case FF_TCOPARAM_FRAMERATE_29_97dfps:
            tc[1] |= FF_TCO1_LTC_FORMAT_29_97dpfs; break;
        case FF_TCOPARAM_FRAMERATE_30fps:
            tc[1] |= FF_TCO1_LTC_FORMAT_30fps; break;
        case FF_TCOPARAM_FRAMERATE_30dfps:
            tc[1] |= FF_TCO1_LTC_FORMAT_30dfps; break;
    }

    switch (tco_settings->word_clock) {
        case FF_TCOPARAM_WORD_CLOCK_CONV_1_1:
            tc[2] |= FF_TCO2_WORD_CLOCK_CONV_1_1; break;
        case FF_TCOPARAM_WORD_CLOCK_CONV_44_48:
            tc[2] |= FF_TCO2_WORD_CLOCK_CONV_44_48; break;
        case FF_TCOPARAM_WORD_CLOCK_CONV_48_44:
            tc[2] |= FF_TCO2_WORD_CLOCK_CONV_48_44; break;
    }

    switch (tco_settings->sample_rate) {
        case FF_TCOPARAM_SRATE_44_1:
            tc[2] |= FF_TCO2_SRATE_44_1; break;
        case FF_TCOPARAM_SRATE_48:
            tc[2] |= FF_TCO2_SRATE_48; break;
        case FF_TCOPARAM_SRATE_FROM_APP:
            tc[2] |= FF_TCO2_SRATE_FROM_APP; break;
    }

    switch (tco_settings->pull) {
        case FF_TCPPARAM_PULL_NONE:
            tc[2] |= FF_TCO2_PULL_0; break;
        case FF_TCOPARAM_PULL_UP_01:
            tc[2] |= FF_TCO2_PULL_UP_01; break;
        case FF_TCOPARAM_PULL_DOWN_01:
            tc[2] |= FF_TCO2_PULL_DOWN_01; break;
        case FF_TCOPARAM_PULL_UP_40:
            tc[2] |= FF_TCO2_PULL_UP_40; break;
        case FF_TCOPARAM_PULL_DOWN_40:
            tc[2] |= FF_TCO2_PULL_DOWN_40; break;
    }

    if (tco_settings->termination == FF_TCOPARAM_TERMINATION_ON)
        tc[2] |= FF_TCO2_SET_TERMINATION;

    return write_tco(tc, 4);

    return 0;
}

}