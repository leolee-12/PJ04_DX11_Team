#pragma once
#include "ContainerObject.h"
#include "GameContent_Defines.h"

NS_BEGIN(Engine)
class CCollider;
NS_END

NS_BEGIN(Client)
class CAbility_Model;

class CAbility_Bubble abstract : public CContainerObject
{
	GENERATED_BODY_ABSTRACT(CAbility_Bubble)

public:
    static constexpr const _tchar* PART_MODEL_TAG = L"Part_AbilityModel";

public:
    struct ABILITY_BUBBLE_DESC : public CContainerObject::COTAINEROBJECT_DESC
    {
        COPY_ABILITY_TYPE   eAbility = { COPY_ABILITY_TYPE::NONE };
        const _tchar*       szModelProtoTag = { nullptr };
    };

protected:
    CAbility_Bubble(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CAbility_Bubble(const CAbility_Bubble& Prototype);
    virtual ~CAbility_Bubble() = default;

public:
    virtual HRESULT     Initialize_Prototype() override;
    virtual HRESULT     Initialize(void* pArg) override;
    virtual void        Late_Update(_float fTimeDelta) override;

public:
    CAbility_Model*     Get_AbilityModel() const { return m_pModelPart; }

    void                Set_Ability(COPY_ABILITY_TYPE eAbility) { m_eAbility = eAbility; }
    COPY_ABILITY_TYPE   Get_Ability() const { return m_eAbility; }

    _bool               Is_Available() const { return m_bAvailable; }
    void                Set_RenderActive(_bool bActive);

protected:
    CAbility_Model*     m_pModelPart = { nullptr };
    CCollider*          m_pCollider = { nullptr };
    COPY_ABILITY_TYPE   m_eAbility = { COPY_ABILITY_TYPE::NONE };
    const _tchar*       m_szModelProtoTag = { nullptr };

    COLLISION_LAYER     m_eCollLayer = { COLLISION_LAYER::ESSENCE_BUBBLE };

    _bool               m_bAvailable = { true };
    _float              m_fTimer = { 0.f };

protected:
    HRESULT             Ready_PartObjects();
    HRESULT				Ready_Collider();
    virtual void        SetUp_Collider_CallBack() = 0;

    virtual void        Free() override;

};

NS_END