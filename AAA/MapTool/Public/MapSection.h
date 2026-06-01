#pragma once

#include "MapSection_Defines.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

class CMapSection final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_MapSection";

private:
	CMapSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapSection(const CMapSection& Prototype);
	virtual ~CMapSection() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	void							Refresh_WorldBounds();
	void							Reset_FrameProfile();
	const BoundingBox&				Get_WorldBounds() const { return m_WorldBounds; }
	MAP_SECTION_TYPE				Get_SectionType() const { return m_eSectionType; }
	RENDERID						Get_RenderID() const { return m_eRenderID; }
	_bool							Is_ShadowCaster() const { return m_bCastShadow; }
	_bool							Is_Culling() const { return m_bEnableCulling; }
	_bool							Is_Renderable() const { return m_bRenderable; }
	const MAP_SECTION_PROFILE&		Get_Profile() const { return m_Profile; }
	const wstring&					Get_SectionName() const { return m_strSectionName; }

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT							Ready_Components(const MAP_SECTION_DESC* pDesc);
	HRESULT							Bind_ShaderResources();
	void							Update_LocalBounds();
	_uint							Get_RenderPassIndex() const;

public:
	static CMapSection*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	wstring							m_strProtoTag = { PROTOTYPE_TAG };
	wstring							m_strSectionName;
	MAP_SECTION_TYPE				m_eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID						m_eRenderID = { RENDERID::NONBLEND };
	CShader*						m_pShaderCom = { nullptr };
	CModel*							m_pModelCom = { nullptr };
	BoundingBox						m_LocalBounds = {};
	BoundingBox						m_WorldBounds = {};
	_bool							m_bCastShadow = { false };
	_bool							m_bEnableCulling = { true };
	_bool							m_bRenderable = { true };
	MAP_SECTION_PROFILE				m_Profile = {};

protected:
	virtual void					Free() override;
};

NS_END
