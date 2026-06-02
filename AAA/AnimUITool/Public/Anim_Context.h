#pragma once
#include "AnimUITool_Defines.h"

NS_BEGIN(Engine)
class CModel;
class CAnimator;
NS_END

NS_BEGIN(AnimUITool)
class CPreview_Actor;

struct ANIM_CONTEXT
{
    CPreview_Actor*     pActor = { nullptr };
    CModel*             pModel = { nullptr };
    CAnimator*          pAnimator = { nullptr };

    _wstring            strName = {};           // 모델 표시명 
    _wstring            strModelPath = {};      // json 경로 파생용

    _int                iClip = { 0 };      
    _bool               bPlaying = { true };
    _bool               bLoop = { true };
    _float              fProgress = { 0.f };

    _int                iRootBone = { -1 };     
    _int                iSelBone = { -1 };     

    _bool               Valid() const { return pActor && pModel && pAnimator; }

};
NS_END

