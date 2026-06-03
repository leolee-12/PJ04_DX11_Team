#pragma once

#include "MapObject.h"
#include "Map_Defines.h"

NS_BEGIN(Client)

class CLIENT_DLL CMapSection final : public CMapObject
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
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render_Shadow() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	void			Refresh_WorldBounds();
	void			Set_ParentMatrix(const _float4x4* pParentMatrix);
	void			Refresh_CombinedWorldMatrix();
#ifdef _DEBUG
	void			Reset_FrameProfile();
#endif

	const BoundingBox&		Get_WorldBounds() const { return m_WorldBounds; }
	MAP_SECTION_TYPE		Get_SectionType() const { return m_eSectionType; }
	RENDERID				Get_RenderID() const { return m_eRenderID; }
	_bool					Is_ShadowCaster() const { return m_bCastShadow; }
	_bool					Is_Culling() const { return m_bEnableCulling; }
	_bool					Is_Renderable() const { return m_bRenderable; }
	const wstring&			Get_SectionName() const { return m_strSectionName; }

#ifdef _DEBUG
	const MAP_SECTION_PROFILE& Get_Profile() const { return m_Profile; }
#endif

private:
	virtual const _tchar*	Get_ModelProtoTag() const override;
	virtual HRESULT			Ready_Events() override { return S_OK; }
	virtual HRESULT			Bind_WorldMatrix() override;
	HRESULT					Ready_ModelPrototype(const MAP_SECTION_DESC* pDesc);
	void					Update_LocalBounds();

private:
	_wstring			m_strProtoTag = { PROTOTYPE_TAG };
	_wstring			m_strSectionName;
	_wstring			m_strModelProtoTag;
	_wstring			m_strModelPath;
	MAP_SECTION_TYPE	m_eSectionType = { MAP_SECTION_TYPE::UNKNOWN };
	RENDERID			m_eRenderID = { RENDERID::NONBLEND };
	BoundingBox			m_LocalBounds = {};
	BoundingBox			m_WorldBounds = {};
	const _float4x4*	m_pParentMatrix = {};
	_float4x4			m_CombinedWorldMatrix = {};
	_bool				m_bCastShadow = { false };
	_bool				m_bEnableCulling = { true };
	_bool				m_bRenderable = { true };

#ifdef _DEBUG
	MAP_SECTION_PROFILE	m_Profile = {};
#endif

public:
	static CMapSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
