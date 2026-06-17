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

public:
	const LD_COMMON_DESC&	Get_LevelDesignDesc() const { return m_tLevelDesignDesc; }
	LD_CATEGORY				Get_LevelDesignCategory() const { return m_tLevelDesignDesc.eCategory; }
	const _wstring&			Get_LevelDesignObjectName() const { return m_tLevelDesignDesc.strObjectName; }
	const _wstring&			Get_LevelDesignKind() const { return m_tLevelDesignDesc.strKind; }
	const _wstring&			Get_LevelDesignSection() const { return m_tLevelDesignDesc.strSection; }
	const _wstring&			Get_LevelDesignEntryKey() const { return m_tLevelDesignDesc.strEntryKey; }
	_uint					Get_LevelDesignUid() const { return m_tLevelDesignDesc.iUid; }
	_uint					Get_TargetRailUid() const { return m_tLevelDesignDesc.iTargetRailUid; }

protected:
	const LD_COMMON_DESC&	Desc() const { return m_tLevelDesignDesc; }
	LD_COMMON_DESC&			Desc() { return m_tLevelDesignDesc; }

protected:
	LD_COMMON_DESC			m_tLevelDesignDesc = {};

protected:
	virtual void Free() override;
};

NS_END