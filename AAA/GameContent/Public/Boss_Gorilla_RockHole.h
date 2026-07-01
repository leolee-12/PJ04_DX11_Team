#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CModel;
class CAnimator;
class CShader;
NS_END

NS_BEGIN(Client)

class CBoss_Gorilla_RockHole final : public CPartObject
{
private:
    CBoss_Gorilla_RockHole(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Gorilla_RockHole(const CBoss_Gorilla_RockHole& Prototype);
    virtual ~CBoss_Gorilla_RockHole() = default;

public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Boss_Gorilla_RockHole";
    static constexpr const _tchar*  MODEL_PROTO_TAG = L"Prototype_Component_Model_RockHole";
    static constexpr const wchar_t* PART_TAG = L"RockHole";

    static constexpr const _float   s_fWaitTime = 4.f;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }
    virtual HRESULT Render() override;

public:
    void            Active_Effect(_fvector vPos);

private:
    CModel*     m_pModelCom = { nullptr };
    CAnimator*  m_pAnimatorCom = { nullptr };
    CShader*    m_pShaderCom = { nullptr };

    enum class HOLE_STATE { START, WAIT, END };

    HOLE_STATE  m_eState = { HOLE_STATE::START };

    _float      m_fWaitTimer = { 0.f };

    _float      m_fDissolve = { 0.f };

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

    void    Update_StateStart(_float fTimeDelta);
    void    Update_StateWait(_float fTimeDelta);
    void    Update_StateEnd(_float fTimeDelta);

public:
    static CBoss_Gorilla_RockHole* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CBoss_Gorilla_RockHole* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END