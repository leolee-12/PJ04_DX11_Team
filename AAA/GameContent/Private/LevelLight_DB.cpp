#include "LevelLight_DB.h"

namespace
{
    const LEVEL_LIGHT DEFAULT_LIGHT = {
        { 0.557f, -0.766f, 0.321f, 0.f },   // vDir
        { 6.2f,   6.15f,   5.76f,  1.f },   // vDiffuse
        { 0.f,    0.f,     0.f,    1.f },   // vAmbient
        { 1.f,    1.f,     1.f,    1.f },   // vSpecular
        TEXT("Default"),
    };

    const LEVEL_LIGHT VOLCANO_LIGHT = {
        { 0.557f, -0.766f, 0.321f, 0.f },   // vDir
        { 3.1f,   2.65f,   2.76f,  1.f },   // vDiffuse
        { 0.f,    0.f,     0.f,    1.f },   // vAmbient
        { 1.f,    1.f,     1.f,    1.f },   // vSpecular
        TEXT("Volcano"),
    };

    const LEVEL_LIGHT ARENA_LIGHT = {
        { 0.557f, -0.766f, 0.321f, 0.f },   // vDir
        { 6.2f,   6.15f,   5.76f,  1.f },   // vDiffuse
        { 0.f,    0.f,     0.f,    1.f },   // vAmbient
        { 1.f,    1.f,     1.f,    1.f },   // vSpecular
        TEXT("Arena"),
    };

    const LEVEL_LIGHT g_LevelLights[ETOUI(LEVEL::END)] = {
        /* STATIC        */ DEFAULT_LIGHT,
        /* FIRST_LOADING */ DEFAULT_LIGHT,
        /* LOADING       */ DEFAULT_LIGHT,
        /* STAGE0_STEP1  */ DEFAULT_LIGHT,
        /* STAGE0_STEP2  */ DEFAULT_LIGHT,
        /* TOWN_STEP1    */ DEFAULT_LIGHT,
        /* TOWN_STEP2    */ DEFAULT_LIGHT,
        /* BOSS_STAGE1   */ DEFAULT_LIGHT,
        /* STAGE1_STEP1  */ VOLCANO_LIGHT,
        /* STAGE1_STEP2  */ VOLCANO_LIGHT,
        /* STAGE1_STEP3  */ VOLCANO_LIGHT,
        /* ARENA         */ ARENA_LIGHT,
        /* ENDING        */ DEFAULT_LIGHT,
        /* TEST          */ { { 0.25f, -1.f, 0.25f, 0.f },
                              { 1.f,   1.f,  1.f,   1.f },
                              { 0.5f,  0.5f, 0.5f,  1.f },
                              { 1.f,   1.f,  1.f,   1.f } },
    };
}

const LEVEL_LIGHT& CLevelLight_DB::Get(LEVEL eLevel)
{
    _uint i = ETOUI(eLevel);
    if (i >= ETOUI(LEVEL::END))
        return DEFAULT_LIGHT;
    return g_LevelLights[i];
}

void CLevelLight_DB::Apply(CGameInstance_Proxy* pProxy, LEVEL eLevel)
{
    if (nullptr == pProxy)
        return;

    const LEVEL_LIGHT& d = Get(eLevel);
    pProxy->Set_ShaderGlobal("g_vLightDir", d.vDir);
    pProxy->Set_ShaderGlobal("g_vLightDiffuse", d.vDiffuse);
    pProxy->Set_ShaderGlobal("g_vLightAmbient", d.vAmbient);
    pProxy->Set_ShaderGlobal("g_vLightSpecular", d.vSpecular);

    if (d.pEnvTag)
        pProxy->Set_CurrentEnvironment(d.pEnvTag);
}
