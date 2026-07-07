#pragma once

#include "Kirby_OnOffPart.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CKirby;

class CKirby_Sword final : public CKirby_OnOffPart
{
	GENERATED_BODY(CKirby_Sword)

public:
	struct KIRBY_SWORD_DESC : public CKirby_OnOffPart::KIRBY_ONONFFPART_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_Sword";
	static constexpr const wchar_t* Kirby_PartTag = L"Sword";

private:
	CKirby_Sword(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Sword(const CKirby_Sword& Prototype);
	virtual ~CKirby_Sword() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	virtual void Set_LadderState(CKirby* pKirby, _bool bOn) override;

public:
	//À±¼®Çö Ãß°¡
	void Reset_HitList() { m_HitTargets.clear(); }
	void Set_HitBox(_bool bOn);

private:
	HRESULT Ready_Components();
	HRESULT Ready_HitBox();
	void	SetUp_HitBox_Callback();

private:
	CCollider* m_pHitBox{};

	unordered_set<CGameObject*> m_HitTargets;

public:
	static CKirby_Sword* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free();
};

NS_END