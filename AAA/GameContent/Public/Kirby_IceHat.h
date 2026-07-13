#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Client)

class CKirby_IceHat final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_IceHat)

public:
	struct KIRBY_ICE_HAT_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_IceHat";
	static constexpr const wchar_t* Kirby_PartTag = L"IceHat";

private:
	CKirby_IceHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_IceHat(const CKirby_IceHat& Prototype);
	virtual ~CKirby_IceHat() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_Components();

public:
	static CKirby_IceHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
