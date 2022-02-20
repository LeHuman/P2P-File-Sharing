/**
 * @file Log.cpp
 * @author IR
 * @brief Basic logging module, used to make it easier to print things to console
 * @version 0.1
 * @date 2022-02-20
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "Log.h"

void Util::Logging::Log_t::enable(bool enable) {
	enabled = enable;
}

struct Util::Logging::Log_t Log;
