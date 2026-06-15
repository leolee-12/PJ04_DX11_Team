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
	void			Get_UIPartObjectsInOrder(vector<pair<_wstring, CUIPartObject*>>* pOutParts) const;
	_int			Get_UIPartOrderIndex(const _wstring& strPartTag) const;
	void			Clear_UIPartObjects();
	

public:
	virtual json Serialize() const override;
	virtual void Deserialize_Internal(const json& j) override;

	// 제작 전용: 선택 컨테이너에 파트 생성·편입
	HRESULT Add_Part(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag, const _wstring& strPartTag, void* pArg = nullptr);
	HRESULT Remove_Part(const _wstring& strPartTag);
	HRESULT Rename_Part(const _wstring& strOldTag, const _wstring& strNewTag);

public:
	void              Set_Tilt(_float fTiltXDeg, _float fTiltYDeg) 
	{
		m_fTiltXDeg = fTiltXDeg; m_fTiltYDeg = fTiltYDeg;
	}
	void              Set_PerspDistance(_float fDist) { m_fPerspDist = fDist; }
	_float            Get_TiltX() const { return m_fTiltXDeg; }
	_float            Get_TiltY() const { return m_fTiltYDeg; }
	_float            Get_PerspDistance() const { return m_fPerspDist; }
	const _float4x4* Get_TiltedParentMatrix() const { return &m_TiltedWorldMatrix; }

protected:
	typedef struct tagUIPartPrototypeInfo
	{
		_uint		iPrototypeLevel = {};
		_wstring	strPrototypeTag = {};
	}UI_PART_PROTOTYPE_INFO;

	unordered_map<_wstring, CUIPartObject*>				m_UIPartObjects;
	unordered_map<_wstring, UI_PART_PROTOTYPE_INFO>		m_UIPartPrototypeInfos;
	vector<_wstring> m_UIPartOrder;

protected:
	_float    m_fTiltXDeg = { 0.f };
	_float    m_fTiltYDeg = { 0.f };
	_float    m_fPerspDist = { 0.f };    
	_float4x4 m_TiltedWorldMatrix = {};

protected:
	HRESULT Add_UIPartObject(_uint iPrototypeLevelIndex, const _wstring& strPrototypeTag,
		const _wstring& strPartTag, void* pArg = nullptr);

	virtual void On_UIPartsChanged() {}
	void    Update_TiltedWorldMatrix();

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free();

};

NS_END