/**
 * RE4 CO-OP MOD - Sistema de Rede
 * 
 * Implementa:
 * - Servidor (Host)
 * - Cliente (Join)
 * - Protocolo de sincronização
 */

#pragma once
#include "coop_core.h"
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <queue>

#pragma comment(lib, "ws2_32.lib")

//=============================================================================
// PACOTES DE REDE
//=============================================================================

#pragma pack(push, 1)

// Tipos de pacote
enum class PacketType : uint8_t {
    // Conexão
    CONNECT_REQUEST = 0x01,
    CONNECT_ACCEPT = 0x02,
    CONNECT_REJECT = 0x03,
    DISCONNECT = 0x04,
    PING = 0x05,
    PONG = 0x06,
    
    // Gameplay
    GAME_STATE = 0x10,      // Host -> Client (estado do jogo)
    PLAYER_INPUT = 0x11,    // Client -> Host (input do P2)
    EVENT = 0x12,           // Eventos especiais
};

// Header comum
struct PacketHeader {
    PacketType type;
    uint32_t sequence;
    uint32_t timestamp;
};

// Pacote de estado do jogo (Host -> Client)
struct GameStatePacket {
    PacketHeader header;
    
    // Estado do Leon (P1)
    Vec leonPos;
    float leonRotation;
    int16_t leonHP;
    uint8_t leonState;
    uint8_t leonAnimation;
    uint8_t leonWeapon;
    
    // Estado da Ashley (P2)
    Vec ashleyPos;
    float ashleyRotation;
    int16_t ashleyHP;
    uint8_t ashleyState;
    uint8_t ashleyAnimation;
    
    // Estado do mundo
    uint8_t roomId;
    uint8_t enemyCount;
    // ... mais dados conforme necessário
    
    uint32_t checksum;
};

// Pacote de input do Player 2 (Client -> Host)
struct PlayerInputPacket {
    PacketHeader header;
    
    // Input analógico
    float moveX;
    float moveY;
    float lookX;
    float lookY;
    
    // Botões (bitmask)
    uint16_t buttons;
    
    // Triggers
    float leftTrigger;
    float rightTrigger;
    
    uint32_t checksum;
};

// Pacote de evento
struct EventPacket {
    PacketHeader header;
    
    uint8_t eventType;
    uint32_t eventData[4];
    
    uint32_t checksum;
};

#pragma pack(pop)

// Bits dos botões
enum ButtonMask : uint16_t {
    BTN_ACTION = 0x0001,    // A
    BTN_RUN = 0x0002,       // B
    BTN_RELOAD = 0x0004,    // X
    BTN_KNIFE = 0x0008,     // Y
    BTN_AIM = 0x0010,       // LT
    BTN_SHOOT = 0x0020,     // RT
    BTN_INVENTORY = 0x0040, // Start
    BTN_MAP = 0x0080,       // Back
    BTN_DPAD_UP = 0x0100,
    BTN_DPAD_DOWN = 0x0200,
    BTN_DPAD_LEFT = 0x0400,
    BTN_DPAD_RIGHT = 0x0800,
};

//=============================================================================
// SERVIDOR (HOST)
//=============================================================================

class CoopServer {
public:
    static CoopServer& Instance() {
        static CoopServer instance;
        return instance;
    }
    
    bool Start(uint16_t port = 27015);
    void Stop();
    void Update();
    
    bool IsRunning() const { return m_running; }
    bool IsClientConnected() const { return m_clientConnected; }
    
    // Getters
    const char* GetRoomCode() const { return m_roomCode; }
    const char* GetLocalIP() const { return m_localIP; }
    uint16_t GetPort() const { return m_port; }
    int GetPing() const { return m_ping; }
    
    // Envia estado do jogo para o cliente
    void SendGameState();
    
    // Obtém último input recebido do cliente
    const PlayerInputPacket& GetClientInput() const { return m_lastClientInput; }
    
private:
    CoopServer() = default;
    ~CoopServer() { Stop(); }
    
    void AcceptThread();
    void ReceiveThread();
    void SendThread();
    
    void GenerateRoomCode();
    void GetLocalIPAddress();
    uint32_t CalculateChecksum(const void* data, size_t size);
    
    // Sockets
    SOCKET m_listenSocket = INVALID_SOCKET;
    SOCKET m_clientSocket = INVALID_SOCKET;
    sockaddr_in m_clientAddr = {};
    
    // Estado
    std::atomic<bool> m_running{false};
    std::atomic<bool> m_clientConnected{false};
    
    // Threads
    std::thread m_acceptThread;
    std::thread m_receiveThread;
    std::thread m_sendThread;
    
    // Dados
    char m_roomCode[8] = {0};
    char m_localIP[64] = {0};
    uint16_t m_port = 27015;
    int m_ping = 0;
    
    // Input do cliente
    PlayerInputPacket m_lastClientInput = {};
    std::mutex m_inputMutex;
    
    // Fila de envio
    std::queue<GameStatePacket> m_sendQueue;
    std::mutex m_sendMutex;
    
    // Sequência
    uint32_t m_sendSequence = 0;
};

//=============================================================================
// IMPLEMENTAÇÃO DO SERVIDOR
//=============================================================================

inline bool CoopServer::Start(uint16_t port) {
    if (m_running) return true;
    
    // Inicializa Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
    
    m_port = port;
    
    // Cria socket TCP
    m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }
    
    // Permite reusar endereço
    int opt = 1;
    setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    // Bind
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(m_listenSocket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }
    
    // Listen
    if (listen(m_listenSocket, 1) == SOCKET_ERROR) {
        closesocket(m_listenSocket);
        WSACleanup();
        return false;
    }
    
    // Gera código e obtém IP
    GenerateRoomCode();
    GetLocalIPAddress();
    
    m_running = true;
    
    // Inicia threads
    m_acceptThread = std::thread(&CoopServer::AcceptThread, this);
    
    return true;
}

inline void CoopServer::Stop() {
    m_running = false;
    m_clientConnected = false;
    
    if (m_clientSocket != INVALID_SOCKET) {
        closesocket(m_clientSocket);
        m_clientSocket = INVALID_SOCKET;
    }
    
    if (m_listenSocket != INVALID_SOCKET) {
        closesocket(m_listenSocket);
        m_listenSocket = INVALID_SOCKET;
    }
    
    if (m_acceptThread.joinable()) m_acceptThread.join();
    if (m_receiveThread.joinable()) m_receiveThread.join();
    if (m_sendThread.joinable()) m_sendThread.join();
    
    WSACleanup();
}

inline void CoopServer::Update() {
    if (!m_running || !m_clientConnected) return;
    
    // Envia estado do jogo periodicamente
    // (chamado do game loop)
    SendGameState();
}

inline void CoopServer::AcceptThread() {
    while (m_running) {
        // Configura timeout para accept
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(m_listenSocket, &readSet);
        
        timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(0, &readSet, nullptr, nullptr, &timeout);
        
        if (result > 0 && FD_ISSET(m_listenSocket, &readSet)) {
            int addrLen = sizeof(m_clientAddr);
            m_clientSocket = accept(m_listenSocket, (sockaddr*)&m_clientAddr, &addrLen);
            
            if (m_clientSocket != INVALID_SOCKET) {
                m_clientConnected = true;
                
                // Inicia threads de comunicação
                m_receiveThread = std::thread(&CoopServer::ReceiveThread, this);
                m_sendThread = std::thread(&CoopServer::SendThread, this);
                
                break; // Só aceita 1 cliente
            }
        }
    }
}

inline void CoopServer::ReceiveThread() {
    while (m_running && m_clientConnected) {
        char buffer[1024];
        int received = recv(m_clientSocket, buffer, sizeof(buffer), 0);
        
        if (received > 0) {
            PacketHeader* header = (PacketHeader*)buffer;
            
            switch (header->type) {
                case PacketType::PLAYER_INPUT:
                    if (received >= sizeof(PlayerInputPacket)) {
                        std::lock_guard<std::mutex> lock(m_inputMutex);
                        memcpy(&m_lastClientInput, buffer, sizeof(PlayerInputPacket));
                    }
                    break;
                    
                case PacketType::PING: {
                    // Responde com PONG
                    PacketHeader pong;
                    pong.type = PacketType::PONG;
                    pong.sequence = header->sequence;
                    pong.timestamp = GetTickCount();
                    send(m_clientSocket, (char*)&pong, sizeof(pong), 0);
                    break;
                }
                
                case PacketType::DISCONNECT:
                    m_clientConnected = false;
                    break;
            }
        }
        else if (received == 0 || received == SOCKET_ERROR) {
            // Conexão perdida
            m_clientConnected = false;
            break;
        }
    }
}

inline void CoopServer::SendThread() {
    while (m_running && m_clientConnected) {
        GameStatePacket packet;
        bool hasPacket = false;
        
        {
            std::lock_guard<std::mutex> lock(m_sendMutex);
            if (!m_sendQueue.empty()) {
                packet = m_sendQueue.front();
                m_sendQueue.pop();
                hasPacket = true;
            }
        }
        
        if (hasPacket) {
            send(m_clientSocket, (char*)&packet, sizeof(packet), 0);
        }
        else {
            Sleep(1); // Evita busy loop
        }
    }
}

inline void CoopServer::SendGameState() {
    GameStatePacket packet = {};
    packet.header.type = PacketType::GAME_STATE;
    packet.header.sequence = m_sendSequence++;
    packet.header.timestamp = GetTickCount();
    
    // Preenche com estado atual do jogo
    cPlayer* leon = PlayerPtr();
    cPlayer* ashley = AshleyPtr();
    
    if (leon) {
        packet.leonPos = GET_POS(leon);
        // packet.leonRotation = ...
        packet.leonHP = GET_HP(leon);
        // ... mais dados
    }
    
    if (ashley) {
        packet.ashleyPos = GET_POS(ashley);
        packet.ashleyHP = GET_HP(ashley);
        // ...
    }
    
    packet.checksum = CalculateChecksum(&packet, sizeof(packet) - 4);
    
    // Adiciona à fila de envio
    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendQueue.push(packet);
}

inline void CoopServer::GenerateRoomCode() {
    const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    srand((unsigned)time(nullptr));
    
    for (int i = 0; i < 6; i++) {
        m_roomCode[i] = chars[rand() % (sizeof(chars) - 1)];
    }
    m_roomCode[6] = '\0';
}

inline void CoopServer::GetLocalIPAddress() {
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        addrinfo hints = {};
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        
        addrinfo* result;
        if (getaddrinfo(hostname, nullptr, &hints, &result) == 0) {
            sockaddr_in* addr = (sockaddr_in*)result->ai_addr;
            inet_ntop(AF_INET, &addr->sin_addr, m_localIP, sizeof(m_localIP));
            freeaddrinfo(result);
        }
    }
    
    if (m_localIP[0] == '\0') {
        strcpy(m_localIP, "127.0.0.1");
    }
}

inline uint32_t CoopServer::CalculateChecksum(const void* data, size_t size) {
    uint32_t checksum = 0;
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        checksum += bytes[i];
        checksum = (checksum << 1) | (checksum >> 31);
    }
    return checksum;
}

//=============================================================================
// CLIENTE (JOIN)
//=============================================================================

class CoopClient {
public:
    static CoopClient& Instance() {
        static CoopClient instance;
        return instance;
    }
    
    bool Connect(const char* ip, uint16_t port = 27015);
    void Disconnect();
    void Update();
    
    bool IsConnected() const { return m_connected; }
    int GetPing() const { return m_ping; }
    
    // Envia input do jogador local
    void SendInput(const CoopInput& input);
    
    // Obtém último estado do jogo recebido
    const GameStatePacket& GetGameState() const { return m_lastGameState; }
    
private:
    CoopClient() = default;
    ~CoopClient() { Disconnect(); }
    
    void ReceiveThread();
    void SendThread();
    
    SOCKET m_socket = INVALID_SOCKET;
    std::atomic<bool> m_connected{false};
    
    std::thread m_receiveThread;
    std::thread m_sendThread;
    
    int m_ping = 0;
    uint32_t m_sendSequence = 0;
    
    GameStatePacket m_lastGameState = {};
    std::mutex m_stateMutex;
    
    std::queue<PlayerInputPacket> m_sendQueue;
    std::mutex m_sendMutex;
};

//=============================================================================
// IMPLEMENTAÇÃO DO CLIENTE
//=============================================================================

inline bool CoopClient::Connect(const char* ip, uint16_t port) {
    if (m_connected) return true;
    
    // Inicializa Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
    
    // Cria socket
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_socket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }
    
    // Conecta
    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);
    
    if (connect(m_socket, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
        WSACleanup();
        return false;
    }
    
    m_connected = true;
    
    // Inicia threads
    m_receiveThread = std::thread(&CoopClient::ReceiveThread, this);
    m_sendThread = std::thread(&CoopClient::SendThread, this);
    
    return true;
}

inline void CoopClient::Disconnect() {
    if (!m_connected) return;
    
    // Envia pacote de desconexão
    PacketHeader disconnect;
    disconnect.type = PacketType::DISCONNECT;
    disconnect.sequence = 0;
    disconnect.timestamp = GetTickCount();
    send(m_socket, (char*)&disconnect, sizeof(disconnect), 0);
    
    m_connected = false;
    
    if (m_socket != INVALID_SOCKET) {
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
    
    if (m_receiveThread.joinable()) m_receiveThread.join();
    if (m_sendThread.joinable()) m_sendThread.join();
    
    WSACleanup();
}

inline void CoopClient::Update() {
    if (!m_connected) return;
    
    // Lê input local e envia
    SendInput(g_P2_Input);
}

inline void CoopClient::SendInput(const CoopInput& input) {
    PlayerInputPacket packet = {};
    packet.header.type = PacketType::PLAYER_INPUT;
    packet.header.sequence = m_sendSequence++;
    packet.header.timestamp = GetTickCount();
    
    packet.moveX = input.moveX;
    packet.moveY = input.moveY;
    packet.lookX = input.lookX;
    packet.lookY = input.lookY;
    packet.leftTrigger = input.leftTrigger;
    packet.rightTrigger = input.rightTrigger;
    
    // Converte botões para bitmask
    packet.buttons = 0;
    if (input.action) packet.buttons |= BTN_ACTION;
    if (input.run) packet.buttons |= BTN_RUN;
    if (input.reload) packet.buttons |= BTN_RELOAD;
    if (input.knife) packet.buttons |= BTN_KNIFE;
    if (input.aim) packet.buttons |= BTN_AIM;
    if (input.shoot) packet.buttons |= BTN_SHOOT;
    if (input.inventory) packet.buttons |= BTN_INVENTORY;
    if (input.map) packet.buttons |= BTN_MAP;
    
    std::lock_guard<std::mutex> lock(m_sendMutex);
    m_sendQueue.push(packet);
}

inline void CoopClient::ReceiveThread() {
    while (m_connected) {
        char buffer[1024];
        int received = recv(m_socket, buffer, sizeof(buffer), 0);
        
        if (received > 0) {
            PacketHeader* header = (PacketHeader*)buffer;
            
            switch (header->type) {
                case PacketType::GAME_STATE:
                    if (received >= sizeof(GameStatePacket)) {
                        std::lock_guard<std::mutex> lock(m_stateMutex);
                        memcpy(&m_lastGameState, buffer, sizeof(GameStatePacket));
                    }
                    break;
                    
                case PacketType::PONG:
                    // Calcula ping
                    m_ping = GetTickCount() - header->timestamp;
                    break;
                    
                case PacketType::DISCONNECT:
                    m_connected = false;
                    break;
            }
        }
        else if (received == 0 || received == SOCKET_ERROR) {
            m_connected = false;
            break;
        }
    }
}

inline void CoopClient::SendThread() {
    uint32_t lastPing = 0;
    
    while (m_connected) {
        // Envia ping periodicamente
        if (GetTickCount() - lastPing > 1000) {
            PacketHeader ping;
            ping.type = PacketType::PING;
            ping.sequence = 0;
            ping.timestamp = GetTickCount();
            send(m_socket, (char*)&ping, sizeof(ping), 0);
            lastPing = GetTickCount();
        }
        
        // Envia pacotes da fila
        PlayerInputPacket packet;
        bool hasPacket = false;
        
        {
            std::lock_guard<std::mutex> lock(m_sendMutex);
            if (!m_sendQueue.empty()) {
                packet = m_sendQueue.front();
                m_sendQueue.pop();
                hasPacket = true;
            }
        }
        
        if (hasPacket) {
            send(m_socket, (char*)&packet, sizeof(packet), 0);
        }
        else {
            Sleep(1);
        }
    }
}
