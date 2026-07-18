#pragma once
#include "MapObject.h"
#include "MapGimmick_Defines.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)

class CMapGimmickSection final : public CMapObject
{
	GENERATED_BODY(CMapGimmickSection)

	PROPERTY(_bool, m_bRenderable, L"Renderable", L"MapGimmickSection")

public:
	enum class BREAK_STATE
	{
		INTACT,
		BREAKING,
		BROKEN,
	};

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_MapGimmickSection";
	static constexpr const _tchar* LAYER_TAG = L"Layer_MapGimmickSection";

	struct MAP_GIMMICK_SECTION_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		const MAP_GIMMICK_SECTION_ENTRY* pEntry = { nullptr };
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
	CMapGimmickSection(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMapGimmickSection(const CMapGimmickSection& Prototype);
	virtual ~CMapGimmickSection() = default;

	virtual HRESULT Initialize(void* pArg) override;
	virtual HRESULT Validate_Initialized() override;

public:
	virtual void    Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual HRESULT	Render_Shadow() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	const _tchar* Get_SectionName() const { return m_pEntry->pSectionName; }
	const MAP_GIMMICK_SECTION_ENTRY* Get_Entry() const { return m_pEntry; }

private:
	const MAP_GIMMICK_SECTION_ENTRY* m_pEntry = { nullptr };

	BREAK_STATE m_eBreakState = BREAK_STATE::INTACT;

	_uint m_iModelProtoLevel = {};

	vector<BREAK_FRAGMENT> m_Fragments;
	vector<_uint> m_MeshFragmentIndices;

	CCollider* m_pTrigger = nullptr;

	static constexpr _uint INVALID_FRAGMENT_INDEX = static_cast<_uint>(-1);

private:
	virtual const _tchar*	Get_ModelProtoTag() const override;
	virtual _uint			Get_ModelProtoLevel() const override;

private:
	HRESULT	Ready_Trigger();
	void	On_TriggerEnter(CCollider* pOther);
	HRESULT	Ready_Fragments();

	void	Update_Trigger();
	void	Update_Fragments(_float fTimeDelta);
	void	Start_Break();
	const BREAK_FRAGMENT*	Find_Fragment(_uint iMesh) const;
	_float4x4	Build_FragmentWorldMatrix(const BREAK_FRAGMENT& Fragment, _fmatrix BreakWallWorld) const;

public:
	static CMapGimmickSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END