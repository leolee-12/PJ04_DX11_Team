#pragma once
#include "LevelDesign_LoadTypes.h"

NS_BEGIN(Client)

class CLevelDesignObject abstract : public CGameObject
{
	GENERATED_BODY_ABSTRACT(CLevelDesignObject)

protected:
	CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesignObject(const CLevelDesignObject& Prototype);
	virtual ~CLevelDesignObject() = default;

	virtual		HRESULT Initialize_Prototype() override;
	virtual		HRESULT Initialize(void* pArg) override;
	virtual		HRESULT Validate_Initialized();
	_wstring	Make_LevelDesignObjectKey() const;

public:
	const LD_OBJECT_DESC& Get_LevelDesignDesc() const { return m_tLevelDesignDesc; }

protected:
	LD_OBJECT_DESC	m_tLevelDesignDesc = {};

protected:
	virtual void Free() override;
};

NS_END