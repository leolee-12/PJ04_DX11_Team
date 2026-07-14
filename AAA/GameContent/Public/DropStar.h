#pragma once
#include "ContainerObject.h"
#include "Inhalable.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CDropStar_Manager;
class CDropStar_Body;

class CDropStar final : public CContainerObject, public IInhalable
{
	GENERATED_BODY(CDropStar)

public:
	static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_DropStar";
	static constexpr const _tchar* BODY_PART_TAG = L"Part_DropStar_Body";

private:
    CDropStar(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CDropStar(const CDropStar& Prototype);
    virtual ~CDropStar() = default;

public:
    virtual HRESULT     Initialize_Prototype() override;
    virtual HRESULT     Initialize(void* pArg) override;
    virtual void        Update(_float fTimeDelta) override;
    virtual void        Late_Update(_float fTimeDelta) override;

    virtual void        Copy_PrototypeName(ENGINE_OBJECT_DATA* pOut) override
    {
        pOut->strPrototypeTag = PROTOTYPE_TAG;
    }

    void                        Set_Pool(CDropStar_Manager* pPool, _uint iLevel, const _wstring& strKey);
    void                        Activate(const _float3& vPos, _float fDelay = 0.f);
    void                        Launch(const _float3& vDir);


    // Inhalable 
    virtual _bool               Can_BeInhaled(const INHALE_QUERY& q) const override;
    virtual void                Be_Captured(CGameObject* pInhaler) override;
    virtual COPY_ABILITY_TYPE   Get_CopyAbility() const override  { return COPY_ABILITY_TYPE::NONE;  }
    virtual CGameObject*        Get_GameObject() override { return this; }
    virtual _float3             Get_SpatPivotOffset() const override  {  return _float3(0.f, 0.f, 0.f);  }
    virtual void                On_SpatBegin() override;
    virtual void                On_SpatEnd() override;

private:
    HRESULT                     Ready_Collider();
    HRESULT                     Ready_PartObjects();

    void                        Update_Captured(_float fTimeDelta);
    void                        On_Swallowed();
    void                        Despawn();
    void                        Return_ToPool();

private:
    CDropStar_Body*             m_pBody = { nullptr };
    CCollider*                  m_pCollider = { nullptr };   // »Ì¿‘ ∞®¡ˆ hurtbox

    CDropStar_Manager*          m_pPool = { nullptr };
    _uint                       m_iPoolLevel = {};
    _wstring                    m_strPoolKey;

    CGameObject*                m_pCaptor = { nullptr };
    
    COLLISION_LAYER             m_eCollLayer = { COLLISION_LAYER::DROP_STAR };     

    _float3                     m_vBaseScale = {};
    _float                      m_fDelay = { 0.f };     
    _float                      m_fTimer = { 0.f };
    _float                      m_fPullSpeed = { 0.f };
    _float                      m_fScaleRatio = { 1.f };
    _bool                       m_bCaptured = { false };
    _bool                       m_bAvailable = { true };

    static constexpr _float     s_fPullAccel = { 40.f };
    static constexpr _float     s_fMinScaleRatio = { 0.45f };
    static constexpr _float     s_fShrinkLerp = { 2.f };
    static constexpr _float     s_fDeSpawnTime = { 10.f };

public:
    static CDropStar*           Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};
NS_END