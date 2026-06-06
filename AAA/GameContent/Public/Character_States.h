#pragma once


namespace Client
{
	// 캐릭터들 모두 사용할 상태값 초안 
	enum class CHARACTER_STATE : int
	{
		IDLE = 0,

		// LOCOMOTION (이동 관련)
		WALK = 1,
		RUN = 2,
		FLIGHT = 3,
		JUMP = 4,
		BACKJUMP = 5,
		SLIDE = 6,

		// COMBAT (전투)
		ATTACK = 7,
		GUARD = 8,


		HIT = 9,
		DEATH = 10,

		END
	};

	// Loop 애니메이션 재생 시에 Start - Loop - End 처리를 위해 만들어둔거라 필요없으면 삭제.
	// Inhale 애니메이션 같은 경우에 계속 누르고 있을 경우 Inhale -> InhaleHustle? 로 넘어가는데 
	// LOOP_INPROGRESS 일 때 사용자 입력이 더 이상 없으면 END로, 아니라면 다음 애니메이션 START or 바로 LOOP_INPROGRESS로 넘어가게 애니메이션 정책이나 로직을 짜면 될듯 
	enum class ACTION_STEP : int
	{
		LOOP_START = 0,
		LOOP_INPROGRESS = 1,
		LOOP_END = 2,
		END
	};
}