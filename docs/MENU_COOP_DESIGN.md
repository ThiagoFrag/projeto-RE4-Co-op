# 🎮 SISTEMA DE MENU CO-OP - DESIGN DOCUMENT

## 💡 Conceito

Ao selecionar "New Game" ou "Continue", aparece uma **nova tela de seleção**:

```
╔══════════════════════════════════════════════╗
║          SELECIONE O MODO DE JOGO            ║
╠══════════════════════════════════════════════╣
║                                              ║
║   ┌─────────────────────────────────────┐   ║
║   │         🎮 SOLO                      │   ║
║   │    Jogue sozinho (modo original)     │   ║
║   └─────────────────────────────────────┘   ║
║                                              ║
║   ┌─────────────────────────────────────┐   ║
║   │      👥 CO-OP LOCAL                  │   ║
║   │   2 jogadores na mesma máquina       │   ║
║   │   (Conecte 2 controles)              │   ║
║   └─────────────────────────────────────┘   ║
║                                              ║
║   ┌─────────────────────────────────────┐   ║
║   │      🌐 CO-OP ONLINE (HOST)          │   ║
║   │   Crie uma sala para seu amigo       │   ║
║   │   conectar via internet              │   ║
║   └─────────────────────────────────────┘   ║
║                                              ║
║   ┌─────────────────────────────────────┐   ║
║   │      🔗 CO-OP ONLINE (JOIN)          │   ║
║   │   Conecte na sala de um amigo        │   ║
║   └─────────────────────────────────────┘   ║
║                                              ║
╚══════════════════════════════════════════════╝
```

---

## 🎯 MODOS DE JOGO

### 1. 🎮 SOLO
- Modo original do jogo
- Nenhuma modificação
- Ashley controlada por IA normalmente

### 2. 👥 CO-OP LOCAL
- **2 jogadores na mesma máquina**
- Player 1: Controller 1 (ou Teclado/Mouse)
- Player 2: Controller 2
- Opções de câmera:
  - **Mesma Tela:** Câmera dinâmica que mostra os dois
  - **Split-Screen:** Tela dividida ao meio

### 3. 🌐 CO-OP ONLINE (HOST)
- Cria um **servidor local**
- Mostra IP/Código para o amigo conectar
- Player 1 = Leon (Host)
- Player 2 = Ashley (Client remoto)
- Sincronização via **TCP/UDP**

### 4. 🔗 CO-OP ONLINE (JOIN)
- **Conecta no servidor** de um amigo
- Insere IP ou Código da sala
- Controla Ashley remotamente

---

## 🔧 IMPLEMENTAÇÃO TÉCNICA

### Estrutura de Modos

```cpp
enum class GameMode : uint8_t {
    SOLO = 0,           // Jogo normal
    COOP_LOCAL = 1,     // 2 controles, mesma máquina
    COOP_HOST = 2,      // Servidor (P1 = Leon)
    COOP_CLIENT = 3,    // Cliente (P2 = Ashley)
};

enum class CameraMode : uint8_t {
    SINGLE = 0,         // Câmera normal/dinâmica
    SPLIT_HORIZONTAL = 1,  // Dividido horizontalmente
    SPLIT_VERTICAL = 2,    // Dividido verticalmente
};

struct CoopSettings {
    GameMode mode;
    CameraMode camera;
    
    // Network settings (para online)
    char hostIP[64];
    uint16_t port;
    char roomCode[8];     // Código amigável tipo "ABC123"
    
    // Local settings
    bool p1UsesKeyboard;  // P1 usa teclado ou controle?
    int p1ControllerIndex;
    int p2ControllerIndex;
};

CoopSettings g_CoopSettings;
```

---

## 🖥️ SISTEMA DE MENU

### Hook no Menu Principal

```cpp
// Hook quando seleciona "New Game" ou "Continue"
void __fastcall TitleMenu_Select_Hook(void* menu, void* unused, int option) {
    if (option == MENU_NEW_GAME || option == MENU_CONTINUE) {
        // Mostra nosso menu de seleção de modo
        ShowCoopModeSelection();
        return;
    }
    
    // Outras opções funcionam normal
    Original_TitleMenu_Select(menu, unused, option);
}
```

### Renderização do Menu

```cpp
void RenderCoopModeMenu() {
    // Usa o sistema de UI do jogo (IDSystem)
    
    // Fundo semi-transparente
    DrawRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x80000000);
    
    // Título
    DrawText(CENTER, 50, "SELECIONE O MODO DE JOGO", 0xFFFFFFFF);
    
    // Opções
    int selected = g_MenuSelection;
    
    DrawMenuOption(CENTER, 120, "SOLO", selected == 0);
    DrawMenuOption(CENTER, 180, "CO-OP LOCAL", selected == 1);
    DrawMenuOption(CENTER, 240, "CO-OP ONLINE (HOST)", selected == 2);
    DrawMenuOption(CENTER, 300, "CO-OP ONLINE (JOIN)", selected == 3);
    
    // Descrição da opção selecionada
    const char* descriptions[] = {
        "Jogue sozinho - modo original",
        "2 jogadores na mesma máquina",
        "Crie uma sala para seu amigo",
        "Conecte na sala de um amigo"
    };
    
    DrawText(CENTER, 400, descriptions[selected], 0xFFCCCCCC);
}
```

---

## 🌐 SISTEMA DE REDE (CO-OP ONLINE)

### Arquitetura

```
┌─────────────────┐         ┌─────────────────┐
│   HOST (P1)     │ <─────> │  CLIENT (P2)    │
│   Leon          │   TCP   │   Ashley        │
│   Servidor      │   UDP   │   Conecta       │
└─────────────────┘         └─────────────────┘
        │                           │
        │    Sincronização:         │
        │    - Posição              │
        │    - Rotação              │
        │    - Estado/Animação      │
        │    - HP                   │
        │    - Eventos              │
        └───────────────────────────┘
```

### Pacotes de Rede

```cpp
// Pacote do Host para Client (estado do jogo)
struct HostPacket {
    uint32_t tick;           // Frame atual
    
    // Estado do Leon
    Vec leonPos;
    float leonRot;
    int16_t leonHP;
    uint8_t leonState;
    uint8_t leonAnim;
    
    // Estado do mundo
    uint8_t enemies[MAX_ENEMIES]; // Estados dos inimigos
    uint8_t triggers[32];         // Triggers ativados
    
    // Checksum
    uint32_t crc;
};

// Pacote do Client para Host (input do P2)
struct ClientPacket {
    uint32_t tick;
    
    // Input da Ashley
    float moveX, moveY;
    float lookX, lookY;
    uint16_t buttons;        // Bitmask dos botões
    
    uint32_t crc;
};
```

### Servidor (Host)

```cpp
class CoopServer {
public:
    bool Start(uint16_t port = 27015) {
        // Inicializa Winsock
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);
        
        // Cria socket TCP para conexão
        m_tcpSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        // Bind
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        bind(m_tcpSocket, (sockaddr*)&addr, sizeof(addr));
        
        // Listen
        listen(m_tcpSocket, 1);
        
        // Gera código da sala
        GenerateRoomCode();
        
        return true;
    }
    
    void WaitForClient() {
        // Aceita conexão
        m_clientSocket = accept(m_tcpSocket, nullptr, nullptr);
        
        // Cria socket UDP para gameplay (menor latência)
        m_udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        
        m_connected = true;
    }
    
    void SendGameState() {
        HostPacket packet = {};
        
        // Preenche com estado atual
        cPlayer* leon = PlayerPtr();
        if (leon) {
            packet.leonPos = leon->pos_94;
            // ...
        }
        
        // Envia via UDP
        sendto(m_udpSocket, (char*)&packet, sizeof(packet), 0, ...);
    }
    
    void ReceiveClientInput() {
        ClientPacket packet;
        recvfrom(m_udpSocket, (char*)&packet, sizeof(packet), 0, ...);
        
        // Aplica input na Ashley
        g_P2_Input.moveX = packet.moveX;
        g_P2_Input.moveY = packet.moveY;
        // ...
    }
    
private:
    SOCKET m_tcpSocket;
    SOCKET m_udpSocket;
    SOCKET m_clientSocket;
    bool m_connected;
    char m_roomCode[8];
    
    void GenerateRoomCode() {
        // Gera código tipo "ABC123"
        const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
        for (int i = 0; i < 6; i++) {
            m_roomCode[i] = chars[rand() % (sizeof(chars) - 1)];
        }
        m_roomCode[6] = '\0';
    }
};
```

### Cliente (Join)

```cpp
class CoopClient {
public:
    bool Connect(const char* ip, uint16_t port) {
        // Conecta ao host
        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        
        sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &addr.sin_addr);
        
        if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) != 0) {
            return false;
        }
        
        m_connected = true;
        return true;
    }
    
    void SendInput() {
        ClientPacket packet = {};
        packet.moveX = g_LocalInput.moveX;
        packet.moveY = g_LocalInput.moveY;
        // ...
        
        send(m_socket, (char*)&packet, sizeof(packet), 0);
    }
    
    void ReceiveGameState() {
        HostPacket packet;
        recv(m_socket, (char*)&packet, sizeof(packet), 0);
        
        // Atualiza estado local do jogo
        // (Renderiza com base neste estado)
    }
    
private:
    SOCKET m_socket;
    bool m_connected;
};
```

---

## 📡 SISTEMA DE CÓDIGO DE SALA

### Por que Código em vez de IP?

- **Mais fácil:** "ABC123" é mais fácil de lembrar que "192.168.1.100"
- **Mais seguro:** Não expõe IP diretamente
- **Funciona com NAT:** Pode usar servidor relay

### Opção 1: Código = IP Codificado

```cpp
// Converte IP para código
std::string IPToCode(const char* ip) {
    // 192.168.1.100 -> Base36 encode
    uint32_t ipNum = inet_addr(ip);
    char code[8];
    // Encode para base36...
    return code;
}

// Converte código para IP
std::string CodeToIP(const char* code) {
    // Decode base36...
    return ip;
}
```

### Opção 2: Servidor Master (Avançado)

```
1. Host registra no servidor master
2. Servidor retorna código "ABC123"
3. Client entra código
4. Servidor retorna IP do host
5. Client conecta diretamente
```

---

## 🎨 UI/UX DO MENU

### Tela de Seleção de Modo

```
┌────────────────────────────────────────────────────┐
│                                                    │
│            ★ RESIDENT EVIL 4 CO-OP ★               │
│                                                    │
│   ╔════════════════════════════════════════════╗   │
│   ║  > SOLO                                    ║   │
│   ╚════════════════════════════════════════════╝   │
│                                                    │
│   ┌────────────────────────────────────────────┐   │
│   │    CO-OP LOCAL                             │   │
│   └────────────────────────────────────────────┘   │
│                                                    │
│   ┌────────────────────────────────────────────┐   │
│   │    CO-OP ONLINE (HOST)                     │   │
│   └────────────────────────────────────────────┘   │
│                                                    │
│   ┌────────────────────────────────────────────┐   │
│   │    CO-OP ONLINE (JOIN)                     │   │
│   └────────────────────────────────────────────┘   │
│                                                    │
│          [A] Selecionar    [B] Voltar              │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Tela de HOST (Aguardando)

```
┌────────────────────────────────────────────────────┐
│                                                    │
│              🌐 CO-OP ONLINE - HOST                │
│                                                    │
│         ┌──────────────────────────────┐           │
│         │    CÓDIGO DA SALA:           │           │
│         │                              │           │
│         │      ★ ABC123 ★              │           │
│         │                              │           │
│         │    IP: 192.168.1.100:27015   │           │
│         └──────────────────────────────┘           │
│                                                    │
│         Aguardando jogador conectar...             │
│                  ◐ ◓ ◑ ◒                           │
│                                                    │
│         [B] Cancelar                               │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Tela de JOIN (Inserir Código)

```
┌────────────────────────────────────────────────────┐
│                                                    │
│              🔗 CO-OP ONLINE - JOIN                │
│                                                    │
│         ┌──────────────────────────────┐           │
│         │    INSIRA O CÓDIGO:          │           │
│         │                              │           │
│         │      [ A ] [ B ] [ C ]       │           │
│         │      [ 1 ] [ 2 ] [ 3 ]       │           │
│         │           ▲                  │           │
│         └──────────────────────────────┘           │
│                                                    │
│         OU insira IP diretamente:                  │
│         [ 192.168.1.100 ]                          │
│                                                    │
│         [A] Conectar    [B] Voltar                 │
│                                                    │
└────────────────────────────────────────────────────┘
```

### Tela Conectado!

```
┌────────────────────────────────────────────────────┐
│                                                    │
│              ✓ CONECTADO!                          │
│                                                    │
│      ┌─────────────┐    ┌─────────────┐            │
│      │   PLAYER 1  │    │   PLAYER 2  │            │
│      │    LEON     │    │   ASHLEY    │            │
│      │    (Host)   │    │  (Client)   │            │
│      │   🎮 P1     │    │   🎮 P2     │            │
│      └─────────────┘    └─────────────┘            │
│                                                    │
│           Ping: 45ms    Conexão: ████████          │
│                                                    │
│         [A] Iniciar Jogo    [B] Desconectar        │
│                                                    │
└────────────────────────────────────────────────────┘
```

---

## 📋 CHECKLIST DE IMPLEMENTAÇÃO

### Menu
- [ ] Hook no menu principal
- [ ] Renderizar tela de seleção de modo
- [ ] Navegação com controle/teclado
- [ ] Transições suaves

### Co-op Local
- [ ] Detectar Controller 2
- [ ] Modo split-screen
- [ ] Modo câmera dinâmica

### Co-op Online - Host
- [ ] Criar servidor TCP/UDP
- [ ] Gerar código da sala
- [ ] Mostrar IP local
- [ ] Aguardar conexão
- [ ] Sincronizar estado do jogo
- [ ] Enviar pacotes de gamestate

### Co-op Online - Client
- [ ] Tela de inserir código
- [ ] Conectar via IP
- [ ] Receber gamestate
- [ ] Enviar input
- [ ] Renderizar jogo sincronizado

### Rede
- [ ] Serialização de pacotes
- [ ] Compressão (opcional)
- [ ] Predição de movimento
- [ ] Interpolação
- [ ] Reconexão automática

---

## 🔮 FUTURO: CO-OP PELA INTERNET

Para funcionar **através da internet** (não só LAN), precisamos de:

1. **Port Forwarding** - Usuário abre porta no roteador
2. **UPnP** - Abre porta automaticamente
3. **Hole Punching** - Técnica para atravessar NAT
4. **Servidor Relay** - Intermediário para conexões difíceis

Isso é mais avançado, mas possível!

---

*Este é um documento vivo - será atualizado conforme desenvolvemos!*
