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
        // 커비 부착
        inline constexpr const _tchar* Kirby_AttachmentBegin = L"Kirby.Attachment.Begin";
        inline constexpr const _tchar* Kirby_AttachmentEnd = L"Kirby.Attachment.End";

        // Kirby 이동
        inline constexpr const _tchar* Kirby_PositionSyncBegin = L"Kirby.Position.Sync.Begin";
        inline constexpr const _tchar* Kirby_PositionSyncEnd = L"Kirby.Position.Sync.End";

        // 커비 어빌리티 / UI
        inline constexpr const _tchar* Kirby_PointStarGained = L"Kirby.PointStarGained";
        inline constexpr const _tchar* Kirby_HP_Updated = L"Kirby.HPUpdated";
        inline constexpr const _tchar* Kirby_Name_Updated = L"Kirby.NameUpdated";
        inline constexpr const _tchar* Kirby_Ability_Changed = L"Kirby.AbilityChanged";
        inline constexpr const _tchar* Swallowed = L"OnSwallowed";

        // 보스
        inline constexpr const _tchar* Boss_HP_Appeared = L"Boss.HPAppeared";
        inline constexpr const _tchar* Boss_HP_Updated = L"Boss.HPUpdated";
        inline constexpr const _tchar* Boss_Died = L"Boss.Died";
        inline constexpr const _tchar* NamePlate_Appeared = L"Boss.NamePlateOn";

        // 컷씬
        inline constexpr const _tchar* Cutscene_GorillaAppear = L"Cutscene.GorillaAppear";   // 트리거 발동
        inline constexpr const _tchar* Cutscene_GorillaBreak = L"Cutscene.GorillaBreak";  // 고릴라 환경 부수기 이벤트
        inline constexpr const _tchar* Cutscene_CameraChange = L"Cutscene.CameraChange";
        inline constexpr const _tchar* Cutscene_StageClear = L"Cutscene.StageClear";

        // QTE
        inline constexpr const _tchar* QTE_Success = L"QTE.Success";

        // 카메라
        inline constexpr const _tchar* Camera_Shake = L"Camera.Shake";
        inline constexpr const _tchar* Camera_Rumble = L"Camera.Rumble";
        inline constexpr const _tchar* BossCam_Focus = L"BossCam_Focus";

        // Screen Fx
        inline constexpr const _tchar* FullScreen_Flash = L"Fx.FullScreenFlashOn";
        inline constexpr const _tchar* Letterbox_Begin = L"Letterbox.Begin";
        inline constexpr const _tchar* Letterbox_End = L"Letterbox.End";
        
        // 맵 
        inline constexpr const _tchar* Stage1_Step2_CarBreakMap = L"Stage1-2.CarBreakMap";

        // 포인터 질의
        inline constexpr const _tchar* Query_Player = L"Query_Player";
        inline constexpr const _tchar* Query_Boss = L"Query_Boss";

        // 사운드 
        inline constexpr const _tchar* BGMChange = L"ChangeBGM";

        // 전역 UI
        inline constexpr const _tchar* StageClear_UIStarted = L"StageClear.UIStarted";
        inline constexpr const _tchar* StageClear_SequenceFinished = L"StageClear.Finished";
        inline constexpr const _tchar* HUD_SetVisible = L"HUD.SetVisible";
    }


#pragma region Kirby 부착
    enum class KIRBY_ATTACHMENT_CONTEXT : _uint
    { 
        GORILLA_SCENE,
        GORILLA_COMBAT,
        DEFORM_CAR_GET_FIRST,
        _COUNT 
    };
    struct KIRBY_ATTACHMENT_BEGIN_DESC
    {
        KIRBY_ATTACHMENT_CONTEXT eType = { KIRBY_ATTACHMENT_CONTEXT::_COUNT };
        const _float4x4* pBoneMatrix = { nullptr };
        const _float4x4* pSourceWorld = { nullptr };
    };

    enum class KIRBY_ATTACHMENT_END_REASON : _uint
    {
        GORILLA_SCENE_HANDOFF,
        GORILLA_COMBAT_ESCAPE, GORILLA_COMBAT_THROWN,
        DEFORM_CAR_GET_FIRST_RELEASE,
        DEFAULT_RELEASE
    };
    struct KIRBY_ATTACHMENT_END_DESC
    {
        KIRBY_ATTACHMENT_END_REASON eType = { KIRBY_ATTACHMENT_END_REASON::DEFAULT_RELEASE };
    };

    // Kirby 이동
    enum class KIRBY_POSITION_SYNC_CONTEXT : _uint
    {
        CAR_BRIDGE,
        DEFAULT_SYNC
    };

    struct KIRBY_POSITION_SYNC_BEGIN_DESC
    {
        KIRBY_POSITION_SYNC_CONTEXT eType = { KIRBY_POSITION_SYNC_CONTEXT::DEFAULT_SYNC };
        _float4x4 AnchorWorld{};
        _float fAnimSpeed{ 1.f };
        _float fBlendDuration = { 0.f };
    };

    enum class KIRBY_POSITION_SYNC_END_REASON : _uint
    {
        CAR_BRIDGE_END,
        DEFAULT_SYNC_END
    };

    struct KIRBY_POSITION_SYNC_END_DESC
    {
        KIRBY_POSITION_SYNC_END_REASON eType = { KIRBY_POSITION_SYNC_END_REASON::DEFAULT_SYNC_END };
    };
#pragma endregion

#pragma region 포인터 질의
    struct PLAYER_QUERY { CGameObject* pPlayer = { nullptr }; };
    struct BOSS_QUERY { CGameObject* pBoss = { nullptr }; };
#pragma endregion

#pragma region 카메라관련 
    enum class ECutsceneCam { Cutscene, Boss, Area };

    struct CAMERA_SHAKE_DESC
    {
        _float fTrauma = { 0.5f };
        _float fDuration = { 0.f };
        _bool  bIgnoreTimeScale = { false };
    };

    struct CUTSCENE_CAMERA_DESC
    {
        ECutsceneCam       eCam = ECutsceneCam::Cutscene;
        const _tchar*      szTrack = nullptr;                // 컷씬캠일 때 재생 트랙 (예: L"DemoAppear2_camera1")
        CAnimator*         pProgress = nullptr;                 // progress 소스(고릴라 애니메이터)
        const _float4x4*   pAnchorWorld = nullptr;        // 로컬->월드 앵커(고릴라 월드행렬)
    };

    struct BOSSCAM_FOCUS_DESC
    {
        CGameObject* pTarget = { nullptr };
        _float       fAimHeight = { 2.f };   // 타겟별 시선 높이
    };
#pragma endregion

#pragma region 커비 어빌리티 / ui
    struct KIRBY_ABILITY_CHANGED
    {
        _bool bBegin = { true }; // true=변신 시작(줌인), false=변신 끝(복귀)
    };

    struct SWALLOW_EVENT
    {
        IInhalable* pInhalable;
    };

    struct KIRBY_NAME_UPDATED
    {
        _wstring strAtkModeName = { L"커비" };
    };

    struct KIRBY_POINTSTAR_GAINED_DESC
    {
        _uint iAmount = 1;
    };

    struct KIRBY_HP_UPDATED
    {
        _float fMaxHP = { 100.f };
        _float fCurrHp = { 100.f };
    };
#pragma endregion

#pragma region 보스
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
#pragma endregion

#pragma region 환경 트리거
    struct TRIGGER_EVENT_PAYLOAD
    {
        CGameObject* pTriggerObject = { nullptr };
        _wstring strTriggerId;
        _wstring strEventTag;
        _wstring strPayload;
    };
#pragma endregion

#pragma region 컷씬
    enum class STAGECLEAR_ANIM { CUT1, DANCE, _COUNT };

    struct CUTSCENE_STAGECLEAR
    {
        _float4x4 AnchorWorld {};
        STAGECLEAR_ANIM eAnim{ STAGECLEAR_ANIM::_COUNT };
        _float fAnimSpeed { 1.f };
        _float fBlendDuration = { 0.f };
    };
#pragma endregion

}