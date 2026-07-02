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

class CKirby_OnOffPart abstract : public CPartObject
{
	GENERATED_BODY_ABSTRACT(CKirby_OnOffPart)

public:
	struct KIRBY_ONONFFPART_DESC : public CPartObject::PARTOBJECT_DESC
	{
		const _float4x4* pSocketBoneMatrix{ nullptr };
		const _float* pHitFlash = { nullptr };
		const _float3* pHitFlashColor = { nullptr };
	};

protected:
	CKirby_OnOffPart(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CKirby_OnOffPart(const CKirby_OnOffPart& Prototype);
	virtual ~CKirby_OnOffPart() = default;

protected:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;

public:
	virtual void Priority_Update(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

public:
	virtual void PartOnOff(_bool bOn) { m_bOn = bOn; }

	void Set_SocketBoneMatrix(const _float4x4* pSocketBoneMatrix) { m_pSocketBoneMatrix = pSocketBoneMatrix; }

protected:
	struct PART_SETUP
	{
		SHADER_DESC   tShader;                       
		const _tchar* szModelProtoTag = nullptr;
		_bool         bAnimated = true;              // 애니 파츠 여부의 단일 기준(정적이면 false)
		const _tchar* szAnimEventFile = nullptr;     // 옵션: 이벤트 트랙 json 경로(생성 여부엔 무관)
	};

	CShader* m_pShaderCom{};
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };

	const _float4x4* m_pSocketBoneMatrix{ nullptr };

	const _float* m_pHitFlash = { nullptr };
	const _float3* m_pHitFlashColor = { nullptr };

	_bool m_bOn{};

protected:
	HRESULT Ready_MeshPart(const PART_SETUP& tSetup);
	HRESULT Bind_ShaderResources();

protected:
	virtual void Free();
};

NS_END