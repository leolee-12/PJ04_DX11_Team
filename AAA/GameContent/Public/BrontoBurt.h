#pragma once
#include "GameContent_Defines.h"
#include "Monster.h"
#include "RailDataReceiver.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)
class CBrontoBurt_Body;

class CBrontoBurt final : public CMonster, public IRailDataReceiver
{
	GENERATED_BODY(CBrontoBurt)

public:
    struct BRONTOBURT_DESC : public CContainerObject::COTAINEROBJECT_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BrontoBurt";

private:
    CBrontoBurt(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBrontoBurt(const CBrontoBurt& Prototype);
    virtual ~CBrontoBurt() = default;

protected:
    virtual HRESULT     Initialize_Prototype() override;
    virtual HRESULT     Initialize(void* pArg) override;

public:
    virtual void        Priority_Update(_float fTimeDelta) override;
    virtual void        Update(_float fTimeDelta) override;
    virtual void        Late_Update(_float fTimeDelta) override;

    virtual void        Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    virtual _float          Get_CapsuleRadius() const override { return 0.5f; }
    virtual _float          Get_CapsuleHeight() const override { return 0.25f; }
    virtual _float          Get_InteractRadius() const override { return (m_iAIType == 0) ? 0.f : 15.f; }
    virtual _bool           Get_HurtBoxDesc( CAPSULE_DESC& Out) const override;

    virtual CAnimator*      Get_BodyAnimator() const override;

    // IRailDataReceiver
    virtual void            Set_RailDesc(const LD_RAIL_DESC& Desc) override;

protected:
    virtual CMonsterBrain*  Create_Brain() override;
    virtual HRESULT         Create_Movement() override;
    virtual HRESULT         Ready_State() override;
    virtual HRESULT         Ready_AnimEvents() override;

    virtual void            Apply_AIVariation(const _wstring& strVariation) override;

    virtual void            On_Exit(MONSTER_STATE_TYPE eNextState) override;

private:
    virtual HRESULT         Ready_PartObjects() override;
    virtual void			On_Deserialized() override;

private:
    CBrontoBurt_Body*       m_pBody = { nullptr };

public:
    static CBrontoBurt*     Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END