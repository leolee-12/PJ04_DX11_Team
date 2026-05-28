#pragma once

#ifdef new
#undef new
#include "nlohmann/json.hpp"
#define new DBG_NEW
#else
#include "nlohmann/json.hpp"
#endif


namespace Engine {
	using json = nlohmann::ordered_json;
}
