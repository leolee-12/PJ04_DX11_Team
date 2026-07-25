#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby_MetaHat final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_MetaHat)

private:
	enum META_HAT_MESH { HOVERING_MASK, HOVERING_STRAP, MASK, STRAP, META_MASK_MESH_END };

public:
	struct KIRBY_METAHAT_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_MetaHat";
	static constexpr const wchar_t* Kirby_PartTag = L"MetaHat";

private:
	CKirby_MetaHat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_MetaHat(const CKirby_MetaHat& Prototype);
	virtual ~CKirby_MetaHat() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	virtual void Set_PartMode(CKirby* pKirby, KIRBY_PART_MODE ePartMode) override;

private:
	HRESULT Ready_Components();

	_bool m_bIsHovering{};

public:
	static CKirby_MetaHat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END