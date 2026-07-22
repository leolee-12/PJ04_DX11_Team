#pragma once
#include "MonsterPart.h"

NS_BEGIN(Client)

class CSirKibble_Body final : public CMonsterPart
{
    GENERATED_BODY(CSirKibble_Body)

public:
    struct SIRKIBBLE_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_SirKibble_Body";

private:
    CSirKibble_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CSirKibble_Body(const CSirKibble_Body& Prototype);
    virtual ~CSirKibble_Body() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    virtual HRESULT         Ready_Components() override;

public:
    static CSirKibble_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END
