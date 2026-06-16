#pragma once
#include "Character.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Engine)
class CMovement;
NS_END

NS_BEGIN(Client)

class CGigantEdge_Body;
class CGigantEdge_Sword;
class CGigantEdge_Shield;

class CGigantEdge final : public CCharacter
{
	GENERATED_BODY(CGigantEdge)

public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_GigantEdge";

	static constexpr _float s_fCCT_Radius = 2.f;
	static constexpr _float s_fCCT_Height = 2.f;

private:
	CGigantEdge(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGigantEdge(const CGigantEdge& Prototype);
	virtual ~CGigantEdge() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

	virtual void On_Deserialized() override;

private:
	CGigantEdge_Body* m_pBody = { nullptr };
	CGigantEdge_Sword* m_pSword = { nullptr };
	CGigantEdge_Shield* m_pShield = { nullptr };

	physx::PxController* m_pCCT = { nullptr };

	CMovement* m_pMovement = { nullptr };

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();


};

NS_END

