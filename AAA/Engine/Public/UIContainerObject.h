#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class CUIPartObject;

class ENGINE_DLL CUIContainerObject abstract : public CGameObject
{
protected:
	CUIContainerObject(ID3D11Device * pDevice, ID3D11DeviceContext * pContext);
	CUIContainerObject(const CUIContainerObject& Prototype);
	virtual ~CUIContainerObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	const unordered_map<_wstring, CUIPartObject*>& Get_UIPartObjects() const
	{
		return m_UIPartObjects;
	}

	void Clear_UIPartObjects();
	

public:
	virtual json Serialize() const override;
	virtual void Deserialize_Internal(const json& j) override;

protected:
	typedef struct tagUIPartPrototypeInfo
	{
		_uint		iPrototypeLevel = {};
		_wstring	strPrototypeTag = {};
	}UI_PART_PROTOTYPE_INFO;

	unordered_map<_wstring, CUIPartObject*>				m_UIPartObjects;
	unordered_map<_wstring, UI_PART_PROTOTYPE_INFO>		m_UIPartPrototypeInfos;

protected:
	HRESULT Add_UIPartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag,
		const _wstring& strPartTag, void* pArg = nullptr);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free();

};

NS_END