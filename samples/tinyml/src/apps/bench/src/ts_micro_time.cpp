/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 *
 * TFLite-Micro timing backend for the tinyml_bench sample.
 *
 * The prebuilt libts_realtek ships a default tflite::GetCurrentTimeTicks() /
 * tflite::ticks_per_second() (ts_realtek_micro_time.cc) that reads the ARM DWT
 * cycle counter and divides by a CPU frequency that must be registered at
 * runtime.  If that frequency is never set the profiler divides by zero and
 * every operator prints "(-1 ms)".
 *
 * We override both symbols here from the application instead.  ts_realtek_micro
 * _time.cc.o in the archive defines ONLY these two functions, so once the
 * application provides them the linker never pulls that archive member and
 * there is no duplicate-symbol conflict.
 *
 * Time source: the Realtek platform microsecond timestamp (sys_timestamp_get_us).
 *   - GetCurrentTimeTicks() returns the current timestamp in microseconds, so
 *     one profiler "tick" == 1 us.
 *   - ticks_per_second() therefore returns 1'000'000.
 * With that pairing the profiler's TicksToMs() (= 1000 * ticks / ticks_per_sec)
 * yields correct milliseconds, and the raw "ticks" column is microseconds -
 * fine enough that sub-millisecond operators no longer collapse to "0 ms".
 */

#include <stdint.h>

/* Realtek platform timestamp API (platform/inc/trace.h).  Declared locally so
 * this file does not depend on that header being on the include path; the
 * symbol has C linkage and is provided by the platform library. */
extern "C" uint32_t sys_timestamp_get_us(void);

namespace tflite
{

uint32_t ticks_per_second(void)
{
    /* One tick == one microsecond (see GetCurrentTimeTicks below). */
    return 1000000u;
}

uint32_t GetCurrentTimeTicks(void)
{
    return sys_timestamp_get_us();
}

}  /* namespace tflite */
