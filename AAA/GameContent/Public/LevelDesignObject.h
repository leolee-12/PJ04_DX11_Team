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

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	_wstring		Make_LevelDesignObjectKey() const;

	const LD_OBJECT_DESC& Get_LevelDesignDesc() const { return m_tLevelDesignDesc; }

protected:
	LD_OBJECT_DESC	m_tLevelDesignDesc = {};

protected:
	virtual HRESULT Validate_Desc() { return S_OK; };

protected:
	virtual void Free() override;
};

NS_END