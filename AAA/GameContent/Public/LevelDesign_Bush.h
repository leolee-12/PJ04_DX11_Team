#pragma once
#include "LevelDesignObject.h"
#include "Damageable.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
class CCollider;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Bush
    : public CLevelDesignObject
    , public IDamageable
{
    GENERATED_BODY(CLevelDesign_Bush);

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Bush";
    static constexpr const _tchar* BUSH_S_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_S";
    static constexpr const _tchar* BUSH_M_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_M";
    static constexpr const _tchar* BUSH_L_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_L";
    static constexpr const _tchar* CUT_S_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_S";
    static constexpr const _tchar* CUT_M_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_M";
    static constexpr const _tchar* CUT_L_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_L";

private:
    enum BUSH_STATE { BASIC, CUT, _COUNT };

private:
    CLevelDesign_Bush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLevelDesign_Bush(const CLevelDesign_Bush& Prototype);
    virtual ~CLevelDesign_Bush() = default;

    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArwlsg) override;
    virtual HRESULT Validate_Initialized() override;

public:
    virtual void    Update(_float fTimeDelta) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

    virtual void Damaged(const ATTACK_INFO& tInfo) override;

    static void Register_LevelDesignSpecs();
    static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
    static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
    CShader* m_pShaderComs[BUSH_STATE::_COUNT] = { nullptr };
    CModel* m_pModelComs[BUSH_STATE::_COUNT] = { nullptr };
    CAnimator* m_pAnimatorCom = { nullptr };
    CCollider* m_pHurtBoxCom = { nullptr };

    LD_BUSH_DESC m_tBushDesc = {};
    BUSH_STATE m_eState = { BUSH_STATE::BASIC };

private:
    virtual void Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const override;

    HRESULT Ready_Components();
    HRESULT Ready_RenderComponents();
    HRESULT Ready_HurtBox();

    HRESULT Bind_ShaderResources(BUSH_STATE eSlot);
    HRESULT Render_Model(BUSH_STATE eSlot);
    const _tchar* Resolve_ModelProtoTag(BUSH_STATE eSlot) const;
    MODEL Resolve_ModelType(BUSH_STATE eSlot) const;

public:
    static CLevelDesign_Bush* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};


NS_END