#include "Kirby_Deform.h"

#include "GameInstance.h"

#include "Kirby.h"
#include "Kirby_Body.h"
#include "Kirby_State.h"

CKirby_Deform::CKirby_Deform()
{
}

HRESULT CKirby_Deform::Initialize()
{   
    return S_OK;
}

void CKirby_Deform::Free()
{
    __super::Free();
}
