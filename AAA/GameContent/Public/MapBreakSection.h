#pragma once
#include "MapObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMapBreakSection final : public CMapObject
{
	GENERATED_BODY(CMapBreakSection)

	PROPERTY(_bool, m_bRenderable, L"Renderable", L"MapSection")

public:
	enum class MAP_BREAK_STATE
	{
		INTACT,
		BREAKING,
		BROKEN,
	};

	static constexpr const _tchar* STAGE12_STAGE_NAME = L"Stage1-2_MapStage";
	static constexpr const _tchar* STAGE12_SECTION_NAME = L"GsDefault_4";
	static constexpr const _tchar* STAGE12_MODEL_PROTO_TAG = L"Prototype_Component_Model_MapBreakSection_Stage1-2_GsDefault_4";
	static constexpr const _tchar* STAGE12_MODEL_PATH = L"../../Resources/Map/Stage1-2/Section/GsDefault_4.ysh";
	static constexpr const _tchar* STAGE12_OBJECT_TAG = L"MapBreakSection_Stage1-2_GsDefault_4";

	struct MAP_BREAK_SECTION_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring strSectionName;
		_wstring wstrModelProtoTag;
		_uint iModelProtoLevel = {};

		_bool bRenderable = true;
	};

	struct MAP_BREAK_FRAGMENT
	{
		string strFragmentName;

		_float3 vPivot = {};
		vector<_uint> MeshIndices;

		_float3 vOffset = {};
		_float3 vVelocity = {};
		_float3 vAngularVelocity = {};
		_float4 vRotation = { 0.f, 0.f, 0.f, 1.f };

		_bool bActive = false;
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MapBreakSection";
	static constexpr const _tchar* LAYER_TAG = L"Layer_MapBreakSection";

private:
	CMapBreakSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapBreakSection(const CMapBreakSection& Prototype);
	virtual ~CMapBreakSection() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	const MAP_BREAK_SECTION_DESC& Get_Desc() const { return m_tBreakDesc; }
	const _wstring& Get_SectionName() const { return m_strSectionName; }
	_bool Is_Renderable() const { return m_bRenderable; }
	void Set_Renderable(_bool bRenderable) { m_bRenderable = bRenderable; }

private:
	MAP_BREAK_SECTION_DESC m_tBreakDesc = {};
	MAP_BREAK_STATE m_eBreakState = MAP_BREAK_STATE::INTACT;

	_wstring m_strSectionName;
	_wstring m_strModelProtoTag;
	_uint m_iModelProtoLevel = {};

	vector<MAP_BREAK_FRAGMENT> m_Fragments;
	vector<_uint> m_MeshFragmentIndices;

	CCollider* m_pBoostTrigger = nullptr;

	static constexpr _uint INVALID_FRAGMENT_INDEX = static_cast<_uint>(-1);

private:
	virtual const _tchar* Get_ModelProtoTag() const override;
	virtual _uint Get_ModelProtoLevel() const override;
	virtual _bool Should_RenderMesh(_uint iMesh) const override;

private:
	HRESULT Ready_BoostTrigger();
	void On_BoostTriggerEnter(CCollider* pOther);
	void Start_Break();
	HRESULT Ready_Fragments();
	const MAP_BREAK_FRAGMENT* Find_Fragment(_uint iMesh) const;
	_bool Is_FragmentMesh(_uint iMesh) const;

public:
	static CMapBreakSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END