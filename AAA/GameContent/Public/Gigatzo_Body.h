#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CGigatzo_Body final : public CMonsterPart
{
    GENERATED_BODY(CGigatzo_Body)

public:
    struct GIGATZO_BODY_DESC
        : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Gigatzo_Body";

private:
    CGigatzo_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CGigatzo_Body(const CGigatzo_Body& Prototype);
    virtual ~CGigatzo_Body() = default;

private:
    virtual HRESULT             Initialize_Prototype() override;
    virtual HRESULT             Initialize(void* pArg) override;
    virtual void                Update(_float fTimeDelta) override;
    virtual HRESULT             Render() override;
    virtual HRESULT             Render_Shadow() override;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

private:
    virtual HRESULT             Ready_Components() override;

public:
    static CGigatzo_Body*       Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END