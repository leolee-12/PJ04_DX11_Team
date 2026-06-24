#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CNormalEnemy_Body final : public CMonsterPart
{
	GENERATED_BODY(CNormalEnemy_Body)

public:
	struct NORMALENEMY_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_NormalEnemy_Body";

private:
	CNormalEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNormalEnemy_Body(const CNormalEnemy_Body& Prototype);
	virtual ~CNormalEnemy_Body() = default;

private:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

	virtual HRESULT				Render() override;                  // 특수 렌더 필요 시 자식이 오버라이드

public:
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	virtual HRESULT				Ready_Components() override;

public:
	static CNormalEnemy_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

protected:
	virtual void				Free() override;

};

NS_END