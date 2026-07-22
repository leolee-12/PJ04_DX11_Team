#pragma once
#include "MonsterPart.h"

NS_BEGIN(Engine)
class CTexture;
NS_END

NS_BEGIN(Client)

class CRangerEnemy_Body final : public CMonsterPart
{
    GENERATED_BODY(CRangerEnemy_Body)

public:
    struct RANGERENEMY_BODY_DESC : public CMonsterPart::MONSTERPART_DESC
    {
    };

    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_RangerEnemy_Body";

private:
    CRangerEnemy_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CRangerEnemy_Body(const CRangerEnemy_Body& Prototype);
    virtual ~CRangerEnemy_Body() = default;

private:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Render() override;

public:
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override
    {
        pOutData->strPrototypeTag = PROTOTYPE_TAG;
    }
    void Set_Face(_uint iIndex) { m_iFaceIndex = iIndex; }
    _uint Get_Face() const { return m_iFaceIndex; }

private:
    virtual HRESULT         Ready_Components() override;

private:
    static const _uint      FACE_COUNT = 2;
    CTexture*               m_pFaceTextureCom = { nullptr };
    _uint                   m_iFaceIndex = { 0 };

public:
    static CRangerEnemy_Body*  Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject*       Clone(void* pArg) override;

protected:
    virtual void               Free() override;
};

NS_END
