#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Client)

class CKirby_SleepHat final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_SleepHat)

public:
	struct KIRBY_SLEEP_HAT_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_SleepHat";
	static constexpr const wchar_t* Kirby_PartTag = L"SleepHat";

private:
	CKirby_SleepHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_SleepHat(const CKirby_SleepHat& Prototype);
	virtual ~CKirby_SleepHat() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_Components();

public:
	static CKirby_SleepHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END
