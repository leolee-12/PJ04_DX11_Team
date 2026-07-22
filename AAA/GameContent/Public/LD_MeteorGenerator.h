#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CCollider;
NS_END


NS_BEGIN(Client)

struct LD_SPAWN_SPEC;
class CLightShaft;
class CProjectile;

class CLD_MeteorGenerator final : public CLevelDesignObject
{
    GENERATED_BODY(CLD_MeteorGenerator)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_MeteorGenerator";
    static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";

private:
    CLD_MeteorGenerator(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLD_MeteorGenerator(const CLD_MeteorGenerator& Prototype);
    virtual ~CLD_MeteorGenerator() = default;

    virtual HRESULT                 Initialize_Prototype() override;
    virtual HRESULT                 Initialize(void* pArg) override;

public:
    virtual void                    Update(_float fTimeDelta) override;
    virtual void                    Late_Update(_float fTimeDelta) override;
    virtual void                    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

protected:
    virtual void                    On_LDEventReceived(const _wstring& strEventTag) override;

private:
    HRESULT                         Ready_Trigger();
    void                            Fire_Rock();
    void                            Ensure_LightShaft();

private:
    LD_METEOR_DESC                  m_tMeteorDesc = {};
    CLightShaft*                    m_pLightShaft = { nullptr };
    _float                          m_fShaftTimer = { 0.f };
    _float                          m_fShaftDur = { 0.f };
    _bool                           m_bFalling = { false };
    CCollider*                      m_pTrigger = { nullptr };
    _bool                           m_bFired = { false };
    _float                          m_fFireTimer = { 0.f };
    _bool                           m_bLarge = { false };
    _bool                           m_bHeroInRange = { false };
    _bool                           m_bFiredOnce = { false };
    _bool                           m_bEventFired = { false };
    _bool                           m_bStopped = { false };

public:
    static void                     Register_LevelDesignSpecs();
    static _bool                    Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
    static CGameObject*             Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    static CLD_MeteorGenerator*     Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*            Clone(void* pArg) override;

protected:
    virtual void                    Free() override;
};

NS_END
