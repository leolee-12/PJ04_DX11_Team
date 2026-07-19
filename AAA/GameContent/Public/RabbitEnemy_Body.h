#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

class CRabbitEnemy_Body final : public CMonsterPart
{
    GENERATED_BODY(CRabbitEnemy_Body)

public:
    struct RABBITENEMY_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_RabbitEnemy_Body";

private:
    CRabbitEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRabbitEnemy_Body(const CRabbitEnemy_Body& Prototype);
    virtual ~CRabbitEnemy_Body() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }
    void Set_Eye(_uint iIndex)  {  m_iFaceIndex = (iIndex < FACE_COUNT) ? iIndex : 0;  }
    _uint Get_Eye() const  { return m_iFaceIndex; }

private:
    virtual HRESULT         Ready_Components() override;

private:
    static constexpr _uint  FACE_COUNT = { 5 };
    CTexture*               m_pFaceTextureCom = { nullptr };
    _uint                   m_iFaceIndex = { 0 };

public:
    static CRabbitEnemy_Body*   Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*        Clone(void* pArg) override;

protected:
    virtual void                Free() override;
};

NS_END
