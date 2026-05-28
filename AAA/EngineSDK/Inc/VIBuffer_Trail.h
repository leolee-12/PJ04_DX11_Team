#pragma once

#include "VIBuffer.h"

NS_BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Trail final : public CVIBuffer
{
public:
    /* CPU 측 큐 원소: 본 1쌍 + 현재 나이 */
    typedef struct tagTrailSample
    {
        _float3 vBase;
        _float3 vTip;
        _float  fAge;
    } TRAIL_SAMPLE;

    /* Initialize(pArg)로 넘기는 디스크립터 */
    typedef struct tagTrailDesc
    {
        _uint  iMaxSamples = 32;     /* 큐 최대 샘플 쌍 수 */
        _float fSampleLifeTime = 0.3f;   /* 샘플이 살아있는 시간(초) */
    } TRAIL_DESC;

private:
    CVIBuffer_Trail(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    CVIBuffer_Trail(const CVIBuffer_Trail& Prototype);
    virtual ~CVIBuffer_Trail() = default;

public:
    virtual HRESULT Initialize_Prototype() override;
    virtual HRESULT Initialize(void* pArg) override;            /* pArg = TRAIL_DESC* */
    virtual HRESULT Bind_Resources() override;
    virtual HRESULT Render() override;

public:
    /* 매 프레임 Effect 측에서 호출 */
    void  Push_Sample(const _float3& vBase, const _float3& vTip);
    void  Update(_float fTimeDelta);
    void  Clear();

    _bool Is_Empty() const { return m_Samples.size() < 2; }
    _float Get_LifeTime() const { return m_fSampleLifeTime; }

private:
    _uint   m_iMaxSamples = { 0 };
    _float  m_fSampleLifeTime = { 0.3f };

    deque<TRAIL_SAMPLE>  m_Samples;

    HRESULT Create_DynamicVB();
    void    Upload_Vertices();

public:
    static CVIBuffer_Trail* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    virtual CComponent* Clone(void* pArg) override;
    virtual void Free() override;
};

NS_END