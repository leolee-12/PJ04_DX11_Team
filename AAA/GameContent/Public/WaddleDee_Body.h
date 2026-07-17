#pragma once
#include "GameContent_Defines.h"
#include "PartObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

class CWaddleDee_Body final : public CPartObject
{
	GENERATED_BODY(CWaddleDee_Body)

public:
	struct WADDLEDEE_BODY_DESC : public CPartObject::PARTOBJECT_DESC
	{
	};

	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_WaddleDee_Body";
	static constexpr const _tchar* PART_TAG = L"Body";

private:
	CWaddleDee_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CWaddleDee_Body(const CWaddleDee_Body& Prototype);
	virtual ~CWaddleDee_Body() = default;

public:
	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual void	Update(_float fTimeDelta) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual HRESULT	Render_Shadow() override;

	virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override { pOutData->strPrototypeTag = PROTOTYPE_TAG; }

public:
	CAnimator* Get_Animator() const { return m_pAnimatorCom; }
	_bool Has_Animation(const _char* pAnimName) const;
	const _float4x4* Get_BoneMatrixPtr(const _char* pBoneName) const;

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	CShader*	m_pShaderCom = { nullptr };
	CModel*		m_pModelCom = { nullptr };
	CAnimator*	m_pAnimatorCom = { nullptr };

public:
	static CWaddleDee_Body*	Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject*	Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END