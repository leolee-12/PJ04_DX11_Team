#include "framework.h"
#include "AnimUITool.h"

#include "AnimUITool_App.h"
#include "GameInstance.h"
#include "Ancor_WorkingDirectory.h"

#include "imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

using namespace Engine;

#define MAX_LOADSTRING 100

// AnimUITool_Defines.h 의 extern 정의부
HINSTANCE   g_hInstance = nullptr;
HWND        g_hWnd = nullptr;

bool g_bNeedResize = false;
bool g_bAppClosing = false;
UINT g_iResizeWidth = 0, g_iResizeHeight = 0;

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    Anchor_WorkingDirectory();

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ANIMUITOOL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    CAnimUITool_App* pApp = CAnimUITool_App::Create();
    if (nullptr == pApp)
        return FALSE;

    CGameInstance_Proxy* pProxy = CGameInstance::GetProxy();

    if (FAILED(pProxy->Add_Timer(TEXT("Timer_Default"))))
        return E_FAIL;

    if (FAILED(pProxy->Add_Timer(TEXT("Timer_60"))))
        return E_FAIL;

    _float fTimeAcc = {};

    MSG msg{};
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (WM_QUIT == msg.message)
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        if (!g_bAppClosing)
        {
            if (g_bNeedResize)
            {
                pProxy->OnResize(g_iResizeWidth, g_iResizeHeight);
                g_bNeedResize = false;
            }

            pProxy->Compute_Timer(TEXT("Timer_Default"));

            fTimeAcc += pProxy->Get_TimeDelta(TEXT("Timer_Default"));

            if (fTimeAcc >= 1.f / 60.f)
            {
                pProxy->Compute_Timer(TEXT("Timer_60"));

                pApp->Update(pProxy->Get_TimeDelta(TEXT("Timer_60")));
                pApp->Render();

                fTimeAcc = { 0.f };
            }
        }
    }

    Safe_Release(pProxy);
    Safe_Release(pApp);

    return (int)msg.wParam;

}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ANIMUITOOL));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    g_hInstance = hInstance;

    RECT rc = { 0, 0, (LONG)g_iWinSizeX, (LONG)g_iWinSizeY };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top,
        nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return FALSE;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_hWnd = hWnd;
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
        return true;

    switch (message)
    {
    case WM_SIZE:
        if (!g_bAppClosing && wParam != SIZE_MINIMIZED)
        {
            g_bNeedResize = true;
            g_iResizeWidth = LOWORD(lParam);
            g_iResizeHeight = HIWORD(lParam);
        }
        return 0;
    case WM_DESTROY:
        g_bAppClosing = true;
        g_bNeedResize = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}