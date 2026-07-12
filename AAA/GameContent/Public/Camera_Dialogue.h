#pragma once
#include "GameContent_Defines.h"
#include "Camera_Shakeable.h"

NS_BEGIN(Client)

// 대화 전용 카메라: 앵커 기준 샷 프리셋(투샷/화자 클로즈업) + 샷 간 이징 블렌드
class CLIENT_DLL CCamera_Dialogue final : public CCamera_Shakeable
{
    GENERATED_BODY(CCamera_Dialogue)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraDialogue";
    struct DIALOGUECAM_DESC : public CCamera::CAMERA_DESC {};

    struct SHOT
    {
        _float3 vEye{};       // 월드공간(Begin 시점에 확정)
        _float3 vAim{};
        _float  fFovy = { 0.61f };
    };

private:
    CCamera_Dialogue(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_Dialogue(const CCamera_Dialogue& Prototype);
    virtual ~CCamera_Dialogue() = default;

public:
    virtual HRESULT Initialize_Prototype() override { return S_OK; }
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

    // 디렉터가 호출(암전 중): 앵커/배우 위치로 샷 프리셋 생성 후 투샷으로 스냅
    void Begin(const _float4x4& AnchorWorld, _fvector vPosA, _fvector vPosB);
    void Set_Shot(const _wstring& strName, _float fBlendDur);

private:
    virtual HRESULT Ready_Events() override;
    void Apply_Pose();

private:
    unordered_map<_wstring, SHOT> m_Shots;
    SHOT m_tFrom{}, m_tTo{}, m_tCur{};
    _float m_fBlendTime = { 0.f };
    _float m_fBlendTotal = { 0.f };

    _float m_fTwoDist = { 7.f };     // 투샷 측면 거리
    _float m_fCloseDist = { 5.f };  // 클로즈업 측면 거리
    _float m_fEyeHeight = { 1.2f };
    _float m_fAimHeight = { 0.3f };
    _float m_fShotBlend = { 0.35f }; // 샷 전환 기본 블렌드 시간

public:
    static CCamera_Dialogue* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END