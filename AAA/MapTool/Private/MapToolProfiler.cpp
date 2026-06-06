#include "MapToolProfiler.h"

#ifdef _DEBUG

#include "EnvObject.h"

IMPLEMENT_SINGLETON(CMapToolProfiler)

void CMapToolProfiler::Set_Stage(Client::CMapStage* pStage)
{
    if (m_pStage == pStage)
        return;

    m_pStage = pStage;
    Reset_CaptureSession();
}

void CMapToolProfiler::Register_EnvObject(Client::CEnvObject* pEnvObject)
{
    if (nullptr != pEnvObject)
        m_EnvObjects.insert(pEnvObject);
}

void CMapToolProfiler::Clear_EnvObjects()
{
    m_EnvObjects.clear();
}

void CMapToolProfiler::Update(_float fTimeDelta)
{
    UNREFERENCED_PARAMETER(fTimeDelta);

    if (!m_bEnabled)
        return;

    if (nullptr == m_pStage && m_EnvObjects.empty())
    {
        m_Frame = {};
        m_bFrameBaseValid = false;
    }
}

void CMapToolProfiler::Capture_Frame()
{
    if (!m_bEnabled)
        return;

    MAPTOOL_PROFILE_FRAME Frame{};

    if (nullptr != m_pStage)
    {
        const Client::MAP_STAGE_PROFILE StageFrame = m_pStage->Get_Profile();

        if (!m_bFrameBaseValid)
        {
            m_iFrameBase = StageFrame.iFrameIndex;
            m_bFrameBaseValid = true;
        }

        Frame.iFrameIndex = (StageFrame.iFrameIndex >= m_iFrameBase)
            ? (StageFrame.iFrameIndex - m_iFrameBase)
            : 0;

        Frame.iMapSectionTotal = StageFrame.iTotalSections;

        Frame.MapMain.iCandidate = StageFrame.iMainCandidateSections;
        Frame.MapMain.iVisible = StageFrame.iMainVisibleSections;
        Frame.MapMain.iCulled = StageFrame.iMainCulledSections;
        Frame.MapMain.iSubmitted = StageFrame.iMainSubmittedSections;

        Frame.MapShadow.iCandidate = StageFrame.iShadowCandidateSections;
        Frame.MapShadow.iVisible = StageFrame.iShadowVisibleSections;
        Frame.MapShadow.iCulled = StageFrame.iShadowCulledSections;
        Frame.MapShadow.iSubmitted = StageFrame.iShadowSubmittedSections;

        Frame.dMapCullingCpuMs = StageFrame.dCullingCpuMs;
    }

    for (Client::CEnvObject* pEnvObject : m_EnvObjects)
    {
        if (nullptr == pEnvObject)
            continue;

        ++Frame.iEnvObjectTotal;

        if (!pEnvObject->Is_ProfileRenderable())
            continue;

        ++Frame.EnvMain.iCandidate;
        if (pEnvObject->Is_Visible_Main())
        {
            ++Frame.EnvMain.iVisible;
            ++Frame.EnvMain.iSubmitted;
        }
        else
        {
            ++Frame.EnvMain.iCulled;
        }

        if (!pEnvObject->Is_ShadowCaster())
            continue;

        ++Frame.EnvShadow.iCandidate;
        if (pEnvObject->Is_Visible_Shadow())
        {
            ++Frame.EnvShadow.iVisible;
            ++Frame.EnvShadow.iSubmitted;
        }
        else
        {
            ++Frame.EnvShadow.iCulled;
        }
    }

    m_Frame = Frame;
}

void CMapToolProfiler::Reset_CaptureSession()
{
    m_Frame = {};
    m_iFrameBase = {};
    m_bFrameBaseValid = false;
}

void CMapToolProfiler::Reset()
{
    Reset_CaptureSession();
}

void CMapToolProfiler::Free()
{
    m_pStage = nullptr;
    m_EnvObjects.clear();
    __super::Free();
}

#endif