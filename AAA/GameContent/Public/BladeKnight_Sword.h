#pragma once

#include "GameContent_Defines.h"
#include "PartObject.h"

// 이후 무기 공통 부분은 PartObject - Weapon -> 상속받는 것으로 확장

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBladeKnight_Sword final : public CPartObject
{
	GENERATED_BODY(CBladeKnight_Sword)

public:
	struct BLADEKNIGHT_SWORD_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4* pSocketBoneMatrix = { nullptr }; //  소켓의 본 행렬
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BladeKnight_Sword";
	static constexpr const _tchar* PART_TAG = L"Sword";

private:
    CBladeKnight_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBladeKnight_Sword(const CBladeKnight_Sword& Prototype);
    virtual ~CBladeKnight_Sword() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Priority_Update(_float fTimeDelta) override;
    virtual void Update(_float fTimeDelta) override;
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    CAnimator* Get_Animator() { return m_pAnimatorCom; }

private:
    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources();

private:
    const _float4x4* m_pSocketBoneMatrix = { nullptr };

    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };

private:
    _float4 m_vConstantDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
    _float3 m_vConstantMRA = { 0.25f, 0.18f, 1.f };
    _float4 m_vConstantEmissive = { 0.05f, 0.025f, 0.f, 1.f };


public:
    static CBladeKnight_Sword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END