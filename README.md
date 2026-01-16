# 🎮 Resident Evil 4 (PC Original) - Co-op Mod

## 📋 Informações do Binário

| Campo | Valor |
|-------|-------|
| **Jogo** | Resident Evil 4 (2005 PC Port / Steam) |
| **Executável** | bio4.exe |
| **Arquitetura** | x86 (32-bit) |
| **MD5** | bc342c1d5060166c26c515375c2... |
| **Caminho** | `C:\Program Files (x86)\Steam\steamapps\common\Resident Evil 4\Bin32\` |
| **Engine** | Proprietária Capcom |

---

## 🔍 Análise Inicial no Ghidra

### Passo 1: Abrir e Analisar
1. ✅ Você já abriu o bio4.exe no Ghidra
2. Clique duplo no `bio4.exe` para abrir o CodeBrowser
3. Quando perguntar se quer analisar, clique **Yes**
4. Deixe todas opções padrão e clique **Analyze**
5. Aguarde a análise terminar (pode demorar 5-15 minutos)

### Passo 2: Verificar Imports (CRÍTICO)
Vá em **Window → Symbol Tree** e expanda **Imports**

Procure por:
- `dinput8.dll` ou `xinput*.dll` → Sistema de Input
- `d3d9.dll` ou `d3d8.dll` → DirectX (renderização)
- `dsound.dll` → Áudio
- `ws2_32.dll` → Se tiver, tem código de rede

### Passo 3: Buscar Strings Úteis
1. **Search → For Strings...**
2. Minimum Length: 5
3. Clique **Search**

Procure por:
- `player` → Estruturas do jogador
- `leon` → Modelo/animações do Leon
- `ashley` → Ashley (ela já existe no jogo!)
- `health` ou `life` → Sistema de vida
- `stage` ou `room` → Sistema de fases
- `.dat`, `.bin`, `.arc` → Arquivos de dados

---

## 🎯 Estratégia para Co-op no RE4

### A Grande Sacada: Ashley já existe!

O RE4 **já tem um segundo personagem**: Ashley! Isso significa:
- ✅ O jogo já sabe renderizar 2 personagens
- ✅ Já tem sistema de IA para segundo personagem
- ✅ Já tem colisão entre personagens
- ✅ Algumas fases já suportam ela

**Estratégia Principal:** Transformar Ashley em personagem jogável pelo Player 2!

### Alternativa: Clonar Leon
Se quiser dois Leons:
- Criar segunda instância da struct Player
- Duplicar processamento de input
- Renderizar segundo modelo

---

## 🔬 O Que Precisamos Encontrar

### 1. Player Structure (Leon)
```c
// Tamanho estimado: ~0x2000 bytes
struct Player {
    /* 0x0000 */ void* vtable;
    /* 0x???? */ float pos_x;           // Posição X
    /* 0x???? */ float pos_y;           // Posição Y (altura)
    /* 0x???? */ float pos_z;           // Posição Z
    /* 0x???? */ float rotation;        // Rotação Y
    /* 0x???? */ int hp;                // Vida (max 1000-1200)
    /* 0x???? */ int max_hp;            // Vida máxima
    /* 0x???? */ int state;             // Estado atual
    /* 0x???? */ int weapon_id;         // Arma equipada
    /* 0x???? */ int ammo;              // Munição atual
    /* 0x???? */ void* inventory;       // Inventário
    /* 0x???? */ void* model;           // Modelo 3D
    /* 0x???? */ int animation;         // Animação atual
};
```

### 2. Ashley Structure
```c
struct Ashley {
    /* Similar ao Player, mas com IA */
    /* 0x???? */ float pos_x, pos_y, pos_z;
    /* 0x???? */ int hp;                // Ashley TEM HP!
    /* 0x???? */ int ai_state;          // Estado da IA
    /* 0x???? */ bool is_following;     // Seguindo Leon
    /* 0x???? */ bool is_hiding;        // Escondida
    /* 0x???? */ void* target;          // Quem ela segue
};
```

### 3. Input System
```c
struct InputState {
    int buttons;                // Botões pressionados
    float left_x, left_y;       // Analógico esquerdo
    float right_x, right_y;     // Analógico direito (câmera)
};
```

---

## 📁 Arquivos do RE4

```
Resident Evil 4/
├── Bin32/
│   └── bio4.exe              ← Estamos analisando
├── BIO4/
│   ├── pl/
│   │   ├── pl00.dat          ← Leon (normal)
│   │   ├── pl01.dat          ← Ashley!
│   │   ├── pl02.dat          ← Leon (jacket)
│   │   └── pl03.dat          ← Ashley (armor)
│   ├── em/
│   │   └── em*.dat           ← Inimigos
│   ├── st/
│   │   └── st*.dat           ← Stages
│   └── event/
│       └── *.dat             ← Cutscenes
```

---

## 🪝 Hooks Necessários

### Hook 1: Input - Ler Segundo Controle
```c
// Interceptar leitura de input
void Hooked_ReadInput() {
    Original_ReadInput();  // P1
    
    if (g_coopEnabled) {
        ReadController2Input(&g_input_p2);
    }
}
```

### Hook 2: Ashley Update - Controle Manual
```c
// Substituir IA da Ashley por input do P2
void Hooked_AshleyUpdate(Ashley* ashley) {
    if (g_coopEnabled && g_controller2_connected) {
        // P2 controla Ashley
        ProcessMovementInput(ashley, &g_input_p2);
        ProcessActionInput(ashley, &g_input_p2);
    } else {
        // IA original
        Original_AshleyAI(ashley);
    }
}
```

### Hook 3: Dar Armas para Ashley
```c
// Ashley não usa armas originalmente
void EnableAshleyWeapons(Ashley* ashley) {
    // Copiar sistema de armas do Leon
    ashley->can_shoot = true;
    ashley->weapon_id = WEAPON_HANDGUN;
    ashley->ammo = 30;
}
```

### Hook 4: Câmera
```c
void Hooked_CameraUpdate(Camera* cam) {
    if (g_splitScreen) {
        // Split screen vertical
        RenderHalfScreen(LEFT, g_leon);
        RenderHalfScreen(RIGHT, g_ashley);
    } else {
        // Câmera compartilhada - segue ponto médio
        cam->target = MidPoint(g_leon->pos, g_ashley->pos);
        
        // Ajusta zoom baseado na distância
        float dist = Distance(g_leon, g_ashley);
        cam->distance = BASE_DISTANCE + dist * 0.3f;
    }
}
```

---

## 🔧 Técnicas de Busca

### Método 1: Cheat Engine → Ghidra

1. **Encontrar HP no Cheat Engine:**
   - Inicie RE4
   - Busque `1000` (HP full Normal) ou `1200` (Easy)
   - Leve dano de um inimigo
   - Busque novo valor
   - Repita até achar o endereço certo

2. **No Ghidra:**
   - Vá para esse endereço
   - Clique direito → References → Find References to...
   - Encontre funções que ESCREVEM nele
   - Uma delas é `ApplyDamage` ou `TakeDamage`

### Método 2: Strings

No Ghidra: **Search → For Strings**

| String a buscar | O que encontrar |
|-----------------|-----------------|
| `pl00` | Carregamento do Leon |
| `pl01` | Carregamento da Ashley |
| `health` | Sistema de vida |
| `damage` | Sistema de dano |
| `weapon` | Sistema de armas |
| `stage` | Sistema de fases |

### Método 3: Imports de DirectInput

1. Symbol Tree → Imports → dinput8.dll
2. Encontre `GetDeviceState`
3. Veja quem chama essa função
4. Essa é a função de leitura de input!

---

## 📚 Recursos Importantes

### Projetos Open Source de Referência

1. **RE4 Tweaks** - https://github.com/nipkownern/re4_tweaks
   - Código fonte de hooks no RE4!
   - Mostra como interceptar funções
   - Excelente referência

2. **RE4 HD Project** - https://www.re4hd.com/
   - Texturas HD
   - Documentação da comunidade

3. **Fluffy Manager** - Gerenciador de mods

### Documentação Existente

A comunidade de RE4 já mapeou várias coisas:
- Formatos de arquivo (.dat, .bin)
- Estrutura de memória (parcial)
- Tabela de IDs de inimigos/armas

---

## ✅ Checklist de Progresso

### Fase 1: Setup ✅
- [x] Abrir bio4.exe no Ghidra
- [ ] Completar análise automática
- [ ] Verificar imports
- [ ] Buscar strings

### Fase 2: Mapeamento
- [ ] Encontrar HP do Leon (Cheat Engine)
- [ ] Encontrar posição X/Y/Z
- [ ] Mapear Player struct
- [ ] Encontrar Ashley struct
- [ ] Encontrar função de input

### Fase 3: Hooks Básicos
- [ ] Hook de input funcionando
- [ ] Ler segundo controle
- [ ] Ashley responde ao P2

### Fase 4: Gameplay
- [ ] Ashley pode atirar
- [ ] Câmera ajustada
- [ ] Não quebra cutscenes
- [ ] Co-op jogável!

---

## 🎮 Próximo Passo Agora

1. **Clique duplo no bio4.exe** no Ghidra
2. **Aceite analisar** (Yes)
3. **Aguarde** a análise completar
4. Me avise quando terminar e me mande um print!

Vou te guiar passo a passo para encontrar as estruturas!
