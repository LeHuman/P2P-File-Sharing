#include "Log.h"

void Util::Logging::Log_t::enable(bool enable) {
	enabled = enable;
}

struct Util::Logging::Log_t Log;
