#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CWaddleDee_Hat final : public CMonsterPart
{
	GENERATED_BODY(CWaddleDee_Hat)

public:
	struct WADDLEDEE_HAT_DESC : public CMonsterPart::MONSTERPART_DESC
	{
		const _tchar* szModelProtoTag = { nullptr };
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_WaddleDee_Hat";
	static constexpr const _tchar* PART_TAG = L"Hat";

private:
	CWaddleDee_Hat(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee_Hat(const CWaddleDee_Hat& Prototype);
	virtual ~CWaddleDee_Hat() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render() override;

public:
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

private:
	virtual HRESULT Ready_Components() override;

private:
	const _tchar* m_szModelProtoTag = { nullptr };

public:
	static CWaddleDee_Hat* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END