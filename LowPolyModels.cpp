#include "LowPolyModels.h"
#include "DxLib.h"
#include <algorithm>
#include <vector>

namespace
{
    struct LowPolyEntry
    {
        const wchar_t* path;
        const wchar_t* name;
        int handle;
    };

    LowPolyEntry s_entries[static_cast<int>(LowPolyId::Count)] = {
        { L"Data/Model/LowPoly/Minion.x",    L"Minion",    -1 },
        { L"Data/Model/LowPoly/Hopper.x",    L"Hopper",    -1 },
        { L"Data/Model/LowPoly/Tanker.x",    L"Tanker",    -1 },
        { L"Data/Model/LowPoly/Float.x",     L"Float",     -1 },
        { L"Data/Model/LowPoly/Charge.x",    L"Charge",    -1 },
        { L"Data/Model/LowPoly/WhaleBoss.x", L"WhaleBoss", -1 },
    };

    bool s_loaded = false;

    // Màu gốc của vật liệu cá voi (để đổi qua lại giữa phase 1 và phase 2)
    std::vector<COLOR_F> s_whaleBaseColors;
    int s_whaleAppliedPhase = -1;
}

namespace LowPolyModels
{
    void Load()
    {
        if (s_loaded) return;
        s_loaded = true;

        for (auto& e : s_entries)
        {
            if (e.handle >= 0) continue;
            e.handle = MV1LoadModel(e.path);
            if (e.handle >= 0)
            {
                AppLogAdd(L"[LOWPOLY] Load %s (%s) SUCCESS, handle=%d, materials=%d\n",
                    e.name, e.path, e.handle, MV1GetMaterialNum(e.handle));
            }
            else
            {
                AppLogAdd(L"[LOWPOLY] ERROR: Failed to load %s (%s) - object will not be drawn (no box fallback).\n", e.name, e.path);
                printfDx(L"ERROR: model %s failed to load (%s)\n", e.name, e.path);
                OutputDebugStringW(L"[LOWPOLY] ERROR: model failed to load: ");
                OutputDebugStringW(e.path);
                OutputDebugStringW(L"\n");
            }
        }

        const int whale = s_entries[static_cast<int>(LowPolyId::WhaleBoss)].handle;
        s_whaleBaseColors.clear();
        s_whaleAppliedPhase = -1;
        if (whale >= 0)
        {
            const int matNum = MV1GetMaterialNum(whale);
            for (int m = 0; m < matNum; ++m)
            {
                s_whaleBaseColors.push_back(MV1GetMaterialDifColor(whale, m));
            }
        }
    }

    void Release()
    {
        for (auto& e : s_entries)
        {
            if (e.handle >= 0)
            {
                MV1DeleteModel(e.handle);
                e.handle = -1;
            }
        }
        s_whaleBaseColors.clear();
        s_whaleAppliedPhase = -1;
        s_loaded = false;
    }

    int GetHandle(LowPolyId id)
    {
        const int idx = static_cast<int>(id);
        if (idx < 0 || idx >= static_cast<int>(LowPolyId::Count)) return -1;
        return s_entries[idx].handle;
    }

    void ApplyWhalePhaseColors(bool phase2)
    {
        const int whale = s_entries[static_cast<int>(LowPolyId::WhaleBoss)].handle;
        const int phaseIdx = phase2 ? 2 : 1;
        if (whale < 0 || s_whaleAppliedPhase == phaseIdx) return;
        s_whaleAppliedPhase = phaseIdx;

        for (int m = 0; m < static_cast<int>(s_whaleBaseColors.size()); ++m)
        {
            COLOR_F c = s_whaleBaseColors[m];
            if (phase2)
            {
                // Mắt (vật liệu đỏ sáng) -> vàng; phần thân -> tông đỏ sẫm như khối hộp phase 2 cũ
                const bool isEye = (c.r > 0.8f && c.g < 0.4f && c.b < 0.4f);
                if (isEye)
                {
                    c = GetColorF(1.0f, 0.9f, 0.16f, 1.0f);
                }
                else
                {
                    c = GetColorF((std::min)(1.0f, c.r * 3.2f), c.g * 0.43f, c.b * 0.43f, c.a);
                }
            }
            MV1SetMaterialDifColor(whale, m, c);
        }
    }
}
