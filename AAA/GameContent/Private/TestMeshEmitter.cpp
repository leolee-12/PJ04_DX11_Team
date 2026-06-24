#include "TestMeshEmitter.h"

#include "GameInstance.h"
#include "GameContent_const.h"

CTestMeshEmitter::CTestMeshEmitter(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CEffect_MeshEmitter{ pDevice, pContext }
{
}

CTestMeshEmitter::CTestMeshEmitter(const CTestMeshEmitter& Prototype)
    : CEffect_MeshEmitter(Prototype)
{
}

HRESULT CTestMeshEmitter::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CTestMeshEmitter::Initialize(void* pArg)
{
    TEST_MESHEMITTER_DESC tDesc{};

    tDesc.iModelLevel = m_iPrototypeLevel;
    tDesc.wstrModelTag = TEXT("Prototype_Component_Model_SmokeSphereOriginal");

    tDesc.bUseDiffuseTexture = false;
    tDesc.bUseUnKnownTexture = true;

    tDesc.bUseTextureCom = false;
    tDesc.iTextureLevel = 0;
    tDesc.wstrTextureTag = L"";

    tDesc.bUseMaskCom = false;
    tDesc.iMaskLevel = 0;
    tDesc.wstrMaskTag = L"";

    tDesc.bCustomShader = false;
    //tDesc.iShaderLevel = 0;
    //tDesc.wstrShaderTag = L"";

    if (FAILED(__super::Initialize(&tDesc)))
        return E_FAIL;

    m_fDuration = 4.f;

    m_iEmitterMaxParticleCount = 200;
    m_fEmitterRateOverTime = 10.f;

    m_fEmitterLifeTime = 15.f;
    m_bEmitterRandomLifeTime = true;
    m_vEmitterLifeTimeRange.x = 10.f;
    m_vEmitterLifeTimeRange.y = 15.f;

    m_iEmitterShapeType = 3;
    m_vEmitterBoxSize.x = 30.f;
    m_vEmitterBoxSize.y = 10.f;
    m_vEmitterBoxSize.z = 30.f;

    m_iEmitterBoxSpawnMode = 2;

    m_iEmitterVelocityMode = 1;
    m_fEmitterStartSpeed = 3.f;

    m_vEmitterVelocityDirection = { 0.4f, -1.f, 0.f };

    m_fEmitterDirectionRandomStrength = 0.6f;
    m_fEmitterStartSpeedRandomRatio = 0.6f;

    m_bEmitterUseFlutter = true;
    m_fEmitterFlutterAmplitude = 0.3f;
    m_fEmitterFlutterFrequency = 0.2f;
    m_fEmitterFlutterRandomRatio = 1.0f;

    m_bEmitterRandomSize = true;
    m_vEmitterStartSizeRange = { 0.3f, 0.5f };

    m_bEmitterSizeChange = true;
    m_fEmitterSizeStartValue = 0.f;
    m_fEmitterSizeEndValue = 0.f;

    m_bActive_EmitterSize_Ratio_0 = true;
    m_fEmitterSize_Ratio_0 = 0.1f;
    m_fEmitterSize_Value_0 = 1.f;

    m_bActive_EmitterSize_Ratio_1 = true;
    m_fEmitterSize_Ratio_1 = 0.95f;
    m_fEmitterSize_Value_1 = 1.f;

    m_bEmitterRandomRotation = true;
    m_vEmitterRandomRotationMin = { 0.f, 0.f, 0.f };
    m_vEmitterRandomRotationMax = { 360.f, 360.f, 360.f };

    m_bEmitterRotationOverLife = true;
    m_vEmitterAngularVelocity = { 120.f, 180.f, 90.f };

    return S_OK;
}

void CTestMeshEmitter::Priority_Update(_float fTimeDelta)
{
    __super::Priority_Update(fTimeDelta);
}

void CTestMeshEmitter::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void CTestMeshEmitter::Late_Update(_float fTimeDelta)
{
    if (m_bIsPlay == false)
        return;

    __super::Late_Update(fTimeDelta);

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
}

HRESULT CTestMeshEmitter::Render()
{
    __super::Render();

    return S_OK;
}

CTestMeshEmitter* CTestMeshEmitter::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CTestMeshEmitter* pInstance = new CTestMeshEmitter(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CTestMeshEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CTestMeshEmitter::Clone(void* pArg)
{
    CTestMeshEmitter* pInstance = new CTestMeshEmitter(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CTestMeshEmitter");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CTestMeshEmitter::Free()
{
    __super::Free();
}
