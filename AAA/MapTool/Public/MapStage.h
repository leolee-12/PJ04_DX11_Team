#pragma once

#include "MapSection.h"

NS_BEGIN(Client)

class CMapStage final : public CGameObject
{
public:
	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_MapStage";

private:
	CMapStage(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapStage(const CMapStage& Prototype);
	virtual ~CMapStage() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	const MAP_STAGE_PROFILE&			Get_Profile() const { return m_Profile; }
	const vector<CMapSection*>&		Get_Sections() const { return m_Sections; }
	const wstring&					Get_StageName() const { return m_strStageName; }

private:
	virtual HRESULT Ready_Events() override { return S_OK; }
	HRESULT							Ready_Sections(const MAP_STAGE_DESC* pDesc);
	void							Reset_ProfileFrame();
	void							Submit_VisibleSections();
	_bool							Build_WorldFrustum(BoundingFrustum* pOutFrustum) const;
	void							Count_Submitted(RENDERID eRenderID);

public:
	static CMapStage*				Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	vector<CMapSection*>				m_Sections;
	wstring							m_strProtoTag = { PROTOTYPE_TAG };
	wstring							m_strStageName;
	_uint							m_iSectionProtoLevel = {};
	MAP_STAGE_PROFILE				m_Profile = {};

protected:
	virtual void					Free() override;
};

NS_END
