#pragma once
#include "Component.h"
#include "Anim_Layer.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CAnimator final : public CComponent
{
public:
    // 기존 방식 보장 (애니메이션 공용 구조체)
    struct ANI_PLAY_INFO
    {
        _string     strAniName;
        _bool       bLoop{ true };
        _bool       bRestart{ false };
        _float      fBlend = { 0.2f };
        _float      fSpeed = { 1.f };
    };

    struct LAYER_PLAY_INFO
    {
        _uint               iSlot = { 1 };
        ANI_PLAY_INFO       tAnim;                          // 재생할 클립 코어 
        vector<_string>     Roots;                          // 마스크 루트 본 (비우면 전신 레이어)
        _float              fTargetWeight = { 1.f };      // 목표 가중치 0~1
        _float              fWeightBlend = { 0.1f };     // Weight 0 ~ Target 보간 시간
    };

public:
    typedef struct tagAnimatorDesc
    {
        CModel* pModel = { nullptr };
        wstring strDataFile = {};
    }ANIMATOR_DESC;

    using EventCallback = function<void(const ANIM_EVENT&, ANIM_EVENT_PHASE)>;

private:
    CAnimator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAnimator(const CAnimator& Prototype);
    virtual ~CAnimator() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    void    Update(_float fTimeDelta);                 // 재생 + 이벤트 판정 소유

public: // 재생 제어 (오브젝트/에디터는 오직 이것만 사용)
    void    Play(const string& strAnimName, _bool bLoop = true, _bool bRestart = false, _float fBlend = 0.2f, _float fSpeed = 1.0f);
    void    Play(const ANI_PLAY_INFO* tAniInfo);
    void    Start_Clip(const ANI_PLAY_INFO& Info);
    
    void    Pause() { m_bPaused = true; }
    void    Resume() { m_bPaused = false; }
    
    void    Pause_Mask(_int iSlot = 1) { m_Layers[iSlot].bPaused  = true; }
    void    Resume_Mask(_int iSlot = 1) { m_Layers[iSlot].bPaused = false; }

    void    Set_OverlayPaused(_int iSlot, _bool bPaused) { m_Layers[iSlot].bPaused = bPaused; }
    _bool   Is_OverlayPaused(_int iSlot = 1) const { return m_Layers[iSlot].bPaused; }

    void    Seek(_float fProgress);
    _bool   Is_Finished() const { return m_bFinished; }
    _bool   Is_Paused()   const { return m_bPaused; }
    const string& Get_CurrentAnimName() const;

    _float  Get_Progress() const;

    // 애니메이션 동안 특정 횟수만큼 돌리고 싶을 때
    void    SpinByProgress(const _char* szBone, _float fTurns, _fvector vAxis, _uint iSlot = 0);

    // 애니메이션 동안 특정 각도만큼 돌리고 복귀 시키고 싶을 때
    void    TiltByProgress(const _char* szBone, _float fPeakDeg, _fvector vAxis, _uint iSlot = 0);

    void    Set_EventCallback(EventCallback cb) { m_Callback = move(cb); }

    // 단일 선언 유지
    void Set_Mask(const _char* szClip, const _char* szRootBone, _bool bLoop,
        _float fTargetWeight, _float fWeightBlend, _float fClipBlend = 0.2f);

    // 벡터 선언 추가
    void Set_Mask(const _char* szClip, const _char* const* pRoots, _uint iRootCount, _bool bLoop,
        _float fTargetWeight, _float fWeightBlend, _float fClipBlend = 0.2f);

    // 바로 지우고 싶으면 그대로 사용 / 부드럽게 내리고 싶다면 BlendTime 주기
    void    Clear_Mask(_float fMaskBlendTime = 0.f);

    // 리팩토링 중
    void    Apply_Overlay(const LAYER_PLAY_INFO& tInfo);            // 같은 슬롯 + 같은 Clip 은 유지 / 다른 Clip은 crossfade / 슬롯 비활성 : weight-in
    void    Clear_Overlay(_uint iSlot, _float fWeightBlend = 0.f);  // Weight->0 Fade 후 슬롯 비활성
    _bool   Is_Overlay_Finished(_uint iSlot = 0) const;                 // 비루프 Clip 종료시 true 

    void    Enqueue(const ANI_PLAY_INFO& info);

    // Progress 직접 사용해서 적용하고 싶을 때 
    void    SetBoneRotation(const _char* szBone, _float fAngleDeg, _fvector vAxis);
    _bool   Has_Bone(const _char* szBone) const;

public: // 에디터(데이터 편집)
    ANIM_EVENT_TRACK& Get_Track(const string& strAnimName);
    const unordered_map<string, ANIM_EVENT_TRACK>& Get_Tracks() const { return m_Tracks; }
    void    Sort_Track(const string& strAnimName);

    HRESULT Load_FromFile(const wstring& strPath);
    HRESULT Save_ToFile(const wstring& strPath);

    virtual json Serialize() const override;
    virtual void Deserialize_Internal(const json& j) override;

    // 에디터 수치 확인용
    void    Set_PlaySpeed(_float fSpeed) { m_fPlaySpeed = fSpeed; }
    _float  Get_PlaySpeed() const { return m_fPlaySpeed; }

    void    Set_BlendDuration(_float fDuration) { m_fBlendDuration = fDuration; }
    _float  Get_BlendDuration() { return m_fBlendDuration; }

    const wstring& Get_DataFilePath() const { return m_strDataFilePath; }
    void Set_DataFilePath(const _wstring& strPath) { m_strDataFilePath = strPath; }

    _float Get_LayerProgress(_uint iSlot = 0) const;

private:
    void    Fire_Point(vector<ANIM_EVENT>& events, _float lo, _float hi);
    void    Process_Range(ANIM_EVENT_TRACK& track, _float fCur);
    void    Reset_RuntimeState(ANIM_EVENT_TRACK* pTrack);

    _wstring Make_DefaultDataFilePath() const;
    _wstring Resolve_DataFilePath(const _wstring& strPath) const;

private:
    CModel* m_pModel = { nullptr };
    EventCallback   m_Callback = { nullptr };

    unordered_map<string, ANIM_EVENT_TRACK> m_Tracks;
    deque<ANI_PLAY_INFO> m_PlayQueue;       // 대기 클립들 ( 현재 재생분은 여기에 없음 )

    _bool               m_bPaused = { false };
    _bool               m_bFinished = { false };

    string              m_strPrevAnimName = {};
    _float              m_fPrevProgress = { 0.f };
    _float              m_fPlaySpeed = { 1.0f };
    _float              m_fBlendDuration = { 0.2f };
    
    _wstring            m_strDataFilePath = {};

    _bool               m_bCurLoop = { false };

    static constexpr _uint MAX_LAYERS = 4;      // 최대 Layer 개수 (이후 Enum으로 확장)
    LAYER               m_Layers[MAX_LAYERS] = {};

    // 회전에 필요한 변수
    _string             m_strRotBone;       // 회전을 적용할 본 이름 "RotL"
    _float              m_fRotAngle = { 0.f };
    _float4             m_vRotAxis = { 1.f, 0.f, 0.f, 0.f };
    _bool               m_bHasRotReq = { false };

public:
    static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END