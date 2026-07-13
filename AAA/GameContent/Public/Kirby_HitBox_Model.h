#pragma once

#include "Kirby_Deform_Model.h"

NS_BEGIN(Engine)
NS_END

NS_BEGIN(Client)

class CKirby;

class CKirby_HitBox_Model abstract : public CKirby_Deform_Model
{
	GENERATED_BODY_ABSTRACT(CKirby_HitBox_Model)

public:
	struct KIRBY_HITBOX_MODEL_DESC : public CKirby_Deform_Model::KIRBY_DEFORM_MODEL_DESC
	{
	};

protected:
	CKirby_HitBox_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_HitBox_Model(const CKirby_HitBox_Model& Prototype);
	virtual ~CKirby_HitBox_Model() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Late_Update(_float fTimeDelta) override;

public:
	void Set_HitBoxEnabled(_bool bOn);

protected:
	CCollider* m_pHitBox{};

protected:
	virtual void Free();
};

NS_END
