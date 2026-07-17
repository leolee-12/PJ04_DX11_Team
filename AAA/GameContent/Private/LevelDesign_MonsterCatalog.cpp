#include "LevelDesign_MonsterCatalog.h"
#include "LevelDesign_Registry.h"

#include "BladeKnight.h"
#include "NormalEnemy.h"
#include "Kabu.h"
#include "BrontoBurt.h"
#include "PoppyBrosJr.h"
#include "Cappy.h"
#include "NormalEnemyWild.h"
#include "Dekabu.h"
#include "Parsing_Utils.h"

NS_BEGIN(Client)

namespace
{
	struct LD_MONSTER_CATALOG
	{
		const _tchar* pObjectName;
		const _tchar* pPrototypeTag;
		LD_OBJECT_PROTO_FACTORY pPrototypeFactory;
		const _tchar* pVariationOwnerName = nullptr;
	};

	CGameObject* Create_BladeKnightPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CBladeKnight::Create(pDevice, pContext);
	}

	CGameObject* Create_NormalEnemyPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CNormalEnemy::Create(pDevice, pContext);
	}

	CGameObject* Create_KabuPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CKabu::Create(pDevice, pContext);
	}

	CGameObject* Create_BrontoBurtPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CBrontoBurt::Create(pDevice, pContext);
	}

	CGameObject* Create_PoppyBrosJrPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CPoppyBrosJr::Create(pDevice, pContext);
	}

	CGameObject* Create_CappyPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CCappy::Create(pDevice, pContext);
	}

	CGameObject* Create_NormalEnemyWildPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CNormalEnemyWild::Create(pDevice, pContext);
	}

	CGameObject* Create_DekabuPrototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	{
		return CDekabu::Create(pDevice, pContext);
	}

	static const LD_MONSTER_CATALOG g_MonsterCatalog[] =
	{
			{ L"BladeKnight",		CBladeKnight::PROTOTYPE_TAG,		&Create_BladeKnightPrototype },
			{ L"NormalEnemy",		CNormalEnemy::PROTOTYPE_TAG,		&Create_NormalEnemyPrototype },
			{ L"Kabu",				CKabu::PROTOTYPE_TAG,				&Create_KabuPrototype },
			{ L"BrontoBurt",		CBrontoBurt::PROTOTYPE_TAG,			&Create_BrontoBurtPrototype },
			{ L"PoppyBrosJr",		CPoppyBrosJr::PROTOTYPE_TAG,		&Create_PoppyBrosJrPrototype },
			{ L"Cappy",				CCappy::PROTOTYPE_TAG,				&Create_CappyPrototype },
			{ L"NormalEnemyWild",	CNormalEnemyWild::PROTOTYPE_TAG,	&Create_NormalEnemyWildPrototype, L"NormalEnemy"},
			{ L"Dekabu",			CDekabu::PROTOTYPE_TAG,				&Create_DekabuPrototype},
	};
}

void CLevelDesign_MonsterCatalog::Register_LevelDesignSpecs()
{
	for (const LD_MONSTER_CATALOG& Entry : g_MonsterCatalog)
	{
		LD_SPAWN_SPEC Spec{};
		Spec.strObjectName = Entry.pObjectName;
		Spec.strPrototypeTag = Entry.pPrototypeTag;
		Spec.strLayerTag = L"Layer_LevelDesign_Enemy";
		Spec.eCategory = LD_CATEGORY::ENEMY;
		Spec.pPrototypeFactory = Entry.pPrototypeFactory;
		Spec.pBuildDesc = &Build_Desc;
		Spec.bUseFactoryResourceLoader = true;
		Spec.strVariationOwnerName = Entry.pVariationOwnerName ? Entry.pVariationOwnerName : L"";

		CLevelDesign_Registry::Register(Spec.strObjectName, Spec);
	}
}

_bool CLevelDesign_MonsterCatalog::Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry)
{
	if (nullptr == pOutEntry)
		return false;
	if (Spec.eCategory != LD_CATEGORY::ENEMY)
		return false;

	LD_PARSED_OBJECT Desc{};
	static_cast<LD_OBJECT_DESC&>(Desc) = CommonDesc;
	Desc.eCategory = Spec.eCategory;

	const _wstring& strOwner = Spec.strVariationOwnerName.empty() ? Spec.strObjectName : Spec.strVariationOwnerName;
	const _string strBase = "Chara." + WstrToStr(strOwner) + ".Variation.";
	if (!JsonUtils::Try_ReadString(jEntry, strBase + "VariationType", &Desc.strAIVariation))
		JsonUtils::Try_ReadString(jEntry, strBase + "TypeStr", &Desc.strAIVariation);

	JsonUtils::Try_ReadString(jEntry, strBase + "ThrowLv", &Desc.strThrowLv);

	XMStoreFloat4(&Desc.vRight, XMVectorNegate(XMLoadFloat4(&Desc.vRight)));
	XMStoreFloat4(&Desc.vLook, XMVectorNegate(XMLoadFloat4(&Desc.vLook)));

	*pOutEntry = Desc;
	return true;
}

NS_END