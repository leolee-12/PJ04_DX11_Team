#pragma once
#include "MapObject.h"

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Client)

class CLIENT_DLL CMapBreakSection final : public CMapObject
{
	GENERATED_BODY(CMapBreakSection)

public:
	enum class MAP_BREAK_STATE
	{
		INTACT,
		BREAKING,
		BROKEN,
		HIDDEN,
	};

	struct MAP_BREAK_SECTION_DESC : public CGameObject::GAMEOBJECT_DESC
	{
		_wstring strSectionName;
		_wstring wstrModelProtoTag;
		_uint iModelProtoLevel = {};

		_wstring wstrBreakEventTag;

		_bool bRenderable = true;
		_bool bCastShadow = false;

		_bool bUseRigidStatic = true;
		_bool bRigidStaticEnabledAtStart = true;
	};

	struct MAP_BREAK_FRAGMENT
	{
		string strFragmentName;
		string strLocatorName;

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

private:
	virtual const _tchar* Get_ModelProtoTag() const override;
	virtual _uint Get_ModelProtoLevel() const override;
	virtual HRESULT Ready_Events() override;
	virtual _bool Should_RenderMesh(_uint iMesh) const override;

private:
	HRESULT Ready_RigidStatic();
	void Release_RigidStatic();
	void Break_Debug();
	HRESULT Ready_Fragments();
	MAP_BREAK_FRAGMENT* Find_Fragment(_uint iMesh);
	const MAP_BREAK_FRAGMENT* Find_Fragment(_uint iMesh) const;
	_bool Is_FragmentMesh(_uint iMesh) const;

private:
	MAP_BREAK_SECTION_DESC m_tBreakDesc = {};
	MAP_BREAK_STATE m_eBreakState = MAP_BREAK_STATE::INTACT;

	_wstring m_strSectionName;
	_wstring m_strModelProtoTag;
	_uint m_iModelProtoLevel = {};

	_bool m_bRenderable = true;
	_bool m_bCastShadow = false;

	vector<MAP_BREAK_FRAGMENT> m_Fragments;
	vector<_bool> m_FragmentMeshFlags;

	physx::PxRigidStatic* m_pRigidStatic = nullptr;

public:
	static CMapBreakSection* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END