#include "ir_remote.h"

#include <memory>

ir_remote::ir_remote(ir_remote const& other) noexcept
{
    *this = other;
}

ir_remote& ir_remote::operator=(ir_remote const& other) noexcept
{
    if (this != &other)
    {
        name = other.name;
        codes = other.codes;
        bits = other.bits;
        flags = other.flags;
        eps = other.eps;
        aeps = other.aeps;
        phead = other.phead;
        shead = other.shead;
        pthree = other.pthree;
        sthree = other.sthree;
        ptwo = other.ptwo;
        stwo = other.stwo;
        pone = other.pone;
        sone = other.sone;
        pzero = other.pzero;
        szero = other.szero;
        plead = other.plead;
        ptrail = other.ptrail;
        pfoot = other.pfoot;
        sfoot = other.sfoot;
        prepeat = other.prepeat;
        srepeat = other.srepeat;
        pre_data_bits = other.pre_data_bits;
        pre_data = other.pre_data;
        post_data_bits = other.post_data_bits;
        post_data = other.post_data;
        pre_p = other.pre_p;
        pre_s = other.pre_s;
        post_p = other.post_p;
        post_s = other.post_s;
        gap = other.gap;
        gap2 = other.gap2;
        repeat_gap = other.repeat_gap;
        toggle_bit = other.toggle_bit;
        toggle_bit_mask = other.toggle_bit_mask;
        min_repeat = other.min_repeat;
        min_code_repeat = other.min_code_repeat;
        freq = other.freq;
        duty_cycle = other.duty_cycle;
        toggle_mask = other.toggle_mask;
        rc6_mask = other.rc6_mask;
        baud = other.baud;
        bits_in_byte = other.bits_in_byte;
        parity = other.parity;
        stop_bits = other.stop_bits;
        ignore_mask = other.ignore_mask;
        toggle_bit_mask_state = other.toggle_bit_mask_state;
        toggle_mask_state = other.toggle_mask_state;
        repeat_countdown = other.repeat_countdown;
        last_code = other.last_code;
        toggle_code = other.toggle_code;
        reps = other.reps;
        last_send = other.last_send;
        min_remaining_gap = other.min_remaining_gap;
        max_remaining_gap = other.max_remaining_gap;
        next = clone(other.next);
    }
    return *this;
}

ir_ncode::ir_ncode(ir_ncode const& other) noexcept
{
    *this = other;
}

ir_ncode& ir_ncode::operator=(ir_ncode const& other) noexcept
{
    if (this != &other)
    {
        name = other.name;
        code = other.code;
        signals = other.signals;
        next = clone(other.next);
        current = other.current;
        transmit_state = other.transmit_state;
    }
    return *this;
}

ir_code_node::ir_code_node(ir_code_node const& other) noexcept
{
    *this = other;
}

ir_code_node& ir_code_node::operator=(ir_code_node const& other) noexcept
{
    if (this != &other)
    {
        code = other.code;
        next = clone(other.next);
    }
    return *this;
}
