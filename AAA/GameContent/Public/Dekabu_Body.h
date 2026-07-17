#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CDekabu_Body final : public CMonsterPart 
{
    GENERATED_BODY(CDekabu_Body)

public:
    struct DEKABU_BODY_DESC
        : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Dekabu_Body";

private:
    CDekabu_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CDekabu_Body(const CDekabu_Body& Prototype);
    virtual ~CDekabu_Body() = default;

private:
    virtual HRESULT             Initialize_Prototype() override;
    virtual HRESULT             Initialize(void* pArg) override;
    virtual void                Update(_float fTimeDelta) override;
    virtual HRESULT             Render() override;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    virtual HRESULT             Ready_Components() override;

public:
    static CDekabu_Body*        Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END