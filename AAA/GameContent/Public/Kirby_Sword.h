#pragma once

#include "GameContent_Defines.h"

#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby_Sword final : public CPartObject
{
	GENERATED_BODY(CKirby_Sword)

public:
	struct KIRBY_SWORD_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4* pSocketBoneMatrix{ nullptr };
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_Sword";

private:
	CKirby_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Sword(const CKirby_Sword& Prototype);
	virtual ~CKirby_Sword() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	CAnimator* Get_Animator() { return m_pAnimatorCom; }

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	CShader* m_pShaderCom{};
	CModel* m_pModelCom{};
	CAnimator* m_pAnimatorCom{};

	_float4 m_vConstantDiffuse = { 1.f, 0.72f, 0.08f, 1.f };
	_float3 m_vConstantMRA = { 0.25f, 0.18f, 1.f };
	_float4 m_vConstantEmissive = { 0.05f, 0.025f, 0.f, 1.f };

private:
	const _float4x4* m_pSocketBoneMatrix{ nullptr };

public:
	static CKirby_Sword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
protected:
	virtual void Free();
};

NS_END