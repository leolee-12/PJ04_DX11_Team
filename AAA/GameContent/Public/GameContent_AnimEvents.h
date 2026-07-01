#pragma once
#include <utility>

namespace Client
{
    // 엔진은 int만 안다. 의미/이름은 여기(클라이언트)에서만 정의.
    enum class EANIM_EVENT : int
    {
        None = 0,
        Fx = 1,
        Hitbox = 2,
        Sound = 3,
        Footstep = 4,
        CamShake = 5,
        IFrame = 6,
        LockMove = 7,

        // 커비 설정용
        SetEye = 8,
        SetMouth = 9,
        SetBody = 10,

        OnOffPart = 11,
        CamTrack  = 12,
        FreezeAnim = 13,
        PubEvent = 14,
        MoveWindow = 15,
        Projectile = 16,
        OnOffMesh = 17,
        WalkSmoke = 18,
        // 추가는 항상 끝에, 기존 값 변경 금지 (저장 데이터 안정성)
    };

    // 에디터 콤보용 이름표 (enum과 같은 곳에서 관리)
    inline const std::pair<EANIM_EVENT, const char*> g_AnimEventNames[] =
    {
        { EANIM_EVENT::None,        "None"     },
        { EANIM_EVENT::Fx,          "Fx"       },
        { EANIM_EVENT::Hitbox,      "Hitbox"   },
        { EANIM_EVENT::Sound,       "Sound"    },
        { EANIM_EVENT::Footstep,    "Footstep" },
        { EANIM_EVENT::CamShake,    "CamShake" },
        { EANIM_EVENT::IFrame,      "IFrame"   },
        { EANIM_EVENT::LockMove,    "LockMove" },
        { EANIM_EVENT::SetEye,      "SetEye"   },
        { EANIM_EVENT::SetMouth,    "SetMouth" },
        { EANIM_EVENT::SetBody,     "SetBody"  },
        { EANIM_EVENT::OnOffPart,   "OnOffPart" },
        { EANIM_EVENT::CamTrack,    "CamTrack"  },
        { EANIM_EVENT::FreezeAnim,  "FreezeAnim" },
        { EANIM_EVENT::PubEvent,    "PubEvent"  },
        { EANIM_EVENT::MoveWindow,  "MoveWindow" },
        { EANIM_EVENT::Projectile,  "Projectile" },
        { EANIM_EVENT::OnOffMesh,   "OnOffMesh" },
        { EANIM_EVENT::WalkSmoke, "WalkSmoke" },
    };
}