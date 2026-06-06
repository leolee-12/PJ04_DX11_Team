#pragma once
#include "MapSection.h"

NS_BEGIN(Client)

class CLIENT_DLL CMapStage final : public CGameObject
{
public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MapStage";

private:
	CMapStage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapStage(const CMapStage& Prototype);
	virtual ~CMapStage() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	const vector<CMapSection*>&		Get_Sections() const { return m_Sections; }
	const _wstring&					Get_StageName() const { return m_strStageName; }

#ifdef _DEBUG
	const MAP_STAGE_PROFILE& Get_Profile() const { return m_Profile; }
#endif

private:
	vector<CMapSection*>	m_Sections;
	_wstring				m_strProtoTag = { PROTOTYPE_TAG };
	_wstring				m_strStageName;
	_uint					m_iSectionProtoLevel = {};

#ifdef _DEBUG
	MAP_STAGE_PROFILE		m_Profile = {};
#endif

private:
	virtual HRESULT	Ready_Events() override { return S_OK; }
	HRESULT			Ready_Sections(const MAP_STAGE_DESC* pDesc);
	void			Refresh_SectionTransforms() const;
	void			Submit_VisibleSections();

#ifdef _DEBUG
	void    Reset_ProfileFrame();
	void    Count_MainSubmitted(RENDERID eRenderID);
#endif

public:
	static CMapStage*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void	Free() override;
};

NS_END
