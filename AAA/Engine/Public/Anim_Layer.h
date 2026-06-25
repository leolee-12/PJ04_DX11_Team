#pragma once

#include "Engine_Defines.h"

NS_BEGIN(Engine)

// 레이어에 들어온 정보를 풀어 보관 (base는 index 0, weight 1 고정)
struct LAYER
{
    _string         strClip;                                    // 재생 중인 클립 ("" : 비활성)
    _int            iAnimIndex = { -1 };                          // 해석된 인덱스 - 매 프레임 Get_AnimtionIndex 제거 
    _float          fLocalTime = { 0.f };               // 레이어 자체 재생 

    _float          fClipBlend = { 0.f };              // crossfade 총 시간
    _float          fClipBlendElapsed = { 0.f };              // 경과
    
    _bool           bClipBlending = { false };          // 
    _int            iPrevAnimIndex = { -1 };
    _float          fPrevLocalTime = { 0.f };
    _bool           bPrevLoop = { true };
    vector<_uint>   PrevCursors;

    _float          fWeight = { 0.f };              // Weight Blend 
    _float          fTarget = { 1.f };
    _float          fWeightBlend = { 0.1f };

    _float          fSpeed = { 1.f };              // 레이어 독립 속도
    _bool           bLoop = { true };
    _bool           bPaused = { false };
    _bool           bFinished = { false };            // 레이어 완료 신호

    vector<_uint>   MaskBones;                                  // 해석된 마스크 본 인덱스
    vector<_uint>   KeyFrameCursors;
};

NS_END