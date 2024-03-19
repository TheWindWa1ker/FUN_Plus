#pragma once
#include <iostream>
#include <vector>
#include "Player.hpp"
#include "LocalPlayer.hpp"
#include "Offsets.hpp"
#include "Camera.hpp"
#include "GlowMode.hpp"
#include "Spectator.hpp"

#include "DMALibrary/Memory/Memory.h"
#include <array>

struct Sense {
    // Variables
    Camera* GameCamera;
    LocalPlayer* Myself;
    Spectator* Spectators;
    std::vector<Player*>* Players;
    int TotalSpectators = 0;
    //std::vector<std::string> Spectators;
    uint64_t HighlightSettingsPointer;
    uint64_t ViewModelPtr;

    bool ItemGlow = false;
    // 34 = White, 35 = Blue, 36 = Purple, 37 = Gold, 38 = Red
    int MinimumItemRarity = 36;

    //武器上色
    bool ViewModelGlow = false;

    //Colors
    float InvisibleGlowColor[3] = { 0, 1, 0 };
    float VisibleGlowColor[3] = { 1, 0, 0 };
    float AdminColor[3] = { 100, 0, 0 };
    float rnot = 20;
    float gnot = 1;
    float bnot = 0;
    float rviz = 0;
    float gviz = 5;
    float bviz = 0;
    float radm = 100;
    float gadm = 0;
    float badm = 0;

    //GlowType
    int insidetype=101;
    int outlinetype=101;
    int outlinesize=96;

    Sense(std::vector<Player*>* Players, Camera* GameCamera, LocalPlayer* Myself) {
        this->Players = Players;
        this->GameCamera = GameCamera;
        this->Myself = Myself;
    }

    void Initialize() {
        // idk, nothing for now
    }

    //判断权限
    bool isUIDInAdminList(uint64_t UID, const std::vector<uint64_t>& AdminList) {
        return std::find(AdminList.begin(), AdminList.end(), UID) != AdminList.end();
    }

    uint64_t playeruid;

    void setCustomGlow(Player* Target, int enable, int wall, bool isVisible)
    {
        //权限名单
        std::vector<uint64_t> AdminList = {
            1005925886332, 1009358754932, 1009170064275, 1009567761626, 1003597319170, 1006764780045, 1000050911301, 1003135122829, 1002891191646, 2250191893, 1001437176616, 1002169134673, 1006361385238, 1012440181244, 1006270645275, 1008890995363,
                1004874025016, 1009985214916, 1007651102246, 1008742738373, 1008654450535, 2316806708, 1012581167536, 1000105936709, 1008897046431, 1002905770119, 1014014708121, 1009271383747, 1009421384892, 1008263887754, 1009925474849, 1008296445608, 1006806947777, 1008939370770, 1010874466343,
                1011091498411, 1009259705635, 1012484574645, 1001953753749, 1007896757931, 1008921103204, 1007281853595, 1007716701440, 1011244886000, 1007716701440, 100834875807, 1008786303990, 100801124966, 1010071713191,
                1000476312989, 1008718596481, 100033778093, 1009821823447, 1007848623124, 1006623034196, 1007970864045, 1007903032308, 1008053680955, 11011844131064, 1009332336890, 1008850500345, 1011259862838, 1009101881635, 1006623034196, 1004588431813, 2818762051, 1002896792850, 1005716934920,
                2630146110, 1010987266469, 1007400077911, 1006346684588, 1006043333028, 1007857499119, 1012677552858, 1008621412819, 1011613872194, 1012676925005, 1009010701702, 2299876744, 1007926889395, 1008753319886, 1009026725698, 1007351062257, 2796198857, 1003683527065, 1003135122829,
                1007974856521, 1002650384273, 1010831759397, 1010608223121, 1009142575769, 1010605516073, 1008893919249, 1014101759425, 2289832195, 1008935753684, 1003883215136, 1007882413578, 1007775721356, 1011038384612, 1006807337873, 1008588433684, 1004719847420, 1008767702836, 2460931426,
                1008996919246, 1005622597106, 1008679516046, 1012798983197, 1009472954428, 1009375230185, 1000139018547, 1007104002644, 2403040211, 1007610466386, 1008729761283, 1008087845521, 1012429795168, 2843051300, 1009156112040, 1007839656086, 1008856502905, 1002654942758, 1002037198565, 1000380902857,
                1010315216177, 1010465377990, 1005963113414, 1006651982189, 1012295384000, 1009919873056, 1011920570828, 1007022292352, 1007956264701, 2361173869, 1012645502211, 1009177975708, 1009349753318, 1012016778141, 1008807806584, 1013071184653, 1011248902977, 1009668604017, 1010583192512, 2402108786,
                1003743785988, 1008884319166, 1007648564562, 1007856372567, 1007824423634, 1000308169569, 1006578609480, 1011823183474, 1003853420879, 1009869945505, 1010152055016, 1010872331092, 1006623034196, 1000451336870, 1008213374340, 1007972058040, 1011925559786, 1010302639654, 1011492753583, 1008172207747,
                1003279794876, 1009026175349, 1003506356103, 1009053337224, 1007358685861, 1008864797287, 1009077511495, 1009590163522, 1002292933320, 2292200863, 1007340027525, 1004868074776, 1006345544495, 2419930962, 1003804048029, 1010269317470, 1003649150165, 1004795895477, 1011740583759, 2270970192, 1007972058040,
                2308756905, 1009337890049, 1013268043057, 1007863297360, 1004547042543, 1009044672858, 1009211985280, 1007333548851, 1012048159430, 1008922783948, 1007972058040, 1010553263709, 1003089965155, 1004338230298, 1012841500387, 1008965520265, 1006496436518, 1012771159292, 1009375405345, 1000701256864, 2808308967,
                1009882813641, 1011521583642, 1011650112271, 1003935249690, 1009497970203, 2421096545, 1010496510143, 1008254274350, 1007677647640, 1010943274769, 2816169741, 1008633771133, 1012848392702, 1007763929469, 1008786911422, 1000194939529, 1007496859881, 1005911887174, 1010072362480, 1007559430115, 1007389129295,
                1009219995093, 1008187499396, 1009558433788, 2322529980, 1007917656958, 1009753445967, 1007382032061, 1009620099575, 2457473946, 1010132807955, 1010576394474, 2634654542, 1003625169092, 1008659680052, 1000281568697, 2471101342, 1009976640607, 1009148750696, 1009233592678, 1000641526219, 1012263516749, 1007728598765,
                1009525113150, 1006808508233, 1010791304069, 1006155443541, 1009222900705, 1008051679564,
                2290117003, 2315548295, 2364404148, 2402108786, 2414503637, 2433227295, 2546369203, 2716963345, 2806117224,
                1009656454625, 1011101685612 ,2479489866
        };
        if (Target->GlowEnable == 0 && Target->GlowThroughWall == 0 && Target->HighlightID == 0) {
			return;
		}
        uint64_t basePointer = Target->BasePointer;

        int settingIndex = 65;
        std::array<float, 3> glowColorRGB = { 0, 0, 0 };

        if (!isVisible) {
            settingIndex = 65;
            glowColorRGB = { InvisibleGlowColor[0], InvisibleGlowColor[1], InvisibleGlowColor[2] }; // Invisible Enemies
        }
        else if (isVisible) {
            settingIndex = 70;
            glowColorRGB = { VisibleGlowColor[0], VisibleGlowColor[1], VisibleGlowColor[2] }; // Visible Enemies
        }
        else if (isUIDInAdminList(playeruid, AdminList)) {
            settingIndex = 75;
            glowColorRGB = { AdminColor[0], AdminColor[1], AdminColor[2] }; //Admin Players
        }

        if (Target->GlowEnable != enable) {
            uint64_t glowEnableAddress = basePointer + OFF_GLOW_ENABLE;
            mem.Write<int>(glowEnableAddress, enable);
        }

        if (Target->GlowThroughWall != wall) {
            uint64_t glowThroughWallAddress = basePointer + OFF_GLOW_THROUGH_WALL;
            mem.Write<int>(glowThroughWallAddress, wall);
        }

        uint64_t highlightIdAddress = basePointer + OFF_GLOW_HIGHLIGHT_ID;
        unsigned char value = settingIndex;
        mem.Write<unsigned char>(highlightIdAddress, value);

        uint64_t glowFixAddress = basePointer + OFF_GLOW_FIX;
        mem.Write<int>(glowFixAddress, 0);
    }

    void setHighlightSettings() {
        int InvisibleIndex = 65; // Invis
        int VisibleIndex = 70; // Vis
        int AdminIndex = 75;
        std::array<unsigned char, 4> highlightFunctionBits = {
            insidetype,   // InsideFunction							2
            outlinetype, // OutlineFunction: HIGHLIGHT_OUTLINE_OBJECTIVE			125
            outlinesize,  // OutlineRadius: size * 255 / 8				64
            64   // (EntityVisible << 6) | State & 0x3F | (AfterPostProcess << 7) 	64
        };
        std::array<float, 3> invisibleGlowColorRGB = { rnot, gnot , bnot };
        std::array<float, 3> visibleGlowColorRGB = { rviz, gviz , bviz };
        std::array<float, 3> adminGlowColorRGB = { radm, gadm , badm };

        uint64_t highlightSettingsPtr = HighlightSettingsPointer;
        if (mem.IsValidPointer(highlightSettingsPtr)) {
            auto handle = mem.CreateScatterHandle();

            // Invisible
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * InvisibleIndex + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * InvisibleIndex + 0x4, &invisibleGlowColorRGB, sizeof(invisibleGlowColorRGB));

            // Visible
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * VisibleIndex + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * VisibleIndex + 0x4, &visibleGlowColorRGB, sizeof(visibleGlowColorRGB));

            //Admin
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * AdminIndex + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
            mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * AdminIndex + 0x4, &adminGlowColorRGB, sizeof(adminGlowColorRGB));

            mem.ExecuteWriteScatter(handle);
            mem.CloseScatterHandle(handle);
        }

    }

    void enableViewModelGlow(uint64_t ViewModelPtr) {
        if (ViewModelGlow == true)
        {
            std::array<unsigned char, 4> highlightFunctionBits = {
                0,   // InsideFunction  HIGHLIGHT_FILL_LOOT_SCANNED
                101, // OutlineFunction HIGHLIGHT_OUTLINE_LOOT_SCANNED
                150,
                100
            };
            int contextId = 8;
            if (Spectators->spec_glow == 50) {
                contextId = 9;
            }
            std::array<float, 3> highlightParameter = { Spectators->spec_glow, 10 , 1 };
            int THROUGH_WALLS = 2;
            int settingIndex = 84;
            uint64_t highlightSettingsPtr = HighlightSettingsPointer;
            if (mem.IsValidPointer(highlightSettingsPtr) && mem.IsValidPointer(ViewModelPtr)) {
                auto handle = mem.CreateScatterHandle();
                mem.AddScatterWriteRequest(handle, ViewModelPtr + OFF_GLOW_ENABLE, &contextId, sizeof(contextId));
                mem.AddScatterWriteRequest(handle, ViewModelPtr + OFF_GLOW_THROUGH_WALL, &THROUGH_WALLS, sizeof(THROUGH_WALLS));
                mem.AddScatterWriteRequest(handle, ViewModelPtr + OFF_GLOW_HIGHLIGHT_ID, &settingIndex, sizeof(settingIndex));
                mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * contextId + 0x0, &highlightFunctionBits, sizeof(highlightFunctionBits));
                mem.AddScatterWriteRequest(handle, highlightSettingsPtr + OFF_GLOW_HIGHLIGHT_TYPE_SIZE * contextId + 0x4, &highlightParameter, sizeof(highlightParameter));
                mem.ExecuteWriteScatter(handle);
                mem.CloseScatterHandle(handle);
            }
        }
        else{
                int zeroValue = 0;
                int minusOne = -1;
                auto handle = mem.CreateScatterHandle();
                mem.AddScatterWriteRequest(handle, ViewModelPtr + OFF_GLOW_THROUGH_WALL, &zeroValue, sizeof(zeroValue));
                mem.AddScatterWriteRequest(handle, ViewModelPtr + OFF_GLOW_ENABLE, &minusOne, sizeof(minusOne));
                mem.ExecuteWriteScatter(handle);
                mem.CloseScatterHandle(handle);
        }
    }

    Vector2D DummyVector = { 0, 0 };
    void Update() {
        if (Myself->IsDead) return;

        if (ItemGlow) {
            uint64_t highlightSettingsPtr = HighlightSettingsPointer;
            if (mem.IsValidPointer(highlightSettingsPtr)) {
                uint64_t highlightSize = OFF_GLOW_HIGHLIGHT_TYPE_SIZE;
                const GlowMode newGlowMode = { 137,138,35,127 };
                for (int highlightId = MinimumItemRarity; highlightId < 39; highlightId++)
                {
                    const GlowMode oldGlowMode = mem.Read<GlowMode>(highlightSettingsPtr + (highlightSize * highlightId) + 0, true);
                    if (newGlowMode != oldGlowMode) {
                        mem.Write<GlowMode>(highlightSettingsPtr + (highlightSize * highlightId) + 0, newGlowMode);
                    }
                }
            }
        }

        for (int i = 0; i < Players->size(); i++) {
            Player* Target = Players->at(i);
            if (!Target->IsValid()) continue;
            if (Target->IsDummy()) continue;
            if (Target->IsLocal) continue;
            if (!Target->IsHostile) continue;

            if (GameCamera->WorldToScreen(Target->LocalOrigin.ModifyZ(30), DummyVector)) {
                setCustomGlow(Target, 1, 1, Target->IsVisible);
            }
        }

        setHighlightSettings();
        //enableViewModelGlow(ViewModelPtr);
    }
};