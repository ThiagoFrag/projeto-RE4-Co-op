/**
 * RE4 CO-OP MOD - Implementação Principal
 * 
 * Sistema de controle do Player 2 através da Ashley
 */

#include "coop_core.h"
#include <cmath>

//=============================================================================
// VARIÁVEIS GLOBAIS
//=============================================================================

CoopInput g_P2_Input;
CoopConfig g_CoopConfig;

// Ponteiros que serão encontrados via pattern
cPlayer** pPL_ptr = nullptr;
cPlayer** pAS_ptr = nullptr;

// Backup de estado da Ashley
static uint16_t s_AshleyCollisionBackup = 0;
static bool s_AshleyControlTaken = false;

namespace CoopMod {
namespace Hooks {
    void* Original_AshleyAI = nullptr;
    void* Original_CameraUpdate = nullptr;
}

//=============================================================================
// INICIALIZAÇÃO
//=============================================================================

bool Initialize() {
    // Inicializa input do P2
    g_P2_Input.Reset();
    g_P2_Input.connected = false;
    
    // Carrega configurações
    g_CoopConfig = CoopConfig();
    
    // Instala hooks
    Hooks::InstallInputHook();
    Hooks::InstallAshleyAIHook();
    Hooks::InstallCameraHook();
    
    // Verifica se encontrou ponteiros
    if (!pPL_ptr || !pAS_ptr) {
        // Tenta encontrar via pattern matching
        // Pattern do re4_tweaks para pAS_ptr:
        // pattern = hook::pattern("A8 02 74 16 8B 15");
        // pAS_ptr = *pattern.count(1).get(0).get<cPlayer**>(6);
        
        return false;
    }
    
    return true;
}

void Shutdown() {
    // Libera controle da Ashley se estiver tomado
    if (s_AshleyControlTaken) {
        ReleaseAshleyControl();
    }
    
    g_CoopConfig.enabled = false;
}

//=============================================================================
// LOOP PRINCIPAL
//=============================================================================

void Update() {
    if (!g_CoopConfig.enabled) return;
    
    // Atualiza input do Player 2
    UpdatePlayer2Input();
    
    // Se Controller 2 não conectado, não faz nada
    if (!g_P2_Input.connected) return;
    
    // Obtém ponteiros
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (!leon || !ashley) return;
    
    // Toma controle da Ashley se ainda não tomou
    if (!s_AshleyControlTaken) {
        TakeOverAshleyControl();
    }
    
    // Aplica input do P2 na Ashley
    ApplyInputToAshley(ashley, g_P2_Input);
    
    // Verifica distância e teleporta se necessário
    if (g_CoopConfig.teleportIfTooFar) {
        float dist = GetPlayerDistance();
        if (dist > g_CoopConfig.maxDistance) {
            TeleportAshleyToLeon();
        }
    }
    
    // Atualiza câmera para mostrar os dois
    UpdateCoopCamera();
}

//=============================================================================
// SISTEMA DE INPUT
//=============================================================================

void UpdatePlayer2Input() {
    // Aqui você implementa a leitura do Controller 2
    // Pode usar DirectInput ou XInput
    
    // Exemplo com XInput (precisa linkar xinput.lib):
    /*
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(XINPUT_STATE));
    
    // Controller 1 é índice 0, Controller 2 é índice 1
    DWORD result = XInputGetState(1, &state);
    
    if (result == ERROR_SUCCESS) {
        g_P2_Input.connected = true;
        
        // Analógico esquerdo (movimento)
        g_P2_Input.moveX = state.Gamepad.sThumbLX / 32768.0f;
        g_P2_Input.moveY = state.Gamepad.sThumbLY / 32768.0f;
        
        // Analógico direito (câmera)
        g_P2_Input.lookX = state.Gamepad.sThumbRX / 32768.0f;
        g_P2_Input.lookY = state.Gamepad.sThumbRY / 32768.0f;
        
        // Triggers
        g_P2_Input.leftTrigger = state.Gamepad.bLeftTrigger / 255.0f;
        g_P2_Input.rightTrigger = state.Gamepad.bRightTrigger / 255.0f;
        
        // Botões
        g_P2_Input.aim = g_P2_Input.leftTrigger > 0.5f;
        g_P2_Input.shoot = g_P2_Input.rightTrigger > 0.5f;
        g_P2_Input.action = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
        g_P2_Input.run = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
        g_P2_Input.reload = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
        g_P2_Input.knife = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
        g_P2_Input.inventory = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        g_P2_Input.map = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
    } else {
        g_P2_Input.connected = false;
    }
    */
}

bool IsController2Connected() {
    return g_P2_Input.connected;
}

//=============================================================================
// CONTROLE DA ASHLEY
//=============================================================================

void TakeOverAshleyControl() {
    cPlayer* ashley = AshleyPtr();
    if (!ashley) return;
    
    // Backup das flags de colisão
    uint16_t* collisionFlags = (uint16_t*)((uint8_t*)ashley + Offsets::ATARI_FLAG);
    s_AshleyCollisionBackup = *collisionFlags;
    
    // Habilita combate para Ashley
    if (g_CoopConfig.ashleyHasWeapons) {
        EnableAshleyCombat(ashley);
    }
    
    s_AshleyControlTaken = true;
}

void ReleaseAshleyControl() {
    cPlayer* ashley = AshleyPtr();
    if (ashley) {
        // Restaura flags de colisão
        uint16_t* collisionFlags = (uint16_t*)((uint8_t*)ashley + Offsets::ATARI_FLAG);
        *collisionFlags = s_AshleyCollisionBackup;
    }
    
    s_AshleyControlTaken = false;
}

void ApplyInputToAshley(cPlayer* ashley, const CoopInput& input) {
    if (!ashley) return;
    
    // Obtém posição atual
    Vec* pos = (Vec*)((uint8_t*)ashley + Offsets::POS);
    
    // Aplica movimento
    if (fabsf(input.moveX) > 0.1f || fabsf(input.moveY) > 0.1f) {
        // Velocidade de movimento
        const float MOVE_SPEED = 5.0f;
        
        // Move baseado no input
        pos->x += input.moveX * MOVE_SPEED;
        pos->z += input.moveY * MOVE_SPEED;
        
        // TODO: Rotacionar na direção do movimento
        // TODO: Triggar animação de andar
    }
    
    // Processa mira
    if (input.aim && g_CoopConfig.ashleyHasWeapons) {
        ProcessAshleyAim(ashley);
    }
    
    // Processa tiro
    if (input.shoot && input.aim && g_CoopConfig.ashleyHasWeapons) {
        ProcessAshleyShoot(ashley);
    }
    
    // TODO: Processar outras ações (ação contextual, correr, etc)
}

//=============================================================================
// COMBATE DA ASHLEY
//=============================================================================

void EnableAshleyCombat(cPlayer* ashley) {
    if (!ashley) return;
    
    // Verifica se já tem arma
    cPlWep** weaponPtr = (cPlWep**)((uint8_t*)ashley + Offsets::WEAPON_PTR);
    
    if (*weaponPtr == nullptr) {
        // Dar arma inicial (pistola)
        // Aqui precisamos chamar a função do jogo para equipar arma
        // GiveWeaponToAshley(ITEM_ID_HANDGUN);
    }
    
    // Modifica flags para permitir ataque
    uint32_t* flags = (uint32_t*)((uint8_t*)ashley + Offsets::PLAYER_FLAG);
    // TODO: Descobrir flag exata para habilitar ataque
    // *flags |= PLAYER_CAN_ATTACK_FLAG;
}

void GiveWeaponToAshley(int weaponId) {
    cPlayer* ashley = AshleyPtr();
    if (!ashley) return;
    
    // TODO: Chamar função do jogo para dar item
    // Provavelmente: bio4::PutInCase(ITEM_ID, quantidade, tamanho);
    // Ou modificar diretamente o inventário
}

void ProcessAshleyAim(cPlayer* ashley) {
    if (!ashley) return;
    
    // TODO: Colocar Ashley em estado de mira
    // Provavelmente mudar stat_468 para estado de AIM
    // E triggar animação de mira
}

void ProcessAshleyShoot(cPlayer* ashley) {
    if (!ashley) return;
    
    // TODO: Processar tiro
    // 1. Verificar se tem arma
    // 2. Verificar se tem munição
    // 3. Fazer raycast para acertar
    // 4. Aplicar dano
    // 5. Reduzir munição
    // 6. Tocar som
    // 7. Efeito visual
}

//=============================================================================
// SISTEMA DE CÂMERA
//=============================================================================

void UpdateCoopCamera() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (!leon || !ashley) return;
    
    Vec leonPos = GET_POS(leon);
    Vec ashleyPos = GET_POS(ashley);
    
    float distance = CalculateDistance(leonPos, ashleyPos);
    
    // Se usando split-screen, não precisa ajustar câmera
    if (g_CoopConfig.splitScreen) {
        // TODO: Implementar split-screen
        // Isso é BEM mais complexo - precisa renderizar cena 2x
        return;
    }
    
    // Câmera dinâmica - foca no ponto médio
    Vec midpoint = CalculateMidpoint(leonPos, ashleyPos);
    
    // TODO: Acessar CameraControl e ajustar
    // CameraControl* cam = CamCtrl;
    // if (cam) {
    //     // Ajusta target da câmera
    //     cam->target = midpoint;
    //     
    //     // Zoom out se os players estão longe
    //     if (distance > 500.0f) {
    //         cam->Fovy *= 1.1f; // Aumenta FOV para ver mais
    //     }
    // }
}

float GetPlayerDistance() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (!leon || !ashley) return 0.0f;
    
    Vec leonPos = GET_POS(leon);
    Vec ashleyPos = GET_POS(ashley);
    
    return CalculateDistance(leonPos, ashleyPos);
}

Vec GetMidpointBetweenPlayers() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (!leon || !ashley) return {0, 0, 0};
    
    Vec leonPos = GET_POS(leon);
    Vec ashleyPos = GET_POS(ashley);
    
    return CalculateMidpoint(leonPos, ashleyPos);
}

//=============================================================================
// UTILITÁRIOS
//=============================================================================

void TeleportAshleyToLeon() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (!leon || !ashley) return;
    
    Vec leonPos = GET_POS(leon);
    
    // Teleporta Ashley para perto do Leon (com offset)
    Vec newPos = leonPos;
    newPos.x += 100.0f;  // 1 metro ao lado
    
    // Desabilita colisão temporariamente para evitar bugs
    DisableCollision(ashley);
    
    SET_POS(ashley, newPos);
    
    // Reabilita colisão
    EnableCollision(ashley);
}

void SyncPositions() {
    // Usado principalmente para debug
    CopyPosition(PlayerPtr(), AshleyPtr());
}

//=============================================================================
// HOOKS
//=============================================================================

namespace Hooks {

void InstallInputHook() {
    // Hook no sistema de input para capturar Controller 2
    
    // O re4_tweaks já tem hooks de input em input.cpp
    // Podemos usar padrões similares:
    
    // Para DirectInput:
    // Hook IDirectInputDevice8::GetDeviceState
    
    // Para XInput:
    // Hook XInputGetState
}

void InstallAshleyAIHook() {
    // Hook na função de IA da Ashley para desativá-la
    // quando o P2 está controlando
    
    // Precisamos encontrar a função de update da Ashley
    // Provavelmente algo como cSubChar::update ou similar
    
    // Pattern aproximado (precisa verificar):
    // auto pattern = hook::pattern("Ashley AI pattern here");
    // ReadCall(pattern.get_first(), Original_AshleyAI);
    // InjectHook(pattern.get_first(), AshleyAI_Hook, PATCH_JUMP);
}

void InstallCameraHook() {
    // Hook no sistema de câmera para ajustar para co-op
    
    // O CameraControl já está documentado no SDK
    // Função: CameraControl::update ou similar
}

void InstallWeaponHook() {
    // Hook para permitir Ashley usar armas
    
    // Pode haver checks que impedem Ashley de atacar
    // Precisamos bypassar esses checks
}

} // namespace Hooks
} // namespace CoopMod

//=============================================================================
// ENTRY POINT DA DLL
//=============================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            
            // Inicializa o mod em thread separada
            // CreateThread(NULL, 0, InitThread, hModule, 0, NULL);
            break;
            
        case DLL_PROCESS_DETACH:
            CoopMod::Shutdown();
            break;
    }
    return TRUE;
}
