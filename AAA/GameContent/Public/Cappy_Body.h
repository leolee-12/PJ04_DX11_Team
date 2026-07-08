#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CCappy_Body final : public CMonsterPart
{
	GENERATED_BODY(CCappy_Body)

public:
    struct CAPPY_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Cappy_Body";

private:
    CCappy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCappy_Body(const CCappy_Body& Prototype);
    virtual ~CCappy_Body() = default;

private:
    virtual HRESULT         Initialize_Prototype() override;
    virtual HRESULT         Initialize(void* pArg) override;
    virtual HRESULT         Render() override;

public:
    virtual void            Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    void                    Set_Hidden(_bool b) { m_bIsHidden = b; }
    void                    Set_HatAttached(_bool b) { m_bHatAttached = b; }

    _bool                   Is_Hidden() const { return m_bIsHidden; }
    _bool                   Is_HatAttached() const { return m_bHatAttached; }

private:
    virtual HRESULT         Ready_Components() override;

private:
    _bool                   m_bIsHidden = { false };
    _bool                   m_bHatAttached = { true };


public:
    static CCappy_Body*     Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END