#pragma once

#include "Kirby_OnOffPart.h"

#include "Damageable.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CKirby;

class CKirby_MetaSword final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_MetaSword)

public:
	struct KIRBY_METASWORD_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_MetaSword";
	static constexpr const wchar_t* Kirby_PartTag = L"MetaSword";

private:
	CKirby_MetaSword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_MetaSword(const CKirby_MetaSword& Prototype);
	virtual ~CKirby_MetaSword() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	void Begin_Hit(const ATTACK_INFO& tInfo, _bool bResetHitList = true);
	void End_Hit(_bool bResetHitList = true);

	void Reset_DamagedList() { m_DamagedTargets.clear(); }

	void Set_HitBox(_bool bOn);

private:
	HRESULT Ready_Components();
	HRESULT Ready_HitBox();
	void SetUp_HitBox_Callback();

private:
	CCollider* m_pHitBox{};
	ATTACK_INFO m_tAttackInfo{};

	unordered_set<CGameObject*> m_DamagedTargets;

public:
	static CKirby_MetaSword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

private:
	virtual void Free() override;
};

NS_END