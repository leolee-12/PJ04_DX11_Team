#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CLIENT_DLL CCameraDirector final : public CGameObject
{
    GENERATED_BODY(CCameraDirector)
public:
    static constexpr const wchar_t* PROTOTYPE_TAG = L"Proto_CameraDirector";

private:
    CCameraDirector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CCameraDirector(const CCameraDirector& Prototype);
    virtual ~CCameraDirector() = default;

public:
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* p) override { p->strPrototypeTag = PROTOTYPE_TAG; }

protected:
    virtual HRESULT Ready_Events() override;

private:
    void On_CameraChange(void* p);

public:
    static CCameraDirector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END