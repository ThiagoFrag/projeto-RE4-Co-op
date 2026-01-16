# 🪝 Hooks para Co-op Mod - Resident Evil

## 🎯 O Que São Hooks?

Hooks são pontos onde injetamos código personalizado no jogo original.
Para co-op, precisamos de hooks em locais estratégicos.

---

## 📍 Hooks Necessários para Co-op

### 1. 🎮 Input Hook (CRÍTICO)

**Objetivo:** Fazer o jogo ler input de 2 controles

```
Endereço Original: 0x________
Função: read_input() ou similar
```

**Estratégia:**
```c
// Hook original
typedef void (*ReadInput_t)(InputState* input);
ReadInput_t original_ReadInput;

// Nossa função
void hooked_ReadInput(InputState* input) {
    // Chama original para Player 1
    original_ReadInput(input);
    
    // Lê input do controle 2 para Player 2
    if (g_player2 != NULL) {
        InputState input2;
        ReadController2Input(&input2);
        ProcessPlayer2Input(g_player2, &input2);
    }
}
```

---

### 2. 👤 Player Update Hook

**Objetivo:** Atualizar segundo jogador junto com o primeiro

```
Endereço Original: 0x________
Função: player_update() ou similar
```

**Estratégia:**
```c
typedef void (*PlayerUpdate_t)(Player* player, float deltaTime);
PlayerUpdate_t original_PlayerUpdate;

void hooked_PlayerUpdate(Player* player, float deltaTime) {
    // Atualiza Player 1
    original_PlayerUpdate(player, deltaTime);
    
    // Atualiza Player 2
    if (g_player2 != NULL) {
        original_PlayerUpdate(g_player2, deltaTime);
    }
}
```

---

### 3. 🎨 Render Hook

**Objetivo:** Renderizar modelo do segundo jogador

```
Endereço Original: 0x________
Função: render_entities() ou draw_player()
```

**Estratégia:**
```c
typedef void (*RenderPlayer_t)(Player* player);
RenderPlayer_t original_RenderPlayer;

void hooked_RenderPlayer(Player* player) {
    // Renderiza Player 1
    original_RenderPlayer(player);
    
    // Renderiza Player 2
    if (g_player2 != NULL) {
        original_RenderPlayer(g_player2);
    }
}
```

---

### 4. 🏁 Spawn Hook

**Objetivo:** Criar Player 2 quando Player 1 é criado

```
Endereço Original: 0x________
Função: spawn_player() ou create_player()
```

**Estratégia:**
```c
typedef Player* (*SpawnPlayer_t)(SpawnInfo* info);
SpawnPlayer_t original_SpawnPlayer;

Player* hooked_SpawnPlayer(SpawnInfo* info) {
    // Cria Player 1 normalmente
    Player* player1 = original_SpawnPlayer(info);
    g_player1 = player1;
    
    // Cria Player 2 ao lado
    SpawnInfo info2 = *info;
    info2.pos_x += 2.0f;  // Offset para não sobrepor
    info2.model_id = PLAYER2_MODEL;  // Modelo diferente (Claire, Leon, etc)
    
    g_player2 = original_SpawnPlayer(&info2);
    
    return player1;
}
```

---

### 5. 📷 Camera Hook (Opcional)

**Objetivo:** Ajustar câmera para mostrar ambos jogadores

```
Endereço Original: 0x________
Função: camera_update() ou update_camera()
```

**Estratégia:**
```c
void hooked_CameraUpdate(Camera* cam) {
    if (g_player2 == NULL) {
        original_CameraUpdate(cam);
        return;
    }
    
    // Calcula ponto médio entre os dois jogadores
    float mid_x = (g_player1->pos_x + g_player2->pos_x) / 2.0f;
    float mid_z = (g_player1->pos_z + g_player2->pos_z) / 2.0f;
    
    // Calcula distância para ajustar zoom
    float distance = CalcDistance(g_player1, g_player2);
    
    // Ajusta câmera
    cam->look_x = mid_x;
    cam->look_z = mid_z;
    cam->fov = BASE_FOV + (distance * 0.5f);  // Zoom out conforme se afastam
    
    original_CameraUpdate(cam);
}
```

---

### 6. 💀 Damage Hook

**Objetivo:** Garantir que dano seja aplicado corretamente a cada jogador

```
Endereço Original: 0x________
Função: apply_damage() ou player_take_damage()
```

---

### 7. 🚪 Room Transition Hook

**Objetivo:** Teleportar Player 2 junto com Player 1

```
Endereço Original: 0x________
Função: change_room() ou load_stage()
```

---

## 🔧 Técnicas de Hooking

### Método 1: Inline Hook (x86)
```asm
; Original: 5 bytes no início da função
push ebp           ; 55
mov ebp, esp       ; 89 E5
sub esp, ...       ; 83 EC __

; Substituímos por:
jmp our_function   ; E9 __ __ __ __  (5 bytes)
```

### Método 2: VTable Hook
Se o jogo usa classes C++ com vtables:
```c
void** vtable = *(void***)player_object;
original_func = vtable[FUNC_INDEX];
vtable[FUNC_INDEX] = our_function;
```

### Método 3: Import Address Table (IAT) Hook
Para funções de sistema (DirectInput, etc):
```c
// Encontra IAT
// Substitui ponteiro de função
```

---

## 📁 Estrutura da DLL de Injeção

```
mod_coop.dll
├── dllmain.cpp       # Entry point
├── hooks.cpp         # Instalação dos hooks
├── player2.cpp       # Lógica do Player 2
├── input.cpp         # Leitura do segundo controle
├── network.cpp       # (Opcional) Para co-op online
└── utils.cpp         # Funções auxiliares
```

### dllmain.cpp Exemplo
```cpp
#include <Windows.h>
#include "hooks.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)InitMod, NULL, 0, NULL);
            break;
        case DLL_PROCESS_DETACH:
            UninstallHooks();
            break;
    }
    return TRUE;
}

void InitMod() {
    // Espera o jogo carregar
    Sleep(5000);
    
    // Encontra endereços
    FindGameFunctions();
    
    // Instala hooks
    InstallInputHook();
    InstallPlayerUpdateHook();
    InstallRenderHook();
    InstallSpawnHook();
    
    // Cria console para debug
    AllocConsole();
    printf("[CO-OP MOD] Inicializado!\n");
}
```

---

## ⚠️ Problemas Comuns

| Problema | Solução |
|----------|---------|
| Game crash ao hookear | Endereço errado ou tamanho do hook incorreto |
| Player 2 não aparece | Hook de render não está funcionando |
| Player 2 não move | Hook de input não captura controle 2 |
| Câmera bugada | Ajustar lógica de câmera split/shared |
| Colisão entre players | Desabilitar colisão player-player |
