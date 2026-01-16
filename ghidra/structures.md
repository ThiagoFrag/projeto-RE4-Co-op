# 🏗️ Estruturas de Dados - Resident Evil 4

## 🎮 Jogo: RE4 Original (bio4.exe)

## Como Criar Structures no Ghidra

1. **Window → Data Type Manager**
2. Clique direito na pasta do programa (bio4.exe)
3. **New → Structure**
4. Adicione campos com tipo e offset

---

## 👤 Leon (Player) Structure

### Endereço Base
```c
// Ponteiro global para o Leon
Player* g_Leon = *(Player**)0x________;
```

### Estrutura Mapeada

```c
// Resident Evil 4 - Player Structure
// Tamanho estimado: ~0x2000 bytes (8192)

struct Player_Leon {
    /* 0x0000 */ void* vtable;           // Virtual table
    /* 0x0004 */ int entity_id;          // ID único
    /* 0x0008 */ int entity_type;        // Tipo (0 = player)
    
    // === POSIÇÃO E ROTAÇÃO ===
    /* 0x???? */ float pos_x;            // Coordenada X
    /* 0x???? */ float pos_y;            // Coordenada Y (altura)
    /* 0x???? */ float pos_z;            // Coordenada Z
    /* 0x???? */ float rot_x;            // Rotação X (pitch)
    /* 0x???? */ float rot_y;            // Rotação Y (yaw) - direção
    /* 0x???? */ float rot_z;            // Rotação Z (roll)
    
    // === VIDA E STATUS ===
    /* 0x???? */ int hp;                 // HP atual (max: 1000-1200)
    /* 0x???? */ int hp_max;             // HP máximo
    /* 0x???? */ int status_effect;      // Envenenado, etc
    
    // === ESTADO E ANIMAÇÃO ===
    /* 0x???? */ int state;              // Estado atual (enum abaixo)
    /* 0x???? */ int prev_state;         // Estado anterior
    /* 0x???? */ int anim_id;            // Animação tocando
    /* 0x???? */ float anim_frame;       // Frame atual
    /* 0x???? */ float anim_speed;       // Velocidade da animação
    
    // === ARMAS ===
    /* 0x???? */ int weapon_id;          // Arma equipada
    /* 0x???? */ int ammo_current;       // Munição no pente
    /* 0x???? */ int ammo_reserve;       // Munição reserva
    /* 0x???? */ bool is_aiming;         // Está mirando
    /* 0x???? */ bool is_reloading;      // Está recarregando
    /* 0x???? */ float aim_x;            // Mira X
    /* 0x???? */ float aim_y;            // Mira Y
    
    // === INVENTÁRIO ===
    /* 0x???? */ void* inventory;        // Ponteiro para attache case
    /* 0x???? */ int money;              // Pesetas
    
    // === MODELO E RENDER ===
    /* 0x???? */ void* model_ptr;        // Modelo 3D (pl00.dat)
    /* 0x???? */ void* skeleton;         // Esqueleto
    /* 0x???? */ void* textures;         // Texturas
    
    // === FÍSICA ===
    /* 0x???? */ float velocity_x;       // Velocidade X
    /* 0x???? */ float velocity_y;       // Velocidade Y
    /* 0x???? */ float velocity_z;       // Velocidade Z
    /* 0x???? */ bool is_grounded;       // No chão
    
    // === FLAGS ===
    /* 0x???? */ int flags;              // Flags gerais
    /* 0x???? */ bool can_move;          // Pode se mover
    /* 0x???? */ bool can_attack;        // Pode atacar
    /* 0x???? */ bool is_invincible;     // Invencível (i-frames)
};
```

### Estados do Leon
```c
enum LeonState {
    STATE_IDLE           = 0,    // Parado
    STATE_WALKING        = 1,    // Andando
    STATE_RUNNING        = 2,    // Correndo
    STATE_AIMING         = 3,    // Mirando
    STATE_SHOOTING       = 4,    // Atirando
    STATE_RELOADING      = 5,    // Recarregando
    STATE_KNIFE_ATTACK   = 6,    // Atacando com faca
    STATE_MELEE          = 7,    // Roundhouse kick etc
    STATE_USING_ITEM     = 8,    // Usando item
    STATE_CLIMBING       = 9,    // Escalando
    STATE_JUMPING        = 10,   // Pulando
    STATE_DAMAGED        = 11,   // Levou dano
    STATE_GRABBED        = 12,   // Agarrado por inimigo
    STATE_DYING          = 13,   // Morrendo
    STATE_DEAD           = 14,   // Morto
    STATE_CUTSCENE       = 15,   // Em cutscene
    STATE_QTE            = 16,   // Quick Time Event
    STATE_LADDER         = 17,   // Na escada
    STATE_SWIMMING       = 18,   // Nadando (não usado)
};
```

### IDs de Armas
```c
enum WeaponID {
    WEAPON_NONE          = 0,
    WEAPON_HANDGUN       = 1,    // Pistola inicial
    WEAPON_SHOTGUN       = 2,
    WEAPON_TMP           = 3,    // Submetralhadora
    WEAPON_RIFLE         = 4,
    WEAPON_MAGNUM        = 5,
    WEAPON_KNIFE         = 6,
    WEAPON_GRENADE       = 7,
    WEAPON_FLASH_GRENADE = 8,
    WEAPON_INCENDIARY    = 9,
    WEAPON_ROCKET        = 10,
    // ... mais armas
};
```

---

## � Ashley Structure (IMPORTANTE PARA CO-OP!)

```c
// Ashley Graham - Nosso Player 2!
// Estrutura similar ao Player, mas com campos de IA

struct Ashley {
    /* 0x0000 */ void* vtable;
    /* 0x0004 */ int entity_id;
    /* 0x0008 */ int entity_type;        // Diferente do Leon
    
    // === POSIÇÃO ===
    /* 0x???? */ float pos_x;
    /* 0x???? */ float pos_y;
    /* 0x???? */ float pos_z;
    /* 0x???? */ float rot_y;
    
    // === VIDA ===
    /* 0x???? */ int hp;                 // Ashley TEM HP!
    /* 0x???? */ int hp_max;
    
    // === ESTADO ===
    /* 0x???? */ int state;
    /* 0x???? */ int anim_id;
    
    // === IA (SUBSTITUIR POR INPUT P2!) ===
    /* 0x???? */ int ai_state;           // Estado da IA
    /* 0x???? */ void* follow_target;    // Quem ela segue (Leon)
    /* 0x???? */ float follow_distance;  // Distância de follow
    /* 0x???? */ bool is_following;      // Está seguindo
    /* 0x???? */ bool is_hiding;         // Escondida em container
    /* 0x???? */ bool is_scared;         // Com medo
    /* 0x???? */ bool is_carried;        // Leon carregando ela
    
    // === COMANDOS DO LEON ===
    /* 0x???? */ int command;            // "Wait!" ou "Follow me!"
    
    // === MODELO ===
    /* 0x???? */ void* model;            // pl01.dat
};
```

### Estados da Ashley
```c
enum AshleyState {
    ASHLEY_IDLE       = 0,
    ASHLEY_FOLLOWING  = 1,
    ASHLEY_WAITING    = 2,
    ASHLEY_HIDING     = 3,
    ASHLEY_SCARED     = 4,
    ASHLEY_RUNNING    = 5,
    ASHLEY_GRABBED    = 6,   // Sequestrada por inimigo
    ASHLEY_CARRIED    = 7,   // Nos ombros do Leon
    ASHLEY_DEAD       = 8,
};

enum AshleyCommand {
    CMD_FOLLOW = 0,   // "Follow me!"
    CMD_WAIT   = 1,   // "Wait!"
};
```

### 🎯 OBJETIVO CO-OP:
Quando `ai_state` for atualizado, interceptar e usar input do Player 2 em vez da IA!

```c
// HOOK: Substituir IA por controle do P2
void Hooked_AshleyAI(Ashley* ashley) {
    if (g_coop_enabled && g_p2_connected) {
        // Ignora IA, usa input do P2
        ProcessPlayerInput((Player*)ashley, &g_input_p2);
        return;
    }
    // IA normal
    Original_AshleyAI(ashley);
}
```

---

## 📦 Inventory Structure

```c
struct InventorySlot {
    /* 0x00 */ int item_id;
    /* 0x04 */ int quantity;
    /* 0x08 */ int durability;           // Se aplicável
};

struct Inventory {
    /* 0x00 */ int slot_count;           // Número de slots
    /* 0x04 */ InventorySlot slots[__];  // Array de slots
};
```

---

## 🎮 Input Structure

```c
struct InputState {
    /* 0x00 */ int buttons_pressed;      // Botões pressionados agora
    /* 0x04 */ int buttons_held;         // Botões segurados
    /* 0x08 */ int buttons_released;     // Botões soltos
    /* 0x0C */ float left_stick_x;       // Analógico esquerdo X
    /* 0x10 */ float left_stick_y;       // Analógico esquerdo Y
    /* 0x14 */ float right_stick_x;      // Analógico direito X
    /* 0x18 */ float right_stick_y;      // Analógico direito Y
};

// Máscaras de botões (exemplo)
#define BTN_A       0x0001
#define BTN_B       0x0002
#define BTN_X       0x0004
#define BTN_Y       0x0008
#define BTN_START   0x0010
#define BTN_SELECT  0x0020
#define BTN_L1      0x0040
#define BTN_R1      0x0080
#define BTN_L2      0x0100
#define BTN_R2      0x0200
```

---

## 🎥 Camera Structure

```c
struct Camera {
    /* 0x00 */ float pos_x;
    /* 0x04 */ float pos_y;
    /* 0x08 */ float pos_z;
    /* 0x0C */ float look_x;             // Ponto de mira X
    /* 0x10 */ float look_y;             // Ponto de mira Y
    /* 0x14 */ float look_z;             // Ponto de mira Z
    /* 0x18 */ float fov;                // Field of View
    /* 0x1C */ int camera_mode;          // Modo (fixed, follow, etc)
};
```

---

## 📝 Como Encontrar Esses Valores

### Método 1: Cheat Engine
1. Abra o jogo e Cheat Engine
2. Procure valor de HP (ex: 100)
3. Leve dano, procure novo valor
4. Encontre endereço estável
5. Veja "o que acessa este endereço"
6. Copie endereço base para Ghidra

### Método 2: String Search no Ghidra
1. Search → For Strings
2. Procure "player", "health", "damage"
3. Siga as XREFs

### Método 3: Imports
1. Veja Symbol Tree → Imports
2. Procure DirectInput/XInput
3. Siga até a função de leitura
