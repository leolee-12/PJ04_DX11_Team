#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CAnimator;
NS_END

NS_BEGIN(Client)

// 케이지 메쉬 파트 (기존 CBoss_Cage 파트의 리네임)
class CBoss_Cage_Body final : public CMonsterPart
{
    GENERATED_BODY(CBoss_Cage_Body)

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Boss_Cage_Body";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Prototype_Component_Model_Cage";
    static constexpr const wchar_t* PART_TAG = L"Body";

private:
    CBoss_Cage_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CBoss_Cage_Body(const CBoss_Cage_Body& Prototype);
    virtual ~CBoss_Cage_Body() = default;

public:
    virtual HRESULT             Initialize_Prototype() override;
    virtual HRESULT             Initialize(void* pArg) override;
    virtual HRESULT             Render() override;

public:
    virtual void                Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }

public:
    void    Set_RenderBird(_bool b) { m_bRenderBird = b; }

private:
    _bool m_bRenderBird = { false };

private:
    virtual HRESULT             Ready_Components() override;
    virtual HRESULT             Bind_ShaderResources() override;

public:
    static CBoss_Cage_Body* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END