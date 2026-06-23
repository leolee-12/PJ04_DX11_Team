#pragma once
#include "GameContent_Defines.h"

NS_BEGIN(Client)

enum class LD_CATEGORY : _uint
{
	PORTAL,
	DOOR,
	RAIL,
	VOLUME,
	GUIDE_AREA,
	AUDIO_AREA,
	ITEM,
	FOOD,
	BREAKABLE,
	FOLIAGE,
	ENEMY,
	GIMMICK,
	META,
	UNSUPPORTED,
	END
};

NS_END