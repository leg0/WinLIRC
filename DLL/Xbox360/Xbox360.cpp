/*
 * This file is part of the WinLIRC package, which was derived from
 * LIRC (Linux Infrared Remote Control) 0.9.0.
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
 * Copyright (C) 2011 Ian Curtis
 */

#include <Windows.h>
#include "SendReceive.h"
#include <winlirc/WLPluginAPI.h>
#include <memory>

using namespace std::chrono_literals;

std::unique_ptr<SendReceive> sendReceive;

static int xbox360_init(WLEventHandle exitEvent) {

    sendReceive = std::make_unique<SendReceive>(reinterpret_cast<HANDLE>(exitEvent));
    return 1;
}

static void xbox360_deinit() {
    sendReceive.reset();
}

static int xbox360_decodeIR(struct ir_remote* remotes, char* out, size_t out_size) {

    if (sendReceive) {
        if (!sendReceive->waitTillDataIsReady(0us)) {
            return 0;
        }

        return sendReceive->decodeCommand(out, out_size);
    }

    return 0;
}

WL_API plugin_interface const* getPluginInterface()
{
    static plugin_interface const pi{
        .plugin_api_version = winlirc_plugin_api_version,
        .init = xbox360_init,
        .deinit = xbox360_deinit,
        .hasGui = []() { return FALSE; },
        .loadSetupGui = [] {},
        .sendIR = [](auto, auto, auto) { return 0; },
        .decodeIR = xbox360_decodeIR
    };
    return &pi;
}
