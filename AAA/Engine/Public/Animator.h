#pragma once
#include "Component.h"

NS_BEGIN(Engine)

class CModel;

class ENGINE_DLL CAnimator final : public CComponent
{
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
    void    Pause() { m_bPaused = true; }
    void    Resume() { m_bPaused = false; }
    void    Seek(_float fProgress);
    _bool   Is_Finished() const { return m_bFinished; }
    _bool   Is_Paused()   const { return m_bPaused; }
    const string& Get_CurrentAnimName() const;
    _float  Get_Progress() const;

    void    Set_EventCallback(EventCallback cb) { m_Callback = move(cb); }

public: // 에디터(데이터 편집)
    ANIM_EVENT_TRACK& Get_Track(const string& strAnimName);
    const unordered_map<string, ANIM_EVENT_TRACK>& Get_Tracks() const { return m_Tracks; }
    void    Sort_Track(const string& strAnimName);

    HRESULT Load_FromFile(const wstring& strPath);
    HRESULT Save_ToFile(const wstring& strPath) const;

    virtual json Serialize() const override;
    virtual void Deserialize_Internal(const json& j) override;

    // 에디터 수치 확인용
    void    Set_PlaySpeed(_float fSpeed) { m_fPlaySpeed = fSpeed; }
    _float  Get_PlaySpeed() const { return m_fPlaySpeed; }

    void    Set_BlendDuration(_float fDuration) { m_fBlendDuration = fDuration; }
    _float  Get_BlendDuration() { return m_fBlendDuration; }

private:
    void    Fire_Point(const vector<ANIM_EVENT>& events, _float lo, _float hi);
    void    Process_Range(ANIM_EVENT_TRACK& track, _float fCur);
    void    Reset_RuntimeState(ANIM_EVENT_TRACK* pTrack);

private:
    CModel* m_pModel = { nullptr };
    EventCallback   m_Callback = { nullptr };

    unordered_map<string, ANIM_EVENT_TRACK> m_Tracks;

    _bool   m_bPaused = { false };
    _bool   m_bFinished = { false };

    string  m_strPrevAnimName = {};
    _float  m_fPrevProgress = { 0.f };
    _float  m_fPlaySpeed = { 1.0f };
    _float  m_fBlendDuration = { 0.2f };


public:
    static CAnimator* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END