#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

class CNoddy_Body final : public CMonsterPart
{
    GENERATED_BODY(CNoddy_Body)

public:
    struct NODDY_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_Noddy_Body";

private:
    CNoddy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CNoddy_Body(const CNoddy_Body& Prototype);
    virtual ~CNoddy_Body() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }
    void Set_Face(_uint iIndex)  { m_iFaceIndex = iIndex; }
    _uint Get_Face() const  { return m_iFaceIndex; }

private:
    virtual HRESULT         Ready_Components() override;

private:
    static const _uint      FACE_COUNT = 3;
    CTexture*               m_pFaceTextureCom = { nullptr };
    _uint                   m_iFaceIndex = { 2 };

public:
    static CNoddy_Body*     Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*    Clone(void* pArg) override;

protected:
    virtual void            Free() override;
};

NS_END
