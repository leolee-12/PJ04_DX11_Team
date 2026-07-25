#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)

struct LD_SPAWN_SPEC;

class CLD_Frame final : public CLevelDesignObject
{
    GENERATED_BODY(CLD_Frame)

public:
    static constexpr const _tchar* OBJECT_NAME = L"CreditEstoryKirbyHouseFrame";
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Frame";
    static constexpr const _tchar* MODEL_PROTO_TAG = L"Proto_Component_Model_Frame";
    static constexpr const _tchar* LAYER_TAG = L"Layer_LevelDesign_Gimmick";
    static constexpr const _char* MODEL_PATH = "../../Resources/Map/Gimmick/NonAnim/CreditEstoryKirbyHouseFrame/CreditEstoryKirbyHouseFrame.ysh";

private:
    CLD_Frame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLD_Frame(const CLD_Frame& Prototype);
    virtual ~CLD_Frame() = default;

    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual HRESULT Validate_Initialized() override;

public:
    virtual void Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

    static void Register_LevelDesignSpecs();
    static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
    static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
    CShader* m_pShaderCom = { nullptr };
    CModel* m_pModelCom = { nullptr };

    LD_STATIC_MODEL_DESC m_tStaticModelDesc = {};

private:
    HRESULT Ready_RenderComponents();
    HRESULT Bind_ShaderResources();
    HRESULT Render_Model();

public:
    static CLD_Frame* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};

NS_END