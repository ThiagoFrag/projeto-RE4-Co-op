# 📝 Progresso - RE4 Co-op Mod

## 🎮 Informações do Jogo

| Campo | Valor |
|-------|-------|
| **Jogo** | Resident Evil 4 (Steam) |
| **Executável** | bio4.exe |
| **MD5** | bc342c1d5060166c26c515375c2... |
| **Arquitetura** | x86 (32-bit) |
| **Ghidra Project** | CleanClean_Analysis |

---

## 📅 Sessões de Análise

### 16/01/2026 - Sessão 1: Setup Inicial

**Objetivo:** Configurar Ghidra e iniciar análise

**Status:**
- [x] bio4.exe importado no Ghidra
- [ ] Análise automática completada
- [ ] Imports verificados
- [ ] Strings buscadas

**Descobertas:**
- (preencher após análise)

**Próximo:**
- Completar análise
- Verificar dinput8.dll imports
- Buscar strings "ashley", "player"

---

### [DATA] - Sessão 2: Encontrar Player

**Objetivo:** Mapear estrutura do Leon

**Método:**
1. Cheat Engine: buscar HP (1000)
2. Encontrar endereço estável
3. Ver referências no Ghidra
4. Mapear struct

**Descobertas:**
- Endereço base Player: `0x________`
- Offset HP: `+0x____`
- Offset Pos X: `+0x____`
- Offset Pos Y: `+0x____`
- Offset Pos Z: `+0x____`
- Offset Rotation: `+0x____`

---

### [DATA] - Sessão 3: Encontrar Ashley

**Objetivo:** Mapear estrutura da Ashley

**Descobertas:**
- Endereço base Ashley: `0x________`
- Offset HP: `+0x____`
- Função de IA: `0x________`
- Função de Update: `0x________`

---

### [DATA] - Sessão 4: Sistema de Input

**Objetivo:** Entender leitura de controle

**Descobertas:**
- dinput8.dll usado: [ ] Sim [ ] Não
- Função GetDeviceState: `0x________`
- Função ProcessInput: `0x________`
- Buffer de input: `0x________`

---

## 🔍 Endereços Importantes Encontrados

### Funções
| Nome | Endereço | Descrição |
|------|----------|-----------|
| main | 0x________ | Entry point |
| GameLoop | 0x________ | Loop principal |
| PlayerUpdate | 0x________ | Atualiza Leon |
| AshleyUpdate | 0x________ | Atualiza Ashley |
| AshleyAI | 0x________ | IA da Ashley |
| ReadInput | 0x________ | Lê controle |
| RenderPlayer | 0x________ | Renderiza personagem |
| ApplyDamage | 0x________ | Aplica dano |
| LoadStage | 0x________ | Carrega fase |

### Variáveis Globais
| Nome | Endereço | Tipo | Descrição |
|------|----------|------|-----------|
| g_Player | 0x________ | Player* | Ponteiro para Leon |
| g_Ashley | 0x________ | Ashley* | Ponteiro para Ashley |
| g_Camera | 0x________ | Camera* | Câmera atual |
| g_InputState | 0x________ | InputState | Estado do controle |
| g_GameState | 0x________ | int | Estado do jogo |

### Estruturas Mapeadas
| Struct | Endereço Base | Tamanho | Status |
|--------|---------------|---------|--------|
| Player | 0x________ | ~0x____ | 🔴 Não mapeado |
| Ashley | 0x________ | ~0x____ | 🔴 Não mapeado |
| Camera | 0x________ | ~0x____ | 🔴 Não mapeado |
| Input | 0x________ | ~0x____ | 🔴 Não mapeado |

---

## 🎯 Milestones

| # | Milestone | Status | Data |
|---|-----------|--------|------|
| 1 | Ghidra setup | ✅ | 16/01/2026 |
| 2 | Análise completa | ⬜ | |
| 3 | HP encontrado | ⬜ | |
| 4 | Player struct mapeada | ⬜ | |
| 5 | Ashley struct mapeada | ⬜ | |
| 6 | Input system encontrado | ⬜ | |
| 7 | Primeiro hook funcionando | ⬜ | |
| 8 | P2 controla Ashley | ⬜ | |
| 9 | Ashley pode atirar | ⬜ | |
| 10 | Co-op jogável | ⬜ | |

---

## 🐛 Problemas Encontrados

| # | Problema | Status | Solução |
|---|----------|--------|---------|
| | | | |

---

## 💡 Notas e Ideias

### Observações:
- 

### Ideias futuras:
- [ ] Adicionar mais personagens jogáveis (Ada, Krauser)
- [ ] Modo versus
- [ ] Co-op online (networking)
- [ ] Dificuldade rebalanceada para 2 players
