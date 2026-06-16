#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CGigantEdge_Body final : public CPartObject
{
    GENERATED_BODY(CGigantEdge_Body)

private:
    CGigantEdge_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigantEdge_Body(const CGigantEdge_Body& Prototype);
    virtual ~CGigantEdge_Body() = default;

public:
    struct GIGANTEDGE_BODY_DESC : public CPartObject::PARTOBJECT_DESC
    {
    };

    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge_Body";
    static constexpr const wchar_t* PART_TAG = L"Body";

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
    const _float4x4 * Get_BoneMatrixPtr(const _char * pBoneName) const;
    CAnimator* Get_Animator() { return m_pAnimatorCom; }

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

public:
    static CGigantEdge_Body* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
    virtual CGigantEdge_Body* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END