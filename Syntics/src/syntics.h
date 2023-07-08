#pragma once

#include "defines.h"
#include "vulkan_types.h"
#include "file_reading.h"

#include "logging.cc"
#include "region_alloc.cc"
#include "file_reading.cc"

#include "vulkan_api.h"
#include "vulkan_types.h"
#include "event_system.h"
#ifdef LINUX
#include "linux/linux_platform.h"
#else
#include "win32/win32_platform.h"
#endif
#include "region_alloc.h"
#include "render.h"
#include "gui.h"
#include "random.h"
