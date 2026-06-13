#pragma once

#include "GameContent_Defines.h"

#include "Kirby_OnOffPart.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby_SwordHat final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_SwordHat)

public:
	struct KIRBY_SWORDHAT_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_SwordHat";
	static constexpr const wchar_t* Kirby_PartTag = L"SwordHat";

private:
	CKirby_SwordHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_SwordHat(const CKirby_SwordHat& Prototype);
	virtual ~CKirby_SwordHat() = default;

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

public:
	static CKirby_SwordHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END