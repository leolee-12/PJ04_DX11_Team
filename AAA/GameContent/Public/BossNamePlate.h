#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader; class CModel;
NS_END

NS_BEGIN(Client)

class CBossNamePlate abstract : public CGameObject
{
    GENERATED_BODY_ABSTRACT(CBossNamePlate)

protected:
    CBossNamePlate(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBossNamePlate(const CBossNamePlate& Prototype);
    virtual ~CBossNamePlate() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override = 0;                 // 자식 구현

protected:
    virtual HRESULT Ready_Events() override;

    // ---- 자식이 알려주는 것 ----
    virtual const _tchar* Get_ModelProtoTag() const = 0;  // 어떤 모델
    virtual const _tchar* Get_ShaderProtoTag() const { return TEXT("Prototype_Component_Shader_NonAnimMesh_PBR"); }
    virtual const _tchar* Get_ActivateEventTag() const { return TEXT("Boss.NamePlateOn"); }

    // ---- 베이스 제공 ----
    HRESULT Bind_ShaderResources();                       // 디더 관련 값(매트릭스 + g_fDissolve) 던짐
    void    Activate();

    enum class EState { Idle, Hold, Dissolve };

    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    EState  m_eState = { EState::Idle };
    _float  m_fTimer = { 0.f };
    _float  m_fDissolve = { 0.f };                         // g_fDissolve (0=보임, 1=사라짐)
    _float  m_fHoldTime = { 3.f };                         // 자식이 Initialize에서 조절 가능
    _float  m_fFadeTime = { 1.f };

private:
    HRESULT Ready_Components();

protected:
    virtual void Free() override;
};

NS_END