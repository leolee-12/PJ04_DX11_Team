#pragma once

namespace Client
{
    struct FONT_ENTRY
    {
        const wchar_t* szTag;
        const wchar_t* szPath;
    };

    inline const FONT_ENTRY g_UIFonts[] =
    {
        {L"K15-LocalCharacter-M",           L"../../Resources/Fonts/K15-LocalCharacter-M.spritefont"},
        { L"KOR-FOT-ComicReggaeStd-B",      L"../../Resources/Fonts/KOR-FOT-ComicReggaeStd-B.spritefont" },
        { L"KOR-FOT-RodinNTLGPro-B",        L"../../Resources/Fonts/KOR-FOT-RodinNTLGPro-B.spritefont" },
        { L"KOR-FOT-RodinNTLGPro-UB",       L"../../Resources/Fonts/KOR-FOT-RodinNTLGPro-UB.spritefont" },
        { L"KOR-FOT-SeuratProN-EB",         L"../../Resources/Fonts/KOR-FOT-SeuratProN-EB.spritefont" },
        { L"KOR-NINP-VDL-LineG-Ultra",      L"../../Resources/Fonts/KOR-NINP-VDL-LineG-Ultra.spritefont" },
        { L"KOR-VDL-GigaMaru-ExtraBold",    L"../../Resources/Fonts/KOR-VDL-GigaMaru-ExtraBold.spritefont" },
        { L"KOR-VDL-GigaMaru-Ultra",        L"../../Resources/Fonts/KOR-VDL-GigaMaru-Ultra.spritefont" },
        { L"KOR-VDL-LogoG-Ultra",           L"../../Resources/Fonts/KOR-VDL-LogoG-Ultra.spritefont" },
        { L"KOR-VDL-LogoGBlack-Black",      L"../../Resources/Fonts/KOR-VDL-LogoGBlack-Black.spritefont" },
        { L"KOR-VDL-LogoJrBlack-Black",     L"../../Resources/Fonts/KOR-VDL-LogoJrBlack-Black.spritefont" },
        { L"KOR-VDL-LogoMaru-Ultra",        L"../../Resources/Fonts/KOR-VDL-LogoMaru-Ultra.spritefont" },
        { L"KOR-VDL-LogoGBlack-Black_Big",  L"../../Resources/YSH/Font/KOR-VDL-LogoGBlack-Black.spritefont" },
        
    };
}