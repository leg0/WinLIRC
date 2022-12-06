/*
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.8.6.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Copyright (C) 2010 Ian Curtis
 */

#include "Globals.h"
#include <stdio.h>
#include "SerialDialog.h"
#include "irdriver.h"

#include <winlirc/winlirc_api.h>
#include <winlirc/WLPluginAPI.h>
#include "Transmit.h"

static rbuf rec_buffer;


static lirc_t serial_readData(lirc_t timeout) {

    if (!irDriver) return 0;

    return irDriver->readData(std::chrono::microseconds{ timeout });
}

static void serial_wait_for_data(lirc_t timeout) {

    if (!irDriver) return;

    irDriver->waitTillDataIsReady(std::chrono::microseconds{ timeout });
}

static int serial_data_ready() {

    if (!irDriver) return 0;

    if (irDriver->dataReady()) return 1;

    return 0;
}

static hardware const serial_hw{
    .plugin_api_version = winlirc_plugin_api_version,
    .device = "hw",
    .name = "SerialDevice",

    .features = LIRC_CAN_REC_MODE2,
    .send_mode = 0,
    .rec_mode = LIRC_MODE_MODE2,
    .code_length = 0,
    .resolution = 0,

    .decode_func = &winlirc_receive_decode,
    .readdata = &serial_readData,
    .wait_for_data = &serial_wait_for_data,
    .data_ready = &serial_data_ready,
    .get_ir_code = nullptr,
};


static int serial_init(WLEventHandle exitEvent) {

    threadExitEvent = reinterpret_cast<HANDLE>(exitEvent);

    winlirc_init_rec_buffer(&rec_buffer);

    irDriver = new CIRDriver();
    if (irDriver->InitPort()) return 1;

    return 0;
}

static void serial_deinit() {

    threadExitEvent = nullptr;	//this one is created outside the DLL

    if (irDriver) {
        delete irDriver;
        irDriver = nullptr;
    }
}

static void serial_loadSetupGui() {

    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    SerialDialog serialDialog;
    serialDialog.DoModal();
}

static int serial_sendIR(struct ir_remote* remotes, struct ir_ncode* code, int repeats) {

    return Transmit(code, remotes, repeats);
}

static int serial_decodeIR(struct ir_remote* remotes, char* out, size_t out_size) {

    if (irDriver) {
        using namespace std::chrono_literals;
        if (!irDriver->waitTillDataIsReady(0us)) {
            return 0;
        }
    }

    winlirc_clear_rec_buffer(&rec_buffer, &serial_hw);

    if (winlirc_decodeCommand(&rec_buffer, &serial_hw, remotes, out, out_size)) {
        return 1;
    }

    return 0;
}

WL_API plugin_interface const* getPluginInterface() {
    static constexpr plugin_interface pi{
        .plugin_api_version = winlirc_plugin_api_version,
        .init = serial_init,
        .deinit = serial_deinit,
        .hasGui = [] { return 1; },
        .loadSetupGui = serial_loadSetupGui,
        .sendIR = serial_sendIR,
        .decodeIR = serial_decodeIR,
        .getHardware = [] { return &serial_hw; },
        .hardware = &serial_hw
    };
    return &pi;
}
