#pragma once

#include "GameContent_Defines.h"

#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby_Body final : public CPartObject
{
	GENERATED_BODY(CKirby_Body)

public:
	struct KIRBY_BODY_DESC : public CPartObject::PARTOBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_Body";

private:
	CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Body(const CKirby_Body& Prototype);
	virtual ~CKirby_Body() = default;

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
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };

	_float4 m_vBodyColor = { 1.f, 0.45f, 0.55f, 1.f };
	_float4 m_vFootColor = { 1.f, 0.1882353f, 0.3764706f, 1.f };
	_float4 m_vBlushColor = { 1.f, 0.25f, 0.4f, 1.f };

public:
	static CKirby_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
protected:
	virtual void Free();
};

NS_END