#pragma once
#include <utility>

namespace Client
{
    // 적용된 커비의 현재 능력 상태 
    enum class KIRBY_ABILITY_STATE : int
    {
        NORMAL = 0,
        BOMB = 1,
        CUTTER = 2,
        ICE = 3,
        END
    };

    // Body Mesh 결정 축
    enum class KIRBY_BODY_STATE : int
    {
        NORMAL = 0,        // BodyM        (Mesh 1번) 
        STUFFED = 1,    // BodyBigM        (Mesh 0번, 입 내장 되어 있음)
        INHALE = 2,        // BodyVacuumM    (Mesh 2번, 입 내장 되어 있음)
        END
    };

    // Eye 텍스처 인덱스 결정 축
    enum class KIRBY_EYE_STATE : int
    {
        IDLE = 0,
        DOUBT = 1,
        BLINK = 2,
        CLOSE = 3,
        ANGRY = 4,
        SURPRISED = 5,
        SADNESS = 6,
        END
    };

    // Mouth Mesh 결정 축 
    enum class KIRBY_MOUTH_STATE : int
    {
        IDLE = 0,            // Mesh - MouthNormalM_BodyC / Index : 4번 
        OPEN = 1,            // Mesh - MouthOpenM_BodyC / Index : 5번
        ANGRY = 2,            // Mesh - MouthAngryCloseM_BodyC / Index : 3번
        SMILE_OPEN = 3,        // Mesh - MouthSmileOpenM_BodyC / Index : 7번 
        SMILE_CLOSE = 4,    // Mesh - MouthSmileCloseM_BodyC / Index : 6번 
        END
    };

    inline const std::pair<KIRBY_BODY_STATE, const char*> g_KirbyBodyNames[] = {
          { KIRBY_BODY_STATE::NORMAL,  "NORMAL"  },
          { KIRBY_BODY_STATE::STUFFED, "STUFFED" },
          { KIRBY_BODY_STATE::INHALE,  "INHALE"  },
    };
    inline const std::pair<KIRBY_MOUTH_STATE, const char*> g_KirbyMouthNames[] = {
        { KIRBY_MOUTH_STATE::IDLE,        "IDLE"        },
        { KIRBY_MOUTH_STATE::OPEN,        "OPEN"        },
        { KIRBY_MOUTH_STATE::ANGRY,       "ANGRY"       },
        { KIRBY_MOUTH_STATE::SMILE_OPEN,  "SMILE_OPEN"  },
        { KIRBY_MOUTH_STATE::SMILE_CLOSE, "SMILE_CLOSE" },
    };
    inline const std::pair<KIRBY_EYE_STATE, const char*> g_KirbyEyeNames[] = {
        { KIRBY_EYE_STATE::IDLE,      "IDLE"      },
        { KIRBY_EYE_STATE::DOUBT,     "DOUBT"     },
        { KIRBY_EYE_STATE::BLINK,     "BLINK"     },
        { KIRBY_EYE_STATE::CLOSE,     "CLOSE"     },
        { KIRBY_EYE_STATE::ANGRY,     "ANGRY"     },
        { KIRBY_EYE_STATE::SURPRISED, "SURPRISED" },
        { KIRBY_EYE_STATE::SADNESS,   "SADNESS"   },
    };
}