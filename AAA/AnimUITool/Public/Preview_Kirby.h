#pragma once
#include "AnimUITool_Defines.h"
#include "GameObject.h"
#include "Character_States.h"
#include "Kirby_States.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(AnimUITool)

class CPreview_Kirby final : public CGameObject
{
	GENERATED_BODY(CPreview_Kirby)

public:
	enum KIRBY_MESH : int
	{
		KMESH_BODY_BIG = 0,
		KMESH_BODY = 1,
		KMESH_BODY_VACUUM = 2,
		KMESH_MOUTH_ANGRY = 3,
		KMESH_MOUTH_NORMAL = 4,
		KMESH_MOUTH_OPEN = 5,
		KMESH_MOUTH_SMILE_CL = 6,
		KMESH_MOUTH_SMILE_OP = 7,
		KMESH_LIMBS = 8,
		END
	};

private:
	// iIntParam 깨질 수도 있어서 조정!
	template<typename T>
	static T Clamp_State(T e)
	{
		_int v = ETOI(e);
		if (v < 0)
			v = 0;
		else if (v >= ETOI(T::END))
			v = ETOI(T::END) - 1;
		return static_cast<T>(v);
	}

public:
	typedef struct tagPreviewKirbyDesc : public CGameObject::GAMEOBJECT_DESC
	{
		const _tchar* szShaderTag = { L"Proto_Shader_Kirby" };
		const _tchar* szModelTag = { L"Proto_Model_Kirby" };
		_uint				iProtoLevel = { 0 };
		_wstring			strAnimEvents = {};
	}PREVIEW_KIRBY_DESC;

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Preview_Kirby";

protected:
	CPreview_Kirby(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CPreview_Kirby(const CPreview_Kirby& Prototype);
	virtual ~CPreview_Kirby() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Priority_Update(_float fTimeDelta) override {}
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	// 커비 Mesh 변경에 필요한 함수들
	void					Set_Body(KIRBY_BODY_STATE eState) { m_eBody = Clamp_State(eState); }
	void					Set_Mouth(KIRBY_MOUTH_STATE eState) { m_eMouth = Clamp_State(eState); }
	void					Set_Eye(KIRBY_EYE_STATE eState) { m_eEye = Clamp_State(eState); }

	KIRBY_BODY_STATE		Get_Body() const { return m_eBody; }
	KIRBY_MOUTH_STATE		Get_Mouth() const { return m_eMouth; }
	KIRBY_EYE_STATE			Get_Eye() const { return m_eEye; }

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };
	CTexture* m_pEyeTextureCom = { nullptr };
	CTexture* m_pEyeMaskTextureCom = { nullptr };

	PREVIEW_KIRBY_DESC		m_Desc{};

	KIRBY_BODY_STATE		m_eBody = { KIRBY_BODY_STATE::NORMAL };
	KIRBY_MOUTH_STATE		m_eMouth = { KIRBY_MOUTH_STATE::IDLE };
	KIRBY_EYE_STATE			m_eEye = { KIRBY_EYE_STATE::IDLE };

	vector<bool>			m_MeshVisible;

	// Shader cbuffer 색상
	_float4					m_vBodyColor = { 1.f, 0.45f, 0.55f, 1.f };
	_float4					m_vFootColor = { 1.f, 0.1882353f, 0.3764706f, 1.f };
	_float4					m_vBlushColor = { 1.f, 0.25f, 0.4f, 1.f };

private:
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();
	HRESULT					Ready_EyeTextures();
	void					Resolve_VisibleMeshes();

public:
	static CPreview_Kirby* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END