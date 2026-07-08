#pragma once

#include "PartObject.h"

#include "GameContent_const.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKirby;

class CKirby_OnOffPart abstract : public CPartObject
{
	GENERATED_BODY_ABSTRACT(CKirby_OnOffPart)

public:
	struct KIRBY_ONONFFPART_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4* pSocketBoneMatrix{};
		const _float* pHitFlashIntensity{};
		const _float3* pHitFlashColor{};
	};

protected:
	CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_OnOffPart(const CKirby_OnOffPart& Prototype);
	virtual ~CKirby_OnOffPart() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render_Shadow() override;

public:
	CAnimator* Get_Animator() { return m_pAnimatorCom; }

	void Set_SocketBoneMatrix(const _float4x4* pSocketBoneMatrix) { m_pSocketBoneMatrix = pSocketBoneMatrix; }

	void PartOnOff(_bool bOn) { m_bOn = bOn; }

	virtual void Put_OnBack(CKirby* pKirby, _bool bOn);

protected:
	struct KIRBY_PART_COMPONENT_DESC°¡
	{
		SHADER_DESC   tShaderDesc{};
		const _tchar* szModelProtoTag{};
		_bool         bCreateAnimator{};
		const _tchar* szAnimEventFile{};
	};

	CShader* m_pShaderCom{};
	CModel* m_pModelCom{};
	CAnimator* m_pAnimatorCom{};

	const _float4x4* m_pSocketBoneMatrix{};

	const _float* m_pHitFlashIntensity{};
	const _float3* m_pHitFlashColor{};

	_bool m_bOn{};

protected:
	HRESULT Ready_PartComponents(const KIRBY_PART_COMPONENT_DESC°¡& tDesc);
	HRESULT Bind_ShaderResources();

protected:
	virtual void Free();
};

NS_END