#pragma once

#include "GameContent_Defines.h"

#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(Client)

enum class KIRBY_MESH { BODY_BIG, BODY, BODY_VACUUM,
	MOUTH_ANGRY, MOUTH_NORMAL, MOUTH_OPEN, MOUTH_SMILE_CLOSE, MOUTH_SMILE_OPEN,
	LIMBS };
enum class KIRBY_BODY_STATE { NORMAL, STUFFED, INHALE, END };
enum class KIRBY_EYE_STATE { IDLE, DOUBT, BLINK, CLOSE, ANGRY, SURPRISED, SADNESS, END };
enum class KIRBY_MOUTH_STATE { IDLE, OPEN, ANGRY, SMILE_OPEN, SMILE_CLOSE, END };

class CKirby_Body final : public CPartObject
{
	GENERATED_BODY(CKirby_Body)

public:
	struct KIRBY_BODY_DESC : public CPartObject::PARTOBJECT_DESC
	{
	};

	static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_Kirby_Body";

private:
	CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_Body(const CKirby_Body& Prototype);
	virtual ~CKirby_Body() = default;

private:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName) const;

	CAnimator* Get_Animator() { return m_pAnimatorCom; }

public:
	void Set_Body(KIRBY_BODY_STATE eState) { m_eBody = eState; }
	void Set_Eye(KIRBY_EYE_STATE eState) { m_eEye = eState; }
	void Set_Mouth(KIRBY_MOUTH_STATE eState) { m_eMouth = eState; }

	KIRBY_BODY_STATE Get_BodyState() const { return m_eBody; }
	KIRBY_EYE_STATE Get_Eye() const { return m_eEye; }
	KIRBY_MOUTH_STATE Get_Mouth() const { return m_eMouth; }

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

	HRESULT Set_VisibleMeshes();

private:
	CShader*		m_pShaderCom{};
	CModel*			m_pModelCom{};
	CAnimator*		m_pAnimatorCom{};

private:
	vector<_bool>	m_VisibleMeshes;

	CTexture*		m_pEyeTextureCom{};
	CTexture*		m_pEyeMaskTextureCom{};

	KIRBY_BODY_STATE m_eBody{};
	KIRBY_EYE_STATE m_eEye{};
	KIRBY_MOUTH_STATE m_eMouth{};

private:
	_float4 m_vBodyColor = { 1.f, 0.1882353f, 0.3764706f, 1.f };
	_float4 m_vFootColor = { 0.67f, 0.f, 0.f, 1.f };
	_float4 m_vBlushColor = { 1.f, 0.05f, 0.12f, 1.f };

public:
	static CKirby_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
protected:
	virtual void Free();
};

NS_END
