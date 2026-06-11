#pragma once
#include "GameContent_Defines.h"
#include "Camera.h"
#include "Camera_AreaData.h"
NS_BEGIN(Client)

class CLIENT_DLL CCamera_AreaCam final : public CCamera
{
    GENERATED_BODY(CCamera_AreaCam)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraAreaCam";
    typedef struct tagAreaCamDesc final : public CCamera::CAMERA_DESC {
        wstring strTargetLayer = L"Layer_Player";
        wstring strTargetObj = L"Kirby";
        wstring strDataPath = L"../../Resources/YSH/CameraData/Level0_Stage1_Step01_cam.json";
    } AREACAM_DESC;
private:
    CCamera_AreaCam(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCamera_AreaCam(const CCamera_AreaCam& Prototype);
    virtual ~CCamera_AreaCam() = default;
public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Priority_Update(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override {}
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }
private:
    virtual HRESULT Ready_Events() override { return S_OK; }
private:
    CAreaCameraSolver m_solver;
    CGameObject* m_pTarget = nullptr;
    wstring           m_strTargetLayer, m_strTargetObj, m_strDataPath;
    _float3 m_eyeCur = {}, m_atCur = {}, m_eyeVel = {}, m_atVel = {};

    //_float  m_smoothTime = 0.35f;
    _float m_smoothBase = { 0.35f };
    _float m_smoothTrans = { 0.90f };
    _float m_blendDur = { 0.60f };
    _float m_blendTimer = { 0.f };
    _int m_lastArea = { -1 };
    _bool   m_bInit = { false };

public:
    static CCamera_AreaCam* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END