#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

class CKabu_Body final : public CMonsterPart
{
    GENERATED_BODY(CKabu_Body)

public:
    struct KABU_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Kabu_Body";

private:
    CKabu_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CKabu_Body(const CKabu_Body& Prototype);
    virtual ~CKabu_Body() = default;

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

    void                        Set_Visible(_bool bVisible) { m_bVisible = bVisible; }

private:
    virtual HRESULT             Ready_Components() override;

private:
    _bool                       m_bVisible = { false };         // 워프 중일 때 안보이게 하기 위함

public:
    static CKabu_Body*          Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END
