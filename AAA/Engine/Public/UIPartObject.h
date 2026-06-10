#pragma once

#include "UIObject.h"

NS_BEGIN(Engine)

class ENGINE_DLL CUIPartObject abstract : public CUIObject
{
public:
	typedef struct tagUIPartObjectDesc : public CUIObject::UIOBJECT_DESC
	{
		const _float4x4* pParentMatrix;
	}UI_PARTOBJECT_DESC;

protected:
	CUIPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIPartObject(const CUIPartObject& Prototype);
	virtual ~CUIPartObject() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;

public:
	void					Set_ParentMatrix(const _float4x4* pParentMatrix);
	_bool					Has_ParentMatrix() const { return m_pParentMatrix != nullptr; }

protected:
	const _float4x4*		m_pParentMatrix = {};
	_float4x4				m_CombinedWorldMatrix = {};

protected:
	void					Compute_CombinedWorldMatrix(_fmatrix ChildMatrix);

public:
	virtual CGameObject*	Clone(void* pArg) = 0;
	virtual void			Free();
};

NS_END