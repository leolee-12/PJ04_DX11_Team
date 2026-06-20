#pragma once

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Navigation.h"
#include "VIBuffer_Rect.h"
#include "VIBuffer_Terrain.h"
#include "VIBuffer_Trail.h"
#include "VIBuffer_Point.h"
#include "Collider.h"
#include "Effect_Container.h"
#include "Animator.h"
#include "Movement.h"
#include "RigIdBody.h"
#include "Controller.h"
#include <shared_mutex>

/* 1. 원형객체(CGameObject, CComponent)를 보관한다. */
/* 2. 픽된 원형객체를 복제하여 리턴해준다. */

NS_BEGIN(Engine)

class CPrototype_Manager final : public CBase
{
private:
	CPrototype_Manager();
	virtual ~CPrototype_Manager() = default;

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag, CBase* pPrototype);
	CBase* Clone_Prototype(PROTOTYPE eType, _uint iLevelIndex, const _wstring& strPrototypeTag, void* pArg);
	void Clear(_uint iLevelIndex);

public:
	_bool Has_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);

private:
	size_t										m_iNumLevels = {};
	unordered_map<_wstring, CBase*>*			m_pPrototypes = { nullptr };
	shared_mutex								m_Mutex;
	typedef unordered_map<_wstring, CBase*>	PROTOTYPES;

private:
	CBase* Find_Prototype(_uint iLevelIndex, const _wstring& strPrototypeTag);

public:
	static CPrototype_Manager* Create(_uint iNumLevels);
	virtual void Free() override;
};

NS_END