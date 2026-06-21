#include "LevelDesignObject.h"

NS_BEGIN(Client)

CLevelDesignObject::CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CLevelDesignObject::CLevelDesignObject(const CLevelDesignObject& Prototype)
	: CGameObject(Prototype)
	, m_tLevelDesignDesc(Prototype.m_tLevelDesignDesc)
{
}

HRESULT CLevelDesignObject::Initialize_Prototype()
{
	m_eProjType = PROJ_TYPE::PERSPEC;
	return S_OK;
}

HRESULT CLevelDesignObject::Initialize(void* pArg)
{
	if (nullptr == pArg)
		return E_FAIL;

	const LD_OBJECT_DESC* pDesc = static_cast<const LD_OBJECT_DESC*>(pArg);

	m_tLevelDesignDesc = *pDesc;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	return S_OK;
}

_wstring CLevelDesignObject::Make_LevelDesignObjectKey() const
{
	if (m_tLevelDesignDesc.iUid != 0)
		return m_tLevelDesignDesc.strSection + L":" + to_wstring(m_tLevelDesignDesc.iUid);

	return m_tLevelDesignDesc.strSection
		+ L":"
		+ m_tLevelDesignDesc.strEntryKey
		+ L":"
		+ m_tLevelDesignDesc.strObjectName;
}

void CLevelDesignObject::Free()
{
	__super::Free();
}

NS_END