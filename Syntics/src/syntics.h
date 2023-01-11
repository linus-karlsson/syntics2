#pragma once

#include "vulkan_api.h"
#include "vulkan_types.h"
#include "event_system.h"
#ifdef LINUX
#include "linux/linux_platform.h"
#else
#endif
#include "region_alloc.h"
#include "render.h"
