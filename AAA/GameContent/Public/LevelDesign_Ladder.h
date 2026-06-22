#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END


NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Ladder : public CLevelDesignObject
{
	GENERATED_BODY(CLevelDesign_Ladder)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Ladder";
	static constexpr const _tchar* BOT_MODEL_PROTO_TAG = L"Proto_Component_Model_Ladder_Bottom";
	static constexpr const _tchar* MID_MODEL_PROTO_TAG = L"Proto_Component_Model_Ladder_Middle";
	static constexpr const _tchar* TOP_MODEL_PROTO_TAG = L"Proto_Component_Model_Ladder_Top";

private:
	enum SEGMENT { TOP, MID, BOT, _COUNT };

private:
	CLevelDesign_Ladder(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesign_Ladder(const CLevelDesign_Ladder& Prototype);
	virtual ~CLevelDesign_Ladder() = default;

public:
	virtual HRESULT	Initialize_Prototype() override;
	virtual HRESULT	Initialize(void* pArg) override;
	virtual void	Late_Update(_float fTimeDelta) override;
	virtual HRESULT	Render() override;
	virtual void	Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

	static void Register_LevelDesignSpecs();
	static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
	static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
	CShader* m_pShaderCom = nullptr;
	CModel* m_ModelComs[SEGMENT::_COUNT] = { nullptr };

	LD_LADDER_DESC m_tLadderDesc = {};
	_uint m_iModelProtoLevel = { ETOUI(LEVEL::GAMEPLAY) };
	_float m_fSegmentStepY = { 1.f };

private:
	virtual HRESULT	Validate_Desc() override;

	HRESULT	Ready_Components();
	HRESULT Resolve_SegmentStepY();
	HRESULT	Bind_ShaderResources(const _float4x4& WorldMatrix);
	HRESULT	Render_Model(CModel* pModel);

public:
	static CLevelDesign_Ladder* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;

protected:
	virtual void Free() override;
};

NS_END