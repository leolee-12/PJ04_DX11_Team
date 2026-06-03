#pragma once
#include "AnimUITool_Defines.h"
#include "GameObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(AnimUITool)

class CPreview_Actor final : public CGameObject
{
	GENERATED_BODY(CPreview_Actor)
	PROPERTY(ANIM_INDEX, m_iAnimationIndex, L"Animation_Index", L"Animation")

public:
	typedef struct tagPreviewDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar*		szShaderTag = { L"Proto_Shader_AnimMesh" };
		const _tchar*		szModelTag = { L"Proto_Model_0" };
		_uint				iProtoLevel = { 0 };
		MODEL				eType = { MODEL::ANIM };
		_wstring			strAnimEvents = {};
	}PREVIEW_DESC;

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Preview_Actor";

protected:
	CPreview_Actor(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPreview_Actor(const CPreview_Actor& Prototype);
	virtual ~CPreview_Actor() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override {}
	virtual void			Update(_float fTimeDelta) override {}
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	MODEL					Get_Type() const { return m_Desc.eType; }
	PREVIEW_SHADER_MODE		Get_PreviewShaderMode() const { return m_ePreviewShaderMode; }
	void					Set_PreviewShaderMode(PREVIEW_SHADER_MODE eMode) { m_ePreviewShaderMode = eMode; }

	_int					Get_PreviewPassOverride() const { return m_iPreviewPassOverride; }
	void					Set_PreviewPassOverride(_int iPass) { m_iPreviewPassOverride = iPass; }

	const _char*			Get_ResolvedShaderName() const;
	_uint					Get_ResolvedPassIndex() const;

	void					Reset_MeshVisibility();
	_bool					Is_MeshVisible(_uint iMesh) const;
	void					Set_MeshVisible(_uint iMesh, _bool bVisible);
	void					Set_AllMeshVisible(_bool bVisible);
	void					Set_SoloMesh(_uint iMesh);

	CAnimator*				Get_Animator() const { return m_pAnimatorCom; }
	CModel*					Get_Model() const { return m_pModelCom; }

private:
	CShader*				m_pShaderCom = { nullptr };				// 현재 선택된 렌더용 셰이더
	CShader*				m_pAnimShaderCom = { nullptr };
	CShader*				m_pNonAnimShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CAnimator*				m_pAnimatorCom = { nullptr };

	PREVIEW_DESC			m_Desc{};
	PREVIEW_SHADER_MODE		m_ePreviewShaderMode = { PREVIEW_SHADER_MODE::AUTO };
	_int					m_iPreviewPassOverride = { -1 };	// -1 = AUTO
	vector<_bool>			m_MeshVisible;

private:
	virtual HRESULT			Ready_Events() override { return S_OK; }
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();
	HRESULT					Bind_MaterialOrClear(const _char* pConstantName, _uint iMeshIndex, MTEX_TYPE eType, _uint iIndex = 0);

	CShader*				Get_ResolvedShader() const;

public:
	static CPreview_Actor*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END