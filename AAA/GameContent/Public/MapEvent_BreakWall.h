#pragma once
#include "MapObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMapEvent_BreakWall final : public CMapObject
{
	GENERATED_BODY(CMapEvent_BreakWall)

	PROPERTY(_bool, m_bRenderable, L"Renderable", L"MapEvent_BreakWall")

public:
	enum class BREAK_STATE
	{
		INTACT,
		BREAKING,
		BROKEN,
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MapEvent_BreakWall";
	static constexpr const _tchar* LAYER_TAG = L"Layer_MapEvent_BreakWall";

	static constexpr const _tchar* STAGE12_STAGE_NAME = L"Stage1-2_MapStage";
	static constexpr const _tchar* STAGE12_SECTION_NAME = L"GsDefault_4";
	static constexpr const _tchar* STAGE12_MODEL_PROTO_TAG = L"Prototype_Component_Model_MapEvent_BreakWall_Stage1-2_GsDefault_4";
	static constexpr const _tchar* STAGE12_MODEL_PATH = L"../../Resources/Map/Stage1-2/Section/GsDefault_4.ysh";
	static constexpr const _tchar* STAGE12_OBJECT_TAG = L"MapEvent_BreakWall_Stage1-2_GsDefault_4";

	struct MAP_EVENT_BREAK_WALL_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		_uint iModelProtoLevel = {};
		_bool bRenderable = true;
	};

	struct BREAK_FRAGMENT
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

private:
	CMapEvent_BreakWall(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapEvent_BreakWall(const CMapEvent_BreakWall& Prototype);
	virtual ~CMapEvent_BreakWall() = default;

	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	const _tchar*	Get_SectionName() const { return STAGE12_SECTION_NAME; }

private:
	BREAK_STATE m_eBreakState = BREAK_STATE::INTACT;

	_uint m_iModelProtoLevel = {};

	vector<BREAK_FRAGMENT> m_Fragments;
	vector<_uint> m_MeshFragmentIndices;

	CCollider* m_pBoostTrigger = nullptr;

	static constexpr _uint INVALID_FRAGMENT_INDEX = static_cast<_uint>(-1);

private:
	virtual const _tchar*	Get_ModelProtoTag() const override;
	virtual _uint			Get_ModelProtoLevel() const override;

private:
	HRESULT	Ready_BoostTrigger();
	void	On_BoostTriggerEnter(CCollider* pOther);
	void	Start_Break();
	HRESULT	Ready_Fragments();
	const BREAK_FRAGMENT* Find_Fragment(_uint iMesh) const;

public:
	static CMapEvent_BreakWall* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END