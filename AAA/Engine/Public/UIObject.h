#pragma once

#include "GameObject.h"

NS_BEGIN(Engine)

class CGameInstance_Proxy;
class CTexture;
class CShader;

class ENGINE_DLL CUIObject abstract : public CGameObject
{
	GENERATED_BODY_ABSTRACT(CUIObject)
	PROPERTY(int, m_iTexIndex, L"TextureIndex", L"Texture")

public:
	typedef struct tagUIObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{

	}UIOBJECT_DESC;

protected:
	CUIObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CUIObject(const CUIObject& Prototype);
	virtual ~CUIObject() = default;

public:
	virtual HRESULT			Initialize_Prototype();
	virtual HRESULT			Initialize(void* pArg);
	virtual void			Priority_Update(_float fTimeDelta);
	virtual void			Update(_float fTimeDelta);
	virtual void			Late_Update(_float fTimeDelta);
	virtual HRESULT			Render();

public:
	_float					Get_ZOrder() const;

	RENDERUIID				Get_RenderLayer() const { return m_eRenderLayer; }
	void					Set_RenderLayer(RENDERUIID eRenderLayer) { m_eRenderLayer = eRenderLayer; }

	_bool					Is_Hovered() const { return m_bHovered; }
	_bool					Is_Clicked() const { return m_bClicked; }

	virtual void			On_Hovered() {}
	virtual void			On_Click() {}
	virtual void			On_Release() {}

protected:
	_float					m_fViewWidth{}, m_fViewHeight{};
	_bool					m_bHovered = { false };
	_bool					m_bClicked = { false };

	RENDERUIID				m_eRenderLayer = { RENDERUIID::MIDDLE };

protected:
	virtual void			Update_UIState(const _float2& fScale);
	HRESULT					Bind_ShaderResource(CShader* pShader, const _char* pConstant, D3DTS eState, PROJ_TYPE eType);
	void					Picking_Update(const _float4x4& WorldMatrix, CTexture* pTex = nullptr, _uint iTexIdx = 0);

public:
	virtual CGameObject* Clone(void* pArg) = 0;
	virtual void Free();
};

NS_END