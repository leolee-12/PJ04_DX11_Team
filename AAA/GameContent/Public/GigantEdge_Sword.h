#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CGigantEdge_Sword final : public CPartObject
{
    GENERATED_BODY(CGigantEdge_Sword)

private:
    CGigantEdge_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge_Sword(const CGigantEdge_Sword& Prototype);
    virtual ~CGigantEdge_Sword() = default;

public:
    struct GIGANTEDGE_SWORD_DESC : public CPartObject::PARTOBJECT_DESC
    {
        const _float4x4* pSocketBoneMatrix{ nullptr };
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Sword";
    static constexpr const wchar_t* PART_TAG = L"Sword";

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual HRESULT Render_Shadow() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    CAnimator * Get_Animator() { return m_pAnimatorCom; }

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

    const _float4x4* m_pSocketBoneMatrix = { nullptr };

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

public:
    static CGigantEdge_Sword* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
    virtual CGigantEdge_Sword* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END