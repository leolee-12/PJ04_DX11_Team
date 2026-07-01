#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

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

	void						Set_Eye(_uint iIndex) { m_iEyeIndex = (iIndex < EYE_COUNT) ? iIndex : 0; }
	_uint						Get_Eye() const { return m_iEyeIndex; }

private:
	virtual HRESULT				Ready_Components() override;

public:
	static CNormalEnemy_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;

private:
	static constexpr _uint		EYE_COUNT = { 5 };
	CTexture*					m_pEyeTextureCom = { nullptr };
	_uint						m_iEyeIndex = { 0 }; // 총 5개 (OPEN, HALF_CLOSED, CLOSED, HIDDEN, SHARPED)

protected:
	virtual void				Free() override;

};

NS_END