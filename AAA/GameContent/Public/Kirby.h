#pragma once

#include "Character.h"

NS_BEGIN(physx)
class PxController;
NS_END

NS_BEGIN(Client)

class CKirby_Body;

class CKirby final : public CCharacter
{
	GENERATED_BODY(CCharacter)

public:
	struct KIRBY_BODY_DESC : public CContainerObject::COTAINEROBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby";

private:
	CKirby(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CKirby(const CKirby& Prototype);
	virtual ~CKirby() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

private:
	HRESULT Ready_Components();
	HRESULT Ready_PartObjects();
	HRESULT Bind_ShaderResources();

private:
	CKirby_Body* m_pBody{};

	physx::PxController* m_pController = { nullptr };
	_float               m_fVerticalVelocity{ 0.f };   // 중력 누적 속도(아래로 음수)
	_bool                m_bGrounded{ false };

	static constexpr _float CCT_RADIUS = 0.5f;
	static constexpr _float CCT_HEIGHT = 1.0f;
	static constexpr _float MOVE_SPEED = 6.0f;
	static constexpr _float GRAVITY = -20.0f;

public:
	static CKirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
private:
	virtual void Free() override;
};

NS_END