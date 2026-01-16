/**
 * RE4 CO-OP MOD - Sistema de Menu de Seleção de Modo
 * 
 * Adiciona menu para escolher:
 * - Solo
 * - Co-op Local
 * - Co-op Online (Host)
 * - Co-op Online (Join)
 */

#pragma once
#include "coop_core.h"
#include <string>

//=============================================================================
// ENUMS E ESTRUTURAS
//=============================================================================

enum class GameMode : uint8_t {
    SOLO = 0,           // Jogo normal
    COOP_LOCAL = 1,     // 2 controles, mesma máquina
    COOP_HOST = 2,      // Servidor (P1 = Leon)
    COOP_CLIENT = 3,    // Cliente (P2 = Ashley)
};

enum class CameraMode : uint8_t {
    DYNAMIC = 0,        // Câmera segue os dois
    SPLIT_VERTICAL = 1, // Dividido verticalmente |
    SPLIT_HORIZONTAL = 2, // Dividido horizontalmente —
};

enum class MenuState : uint8_t {
    HIDDEN = 0,
    MODE_SELECT,        // Seleção de modo
    LOCAL_OPTIONS,      // Opções do co-op local
    HOST_WAITING,       // Aguardando conexão
    JOIN_ENTER_CODE,    // Inserindo código
    CONNECTING,         // Conectando...
    CONNECTED,          // Conectado, pronto para jogar
    ERROR_SCREEN,       // Mostrando erro
};

struct CoopSettings {
    GameMode mode = GameMode::SOLO;
    CameraMode camera = CameraMode::DYNAMIC;
    
    // Network
    char hostIP[64] = {0};
    uint16_t port = 27015;
    char roomCode[8] = {0};
    
    // Local
    bool p1UsesKeyboard = false;
    int p1ControllerIndex = 0;
    int p2ControllerIndex = 1;
    
    // State
    bool isConnected = false;
    int ping = 0;
};

//=============================================================================
// MENU PRINCIPAL
//=============================================================================

class CoopMenu {
public:
    // Singleton
    static CoopMenu& Instance() {
        static CoopMenu instance;
        return instance;
    }
    
    // Estado
    MenuState state = MenuState::HIDDEN;
    int selectedOption = 0;
    int maxOptions = 4;
    
    // Para entrada de código
    char inputCode[8] = {0};
    int inputPos = 0;
    
    // Mensagem de erro
    char errorMessage[128] = {0};
    
    // Settings
    CoopSettings settings;
    
    // Métodos
    void Show();
    void Hide();
    void Update();
    void Render();
    
    void OnConfirm();
    void OnCancel();
    void OnUp();
    void OnDown();
    void OnCharInput(char c);
    
private:
    CoopMenu() = default;
    
    void RenderModeSelect();
    void RenderLocalOptions();
    void RenderHostWaiting();
    void RenderJoinCode();
    void RenderConnecting();
    void RenderConnected();
    void RenderError();
    
    void StartHost();
    void StartJoin();
    void StartLocalCoop();
    void StartSolo();
};

//=============================================================================
// IMPLEMENTAÇÃO DO MENU
//=============================================================================

inline void CoopMenu::Show() {
    state = MenuState::MODE_SELECT;
    selectedOption = 0;
    maxOptions = 4;
}

inline void CoopMenu::Hide() {
    state = MenuState::HIDDEN;
}

inline void CoopMenu::Update() {
    if (state == MenuState::HIDDEN) return;
    
    // Lê input do menu (Controller 1 ou Teclado)
    // TODO: Integrar com sistema de input do jogo
}

inline void CoopMenu::OnUp() {
    if (selectedOption > 0) {
        selectedOption--;
        // PlaySound(SND_MENU_MOVE);
    }
}

inline void CoopMenu::OnDown() {
    if (selectedOption < maxOptions - 1) {
        selectedOption++;
        // PlaySound(SND_MENU_MOVE);
    }
}

inline void CoopMenu::OnConfirm() {
    // PlaySound(SND_MENU_SELECT);
    
    switch (state) {
        case MenuState::MODE_SELECT:
            switch (selectedOption) {
                case 0: // Solo
                    StartSolo();
                    break;
                case 1: // Co-op Local
                    state = MenuState::LOCAL_OPTIONS;
                    selectedOption = 0;
                    maxOptions = 3; // Câmera dinâmica, Split V, Split H
                    break;
                case 2: // Host
                    StartHost();
                    break;
                case 3: // Join
                    state = MenuState::JOIN_ENTER_CODE;
                    memset(inputCode, 0, sizeof(inputCode));
                    inputPos = 0;
                    break;
            }
            break;
            
        case MenuState::LOCAL_OPTIONS:
            settings.camera = (CameraMode)selectedOption;
            StartLocalCoop();
            break;
            
        case MenuState::JOIN_ENTER_CODE:
            if (inputPos >= 6) { // Código completo
                StartJoin();
            }
            break;
            
        case MenuState::CONNECTED:
            // Inicia o jogo!
            Hide();
            // ContinueToGame();
            break;
    }
}

inline void CoopMenu::OnCancel() {
    // PlaySound(SND_MENU_BACK);
    
    switch (state) {
        case MenuState::MODE_SELECT:
            Hide();
            // Volta ao menu principal do jogo
            break;
            
        case MenuState::LOCAL_OPTIONS:
        case MenuState::JOIN_ENTER_CODE:
        case MenuState::ERROR_SCREEN:
            state = MenuState::MODE_SELECT;
            selectedOption = 0;
            maxOptions = 4;
            break;
            
        case MenuState::HOST_WAITING:
        case MenuState::CONNECTING:
            // Cancela conexão
            // g_Network.Disconnect();
            state = MenuState::MODE_SELECT;
            break;
            
        case MenuState::CONNECTED:
            // Desconecta
            // g_Network.Disconnect();
            state = MenuState::MODE_SELECT;
            break;
    }
}

inline void CoopMenu::OnCharInput(char c) {
    if (state == MenuState::JOIN_ENTER_CODE) {
        if (inputPos < 6) {
            // Aceita A-Z e 0-9
            if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
                inputCode[inputPos++] = c;
            }
            else if ((c >= 'a' && c <= 'z')) {
                inputCode[inputPos++] = c - 32; // Uppercase
            }
        }
    }
}

inline void CoopMenu::StartSolo() {
    settings.mode = GameMode::SOLO;
    Hide();
    // Continua jogo normalmente
}

inline void CoopMenu::StartLocalCoop() {
    settings.mode = GameMode::COOP_LOCAL;
    settings.isConnected = true;
    
    // Ativa sistema de co-op local
    g_CoopConfig.enabled = true;
    g_CoopConfig.splitScreen = (settings.camera != CameraMode::DYNAMIC);
    
    Hide();
    // Continua jogo com co-op
}

inline void CoopMenu::StartHost() {
    settings.mode = GameMode::COOP_HOST;
    state = MenuState::HOST_WAITING;
    
    // Inicia servidor
    // if (!g_Server.Start(settings.port)) {
    //     strcpy(errorMessage, "Falha ao criar servidor!");
    //     state = MenuState::ERROR_SCREEN;
    //     return;
    // }
    
    // Gera código da sala
    GenerateRoomCode(settings.roomCode, 6);
    
    // Obtém IP local
    // GetLocalIP(settings.hostIP, sizeof(settings.hostIP));
    strcpy(settings.hostIP, "192.168.1.100"); // Placeholder
}

inline void CoopMenu::StartJoin() {
    settings.mode = GameMode::COOP_CLIENT;
    state = MenuState::CONNECTING;
    
    strcpy(settings.roomCode, inputCode);
    
    // Converte código para IP (ou usa servidor master)
    // CodeToIP(inputCode, settings.hostIP);
    
    // Conecta
    // if (!g_Client.Connect(settings.hostIP, settings.port)) {
    //     strcpy(errorMessage, "Falha ao conectar!");
    //     state = MenuState::ERROR_SCREEN;
    //     return;
    // }
    
    // Sucesso!
    state = MenuState::CONNECTED;
    settings.isConnected = true;
}

//=============================================================================
// RENDERIZAÇÃO DO MENU
//=============================================================================

inline void CoopMenu::Render() {
    if (state == MenuState::HIDDEN) return;
    
    // Fundo escuro semi-transparente
    // DrawRect(0, 0, SCREEN_W, SCREEN_H, 0x80000000);
    
    switch (state) {
        case MenuState::MODE_SELECT:
            RenderModeSelect();
            break;
        case MenuState::LOCAL_OPTIONS:
            RenderLocalOptions();
            break;
        case MenuState::HOST_WAITING:
            RenderHostWaiting();
            break;
        case MenuState::JOIN_ENTER_CODE:
            RenderJoinCode();
            break;
        case MenuState::CONNECTING:
            RenderConnecting();
            break;
        case MenuState::CONNECTED:
            RenderConnected();
            break;
        case MenuState::ERROR_SCREEN:
            RenderError();
            break;
    }
}

inline void CoopMenu::RenderModeSelect() {
    const char* title = "SELECIONE O MODO DE JOGO";
    const char* options[] = {
        "SOLO",
        "CO-OP LOCAL",
        "CO-OP ONLINE (HOST)",
        "CO-OP ONLINE (JOIN)"
    };
    const char* descriptions[] = {
        "Jogue sozinho - modo original",
        "2 jogadores na mesma maquina",
        "Crie uma sala para seu amigo",
        "Conecte na sala de um amigo"
    };
    
    // TODO: Usar sistema de renderização do jogo
    // DrawText(CENTER_X, 50, title, COLOR_WHITE);
    
    for (int i = 0; i < 4; i++) {
        bool selected = (i == selectedOption);
        // DrawMenuBox(CENTER_X, 120 + i * 60, options[i], selected);
    }
    
    // DrawText(CENTER_X, 400, descriptions[selectedOption], COLOR_GRAY);
    // DrawText(CENTER_X, 450, "[A] Selecionar    [B] Voltar", COLOR_GRAY);
}

inline void CoopMenu::RenderLocalOptions() {
    const char* title = "CO-OP LOCAL - CAMERA";
    const char* options[] = {
        "Mesma Tela (Dinamica)",
        "Split-Screen Vertical",
        "Split-Screen Horizontal"
    };
    
    // Similar ao RenderModeSelect
}

inline void CoopMenu::RenderHostWaiting() {
    // DrawText(CENTER_X, 50, "CO-OP ONLINE - HOST", COLOR_WHITE);
    // DrawText(CENTER_X, 150, "CODIGO DA SALA:", COLOR_GRAY);
    // DrawText(CENTER_X, 200, settings.roomCode, COLOR_YELLOW, FONT_LARGE);
    // DrawText(CENTER_X, 280, "IP:", COLOR_GRAY);
    // DrawText(CENTER_X, 310, settings.hostIP, COLOR_WHITE);
    // DrawText(CENTER_X, 400, "Aguardando jogador...", COLOR_GRAY);
    // DrawSpinner(CENTER_X, 430);
    // DrawText(CENTER_X, 500, "[B] Cancelar", COLOR_GRAY);
}

inline void CoopMenu::RenderJoinCode() {
    // DrawText(CENTER_X, 50, "CO-OP ONLINE - JOIN", COLOR_WHITE);
    // DrawText(CENTER_X, 150, "INSIRA O CODIGO:", COLOR_GRAY);
    
    // Mostra código sendo digitado
    char display[16];
    for (int i = 0; i < 6; i++) {
        if (i < inputPos) {
            display[i*2] = inputCode[i];
        } else {
            display[i*2] = '_';
        }
        display[i*2+1] = ' ';
    }
    display[12] = '\0';
    
    // DrawText(CENTER_X, 200, display, COLOR_YELLOW, FONT_LARGE);
    // DrawText(CENTER_X, 400, "[A] Conectar    [B] Voltar", COLOR_GRAY);
}

inline void CoopMenu::RenderConnecting() {
    // DrawText(CENTER_X, 200, "Conectando...", COLOR_WHITE);
    // DrawSpinner(CENTER_X, 250);
}

inline void CoopMenu::RenderConnected() {
    // DrawText(CENTER_X, 50, "CONECTADO!", COLOR_GREEN);
    
    // Player boxes
    // DrawBox(LEFT, 150, "PLAYER 1\nLEON\n(Host)");
    // DrawBox(RIGHT, 150, "PLAYER 2\nASHLEY\n(Client)");
    
    // Ping
    char pingStr[32];
    sprintf(pingStr, "Ping: %dms", settings.ping);
    // DrawText(CENTER_X, 350, pingStr, COLOR_GRAY);
    
    // DrawText(CENTER_X, 450, "[A] Iniciar    [B] Desconectar", COLOR_GRAY);
}

inline void CoopMenu::RenderError() {
    // DrawText(CENTER_X, 200, "ERRO", COLOR_RED);
    // DrawText(CENTER_X, 280, errorMessage, COLOR_WHITE);
    // DrawText(CENTER_X, 400, "[A/B] Voltar", COLOR_GRAY);
}

//=============================================================================
// UTILITÁRIOS
//=============================================================================

inline void GenerateRoomCode(char* code, int length) {
    const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
    const int numChars = sizeof(chars) - 1;
    
    srand((unsigned)time(nullptr));
    
    for (int i = 0; i < length; i++) {
        code[i] = chars[rand() % numChars];
    }
    code[length] = '\0';
}

//=============================================================================
// HOOK NO MENU PRINCIPAL
//=============================================================================

/**
 * Para integrar com o menu do jogo:
 * 
 * 1. Encontre a função chamada quando seleciona "New Game" ou "Continue"
 *    - Provavelmente em TitleMenu ou similar
 *    - Procure por strings "NEW GAME", "CONTINUE" no IDA
 * 
 * 2. Faça hook nessa função:
 * 
 *    void __fastcall TitleMenu_Select_Hook(void* menu, void* unused, int option) {
 *        if (option == MENU_NEW_GAME || option == MENU_CONTINUE) {
 *            CoopMenu::Instance().Show();
 *            return; // Não executa função original ainda
 *        }
 *        Original_TitleMenu_Select(menu, unused, option);
 *    }
 * 
 * 3. No render loop do jogo, adicione:
 * 
 *    CoopMenu::Instance().Update();
 *    CoopMenu::Instance().Render();
 */
