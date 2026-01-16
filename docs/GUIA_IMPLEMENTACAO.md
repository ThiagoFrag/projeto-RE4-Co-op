# 🚀 GUIA DE IMPLEMENTAÇÃO - RE4 CO-OP MOD

## 📋 PRÉ-REQUISITOS

### Software Necessário
- **Visual Studio 2019/2022** (Community é suficiente)
- **Windows SDK** (10.0 ou superior)
- **Git** para clonar repositórios
- **Ghidra** (opcional, para análise adicional)

### Repositórios para Clonar
```bash
# SDK e código base do re4_tweaks
git clone https://github.com/nipkownix/re4_tweaks.git

# Research e IDA database
git clone https://github.com/emoose/re4-research.git
```

### Downloads
- **IDA Database** (80% funções nomeadas): 
  https://www.mediafire.com/file/4t4l7xo7rjrrh6a/bio4-221105.zip/file

---

## 🔧 SETUP DO PROJETO

### 1. Clonar re4_tweaks como Base
```bash
cd C:\Projects
git clone https://github.com/nipkownix/re4_tweaks.git re4_coop
cd re4_coop
```

### 2. Abrir no Visual Studio
1. Abra `re4_tweaks.sln`
2. Configure para **Release | x86** (RE4 é 32-bit!)
3. Build inicial para verificar que compila

### 3. Estrutura de Arquivos Importantes
```
re4_tweaks/
├── dllmain/
│   ├── SDK/              ★ Estruturas do jogo!
│   │   ├── player.h      - cPlayer class
│   │   ├── em.h          - cEm base class
│   │   ├── model.h       - cModel com posição
│   │   ├── pad.h         - Input/gamepad
│   │   ├── global.h      - Enums, GLOBALS
│   │   └── ...
│   ├── Game.cpp          ★ PlayerPtr(), AshleyPtr()
│   ├── Game.h            - Includes do SDK
│   ├── Trainer.cpp       ★ MoveAshleyToPlayer()
│   ├── input.cpp         - Sistema de input
│   └── ControllerTweaks.cpp
└── Wrappers/             - DLL wrapper (dinput8)
```

---

## 📝 FASE 1: CAPTURA DO CONTROLLER 2

### Arquivo: `CoopInput.cpp`

```cpp
#include <Xinput.h>
#pragma comment(lib, "xinput.lib")

struct CoopInput {
    float moveX, moveY;
    float lookX, lookY;
    bool aim, shoot, action, run;
    bool reload, knife, inventory;
    bool connected;
};

CoopInput g_P2_Input;

void UpdatePlayer2Input() {
    XINPUT_STATE state;
    ZeroMemory(&state, sizeof(state));
    
    // Controller index 1 = segundo controle
    if (XInputGetState(1, &state) == ERROR_SUCCESS) {
        g_P2_Input.connected = true;
        
        // Analógicos
        g_P2_Input.moveX = state.Gamepad.sThumbLX / 32768.0f;
        g_P2_Input.moveY = state.Gamepad.sThumbLY / 32768.0f;
        g_P2_Input.lookX = state.Gamepad.sThumbRX / 32768.0f;
        g_P2_Input.lookY = state.Gamepad.sThumbRY / 32768.0f;
        
        // Triggers
        float lt = state.Gamepad.bLeftTrigger / 255.0f;
        float rt = state.Gamepad.bRightTrigger / 255.0f;
        
        g_P2_Input.aim = lt > 0.5f;
        g_P2_Input.shoot = rt > 0.5f;
        
        // Botões
        g_P2_Input.action = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
        g_P2_Input.run = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
        g_P2_Input.reload = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
        g_P2_Input.knife = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
        g_P2_Input.inventory = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
    } else {
        g_P2_Input.connected = false;
    }
}
```

---

## 📝 FASE 2: HOOK DA IA DA ASHLEY

### Conceito
Quando P2 está conectado, **não executamos a IA normal** da Ashley.
Em vez disso, aplicamos o input do P2.

### Encontrar Função de IA
1. No IDA/Ghidra, procure por:
   - `cSubChar` (classe de personagens secundários)
   - `Ashley` nas strings
   - Referências a `pAS_ptr`

2. A função de update provavelmente:
   - É chamada todo frame
   - Acessa posição da Ashley
   - Calcula direção para seguir Leon
   - Define animação

### Código do Hook

```cpp
// Tipo da função original
typedef void(__fastcall* AshleyUpdate_Fn)(cPlayer* ashley, void* unused);
AshleyUpdate_Fn Original_AshleyUpdate = nullptr;

// Nossa função hook
void __fastcall AshleyUpdate_Hook(cPlayer* ashley, void* unused) {
    // Se co-op ativo e P2 conectado
    if (g_CoopEnabled && g_P2_Input.connected) {
        // Não executa IA - aplica input do P2
        ApplyP2InputToAshley(ashley);
        return;
    }
    
    // Caso contrário, IA normal
    Original_AshleyUpdate(ashley, unused);
}

// Instalação do hook
void InstallAshleyHook() {
    // Encontra função via pattern
    // Este pattern é EXEMPLO - precisa descobrir o real no IDA
    auto pattern = hook::pattern("55 8B EC 83 EC ?? 53 56 57 8B F9 89 7D ??");
    
    if (pattern.count() > 0) {
        Original_AshleyUpdate = (AshleyUpdate_Fn)pattern.get_first();
        
        // Redireciona para nossa função
        InjectHook(pattern.get_first(), AshleyUpdate_Hook, PATCH_JUMP);
    }
}
```

---

## 📝 FASE 3: MOVIMENTO DA ASHLEY

### Aplicar Input ao Personagem

```cpp
void ApplyP2InputToAshley(cPlayer* ashley) {
    if (!ashley) return;
    
    // Constantes
    const float MOVE_SPEED = 8.0f;
    const float TURN_SPEED = 0.1f;
    
    // Obtém posição atual (offset 0x94 em cCoord)
    Vec* pos = &ashley->pos_94;
    
    // Calcula movimento
    if (fabsf(g_P2_Input.moveX) > 0.2f || 
        fabsf(g_P2_Input.moveY) > 0.2f) {
        
        // Movimento relativo à câmera
        // TODO: Obter direção da câmera
        
        // Por enquanto, movimento absoluto
        pos->x += g_P2_Input.moveX * MOVE_SPEED;
        pos->z += g_P2_Input.moveY * MOVE_SPEED;
        
        // Triggar animação de andar
        // ashley->Animation = ANIM_WALK;
    }
    
    // Rotação
    if (fabsf(g_P2_Input.lookX) > 0.2f) {
        // TODO: Rotacionar modelo
    }
}
```

---

## 📝 FASE 4: ARMAS PARA ASHLEY

### O Desafio
Ashley normalmente **não pode atacar**. Precisamos:
1. Dar arma a ela
2. Permitir que entre em estado de mira
3. Processar o tiro
4. Animar corretamente

### Análise Necessária
1. Estudar como Leon troca de arma
2. Ver se Ashley tem slots de arma
3. Verificar checks de tipo de personagem

### Código Conceitual

```cpp
void EnableAshleyCombat() {
    cPlayer* ashley = AshleyPtr();
    if (!ashley) return;
    
    // Verifica se tem pointer de arma
    if (ashley->Wep_7D8 == nullptr) {
        // Cria/associa arma
        // Isso é complexo - precisa alocar cPlWep
        // e associar modelo de arma
    }
    
    // Remove checks que impedem ataque
    // Procure patterns como:
    // CMP player_type, 1  ; Ashley
    // JE skip_attack
    
    // NOP esses jumps para permitir ataque
}
```

---

## 📝 FASE 5: CÂMERA CO-OP

### Opção A: Câmera Dinâmica (Mais Fácil)
```cpp
void UpdateCoopCamera() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    if (!leon || !ashley) return;
    
    // Calcula ponto médio
    Vec mid = {
        (leon->pos_94.x + ashley->pos_94.x) / 2,
        (leon->pos_94.y + ashley->pos_94.y) / 2,
        (leon->pos_94.z + ashley->pos_94.z) / 2
    };
    
    // Ajusta target da câmera
    CamCtrl->target = mid;
    
    // Zoom out se distantes
    float dist = CalculateDistance(leon->pos_94, ashley->pos_94);
    if (dist > 500.0f) {
        CamCtrl->Fovy = min(CamCtrl->Fovy + 0.5f, 90.0f);
    }
}
```

### Opção B: Split-Screen (Avançado)
Isso requer:
1. Renderizar cena 2 vezes
2. Ajustar viewport para cada metade
3. Câmera independente para cada player

```cpp
void RenderSplitScreen() {
    // Salva viewport original
    D3DVIEWPORT9 originalVP;
    device->GetViewport(&originalVP);
    
    // Viewport esquerda (Leon)
    D3DVIEWPORT9 leftVP = originalVP;
    leftVP.Width /= 2;
    device->SetViewport(&leftVP);
    RenderFromCamera(leon_camera);
    
    // Viewport direita (Ashley)
    D3DVIEWPORT9 rightVP = originalVP;
    rightVP.X = originalVP.Width / 2;
    rightVP.Width /= 2;
    device->SetViewport(&rightVP);
    RenderFromCamera(ashley_camera);
    
    // Restaura
    device->SetViewport(&originalVP);
}
```

---

## 🎯 PATTERNS ÚTEIS DO RE4_TWEAKS

### Encontrar pPL_ptr (Leon)
```cpp
// Pattern usado no re4_tweaks
pattern = hook::pattern("8B 0D ? ? ? ? 85 C9 74 ? 81 39");
pPL_ptr = *pattern.count(1).get(0).get<cPlayer**>(2);
```

### Encontrar pAS_ptr (Ashley)
```cpp
pattern = hook::pattern("A8 02 74 16 8B 15");
pAS_ptr = *pattern.count(1).get(0).get<cPlayer**>(6);
```

### Hook de Weapon Init
```cpp
pattern = hook::pattern("83 C4 0C E8 ? ? ? ? D9 EE 8B 06");
ReadCall(injector::GetBranchDestination(pattern.get_first(3)), 
         cPlayer__weaponInit);
```

---

## 🔍 DEBUG E TESTE

### Menu de Debug do Jogo
O re4_tweaks já habilita o **Tool Menu** (menu de debug):
1. Configure `bEnableDebugMenu = true` no .ini
2. Pressione F1 no jogo
3. Acessa "TOOL MENU"

### Nosso Debug
```cpp
// Mostrar info na tela
void DrawDebugInfo() {
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (leon && ashley) {
        float dist = CalculateDistance(leon->pos_94, ashley->pos_94);
        
        char buf[256];
        sprintf(buf, "P1: %.1f, %.1f, %.1f\n"
                     "P2: %.1f, %.1f, %.1f\n"
                     "Dist: %.1f\n"
                     "P2 Connected: %s",
                leon->pos_94.x, leon->pos_94.y, leon->pos_94.z,
                ashley->pos_94.x, ashley->pos_94.y, ashley->pos_94.z,
                dist,
                g_P2_Input.connected ? "YES" : "NO");
        
        DrawText(10, 10, buf);
    }
}
```

---

## 📦 BUILD E DEPLOY

### Compilar
1. Visual Studio: **Release | x86**
2. Build Solution (F7)
3. Arquivo gerado: `dinput8.dll`

### Instalar
```
Copy dinput8.dll to:
C:\Program Files (x86)\Steam\steamapps\common\Resident Evil 4\Bin32\
```

### Testar
1. Conecte 2 controles
2. Inicie o jogo
3. Carregue uma fase com Ashley
4. Pressione botões no Controller 2

---

## ⚠️ PROBLEMAS COMUNS

### Crash ao Iniciar
- Verifique se compilou para x86 (não x64!)
- Verifique patterns - podem mudar entre versões

### Ashley Não Responde
- Confirme que Controller 2 está no índice 1
- Verifique se a IA está sendo hookeada

### Animações Bugadas
- Ashley pode não ter todas animações de combate
- Pode precisar usar animações do Leon

### Colisão Errada
- Ao teleportar, desabilite colisão
- Reabilite depois de setar posição

---

## 📚 RECURSOS ADICIONAIS

- **re4_tweaks Wiki:** https://github.com/nipkownix/re4_tweaks/wiki
- **IDA Database:** 80% das funções documentadas
- **RE4 HD Project:** Comunidade ativa de modding

---

**Boa sorte com o desenvolvimento! 🎮**
