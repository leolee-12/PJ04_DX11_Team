#include "Kirby_Body.h"

#include "GameInstance.h"

#include "GameContent_const.h"

#include "Effect_Loader.h"

#include "kirby.h"

CKirby_Body::CKirby_Body(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CKirby_Deform_Model(pDevice, pContext)
{
}

CKirby_Body::CKirby_Body(const CKirby_Body& Prototype)
    : CKirby_Deform_Model(Prototype){
}

HRESULT CKirby_Body::Initialize_Prototype()
{
    m_eProjType = PROJ_TYPE::PERSPEC;
    return S_OK;
}

HRESULT CKirby_Body::Initialize(void* pArg)
{
    KIRBY_BODY_DESC* pDesc = static_cast<KIRBY_BODY_DESC*>(pArg);

    pDesc->fSpeedPerSec = 1.f;

    if (FAILED(__super::Initialize(pDesc)))
        return E_FAIL;

    if (FAILED(Ready_Components()))
        return E_FAIL;
    
    m_pAnimatorCom->Play("Wait", true, true);

    return S_OK;
}

void CKirby_Body::Priority_Update(_float fTimeDelta)
{
}

void CKirby_Body::Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    if (m_pGameInstance_Proxy->Is_EditMode())
        return;

    m_pAnimatorCom->Update(fTimeDelta);
}

void CKirby_Body::Late_Update(_float fTimeDelta)
{
    if (m_bActive == false)
        return;

    CPartObject::Compute_CombinedWorldMatrix(XMLoadFloat4x4(m_pTransformCom->Get_WorldMatrixPtr()));

    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::NONBLEND, this);
    m_pGameInstance_Proxy->Add_RenderGroup(RENDERID::SHADOW, this);
}

HRESULT CKirby_Body::Render()
{
    if (FAILED(Set_VisibleMeshes()))
        return E_FAIL;

    if (FAILED(Bind_ShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    const _uint iNumMeshes = (_uint)m_pModelCom->Get_NumMeshes();

    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        if (i >= m_VisibleMeshes.size() || m_VisibleMeshes[i] == false)
            continue;

        if (FAILED(Render_KirbyMesh(i)))
            return E_FAIL;
    }
    return S_OK;
}

HRESULT CKirby_Body::Render_Shadow()
{
    if (!m_bActive || m_iShadowPass < 0 || nullptr == m_pModelCom)
        return S_OK;

    if (FAILED(Set_VisibleMeshes()))
        return E_FAIL;

    if (FAILED(Bind_ShaderResources(m_pKirbyShaderCom)))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_Matrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
        return E_FAIL;
    m_pKirbyShaderCom->Bind_Matrix("g_ViewMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::VIEW));
    m_pKirbyShaderCom->Bind_Matrix("g_ProjMatrix", m_pGameInstance_Proxy->Get_Shadow_Transform(D3DTS::PROJ));

    const _uint iNumMeshes = (_uint)m_pModelCom->Get_NumMeshes();
    for (_uint i = 0; i < iNumMeshes; ++i)
    {
        m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", i);
        if (FAILED(m_pKirbyShaderCom->Begin(m_iShadowPass)))
            return E_FAIL;
        m_pModelCom->Render(i);
    }
    return S_OK;
}

HRESULT CKirby_Body::Ready_AnimEvents(CKirby* pKirby)
{
    m_pAnimatorCom->Set_EventCallback(
        [this, pKirby](const ANIM_EVENT& e, ANIM_EVENT_PHASE ePhase)
        {
            if (Handle_AnimEventEye(e, ePhase) == true)
                return;

            if (Handle_AnimEventSound(e, ePhase) == true)
                return;

            switch (static_cast<EANIM_EVENT>(e.iEventType))
            { 
                case EANIM_EVENT::Fx:
                {
                    if (ePhase != ANIM_EVENT_PHASE::POINT)
                        break;

                    if (e.strParam.empty())
                        break;

                    switch (e.iIntParam)
                    {
                        case 0: // 캐릭터 중심 이펙트 Spawn
                        {
                            _float3 vPos{};
                            XMStoreFloat3(&vPos, pKirby->Get_Transform()->Get_State(STATE::POSITION) +
                                XMVectorSet(0.f, 1.f, 0.f, 0.f));
                            CEffect_Loader::GetInstance()->Spawn(StrToWstr(e.strParam), pKirby->Get_LevelIndex(), vPos);
                            break;
                        }
                        case 10: // Spit Air 조동아리 앞 오프셋 이펙트
                        {
                            _float3 vPos{}, vLook{};
                            _vector vNormLook = XMVector3Normalize(pKirby->Get_Transform()->Get_State(STATE::LOOK));
                            XMStoreFloat3(&vLook, vNormLook);
                            vNormLook *= 2.f;
                            vNormLook.m128_f32[1] = 0.6f;
                            XMStoreFloat3(&vPos, pKirby->Get_Transform()->Get_State(STATE::POSITION) + vNormLook);
                            CEffect_Loader::GetInstance()->Spawn(StrToWstr(e.strParam), pKirby->Get_LevelIndex(), vPos, vLook);
                            break;
                        }
                    }

                    break;
                }

                case EANIM_EVENT::SetBody:
                {
                    if (ePhase != ANIM_EVENT_PHASE::POINT)
                        break;

                    switch (static_cast<KIRBY_BODY_STATE>(e.iIntParam))
                    {
                        case KIRBY_BODY_STATE::NORMAL:  Set_KirbyBody(KIRBY_BODY_STATE::NORMAL);  break;
                        case KIRBY_BODY_STATE::STUFFED: Set_KirbyBody(KIRBY_BODY_STATE::STUFFED); break;
                        case KIRBY_BODY_STATE::INHALE:  Set_KirbyBody(KIRBY_BODY_STATE::INHALE);  break;
                    }
                    break;
                }

                case EANIM_EVENT::WalkSmoke:
                {
                    if (ePhase != ANIM_EVENT_PHASE::POINT)
                        break;

                    const _float fBackOffset = 1.f;
                    const _float fSideOffset = 0.25f;

                    CTransform* pKirbyTransform = pKirby->Get_Transform();

                    _vector vBackDir = -XMVector3Normalize(
                        XMVectorSetY(pKirbyTransform->Get_State(STATE::LOOK), 0.f));

                    _vector vRightDir = XMVector3Normalize(
                        XMVectorSetY(pKirbyTransform->Get_State(STATE::RIGHT), 0.f));

                    _float3 fPos{};
                    XMStoreFloat3(&fPos, pKirbyTransform->Get_State(STATE::POSITION) + XMVectorSet(0.f, 0.2f, 0.f, 0.f) +
                        vBackDir * fBackOffset + vRightDir * fSideOffset * static_cast<_float>(e.iIntParam));

                    _float3 vSpawnLook{};
                    XMStoreFloat3(&vSpawnLook, vBackDir);

                    CEffect_Loader::GetInstance()->Spawn(L"WalkSmoke", Get_LevelIndex(), fPos, vSpawnLook, _float3(0.f, 0.f, 0.f));

                    break;
                }
            }
        }
    );

    return S_OK;
}

HRESULT CKirby_Body::Ready_Components()
{
    /* For.Com_Shader */
    m_pKirbyShaderCom = Add_Component<CShader>(Shader_Kirby.iLevelID, Shader_Kirby.szProtoTag, TEXT("Com_Shader"));
    if (m_pKirbyShaderCom == nullptr)
        return E_FAIL;

    /* For.Com_Model */
    m_pModelCom = Add_Component<CModel>(m_iPrototypeLevel, TEXT("Prototype_Component_Model_Kirby_Body"), TEXT("Com_Model"));
    if (m_pModelCom == nullptr)
        return E_FAIL;

    m_pEyeTextureCom = Add_Component<CTexture>(TEXT("Com_EyeTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/Kirby/KirbyEye.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
    if (nullptr == m_pEyeTextureCom)
        return E_FAIL;

    m_pEyeMaskTextureCom = Add_Component<CTexture>(TEXT("Com_EyeMaskTexture"),
        CTexture::Create(m_pDevice, m_pContext, L"../../Resources/YSE/Kirby/KirbyEyeMask.%02d.png", ETOUI(KIRBY_EYE_STATE::END)));
    if (nullptr == m_pEyeMaskTextureCom)
        return E_FAIL;

    m_VisibleMeshes.resize(m_pModelCom->Get_NumMeshes(), true);

    /* For.Com_Animator */
    CAnimator::ANIMATOR_DESC AnimDesc{};
    AnimDesc.pModel = m_pModelCom;
    AnimDesc.strDataFile = L"../../Resources/YSE/Kirby/Kirby_AnimEvents.json";

    m_pAnimatorCom = Add_Component<CAnimator>(TEXT("Com_Animator"), CAnimator::Create(m_pDevice, m_pContext));

    if (nullptr == m_pAnimatorCom || FAILED(m_pAnimatorCom->Initialize(&AnimDesc)))
        return E_FAIL;

    return S_OK;
}

HRESULT CKirby_Body::Set_VisibleMeshes()
{
    const _uint iNumMeshes = static_cast<_uint>(m_pModelCom->Get_NumMeshes());

    m_VisibleMeshes.assign(iNumMeshes, false);

    auto ShowMesh = [&](KIRBY_MESH eKirbyMesh)
        {
            _uint iIndex = ETOUI(eKirbyMesh);
            if (iIndex >= 0 && iIndex < iNumMeshes)
                m_VisibleMeshes[iIndex] = true;
        };

    switch (m_eBody)
    {
        case KIRBY_BODY_STATE::NORMAL:  ShowMesh(KIRBY_MESH::BODY);         break;
        case KIRBY_BODY_STATE::STUFFED: ShowMesh(KIRBY_MESH::BODY_BIG);     break;
        case KIRBY_BODY_STATE::INHALE:  ShowMesh(KIRBY_MESH::BODY_VACUUM);  break;
    }

    if (m_eBody == KIRBY_BODY_STATE::NORMAL)
    {
        switch (m_eMouth)
        {
            case KIRBY_MOUTH_STATE::IDLE:           ShowMesh(KIRBY_MESH::MOUTH_NORMAL);         break;
            case KIRBY_MOUTH_STATE::OPEN:           ShowMesh(KIRBY_MESH::MOUTH_OPEN);           break;
            case KIRBY_MOUTH_STATE::ANGRY:          ShowMesh(KIRBY_MESH::MOUTH_ANGRY);          break;
            case KIRBY_MOUTH_STATE::SMILE_OPEN:     ShowMesh(KIRBY_MESH::MOUTH_SMILE_OPEN);     break;
            case KIRBY_MOUTH_STATE::SMILE_CLOSE:    ShowMesh(KIRBY_MESH::MOUTH_SMILE_CLOSE);    break;
        }
    }

    ShowMesh(KIRBY_MESH::LIMBS);

    return S_OK;
}

HRESULT CKirby_Body::Render_KirbyMesh(_uint iMeshIndex)
{
    if (m_pKirbyShaderCom == nullptr)
    {
        MSG_BOX("m_pKirbyShaderCom is nullptr: Kirby_Form");
        return E_FAIL;
    }

    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pEyeTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_NormalTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pEyeMaskTextureCom->Bind_ShaderResource(m_pKirbyShaderCom, "g_EyeMaskTexture", ETOUI(m_eEye))))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_SkinTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 1)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_Material(m_pKirbyShaderCom, "g_MouthTexture", iMeshIndex, MTEX_TYPE::UNKNOWN, 2)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Bind_BoneMatrices(m_pKirbyShaderCom, "g_BoneMatrices", iMeshIndex)))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBodyColor", &m_vBodyColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vFootColor", &m_vFootColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Bind_RawValue("g_vBlushColor", &m_vBlushColor, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(m_pKirbyShaderCom->Begin(0)))
        return E_FAIL;

    if (FAILED(m_pModelCom->Render(iMeshIndex)))
        return E_FAIL;

    return S_OK;
}

CKirby_Body* CKirby_Body::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    CKirby_Body* pInstance = new CKirby_Body(pDevice, pContext);

    if (FAILED(pInstance->Initialize_Prototype()))
    {
        MSG_BOX("Failed to Created: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

CGameObject* CKirby_Body::Clone(void* pArg)
{
    CKirby_Body* pInstance = new CKirby_Body(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned: CKirby_Body");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void CKirby_Body::Free()
{
    __super::Free();
}