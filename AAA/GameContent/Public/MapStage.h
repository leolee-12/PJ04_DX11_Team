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

	virtual HRESULT Initialize(void* pArg) override;
	HRESULT			Validate_Initialized();

public:
	virtual void Late_Update(_float fTimeDelta) override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

public:
	const vector<CMapSection*>&		Get_Sections() const { return m_Sections; }
	const _wstring&					Get_StageName() const { return m_strStageName; }

#ifdef _DEBUG
	void Set_EditorSoloSection(CMapSection* pSection);
	void Clear_EditorSoloSection();
	_bool Should_RenderSection(const CMapSection* pSection) const;
	void Clear_EditorSoloMeshAllSections();
#endif

public:
	virtual json Serialize() const override;
	virtual void Deserialize_Internal(const json& j) override;

private:
	vector<CMapSection*>	m_Sections;
	_wstring				m_strStageName;
	_float4x4				m_LastWorldMatrix = {};
	_bool					m_bSnapshotValid = { false };

#ifdef _DEBUG
	CMapSection* m_pEditorSoloSection = nullptr;
#endif

private:
	virtual HRESULT	Ready_Events() override;
	HRESULT			Ready_Sections(const MAP_STAGE_DESC* pDesc);
	void			Refresh_SectionTransforms();
	void			Submit_VisibleSections();

	void			On_GimmickSectionBreak(const _tchar* pShellSectionName);

public:
	static CMapStage*		Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void	Free() override;
};

NS_END
