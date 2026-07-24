#pragma once
#include "GameContent_Defines.h"
#include "GameObject.h"

NS_BEGIN(Client)

class CCamera_AreaCam;
class CCamera_Cutscene;
class CCamera_Boss;
class CCamera_Dialogue;
class CCamera_Coaster;

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
    virtual void On_Deserialized() override;

private:
    void On_CameraChange(void* p);
    void On_DialogueCamBegin(void* p);

    HRESULT Ensure_Cameras();    
    void    Arrange();           
    void    Sleep_Cameras();     

private:
    _bool m_bClone = { false };

    CCamera_AreaCam* m_pAreaCam = { nullptr };
    CCamera_Cutscene* m_pCutCam = { nullptr };
    CCamera_Boss* m_pBossCam = { nullptr };
    CCamera_Dialogue* m_pDlgCam = { nullptr };
    CCamera_Coaster* m_pCoasterCam = { nullptr };

    static CCameraDirector* s_pActiveDirector;

public:
    static CCameraDirector* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;
    virtual void Free() override;
};
NS_END