# 📍 Ghidra Bookmarks - Resident Evil 4

## 🎮 Jogo: RE4 Original (bio4.exe)
**MD5:** bc342c1d5060166c26c515375c2...

## Como Usar Bookmarks
- **Ctrl + D** = Criar bookmark
- **Ctrl + B** = Abrir lista de bookmarks
- **;** = Adicionar comentário de linha
- **Ctrl + Shift + ;** = Comentário de placa (plate comment)

---

## 🔖 Bookmarks por Categoria

### 🎮 GAME_LOOP
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | WinMain | Entry point do programa |
| `0x________` | main_loop | Loop principal do jogo |
| `0x________` | game_update | Atualização de frame |
| `0x________` | render_frame | Função de renderização D3D |

### 👤 PLAYER_LOGIC (Leon)
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | leon_update | Atualiza estado do Leon |
| `0x________` | leon_move | Processa movimento |
| `0x________` | leon_damage | Aplica dano ao Leon |
| `0x________` | leon_heal | Cura o Leon |
| `0x________` | leon_spawn | Cria instância do Leon |
| `0x________` | leon_weapon_fire | Leon atira |
| `0x________` | leon_reload | Leon recarrega |
| `0x________` | leon_melee | Leon ataque corpo-a-corpo |

### 👧 ASHLEY_LOGIC (Player 2 alvo!)
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | ashley_update | Atualiza Ashley |
| `0x________` | ashley_ai | IA da Ashley (HOOK AQUI!) |
| `0x________` | ashley_follow | Lógica de seguir Leon |
| `0x________` | ashley_hide | Ashley se esconde |
| `0x________` | ashley_damage | Ashley leva dano |
| `0x________` | ashley_spawn | Cria Ashley |
| `0x________` | ashley_piggyback | Leon carrega Ashley |

### 🎮 INPUT_SYSTEM (DirectInput)
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | DirectInput8Create | Cria objeto DirectInput |
| `0x________` | GetDeviceState | Lê estado do controle (CRÍTICO!) |
| `0x________` | read_input | Wrapper de leitura |
| `0x________` | process_input | Converte input em ações |
| `0x________` | input_buffer | Buffer de comandos |

### 👾 ENEMY_SYSTEM
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | enemy_spawn | Spawn de inimigos |
| `0x________` | enemy_update | Atualiza inimigos |
| `0x________` | enemy_ai | IA dos inimigos |
| `0x________` | enemy_damage | Inimigo leva dano |
| `0x________` | enemy_death | Inimigo morre |

### 📦 MEMORY_MANAGEMENT
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | alloc_entity | Aloca memória para entidade |
| `0x________` | free_entity | Libera memória |
| `0x________` | entity_pool | Pool de entidades |

### 🎨 RENDERING
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | draw_model | Desenha modelo 3D |
| `0x________` | camera_update | Atualiza câmera |
| `0x________` | animation_play | Toca animação |

### 📁 FILE_SYSTEM
| Endereço | Nome | Descrição |
|----------|------|-----------|
| `0x________` | load_stage | Carrega fase |
| `0x________` | load_model | Carrega modelo |
| `0x________` | load_archive | Carrega arquivo .arc/.rdb |

---

## 🔍 Strings Importantes do RE4

```
Endereço    | String           | Significado
------------|------------------|-------------
0x________  | "pl00.dat"       | Modelo do Leon
0x________  | "pl01.dat"       | Modelo da Ashley
0x________  | "pl02.dat"       | Leon alternate
0x________  | "pl03.dat"       | Ashley alternate
0x________  | "em%02d.dat"     | Modelos de inimigos
0x________  | "st%02d%c.dat"   | Dados de stages
0x________  | "r%03d.dat"      | Dados de rooms
0x________  | "weapon"         | Sistema de armas
0x________  | "inventory"      | Inventário
```

## 🔗 Imports Críticos (dinput8.dll)

```
Endereço    | Função                  | Uso
------------|-------------------------|-----
0x________  | DirectInput8Create      | Inicializa DirectInput
0x________  | IDirectInput8::CreateDevice | Cria device do controle
0x________  | IDirectInputDevice8::GetDeviceState | LÊ O CONTROLE!
0x________  | IDirectInputDevice8::Acquire | Adquire controle
0x________  | IDirectInputDevice8::Poll | Atualiza estado
```

## 🔗 Imports de D3D9

```
Endereço    | Função                  | Uso
------------|-------------------------|-----
0x________  | Direct3DCreate9         | Cria objeto D3D
0x________  | CreateDevice            | Cria device de render
0x________  | BeginScene              | Início do frame
0x________  | EndScene                | Fim do frame
0x________  | Present                 | Mostra na tela
```

---

## 📝 Notas de Sessão

### Sessão 1 - [DATA]
- Encontrado: 
- Próximo passo:

### Sessão 2 - [DATA]
- Encontrado:
- Próximo passo:
