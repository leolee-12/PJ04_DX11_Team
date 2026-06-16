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

enum class LD_BREAKABLE_TYPE : _uint
{
	UNKNOWN = 0,
	STAR_BLOCK,
	STAR_BLOCK_BIG,
	WOOD_BOX,
	END
};

NS_END