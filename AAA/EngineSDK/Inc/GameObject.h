#pragma once

#include "Transform.h"
#include "Property.h"
#include "Renderer.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CObject_Manager;

class ENGINE_DLL CGameObject abstract : public CBase
{
	friend class CObject_Manager;
public:
	typedef struct tagGameObjectDesc : public CTransform::TRANSFORM_DESC
	{
		_uint		iFlag = {};
	}GAMEOBJECT_DESC;

	typedef struct tagEngineObjectData
	{
		wstring							strPrototypeTag = {};
		wstring							strLayerTag = {};
		CGameObject::GAMEOBJECT_DESC	Desc = {};
	}ENGINE_OBJECT_DATA;

protected:
	CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CGameObject(const CGameObject& Prototype);
	virtual ~CGameObject() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Update(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void Late_Update(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_Shadow() { return S_OK; };
	virtual HRESULT Render_OutlineMask() { return S_OK; }
	virtual HRESULT Render_OutlineHull() { return S_OK; }
	virtual void Initialize_NaviPlacement() {};

public:
	void Set_OwnerLevelLayer(_uint iLevelIndex, const wstring& strLayerTag)
	{
		m_iLevelIndex = iLevelIndex;
		m_strLayerTag = strLayerTag;
	}

	void Set_Dead() { m_bDead = true; }

public:
	CTransform* Get_Transform() { return m_pTransformCom; }
	PROJ_TYPE Get_ProjType() const { return m_eProjType; }
	_uint Get_LevelIndex() const { return m_iLevelIndex; }
	const wstring& Get_LayerTag() { return m_strLayerTag; }
	const wstring& Get_ObjectTag() { return m_strObjectTag; }

	template<typename T>
	T* Get_Component(const wstring& strComponentTag)
	{
		auto iter = m_Components.find(strComponentTag);
		if (iter == m_Components.end())
			return nullptr;
		return dynamic_cast<T*>(iter->second);
	}

	void Set_Active(_bool bActive) { m_bActive = bActive; }
	_bool Is_Active() const { return m_bActive; }

public:
	_bool Is_Dead() const { return m_bDead; }

public: //파싱 관련 함수
	void Copy_ObjectData(ENGINE_OBJECT_DATA* pOutData);

public: // 프로퍼티함수
	virtual vector<FPROPERTY>& Get_Properties() const
	{
		static vector<FPROPERTY> empty;
		return empty;
	}

	virtual const void* Get_PropertyPtr(size_t uOffset) const { return nullptr; }
	virtual void* Get_PropertyPtr(size_t uOffset) { return nullptr; }

	virtual json Serialize() const;
	virtual void Deserialize(const json& j);

public: // Collision 콜백
	virtual void Enter_Collision(CGameObject* pOther) {};
	virtual void On_Collision(CGameObject* pOther) {};
	virtual void Exit_Collision(CGameObject* pOther) {};

protected:
	_bool   m_bDead		  = { false };
	_uint   m_iLevelIndex = {};
	wstring m_strLayerTag = {};
	wstring m_strObjectTag = {};

protected:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	CGameInstance_Proxy* m_pGameInstance_Proxy = { nullptr };

	unordered_map<wstring, CComponent*> m_Components;

	CTransform* m_pTransformCom = { nullptr };
	PROJ_TYPE	m_eProjType = {};
	_uint		m_iFlag = {};

	_bool		m_bActive = { true };

protected:
	HRESULT Add_Component_Internal(const wstring& strComTag, CComponent* pComponent);
	CComponent* Add_Component_Internal(_uint iProtoLevel, const wstring& strPrototypeTag, const wstring& strComTag, void* pArg);

	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) = 0;

	

protected: // 이벤트 버스 헬퍼
	void	Subscribe_Event(const wstring& strEventTag, function<void(void*)> Handler);
	void	UnSubscribe_Event(const wstring& strEventTag);

	virtual HRESULT Ready_Events() { return S_OK; }

protected:
	template<typename T>
	T* Find_Component(const wstring& strComponentTag)
	{
		auto iter = m_Components.find(strComponentTag);
		if (iter == m_Components.end())
			return nullptr;
		return dynamic_cast<T*>(iter->second);
	}

	template<typename T>
	T* Add_Component(_uint iProtoLevel, const wstring& strPrototypeTag, const wstring& strComTag, void* pArg = nullptr)
	{
		if (CComponent* pResult = Add_Component_Internal(iProtoLevel, strPrototypeTag, strComTag, pArg))
			return dynamic_cast<T*>(pResult);
		return nullptr;
	}

	template<typename T>
	T* Add_Component(const wstring& strComTag, CComponent* pComponent)
	{
		if (FAILED(Add_Component_Internal(strComTag, pComponent)))
			return nullptr;
		return dynamic_cast<T*>(pComponent);
	}

private:
	unordered_map<wstring, SUBHANDLE> m_Subhandles;

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free() override;
};

NS_END