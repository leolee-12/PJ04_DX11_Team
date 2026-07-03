#pragma once
#include "Engine_Defines.h"

NS_BEGIN(Engine)
class CGameObject;
class CAnimator;
NS_END

namespace Client
{
    class CMonster;
    class IInhalable;

    namespace EventTag
    {
        inline constexpr const _tchar* Kirby_PointStarGained = L"Kirby.PointStarGained";
        inline constexpr const _tchar* Kirby_HP_Updated = L"Kirby.HPUpdated";
        inline constexpr const _tchar* Kirby_Ability_Changed = L"Kirby.AbilityChanged";

        inline constexpr const _tchar* Boss_HP_Appeared = L"Boss.HPAppeared";
        inline constexpr const _tchar* Boss_HP_Updated = L"Boss.HPUpdated";
        inline constexpr const _tchar* Boss_Died = L"Boss.Died";

        inline constexpr const _tchar* Cutscene_GorillaAppear = L"Cutscene.GorillaAppear";   // 트리거 발동
        inline constexpr const _tchar* Cutscene_GrabKirby =    L"Cutscene.GrabKirby";           // 커비 컷씬위치로
        inline constexpr const _tchar* Cutscene_ReleaseKirby = L"Cutscene.ReleaseKirby";     
        inline constexpr const _tchar* Cutscene_CameraChange = L"Cutscene.CameraChange";
        inline constexpr const _tchar* Cutscene_GorillaHandoff = L"Cutscene.GorillaHandoff";
        inline constexpr const _tchar* Cutscene_GorillaBreak = L"Cutscene.GorillaBreak";  // 고릴라 환경 부수기 이벤트

        inline constexpr const _tchar* Camera_Shake = L"Camera.Shake";
        inline constexpr const _tchar* Camera_Rumble = L"Camera.Rumble";

        inline constexpr const _tchar* NamePlate_Appeared = L"Boss.NamePlateOn";
        inline constexpr const _tchar* FullScreen_Flash = L"Fx.FullScreenFlashOn";

        inline constexpr const _tchar* Stage12_CarBreakWall = L"Stage1-2.CarBreakWall";
        inline constexpr const _tchar* Stage12_CarBreakWall2 = L"Stage1-2.CarBreakWall2";
    }

    inline constexpr const _tchar* EVT_SWALLOWED = L"OnSwallowed";
    inline constexpr const _tchar* EVT_QUERY_PLAYER = L"Query_Player";
    inline constexpr const _tchar* EVT_QUERY_BOSS = L"Query_Boss";

    struct KIRBY_ABILITY_CHANGED
    {
        _bool bBegin = { true }; // true=변신 시작(줌인), false=변신 끝(복귀)
    };

    enum class ECutsceneCam { Cutscene, Boss };
    enum class GRAB_TYPE : _uint{ GORILLA_SCENE, GORILLA_COMBAT, _COUNT };

    struct CAMERA_SHAKE_DESC
    {
        _float fTrauma = { 0.5f };      
        _float fDuration = { 0.f };     
    };

    struct CUTSCENE_CAMERA_DESC
    {
        ECutsceneCam       eCam = ECutsceneCam::Cutscene;
        const _tchar* szTrack = nullptr;                // 컷씬캠일 때 재생 트랙 (예: L"DemoAppear2_camera1")
        CAnimator* pProgress = nullptr;                 // progress 소스(고릴라 애니메이터)
        const _float4x4* pAnchorWorld = nullptr;        // 로컬->월드 앵커(고릴라 월드행렬)
    };

    struct CUTSCENE_HANDOFF_DESC { const _float4x4* pFinalWorld = nullptr; };

    struct CUTSCENE_GRAB_DESC
    {
        const _float4x4* pBoneMatrix = { nullptr }; 
        const _float4x4* pSourceWorld = { nullptr };
        GRAB_TYPE  eType = { GRAB_TYPE::_COUNT };
    };

    struct PLAYER_QUERY { CGameObject* pPlayer = { nullptr }; };
    struct BOSS_QUERY { CGameObject* pBoss = { nullptr }; };

    struct SWALLOW_EVENT 
    { 
        IInhalable* pInhalable; 
    };

    typedef struct tagKirbyPointStarGained
    {
        _uint iAmount = 1;
    }KIRBY_POINTSTAR_GAINED_DESC;

    typedef struct tagKirbyHPUpdated
    {
        _float fMaxHP = { 100.f };
        _float fCurrHp = { 100.f };
    }KIRBY_HP_UPDATED;

    typedef struct tagBossHPUpdated 
    {
        _float fMaxHP = { 100.f };
        _float fCurrHp = { 100.f };
    }BOSS_HP_UPDATED;

    typedef struct tagBossHPAppeared
    {
        _wstring strBossName;
        _float fMaxHP = { 100.f };
        _float fCurrHp = { 100.f };
    }BOSS_HP_APPEARED;


    /* ----------------------- Leo ---------------------- */
    struct TRIGGER_EVENT_PAYLOAD
    {
        CGameObject* pTriggerObject = { nullptr };
        _wstring strTriggerId;
        _wstring strEventTag;
        _wstring strPayload;
    };
    /* -------------------------------------------------- */
}