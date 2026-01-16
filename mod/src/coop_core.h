/**
 * RE4 CO-OP MOD - Código Principal
 * 
 * Baseado no SDK do re4_tweaks por nipkownix e emoose
 * https://github.com/nipkownix/re4_tweaks
 * 
 * Este código usa as estruturas REAIS do jogo documentadas
 * através de anos de reverse engineering da comunidade!
 */

#pragma once
#include <Windows.h>
#include <cstdint>
#include <dinput.h>

//=============================================================================
// ESTRUTURAS DO JOGO (do SDK re4_tweaks)
//=============================================================================

// Vec - Vetor 3D usado em todo o jogo
struct Vec {
    float x, y, z;
};

// Enum de tipos de jogador
enum class PlayerCharacter : uint8_t {
    Leon = 0x0,
    Ashley = 0x1,
    Ada = 0x2,
    HUNK = 0x3,
    Krauser = 0x4,
    Wesker = 0x5,
    LeonAshley = 0x6,
};

// Enum de costumes da Ashley
enum class AshleyCostume : uint8_t {
    Normal,
    Popstar,
    Armor  // Não leva dano!
};

// Forward declarations
class cModel;
class cEm;
class cPlayer;
class cPlWep;
class CameraControl;

//=============================================================================
// PONTEIROS GLOBAIS (encontrados via pattern matching)
//=============================================================================

// Ponteiros para Player e Ashley (já documentados no re4_tweaks)
extern cPlayer** pPL_ptr;  // Leon/Player atual
extern cPlayer** pAS_ptr;  // Ashley

// Funções para obter ponteiros
inline cPlayer* PlayerPtr() {
    if (!pPL_ptr || !*pPL_ptr) return nullptr;
    if (**(int**)pPL_ptr < 0x10000) return nullptr;
    return *pPL_ptr;
}

inline cPlayer* AshleyPtr() {
    if (!pAS_ptr || !*pAS_ptr) return nullptr;
    if (**(int**)pAS_ptr < 0x10000) return nullptr;
    return *pAS_ptr;
}

//=============================================================================
// SISTEMA DE INPUT DO PLAYER 2
//=============================================================================

struct CoopInput {
    // Movimento (analógico esquerdo)
    float moveX;         // -1.0 a 1.0
    float moveY;         // -1.0 a 1.0
    
    // Câmera (analógico direito)
    float lookX;         // -1.0 a 1.0  
    float lookY;         // -1.0 a 1.0
    
    // Botões
    bool aim;            // LT - Mirar
    bool shoot;          // RT - Atirar
    bool action;         // A - Ação
    bool run;            // B - Correr
    bool reload;         // X - Recarregar
    bool knife;          // Y - Faca
    bool inventory;      // Start - Inventário
    bool map;            // Back - Mapa
    
    // Triggers
    float leftTrigger;   // 0.0 a 1.0
    float rightTrigger;  // 0.0 a 1.0
    
    // Estado
    bool connected;      // Controller 2 está conectado?
    
    void Reset() {
        moveX = moveY = 0.0f;
        lookX = lookY = 0.0f;
        aim = shoot = action = run = false;
        reload = knife = inventory = map = false;
        leftTrigger = rightTrigger = 0.0f;
    }
};

// Input global do Player 2
extern CoopInput g_P2_Input;

//=============================================================================
// CONFIGURAÇÕES DO CO-OP
//=============================================================================

struct CoopConfig {
    bool enabled;                    // Co-op está ativo?
    bool ashleyHasWeapons;           // Ashley pode usar armas?
    bool shareInventory;             // Inventário compartilhado?
    bool splitScreen;                // Usar split-screen?
    float maxDistance;               // Distância máxima entre players
    bool teleportIfTooFar;           // Teleportar se muito longe?
    
    CoopConfig() {
        enabled = false;
        ashleyHasWeapons = true;
        shareInventory = true;
        splitScreen = false;
        maxDistance = 2000.0f;
        teleportIfTooFar = true;
    }
};

extern CoopConfig g_CoopConfig;

//=============================================================================
// FUNÇÕES PRINCIPAIS DO MOD
//=============================================================================

namespace CoopMod {
    
    // Inicialização
    bool Initialize();
    void Shutdown();
    
    // Update loop (chamado todo frame)
    void Update();
    
    // Sistema de input
    void UpdatePlayer2Input();
    bool IsController2Connected();
    void ProcessController2(DIJOYSTATE2* state);
    
    // Controle da Ashley
    void TakeOverAshleyControl();
    void ReleaseAshleyControl();
    void ApplyInputToAshley(cPlayer* ashley, const CoopInput& input);
    
    // Sistema de combate para Ashley
    void EnableAshleyCombat(cPlayer* ashley);
    void GiveWeaponToAshley(int weaponId);
    void ProcessAshleyAim(cPlayer* ashley);
    void ProcessAshleyShoot(cPlayer* ashley);
    
    // Câmera
    void UpdateCoopCamera();
    float GetPlayerDistance();
    Vec GetMidpointBetweenPlayers();
    
    // Utilitários
    void TeleportAshleyToLeon();
    void SyncPositions();
    
    // Hooks (instalados na inicialização)
    namespace Hooks {
        void InstallInputHook();
        void InstallAshleyAIHook();
        void InstallCameraHook();
        void InstallWeaponHook();
        
        // Funções originais (para chamar depois do hook)
        extern void* Original_AshleyAI;
        extern void* Original_CameraUpdate;
    }
}

//=============================================================================
// OFFSETS IMPORTANTES (versão 1.1.0)
//=============================================================================

namespace Offsets {
    // Dentro de cEm (base entity)
    constexpr uint32_t HP = 0x324;
    constexpr uint32_t HP_MAX = 0x326;
    constexpr uint32_t FLAGS = 0x3E8;
    
    // Dentro de cModel/cCoord (posição)
    constexpr uint32_t POS = 0x94;          // Vec pos
    constexpr uint32_t POS_OLD = 0x110;     // Vec pos_old
    
    // Dentro de cPlayer
    constexpr uint32_t PLAYER_FLAG = 0x464;
    constexpr uint32_t PLAYER_STATE = 0x468;
    constexpr uint32_t WEAPON_PTR = 0x7D8;  // cPlWep*
    constexpr uint32_t LASER_TYPE = 0x804;
    
    // Atributos de colisão
    constexpr uint32_t ATARI_FLAG = 0x2B4 + 0x1A;  // atari.m_flag
    
    // Globais
    constexpr uint32_t GLOBALS_BASE = 0x85A760;
    constexpr uint32_t JOY_ARRAY = 0xC63008;   // Joy[4]
    constexpr uint32_t KEY_INPUT = 0xC62EF0;
    
    // Player type está em GLOBALS + 0x4FC8
    constexpr uint32_t PLAYER_TYPE = 0x4FC8;
    constexpr uint32_t PLAYER_COSTUME = 0x4FC9;
    constexpr uint32_t SUB_COSTUME = 0x4FCB;    // Ashley costume
}

//=============================================================================
// FLAGS DE COLISÃO
//=============================================================================

enum CollisionFlags : uint16_t {
    SAT_SCA_ENABLE = 0x0001,  // Colisão com mapa
    SAT_OBA_ENABLE = 0x0002,  // Colisão com entidades
};

//=============================================================================
// MACROS DE ACESSO RÁPIDO
//=============================================================================

// Acesso a HP
#define GET_HP(entity) (*(int16_t*)((uint8_t*)(entity) + Offsets::HP))
#define SET_HP(entity, val) (*(int16_t*)((uint8_t*)(entity) + Offsets::HP) = (val))

// Acesso a posição
#define GET_POS(entity) (*(Vec*)((uint8_t*)(entity) + Offsets::POS))
#define SET_POS(entity, pos) (*(Vec*)((uint8_t*)(entity) + Offsets::POS) = (pos))

// Acesso a flags
#define GET_FLAGS(entity) (*(uint32_t*)((uint8_t*)(entity) + Offsets::FLAGS))
#define SET_FLAGS(entity, val) (*(uint32_t*)((uint8_t*)(entity) + Offsets::FLAGS) = (val))

//=============================================================================
// IMPLEMENTAÇÃO DAS FUNÇÕES CORE
//=============================================================================

namespace CoopMod {
    
    inline float CalculateDistance(const Vec& a, const Vec& b) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        float dz = a.z - b.z;
        return sqrtf(dx*dx + dy*dy + dz*dz);
    }
    
    inline Vec CalculateMidpoint(const Vec& a, const Vec& b) {
        return {
            (a.x + b.x) / 2.0f,
            (a.y + b.y) / 2.0f,
            (a.z + b.z) / 2.0f
        };
    }
    
    inline void CopyPosition(cPlayer* from, cPlayer* to) {
        if (!from || !to) return;
        
        Vec* fromPos = (Vec*)((uint8_t*)from + Offsets::POS);
        Vec* toPos = (Vec*)((uint8_t*)to + Offsets::POS);
        Vec* fromPosOld = (Vec*)((uint8_t*)from + Offsets::POS_OLD);
        Vec* toPosOld = (Vec*)((uint8_t*)to + Offsets::POS_OLD);
        
        *toPos = *fromPos;
        *toPosOld = *fromPosOld;
    }
    
    inline void DisableCollision(cPlayer* entity) {
        if (!entity) return;
        
        uint16_t* flags = (uint16_t*)((uint8_t*)entity + Offsets::ATARI_FLAG);
        *flags &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE);
    }
    
    inline void EnableCollision(cPlayer* entity) {
        if (!entity) return;
        
        uint16_t* flags = (uint16_t*)((uint8_t*)entity + Offsets::ATARI_FLAG);
        *flags |= (SAT_SCA_ENABLE | SAT_OBA_ENABLE);
    }
}

//=============================================================================
// NOTA SOBRE HOOKS
//=============================================================================

/**
 * Para implementar os hooks, use o padrão do re4_tweaks:
 * 
 * 1. Use "hook::pattern" para encontrar padrões no código
 * 2. Use "injector::MakeInline" para inline hooks
 * 3. Use "InjectHook" para substituir chamadas
 * 4. Use "ReadCall" para ler endereço de chamadas
 * 
 * Exemplo do re4_tweaks para hook da weaponInit:
 * 
 *   auto pattern = hook::pattern("83 C4 0C E8 ? ? ? ? D9 EE 8B 06");
 *   ReadCall(injector::GetBranchDestination(pattern.get_first(3)), cPlayer__weaponInit);
 *   InjectHook(injector::GetBranchDestination(pattern.get_first(3)), 
 *              cPlayer__weaponInit_Hook, PATCH_JUMP);
 */
