#pragma once
#include "LevelDesignObject.h"

NS_BEGIN(Engine)
class CShader;
class CModel;
NS_END

NS_BEGIN(Client)
struct LD_SPAWN_SPEC;

class CLevelDesign_Bush : public CLevelDesignObject
{
    GENERATED_BODY(CLevelDesign_Bush);

public:
    static constexpr const _tchar* PROTOTYPE_TAG = L"Proto_LevelDesign_Bush";
    static constexpr const _tchar* BUSH_S_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_S";
    static constexpr const _tchar* BUSH_M_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_M";
    static constexpr const _tchar* BUSH_L_MODEL_PROTO_TAG = L"Proto_Component_Model_Bush_L";
    static constexpr const _tchar* CUT_S_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_S";
    static constexpr const _tchar* CUT_M_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_M";
    static constexpr const _tchar* CUT_L_MODEL_PROTO_TAG = L"Proto_Component_Model_BushCut_L";

private:
    enum MODEL_SLOT { BASIC, CUT, _COUNT };

private:
    CLevelDesign_Bush(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CLevelDesign_Bush(const CLevelDesign_Bush& Prototype);
    virtual ~CLevelDesign_Bush() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArwlsg) override;
    virtual void    Late_Update(_float fTimeDelta) override;
    virtual HRESULT Render() override;
    virtual void    Copy_PrototypeName(ENGINE_OBJECT_DATA* pOutData) override;

    static LD_BUSH_TYPE Resolve_BushType(const _wstring& wstrObjName);
    static void Register_LevelDesignSpecs();
    static _bool Build_Desc(const LD_OBJECT_DESC& CommonDesc, const json& jEntry, const LD_SPAWN_SPEC& Spec, LD_OBJECT_ENTRY* pOutEntry);
    static CGameObject* Create_Prototype(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);

private:
    CShader* m_pShaderComs[MODEL_SLOT::_COUNT] = { nullptr };
    CModel* m_pModelComs[MODEL_SLOT::_COUNT] = { nullptr };

    LD_BUSH_DESC m_tBushDesc = {};
    MODEL_SLOT m_eRenderSlot = { MODEL_SLOT::BASIC };
    _uint m_iModelProtoLevel = { ETOUI(LEVEL::GAMEPLAY) };

private:
    virtual HRESULT Validate_Desc() override;

    HRESULT Ready_Components();
    HRESULT Bind_ShaderResources(MODEL_SLOT eSlot);
    HRESULT Render_Model(MODEL_SLOT eSlot);
    const _tchar* Resolve_ModelProtoTag(MODEL_SLOT eSlot) const;
    MODEL Resolve_ModelType(MODEL_SLOT eSlot) const;

public:
    static CLevelDesign_Bush* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CGameObject* Clone(void* pArg) override;

protected:
    virtual void Free() override;
};


NS_END