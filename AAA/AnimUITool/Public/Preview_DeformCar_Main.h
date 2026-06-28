#pragma once

#include "AnimUITool_Defines.h"
#include "GameObject.h"
#include "Kirby_States.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(AnimUITool)

class CPreview_DeformCar_Main final : public CGameObject
{
	GENERATED_BODY(CPreview_DeformCar_Main)

public:
	enum DEFORM_CAR_MESH : _uint
	{
		MESH_CAR = 0,
		MESH_KIRBY = 1,
		MESH_TIRES = 2,
		MESH_END
	};

private:
	template<typename T>
	static T Clamp_State(T eState)
	{
		_int iValue = ETOI(eState);
		if (iValue < 0)
			iValue = 0;
		else if (iValue >= ETOI(T::END))
			iValue = ETOI(T::END) - 1;

		return static_cast<T>(iValue);
	}

public:
	typedef struct tagPreviewDeformCarDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szKirbyShaderTag = { L"Proto_Shader_Kirby" };
		const _tchar* szPBRShaderTag = { L"Proto_Shader_AnimMesh" };
		const _tchar* szModelTag = { L"Proto_Model_DeformCar" };
		_uint iProtoLevel = { 0 };
		_wstring strAnimEvents = {};
	} PREVIEW_DEFORMCAR_DESC;

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Preview_DeformCar";

protected:
	CPreview_DeformCar_Main(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPreview_DeformCar_Main(const CPreview_DeformCar_Main& Prototype);
	virtual ~CPreview_DeformCar_Main() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Update(_float fTimeDelta) override {}
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	void Set_Eye(KIRBY_EYE_STATE eState) { m_eEye = Clamp_State(eState); }
	KIRBY_EYE_STATE Get_Eye() const { return m_eEye; }
	_bool Is_MeshVisible(_uint iMeshIndex) const;
	void Set_MeshVisible(_uint iMeshIndex, _bool bVisible);
	void Set_AllMeshVisible(_bool bVisible);
	void Set_SoloMesh(_uint iMeshIndex);

private:
	CShader* m_pKirbyShaderCom = { nullptr };
	CShader* m_pPBRShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	CTexture* m_pEyeTextureCom = { nullptr };
	CTexture* m_pEyeMaskTextureCom = { nullptr };

	PREVIEW_DEFORMCAR_DESC m_Desc{};
	KIRBY_EYE_STATE m_eEye = { KIRBY_EYE_STATE::IDLE };
	vector<_bool> m_MeshVisible;

	_float4 m_vBodyColor = { 1.f, 0.45f, 0.55f, 1.f };
	_float4 m_vFootColor = { 1.f, 0.1882353f, 0.3764706f, 1.f };
	_float4 m_vBlushColor = { 1.f, 0.25f, 0.4f, 1.f };

private:
	HRESULT Ready_Components();
	HRESULT Ready_EyeTextures();
	HRESULT Bind_CommonResources(CShader* pShader);
	HRESULT Render_PBRMesh(_uint iMeshIndex);
	HRESULT Render_KirbyMesh(_uint iMeshIndex);

public:
	static CPreview_DeformCar_Main* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END
