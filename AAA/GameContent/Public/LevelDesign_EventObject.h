#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_EventObject : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_EventObject);

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_EventObject";
	static constexpr const _tchar* LEVEL1BOSSDEMOBG_MODEL_PROTO_TAG = L"Proto_Component_Model_Level1BossDemoBg";
	static constexpr const _tchar* SLOPEBOARD_A_MODEL_PROTO_TAG = L"Proto_Component_Model_SlopeBoard_A";
	static constexpr const _tchar* SLOPEBOARD_C_MODEL_PROTO_TAG = L"Proto_Component_Model_SlopeBoard_C";

private:
	CLevelDesign_EventObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_EventObject(const CLevelDesign_EventObject& Prototype);
	virtual ~CLevelDesign_EventObject() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArwlsg) override;
	virtual void    Update(_float fTimeDelta) override;
	virtual void    Late_Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	_bool Play_EventAnimation(_uint iAnimSlot, _bool bLoop);
	_bool Play_EventAnimation(const _string& strAnimName, _bool bLoop);

private:
	CShader* m_pShaderCom = { nullptr };
	CModel* m_pModelCom = { nullptr };
	CAnimator* m_pAnimatorCom = { nullptr };

	LD_EVENTOBJECT_DESC m_tEventObjectDesc = {};

private:
	virtual HRESULT Validate_Desc() override;

	HRESULT Ready_Components();
	HRESULT Ready_Level1BossDemoBg();
	HRESULT Ready_SlopeBoardA();
	HRESULT Ready_SlopeBoardC();

	HRESULT Bind_ShaderResources();

	HRESULT Render_Default();
	HRESULT Render_Level1BossDemoBg();
	HRESULT Render_SlopeBoardA(); 
	HRESULT Render_SlopeBoardC();
	HRESULT Render_Mesh(_uint iMeshIndex);
	HRESULT Render_Mesh(_uint iMeshIndex, _uint iAnimPassIndex);

public:
	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

	static CLevelDesign_EventObject* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};


NS_END