#include "RailTrack.h"
#include "LevelDesign_Rail.h"

void CRailTrack::Build(const LD_RAIL_DESC& Desc)
{
    m_Desc = Desc;
    m_Table.clear();
    m_fLength = 0.f;
    m_iHint = 0;
    m_bLoop = (Desc.fRadius > 0.001f) || Desc.bClose;

    const _uint segs = CLevelDesign_Rail::Get_SegmentCount(m_Desc);
    if (0 == segs)
        return;

    _float acc = 0.f; _float3 prev{}; _bool bFirst = true;
    for (_uint s = 0; s < segs; ++s)
        for (_uint k = 0; k <= STEPS; ++k)
        {
            if (0 == k && s != 0) continue;
            const _float t = static_cast<_float>(k) / STEPS;

            _float3 p{};
            if (!CLevelDesign_Rail::Evaluate_Segment(m_Desc, s, t, &p))
                continue;

            if (!bFirst)
            {
                const _float dx = p.x - prev.x;
                const _float dy = p.y - prev.y;
                const _float dz = p.z - prev.z;
                acc += sqrtf(dx * dx + dy * dy + dz * dz);
            }
            m_Table.push_back({ acc, s, t });
            prev = p; bFirst = false;
        }
    m_fLength = acc;
}

_bool CRailTrack::Sample(_float fDist, _float3* pOutPos,  _float3* pOutTangent) const
{
    if (nullptr == pOutPos) return false;
    const _uint n = static_cast<_uint>(m_Table.size());
    if (n < 2) return false;

    if (fDist <= 0.f) {
        const ENTRY& e = m_Table.front();
        return CLevelDesign_Rail::Evaluate_Segment(m_Desc, e.iSeg, e.fT, pOutPos,
            pOutTangent);
    }
    if (fDist >= m_fLength) {
        const ENTRY& e = m_Table.back();
        return CLevelDesign_Rail::Evaluate_Segment(m_Desc, e.iSeg, e.fT, pOutPos,
            pOutTangent);
    }

    _uint i = m_iHint;
    if (i >= n - 1 || m_Table[i].fDist > fDist) i = 0;
    while (i + 1 < n && m_Table[i + 1].fDist <= fDist) ++i;
    m_iHint = i;

    const ENTRY& a = m_Table[i];
    const ENTRY& b = m_Table[i + 1];
    const _float span = b.fDist - a.fDist;
    const _float u = (span > 1e-6f) ? ((fDist - a.fDist) / span) : 0.f;

    _uint iSeg; _float t;
    if (a.iSeg == b.iSeg) { iSeg = a.iSeg; t = a.fT + (b.fT - a.fT) * u; }
    else { iSeg = b.iSeg; t = b.fT * u; }

    return CLevelDesign_Rail::Evaluate_Segment(m_Desc, iSeg, t, pOutPos,
        pOutTangent);
}

CRailTrack* CRailTrack::Create()
{
    return new CRailTrack();
}

void CRailTrack::Free()
{
    __super::Free();
}