#pragma once
#include "haltech_protocol.h"

namespace monitor {
void ingest(const haltech::Frame& frame);
void poll(bool gear_echo_enabled);
} // namespace monitor
