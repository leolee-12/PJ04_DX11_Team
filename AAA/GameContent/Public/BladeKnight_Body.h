#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CBladeKnight_Body final : public CMonsterPart
{
    GENERATED_BODY(CBladeKnight_Body)

public:
    struct BLADEKNIGHT_BODY_DESC : public CMonsterPart::MONSTERPART_DESC {};

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BladeKnight_Body";

private:
    CBladeKnight_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBladeKnight_Body(const CBladeKnight_Body& Prototype);
    virtual ~CBladeKnight_Body() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    virtual HRESULT Ready_Components() override;

public:
    static CBladeKnight_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END