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

class CAbility_Model : public CPartObject
{
    GENERATED_BODY(CAbility_Model)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Ability_Model";

    struct ABILITY_MODEL_DESC : public CPartObject::PARTOBJECT_DESC
    {
        COPY_ABILITY_TYPE       eAbility = { COPY_ABILITY_TYPE::NONE };
        const _tchar*           szModelProtoTag = { nullptr };
        const _float4x4*        pSocketBoneMatrix = { nullptr }; 
    };

protected:
    struct PART_SETUP
    {
        SHADER_DESC   tShader;                       
        const _tchar* szModelProtoTag = nullptr;
        _bool         bAnimated = true;              
        const _tchar* szAnimEventFile = nullptr;     
    };

protected:
    CAbility_Model(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAbility_Model(const CAbility_Model& Prototype);
    virtual ~CAbility_Model() = default;

public:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;   
    virtual void            Update(_float fTimeDelta) override;
    virtual void            Late_Update(_float fTimeDelta) override;
    virtual HRESULT         Render() override;                  
    virtual HRESULT         Render_Shadow() override;

    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

    void                    Set_ModelRenderActive(_bool bActive) { m_bRenderActive = bActive; }

public:
    CAnimator*              Get_Animator() const { return m_pAnimatorCom; }
    CModel*                 Get_Model()    const { return m_pModelCom; }
    const _float4x4*        Get_BoneMatrixPtr(const _char* pBoneName) const;

protected:
    HRESULT                 Ready_MeshPart(const PART_SETUP& tSetup);
    HRESULT                 Ready_Components();
    HRESULT                 Bind_ShaderResources();

protected:
    CShader*                m_pShaderCom = { nullptr };
    CModel*                 m_pModelCom = { nullptr };
    CAnimator*              m_pAnimatorCom = { nullptr };
    const _float4x4*        m_pSocketBoneMatrix = { nullptr };

    COPY_ABILITY_TYPE       m_eAbility = { COPY_ABILITY_TYPE::NONE };
    const _tchar*           m_szModelProtoTag = { nullptr };
    
    _bool                   m_bRenderActive = { true };
    _int                    m_iShadowPassIdx = { 7 };

public:
    static CAbility_Model*  Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END