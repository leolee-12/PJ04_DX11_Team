#pragma once

#include "Component.h"
#include "Bounding_AABB.h"
#include "Bounding_OBB.h"
#include "Bounding_Sphere.h"

NS_BEGIN(Engine)

class CGameObject;

class ENGINE_DLL CCollider final : public CComponent
{
public:
	typedef struct tagColliderDesc
	{
		_float3			vCenter = {};
		_float3			vSize = { 1.f, 1.f, 1.f };
		_float3			vRadians = {};
		_float			fRadius = { 0.5f };
		CGameObject*	pOwner = { nullptr };
	}COLLIDER_DESC;

private:
	CCollider(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CCollider(const CCollider& Prototype);
	virtual ~CCollider() = default;
public:
	virtual HRESULT Initialize_Prototype(COLLIDER eType);
	virtual HRESULT Initialize(void* pArg);

public:
	void Update(_fmatrix TransformMatrix);

public:
	_bool Intersect(CCollider* pTarget);

public:
	void Notify_Enter(CCollider* pOther);
	void Notify_Stay(CCollider* pOther);
	void Notify_Exit(CCollider* pOther);

public:
	void Swap_ContactFrame();                       
	void Add_Contact(CCollider* pOther);            
	void Dispatch_Callbacks();                      
	void Remove_FromContacts(CCollider* pOther);    
	const unordered_set<CCollider*>& Get_CurrContacts() const { return m_CurrContacts; }
	const unordered_set<CCollider*>& Get_PrevContacts() const { return m_PrevContacts; }

	void Mark_Registered(_uint Group) { m_RegisteredGroup = Group; m_bRegistered = true; }
	void Mark_Unregistered() { m_bRegistered = false; }

#ifdef _DEBUG
public:
	virtual HRESULT Render() override;
#endif

private:
	COLLIDER			m_eType = { COLLIDER::END };
	CBounding*			m_pBounding = { nullptr };
	_bool				m_isColl = { false };

	CGameObject*		m_pOwner = { nullptr };

private:
	unordered_set<CCollider*> m_PrevContacts;
	unordered_set<CCollider*> m_CurrContacts;

	_uint m_RegisteredGroup = { UINT_MAX };
	_bool m_bRegistered		= { false };

#ifdef _DEBUG
private:
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
	BasicEffect*						m_pEffect = { nullptr };
	ID3D11InputLayout*					m_pInputLayout = { nullptr };

#endif

public:
	static CCollider* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, COLLIDER eType);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
	

};

NS_END