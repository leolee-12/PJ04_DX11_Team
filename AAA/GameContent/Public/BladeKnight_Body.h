#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CBladeKnight_Body final : public CPartObject
{
	GENERATED_BODY(CBladeKnight_Body)

public:
	struct BLADEKNIGHT_BODY_DESC : public CPartObject::PARTOBJECT_DESC
	{

	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BladeKnight_Body";

private:
	CBladeKnight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CBladeKnight_Body(const CBladeKnight_Body& Prototype);
	virtual ~CBladeKnight_Body() = default;

private:
	virtual HRESULT				Initialize_Prototype() override;
	virtual HRESULT				Initialize(void* pArg) override;

public:
	virtual void				Priority_Update(_float fTimeDelta) override;
	virtual void				Update(_float fTimeDelta) override;
	virtual void				Late_Update(_float fTimeDelta) override;
	virtual HRESULT				Render() override;
	virtual void				Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
	{
		pOutData->strPrototypeTag = PROTOTYPE_TAG;
	}

public:
	CAnimator*					Get_Animator() { return m_pAnimatorCom; }
	CModel*						Get_Model() { return m_pModelCom; }
	const _float4x4*			Get_BoneMatrixPtr(const _char* pBoneName) const;

private:
	HRESULT						Ready_Components();
	HRESULT						Bind_ShaderResources();

private:
	CShader*					m_pShaderCom = { nullptr };
	CAnimator*					m_pAnimatorCom = { nullptr };
	CModel*						m_pModelCom = { nullptr };

public:
	static CBladeKnight_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*		Clone(void* pArg) override;
	
protected:
	virtual void				Free() override;
};

NS_END