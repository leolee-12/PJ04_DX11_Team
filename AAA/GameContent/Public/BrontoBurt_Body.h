#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
class CTexture;
NS_END

NS_BEGIN(Client)

class CBrontoBurt_Body final : public CMonsterPart
{
    GENERATED_BODY(CBrontoBurt_Body)

public:
    struct BRONTOBURT_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_BrontoBurt_Body";

private:
    CBrontoBurt_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBrontoBurt_Body(const CBrontoBurt_Body& Prototype);
    virtual ~CBrontoBurt_Body() = default;

private:
    virtual HRESULT                 Initialize_Prototype() override;
    virtual HRESULT                 Initialize(void* pArg) override;
    virtual void                    Update(_float fTimeDelta) override;
    virtual HRESULT                 Render() override;

public:
    virtual void                    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

    void						    Set_Eye(_uint iIndex) { m_iEyeIndex = (iIndex < EYE_COUNT) ? iIndex : 0; }
    _uint						    Get_Eye() const { return m_iEyeIndex; }

private:
    CTexture*                       m_pEyeTextureCom = { nullptr };

    static constexpr _uint		    EYE_COUNT = { 2 };
    _uint                           m_iEyeIndex = { 0 };

private:
    virtual HRESULT                 Ready_Components() override;

public:
    static CBrontoBurt_Body*        Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*            Clone(void* pArg) override;

protected:
    virtual void                    Free() override;
};

NS_END