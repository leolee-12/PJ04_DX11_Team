#include "Panel_Inspector.h"

#include "EditInstance.h"
#include "Level_Edit.h"

#include "GameInstance.h"
#include "GameObject.h"
#include "Component.h"
#include "Transform.h"
#include "Model.h"
#include "ContainerObject.h"
#include "PartObject.h"
#include "UIContainerObject.h"
#include "UIPartObject.h"

#include "imgui.h"

CPanel_Inspector::CPanel_Inspector(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
    : CPanel(pDevice, pContext)
{
    strcpy_s(m_szName, "Inspector");
}

void CPanel_Inspector::Render()
{
    if (!Begin_Panel())
    {
        End_Panel();
        return;
    }

    CLevel_Edit* pLevel = CEditInstance::GetInstance()->Get_Level();
    if (nullptr == pLevel)
    {
        End_Panel();
        return;
    }

    CGameObject* pSelected = pLevel->Get_Selected();
    if (nullptr == pSelected)
    {
        End_Panel();
        return;
    }

    Draw_Transform(pSelected);

    ImGui::Separator();

    Draw_Properties(pSelected);

    for (auto& [tag, pComponent] : pSelected->Get_Components())
    {
        if (!pComponent) continue;
        if (pComponent->Get_Properties().empty()) continue;

        string strTag = WstrToStr(tag);
        if (ImGui::CollapsingHeader(strTag.c_str()))
        {
            ImGui::PushID(pComponent);
            Draw_Properties(pComponent);
            ImGui::PopID();
        }
    }

    if (auto pContainer = dynamic_cast<CContainerObject*>(pSelected))
    {
        auto& PartObjects = pContainer->Get_PartObjects();
        vector<pair<wstring, CPartObject*>> vecSorted(PartObjects.begin(), PartObjects.end());
        sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        for (auto& [tag, pPart] : vecSorted)
        {
            string strTag = WstrToStr(tag);
            if (ImGui::CollapsingHeader(strTag.c_str()))
            {
                ImGui::PushID(pPart);
                Draw_Transform(pPart, strTag);
                ImGui::Separator();
                Draw_Properties(pPart);
                ImGui::PopID();
            }
        }
    }

    if (auto pUIContainer = dynamic_cast<CUIContainerObject*>(pSelected))
    {
        auto& UIPartObjects = pUIContainer->Get_UIPartObjects();
        vector<pair<wstring, CUIPartObject*>> vecSorted(UIPartObjects.begin(), UIPartObjects.end());
        sort(vecSorted.begin(), vecSorted.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        for (auto& [tag, pPart] : vecSorted)
        {
            string strTag = WstrToStr(tag);
            if (ImGui::CollapsingHeader(strTag.c_str()))
            {
                ImGui::PushID(pPart);
                Draw_Transform(pPart, strTag);
                ImGui::Separator();
                Draw_Properties(pPart);
                ImGui::PopID();
            }
        }
    }

    End_Panel();
}

void CPanel_Inspector::Draw_Properties(IReflectable* pHolder)
{
    string strCurrentCategory = {};

    for (auto& prop : pHolder->Get_Properties())
    {
        const string strPropCategory = WstrToStr(prop.strCategory);
        if (strPropCategory != strCurrentCategory)
        {
            strCurrentCategory = strPropCategory;
            ImGui::Text("[%s]", strCurrentCategory.c_str());
        }

        void* pData = pHolder->Get_PropertyPtr(prop.uOffset);
        if (!pData) continue;

        string strPropName = WstrToStr(prop.strName);

        switch (prop.eType)
        {
        case PROP_TYPE::INT:
            ImGui::Text(strPropName.c_str());
            ImGui::InputInt(("##" + strPropName).c_str(), (int*)pData);
            break;
        case PROP_TYPE::FLOAT:
            ImGui::Text(strPropName.c_str());
            ImGui::DragFloat(("##" + strPropName).c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::BOOL:
            ImGui::Text(strPropName.c_str());
            ImGui::Checkbox(("##" + strPropName).c_str(), (bool*)pData);
            break;
        case PROP_TYPE::FLOAT2:
            ImGui::Text(strPropName.c_str());
            ImGui::DragFloat2(("##" + strPropName).c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::FLOAT3:
            ImGui::Text(strPropName.c_str());
            ImGui::DragFloat3(("##" + strPropName).c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::FLOAT4:
            ImGui::Text(strPropName.c_str());
            ImGui::DragFloat4(("##" + strPropName).c_str(), (float*)pData, 0.1f);
            break;
        case PROP_TYPE::ANIM_INDEX:
        {
            int iVal = (int)*((_uint*)pData);
            CGameObject* pObject = dynamic_cast<CGameObject*>(pHolder);
            CModel* pModel = pObject ? pObject->Get_Component<CModel>(L"Com_Model") : nullptr;
            if (!pModel) break;
            _uint iNumAnims = pModel->Get_MaxAnimationIndex() + 1;
            string strItems;
            for (_uint i = 0; i < iNumAnims; ++i)
                strItems += to_string(i) + ": " + pModel->Get_AnimationName(i) + '\0';
            strItems += '\0';

            ImGui::Text(strPropName.c_str());
            ImGui::PushID(strPropName.c_str());
            if (ImGui::Combo(("##" + strPropName).c_str(), &iVal, strItems.c_str()))
                *(_uint*)pData = (_uint)iVal;
            ImGui::PopID();
            break;
        }
        case PROP_TYPE::WSTRING:
        {
            wstring* pWstr = (wstring*)pData;
            string str = WstrToStr(*pWstr);
            char buf[256] = {};
            strcpy_s(buf, str.c_str());
            if (ImGui::InputText(("##" + strPropName).c_str(), buf, sizeof(buf)))
                *pWstr = StrToWstr(buf);
            break;
        }
        }
    }
}

void CPanel_Inspector::Draw_Transform(CGameObject* pObject, const string& strSuffix)
{
    CTransform* pTransform = pObject->Get_Transform();

    _float4 vPos = {};
    XMStoreFloat4(&vPos, pTransform->Get_State(STATE::POSITION));
    if (ImGui::DragFloat3(("Position##" + strSuffix).c_str(), (float*)&vPos, 0.1f))
        pTransform->Set_State(STATE::POSITION, XMLoadFloat4(&vPos));

    _float3& vEuler = m_RotEditEuler[pObject];
    _float3 vBefore = vEuler;
    if (ImGui::DragFloat3(("Rotate##" + strSuffix).c_str(), (float*)&vEuler, 0.5f))
    {
        _float3 vDelta = { vEuler.x - vBefore.x, vEuler.y - vBefore.y, vEuler.z - vBefore.z };

        auto ApplyDelta = [pTransform](_fvector vAxis, _float fDegree)
            {
                if (fDegree == 0.f) return;
                _vector vQuat = XMQuaternionRotationNormal(vAxis, XMConvertToRadians(fDegree));
                pTransform->Rotate(vQuat);
            };

        _vector vR = XMVector3Normalize(pTransform->Get_State(STATE::RIGHT));
        _vector vU = XMVector3Normalize(pTransform->Get_State(STATE::UP));
        _vector vL = XMVector3Normalize(pTransform->Get_State(STATE::LOOK));

        ApplyDelta(vR, vDelta.x);
        ApplyDelta(vU, vDelta.y);
        ApplyDelta(vL, vDelta.z);
    }

    _float3 vScale = pTransform->Get_Scaled();

    if (dynamic_cast<CUIObject*>(pObject))
    {
        static CGameObject* pPrevObj = nullptr;
        static _float fRatioScale = 1.f;
        static _float fBaseScaleX{}, fBaseScaleY{};

        if (pPrevObj != pObject)
        {
            fRatioScale = 1.f;
            fBaseScaleX = vScale.x;
            fBaseScaleY = vScale.y;
            pPrevObj = pObject;
        }

        _float2 vUIScale = { vScale.x, vScale.y };
        if (ImGui::DragFloat2(("Scale##" + strSuffix).c_str(), (float*)&vUIScale, 0.1f))
            pTransform->Set_Scale(vUIScale.x, vUIScale.y, 1.f);

        if (ImGui::DragFloat(("Uniform Scale##" + strSuffix).c_str(), &fRatioScale, 0.01f, 0.01f, 100.f))
            pTransform->Set_Scale(fBaseScaleX * fRatioScale, fBaseScaleY * fRatioScale, 1.f);
    }
    else
    {
        _float fUniformScale = vScale.x;
        if (ImGui::DragFloat(("Scale##" + strSuffix).c_str(), &fUniformScale, 0.1f))
            pTransform->Set_Scale(fUniformScale, fUniformScale, fUniformScale);
    }
}

CPanel_Inspector* CPanel_Inspector::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    return new CPanel_Inspector(pDevice, pContext);
}

void CPanel_Inspector::Free()
{
    __super::Free();
}
