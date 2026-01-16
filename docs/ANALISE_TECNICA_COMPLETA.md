# 🎮 RESIDENT EVIL 4 CO-OP MOD - ANÁLISE TÉCNICA REVOLUCIONÁRIA

## 📋 RESUMO EXECUTIVO

**Objetivo:** Criar o primeiro mod co-op REAL para Resident Evil 4 (2005 PC)

**Diferencial:** Usar toda a pesquisa já existente do `re4_tweaks` e IDA database com 80%+ das funções nomeadas para criar algo que nenhum outro mod conseguiu!

---

## 🔬 DESCOBERTAS FUNDAMENTAIS

### 1. SDK Completo Disponível!
O projeto **re4_tweaks** tem um SDK C++ completo com as estruturas reais do jogo:

**Repositório:** https://github.com/nipkownix/re4_tweaks
**SDK Path:** `/dllmain/SDK/`

### 2. IDA Database com 80% das Funções!
- **Download:** https://www.mediafire.com/file/4t4l7xo7rjrrh6a/bio4-221105.zip
- **Funções nomeadas:** 20181/25110 (80%)
- **Inclui:** structs, enums, variáveis globais

### 3. Estruturas REAIS do Jogo

---

## 🎯 ESTRUTURAS CRÍTICAS PARA CO-OP

### cPlayer (Estrutura do Jogador) - 0x814 bytes!
```cpp
// De: dllmain/SDK/player.h
class cPlayer : public cEm  // cEm = Entity base class
{
public:
    float m_Work0_408;
    float m_Work1_40C;
    float plunk_deltaTimeRelated_410;
    float plunk_field_414;
    float m_Work4_418;
    // ...
    
    Vec m_VecWork0_44C;          // Posição work
    Vec m_VecWork1_458;          // Posição work 2
    uint32_t m_Flag_464;         // Flags do player
    uint32_t stat_468;           // Estado atual
    
    uint32_t* m_MotTbl_46C;      // Tabela de animações
    uint32_t* m_MotTbl2_470;     // Tabela de animações 2
    MOTION_INFO m_SubMot_474;    // Info de movimento
    
    // Sistema de combate
    Vec m_Yarare_580[10];        // Hitboxes
    
    // Equipamento
    cPlWep* Wep_7D8;             // ARMA ATUAL!
    
    // Posição backup
    Vec pos_bak_7F8;
    
    // Tipo de laser
    LASER_TYPE laser_type_804;
    
    float m_invisi_rate_80C;     // Invisibilidade
    uint32_t pc_func_810;        // Função PC
};
assert_size(cPlayer, 0x814);  // TAMANHO CONFIRMADO!
```

### cEm (Entity Base) - 0x408 bytes
```cpp
// De: dllmain/SDK/em.h
class cEm : public cModel
{
public:
    int16_t hp_324;              // ★★★ HP DO PERSONAGEM! ★★★
    int16_t hp_max_326;          // HP máximo
    
    cDmgInfo m_DmgInfo_328;      // Info de dano
    YARARE_INFO yarare_344;      // Info de hit
    
    float l_pl_378;              // Distância do player
    float l_sub_37C;             // Distância do sub
    
    void* pFile_380;             // Arquivo
    void* pFile2_384;
    
    Vec target_offset0_388;      // Offset do target
    char target_parts0_394;
    
    uint8_t set_395;
    uint8_t emListIndex_3A0;     // Index na lista de entities
    
    uint32_t flags_3E8;          // FLAGS!
    Vec target_offset1_3F8;
};
```

### cModel (Base com Posição) - herda de cCoord
```cpp
// De: dllmain/SDK/model.h
// A posição está em cCoord que é base de cModel

// cCoord tem:
//   Vec pos_94;       // ★★★ POSIÇÃO X,Y,Z! ★★★
//   Vec pos_old_110;  // Posição anterior
```

### PlayerPtr() e AshleyPtr() - FUNÇÕES JÁ EXISTEM!
```cpp
// De: dllmain/Game.cpp
cPlayer** pPL_ptr = nullptr;
cPlayer* PlayerPtr()
{
    if (*pPL_ptr == nullptr)
        return nullptr;
    if (**(int**)pPL_ptr < 0x10000)
        return nullptr;
    return *pPL_ptr;
}

cPlayer** pAS_ptr = nullptr;
cPlayer* AshleyPtr()  // ★★★ ASHLEY JÁ TEM POINTER! ★★★
{
    if (*pAS_ptr == nullptr)
        return nullptr;
    if (**(int**)pAS_ptr < 0x10000)
        return nullptr;
    return *pAS_ptr;
}
```

---

## 🎮 SISTEMA DE INPUT

### PAD Structure
```cpp
// De: dllmain/SDK/pad.h
// O jogo suporta múltiplos controles!

enum PAD_BTN : uint32_t
{
    // Botões do controle
};

// Joy[0] = Primeiro controle
// Joy[1] = Segundo controle (JÁ EXISTE!)
// Joy[2], Joy[3] = Extras
```

---

## 📊 ENUMS IMPORTANTES

### PlayerCharacter - Tipos de Jogador
```cpp
enum class PlayerCharacter : uint8_t
{
    Leon = 0x0,
    Ashley = 0x1,      // ★ Ashley É UM TIPO DE PLAYER!
    Ada = 0x2,
    HUNK = 0x3,
    Krauser = 0x4,
    Wesker = 0x5,
    LeonAshley = 0x6,  // ★ Modo com os dois!
};
```

### PlayerCostume
```cpp
enum class LeonCostume : uint8_t
{
    Jacket,
    Normal,
    Vest,
    RPD,
    Mafia
};

enum class AshleyCostume : uint8_t
{
    Normal,
    Popstar,
    Armor    // ★ ARMADURA = Não leva dano!
};
```

### ActBtnText - Ações do Jogo
```cpp
enum ActBtnText : unsigned __int8
{
    ActBtn_Talk = 0x0,
    ActBtn_Check = 0x1,
    ActBtn_JumpOut = 0x2,
    ActBtn_JumpIn = 0x3,
    ActBtn_JumpDown = 0x4,
    ActBtn_JumpOver = 0x5,
    ActBtn_Push = 0x6,
    ActBtn_Kick = 0x7,
    // ... mais 60+ ações
    ActBtn_Suplex = 0x2C,
    ActBtn_Help = 0x15,
    ActBtn_Piggyback = 0x16,
    ActBtn_FollowMe = 0x21,
    // ...
};
```

---

## 🔧 HOOKS JÁ IMPLEMENTADOS NO RE4_TWEAKS

### Mover Ashley para o Player
```cpp
// De: dllmain/Trainer.cpp
void MoveAshleyToPlayer()
{
    cPlayer* ashley = AshleyPtr();
    cPlayer* player = PlayerPtr();

    if (player && ashley)
    {
        // Backup flags de colisão
        if (!ash_atariInfoFlagSet)
        {
            ash_atariInfoFlagBackup = ashley->atari_2B4.m_flag_1A;
            ash_atariInfoFlagSet = true;
        }
        
        // Desabilita colisão
        ashley->atari_2B4.m_flag_1A &= ~(SAT_SCA_ENABLE | SAT_OBA_ENABLE);

        // ★ COPIA POSIÇÃO! ★
        ashley->pos_94 = player->pos_94;
        ashley->pos_old_110 = player->pos_old_110;
    }
}
```

### Hook de Weapon Init
```cpp
void(__fastcall* cPlayer__weaponInit)(cPlayer* thisptr, void* unused);
void __fastcall cPlayer__weaponInit_Hook(cPlayer* thisptr, void* unused)
{
    // Pode adicionar armas à Ashley aqui!
}
```

---

## 🚀 PLANO DE IMPLEMENTAÇÃO DO CO-OP

### FASE 1: Input Splitting
1. Hook `dinput8.dll` para capturar Controller 2
2. Criar `g_Player2_Input` separado do input do Leon
3. Mapear botões do Controller 2

```cpp
struct Player2Input {
    Vec movement;      // Direção do analógico
    bool aim;          // Mira (LT)
    bool shoot;        // Tiro (RT)
    bool action;       // Ação (A)
    bool run;          // Correr (B)
    bool reload;       // Recarregar
    bool knife;        // Faca
    bool inventory;    // Inventário
};

Player2Input g_P2_Input;
```

### FASE 2: Ashley Control Takeover
1. Hook da função de IA da Ashley
2. Quando Controller 2 conectado, desativar IA
3. Aplicar input do P2 na Ashley

```cpp
void __fastcall Ashley_AI_Hook(cPlayer* ashley, void* unused)
{
    if (g_CoopEnabled && IsController2Connected())
    {
        // Não executa IA normal
        // Aplica input do Player 2
        ApplyPlayer2Input(ashley, g_P2_Input);
        return;
    }
    
    // IA normal da Ashley
    Original_Ashley_AI(ashley, unused);
}
```

### FASE 3: Combat para Ashley
1. Ashley normalmente não ataca
2. Precisamos habilitar:
   - Armas para ela
   - Animações de ataque
   - Sistema de mira

```cpp
void EnableAshleyCombat(cPlayer* ashley)
{
    // Dar arma inicial
    if (ashley->Wep_7D8 == nullptr)
    {
        // Equipar pistola básica
        GiveWeaponToAshley(WEAPON_HANDGUN);
    }
    
    // Habilitar flags de combate
    ashley->m_Flag_464 |= PLAYER_CAN_ATTACK;
}
```

### FASE 4: Camera System
O maior desafio! Opções:
1. **Split-screen:** Dividir tela em 2
2. **Dynamic camera:** Câmera que mostra os 2
3. **Teleport:** Se distância > X, teleporta P2

```cpp
void UpdateCoopCamera()
{
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    float distance = CalculateDistance(leon->pos_94, ashley->pos_94);
    
    if (distance > MAX_DISTANCE)
    {
        // Teleporta Ashley perto do Leon
        ashley->pos_94 = leon->pos_94;
        ashley->pos_94.x += 100.0f; // Offset
    }
    
    // Ajusta câmera para mostrar os dois
    CameraControl* cam = CamCtrl;
    Vec midpoint = CalculateMidpoint(leon->pos_94, ashley->pos_94);
    
    // Zoom out se necessário
    if (distance > 500.0f)
    {
        cam->Fovy *= 1.2f;
    }
}
```

---

## 📁 ARQUIVOS DO SDK PARA ESTUDAR

| Arquivo | Conteúdo |
|---------|----------|
| `SDK/player.h` | Estrutura cPlayer completa |
| `SDK/em.h` | Classe base cEm (entity) |
| `SDK/model.h` | cModel com posição |
| `SDK/pad.h` | Sistema de input |
| `SDK/global.h` | Variáveis globais, enums |
| `SDK/cam_ctrl.h` | Controle de câmera |
| `SDK/item.h` | Sistema de itens |
| `SDK/objWep.h` | Objetos de arma |
| `Game.cpp` | PlayerPtr(), AshleyPtr() |
| `Trainer.cpp` | MoveAshleyToPlayer() |

---

## 🎯 ENDEREÇOS CHAVE (v1.1.0)

| Item | Pattern/Info |
|------|--------------|
| `pPL_ptr` | Pattern: `"A8 02 74 16 8B 15"` +6 |
| `pAS_ptr` | Pattern: `"A8 02 74 16 8B 15"` similar |
| `GLOBALS` | Range: 0x85A760 - 0x862E00 |
| `Joy[4]` | Address: 0xC63008 |
| `Key` | Address: 0xC62EF0 |

---

## 🔥 POR QUE ESTE MOD SERÁ DIFERENTE

### Mods Co-op Anteriores (falhos):
1. **Parsec/Steam Remote Play:** Não é co-op real, é streaming
2. **Trainers com teleporte:** Sem controle real do P2
3. **Mods de modelo:** Só visual, sem gameplay

### Nosso Mod:
1. ✅ **Input real do Controller 2**
2. ✅ **Controle total da Ashley** 
3. ✅ **Combat habilitado**
4. ✅ **Baseado no SDK oficial do re4_tweaks**
5. ✅ **80% das funções já documentadas**
6. ✅ **Hooks já testados funcionando**

---

## 📋 PRÓXIMOS PASSOS

### Imediato:
1. [ ] Clonar re4_tweaks como base
2. [ ] Estudar `Trainer.cpp` - já tem código de manipular Ashley
3. [ ] Estudar `input.cpp` - sistema de input
4. [ ] Estudar `ControllerTweaks.cpp` - hooks de controle

### Desenvolvimento:
1. [ ] Implementar captura de Controller 2
2. [ ] Hook da IA da Ashley
3. [ ] Sistema de input splitting
4. [ ] Habilitar armas para Ashley
5. [ ] Ajustar câmera

### Polish:
1. [ ] HUD para P2
2. [ ] Inventário separado ou compartilhado
3. [ ] Balanceamento de dificuldade
4. [ ] Testes de estabilidade

---

## 🔗 RECURSOS

- **re4_tweaks repo:** https://github.com/nipkownix/re4_tweaks
- **re4-research:** https://github.com/emoose/re4-research
- **IDA Database:** https://www.mediafire.com/file/4t4l7xo7rjrrh6a/bio4-221105.zip
- **RE4 HD Project:** https://www.re4hd.com/

---

## 📜 CRÉDITOS TÉCNICOS

- **emoose:** IDA Database, research inicial
- **nipkownix:** re4_tweaks, SDK C++
- **MeganGrass:** Reverse engineering assistance
- **Comunidade RE4 Modding:** Documentação

---

**Este documento será atualizado conforme avançamos na análise!**

*Última atualização: Janeiro 2026*
