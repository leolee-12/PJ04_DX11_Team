#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"
#include "GameContent_const.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)
class CDropStar_Body final : public CPartObject
{
	GENERATED_BODY(CDropStar_Body)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_DropStar_Body";
	static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_DropStar";

	struct DROPSTAR_BODY_DESC : public CPartObject::PARTOBJECT_DESC
	{
	};

protected:
	CDropStar_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CDropStar_Body(const CDropStar_Body& Prototype);
	virtual ~CDropStar_Body() = default;

public:
	virtual HRESULT			Initialize_Prototype() override;
	virtual HRESULT			Initialize(void* pArg) override;
	virtual void			Update(_float fTimeDelta) override;
	virtual void			Late_Update(_float fTimeDelta) override;
	virtual HRESULT			Render() override;
	virtual HRESULT			Render_Shadow() override;

	virtual void			Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
	{
		pOut->strPrototypeTag = PROTOTYPE_TAG;
	}

	CAnimator*				Get_Animator() const { return m_pAnimatorCom; }

protected:
	HRESULT					Ready_Components();
	HRESULT					Bind_ShaderResources();

protected:
	CShader*				m_pShaderCom = { nullptr };
	CModel*					m_pModelCom = { nullptr };
	CAnimator*				m_pAnimatorCom = { nullptr };


private:
	_float4					m_vDiffuse = { 1.f, 0.85f, 0.15f, 1.f };
	_float3					m_vMRA = { 0.f, 0.45f, 1.f };
	_float4					m_vEmissive = { 0.35f, 0.28f, 0.f, 1.f };

	_int					m_iShadowPassIdx = { 7 };

public:
	static CDropStar_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void			Free() override;
};

NS_END