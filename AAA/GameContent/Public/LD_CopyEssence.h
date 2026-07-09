#pragma once
#include "LevelDesignObject.h"
#include "GameContent_Defines.h"

NS_BEGIN(physx)
class PxRigidStatic;
NS_END

NS_BEGIN(Engine)
class CShader;
class CModel;
class CAnimator;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;
class CEssenceBubble;

class CLD_CopyEssence final : public CLevelDesignObject
{
	GENERATED_BODY(CLD_CopyEssence)

public:
    static constexpr const _tchar* OBJECT_NAME      = L"CopyEssence";
    static constexpr const _tchar* PROTOTYPE_TAG    = L"Proto_LevelDesign_CopyEssence";
    static constexpr const _tchar* MODEL_PROTO_TAG  = L"Proto_Component_Model_CopyEssence";
    static constexpr const _tchar* LAYER_TAG        = L"Layer_LevelDesign_Gimmick";

private:
    enum class STATE { APPEAR, ENABLE };

private:
    CLD_CopyEssence(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLD_CopyEssence(const CLD_CopyEssence& Prototype);
    virtual ~CLD_CopyEssence() = default;

    virtual HRESULT             Initialize_Prototype() override;
    virtual HRESULT             Initialize(void* pArg) override;
    virtual HRESULT             Validate_Initialized() override;

public:
    virtual void                Update(_float fTimeDelta) override;
    virtual void                Late_Update(_float fTimeDelta) override;
    virtual HRESULT             Render() override;
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

    COPY_ABILITY_TYPE           Get_Ability() const { return m_eAbility; }

    static void                 Register_LevelDesignSpecs();
    static _bool                Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
    static CGameObject*         Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
    CShader*                    m_pShaderCom = { nullptr };
    CModel*                     m_pModelCom = { nullptr };
    CAnimator*                  m_pAnimatorCom = { nullptr };
    physx::PxRigidStatic*       m_pRigidStatic = { nullptr };

    LD_EVENTOBJECT_DESC         m_tCopyEssenceDesc = {};

    STATE                       m_eState = { STATE::ENABLE };
    COPY_ABILITY_TYPE           m_eAbility = { COPY_ABILITY_TYPE::NONE };

    CEssenceBubble*             m_pBubble = { nullptr };
    _bool                       m_bBubbleSpawned = { false };

private:
    HRESULT                     Ready_RenderComponents();
    HRESULT                     Ready_RigidStatic();
    void                        Release_RigidStatic();
    HRESULT                     Bind_ShaderResources();
    HRESULT                     Render_Model();

    void                        Apply_AbilityKind(const _wstring& strKind);
    void                        Spawn_AbilityBubble();

public:
    static CLD_CopyEssence*     Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END