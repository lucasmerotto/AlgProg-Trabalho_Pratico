/*
Lucas Lemos Merotto
Pedro Arthur Pereira Rocha
Trabalho prático final
Algoritmos e Programação - CIC
INF 01202
Turma C

----------INF vs. Zombies----------
*/

// Bibliotecas:
#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

// Constantes:
#define NLINHAS 5 // Número de linhas e colunas do grid (em número de células)
#define NCOLUNAS 9
#define LARG_CEL 72 // Largura e altura (em pixels) cada célula do grid
#define ALT_CEL 96
#define MAX_SOIS 20 // Máximo de sóis, zumbis e projéteis (ervilhas) coexistentes na tela
#define MAX_ZUMBIS 10
#define MAX_PROJETEIS 50
#define NHORDAS 3
#define TAMNOME 10
#define TOP 5
#define FALSO 0 // Serão usados como retornos de funções que indicam estado
#define VERDADEIRO 1

// Enums e structs:
typedef enum {
    MENU,
    JOGO,
    RANKING,
    VITORIA
} TELA; // Possibilidades para o estado da tela que o jogador vê

typedef enum
{
    NENHUMA = 0,
    DISPARA_ERVILHA = 1,
    GIRASSOL = 2
} PLANTA; // Tipos de planta diferentes

typedef struct
{
    float x, y;         // Posição vertical e horizontal, respectivamente (a partir do canto superior esquerdo)
    float velocidade;   // Em pixels por segundo
    int ativo;          // Pode ser 0, que corresponde a FALSO, ou 1, que corresponde a VERDADEIRO
    int linha;          // Linha em que foi disparado (constante)
} PROJETIL; // Projétil/ervilha que é disparado pela planta dispara-ervilha

typedef struct
{
    Vector2 posicao;
    int ativo;
} SOL; // Sol que é gerado aleatoriamente ou pela planta girassol

typedef struct
{
    float x;            // Posição horizontal a partir da esquerda
    float velocidade;
    int ativo;
    int linha;          // Linha sobre a qual ele anda (constante)
    int vida;           // Começa em 100 e dimunui em 20 cada vez que é atingido por um projétil
} ZUMBI;

typedef struct
{
    char nome[TAMNOME];
    int pontos;
} RECORDE;

typedef struct
{
    Texture2D fundo, grama, dispara_ervilha, projetil, girassol, sprite_sol, sprite_zumbi, icone_sol, card_dispara_ervilha, card_girassol;
} TEXTURAS; // Estrutura contendo todas as texturas que serão utilizadas

typedef struct
{
    Rectangle jogar, ranking, sair;
} BOTOES_MENU; // Estrutura contendo os retângulos dos botões do menu

// Variáveis globais:
TEXTURAS texturas;
SOL sois[MAX_SOIS] = {0};                       // Arranjo com todos os sóis do momento
ZUMBI zumbis[MAX_ZUMBIS] = {0};                 // Arranjo com todos os zumbis do momento
TELA tela = MENU;                               // A tela selecionada por padrão é o menu
BOTOES_MENU botoes;
PLANTA planta_selecionada = DISPARA_ERVILHA;    // A planta selecionada por padrão é a dispara-ervilha
PROJETIL projeteis[MAX_PROJETEIS] = {0};        // Arranjo com todos os projéteis/ervilhas do momento
RECORDE ranking[TOP];                           // Arranjo com os nomes dos jogadores e pontuações dos 5 recordes

float tempo_sol = 0, tempo_zumbi = 0;           // Contadores de tempo para gerar sóis aleatórios e zumbis
float tempo_girassol[NLINHAS][NCOLUNAS] = {0};
float tempo_dispara[NLINHAS][NCOLUNAS] = {0};   // Contadores de tempo para cada planta girassol e dispara_ervilha
float tempo_horda = 0;                          // Registra o tempo de cada horda

int grid_plantas[NLINHAS][NCOLUNAS] = {0};
int offsetX = 20, offsetY = 120;                // Dimensões da borda, ou seja, onde começa o grid
int cont_sol = 100;                             // Contador de sóis (começa em 100)
int pontos = 0;                                 // Pontos do jogador (ganha 100 a cada zumbi morto)
int game_over = FALSO;
int mostrando_horda = VERDADEIRO;
int hordas[NHORDAS] = {0};                      // Cada posição é o número de zumbis em uma horda
int horda_atual = 0;                            // A horda muda quando todos os zumbis da anterior tiverem sido gerados. horda_atual == 0 é a primeira horda e assim por diante
int zumbis_horda = 0;

char nome[TAMNOME];                             // Nome do jogador que vence as três hordas


// Definições de funções
TEXTURAS carrega_texturas (void) // Carrega as texturas (todas em um struct) no início do main
{
    TEXTURAS textura_func;

    textura_func.fundo = LoadTexture ("menu_background4.png");
    textura_func.grama = LoadTexture ("grass.png");
    textura_func.dispara_ervilha = LoadTexture ("peashooter.png");
    textura_func.projetil = LoadTexture ("peashooter-proj.png");
    textura_func.girassol = LoadTexture ("sunflower.png");
    textura_func.sprite_sol = LoadTexture ("sun.png");
    textura_func.sprite_zumbi = LoadTexture ("zombie.png");
    textura_func.icone_sol = LoadTexture ("hud_sun.png");
    textura_func.card_dispara_ervilha = LoadTexture ("card_peashooter.png");
    textura_func.card_girassol = LoadTexture ("card_sunflower.png");

    return textura_func;
}

void libera_texturas (TEXTURAS *ptr_textura) // Descarrega/libera as texturas (todas em um struct) no final do main
{
    UnloadTexture(ptr_textura->fundo);
    UnloadTexture(ptr_textura->grama);
    UnloadTexture(ptr_textura->dispara_ervilha);
    UnloadTexture(ptr_textura->projetil);
    UnloadTexture(ptr_textura->sprite_sol);
    UnloadTexture(ptr_textura->sprite_zumbi);
    UnloadTexture(ptr_textura->icone_sol);
    UnloadTexture(ptr_textura->card_dispara_ervilha);
    UnloadTexture(ptr_textura->card_girassol);
}

void desenha_menu (BOTOES_MENU *ptr_botoes, TEXTURAS *ptr_texturas) // Desenha o menu principal, recebendo uma estrutura do tipo botões do menu como ponteiro
{
    DrawTexturePro(ptr_texturas->fundo, (Rectangle){0, 0, 800, 600}, (Rectangle){0, 0, 800, 600}, (Vector2){0, 0}, 0, WHITE);
    DrawText("INF vs ZOMBIES", 250, 50, 40, DARKGREEN);

    DrawRectangleRec(ptr_botoes->jogar, DARKGRAY);
    DrawText("JOGAR", 360, 215, 20, WHITE);
    DrawRectangleRec(ptr_botoes->ranking, DARKGRAY);
    DrawText("LEADERBOARD", 315, 295, 20, WHITE);
    DrawRectangleRec(ptr_botoes->sair, DARKGRAY);
    DrawText("SAIR", 370, 375, 20, WHITE);
}

void atualiza_menu (BOTOES_MENU *ptr_botoes, TELA *ptr_tela) // Se o jogador clica em um dos três botões, muda a tela atual para a que ele selecionou. Se ele selecionou o botão sair, encerra o jogo
{
    Vector2 mouse = GetMousePosition();
    if (CheckCollisionPointRec(mouse, ptr_botoes->jogar) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        *ptr_tela = JOGO;
    }
    else if (CheckCollisionPointRec(mouse, ptr_botoes->ranking) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        *ptr_tela = RANKING;
    }
    else if (CheckCollisionPointRec(mouse, ptr_botoes->sair) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        CloseWindow();
    }
}

void carrega_hordas (void) // Lê o arquivo txt com o número de zumbis em cada horda e salva no arranjo int global
{
    FILE *arq = fopen("config.txt", "r");

    if (arq != NULL) // Se o arquivo abriu corretamente
    {
        fscanf (arq, "%d %d %d", &hordas[0], &hordas[1], &hordas[2]);
        fclose (arq);
    }
    else // Senão leu o arquivo corretamente, usa um caso base com o mesmo efeito que ler o arquivo
    {
        hordas[0] = 5;
        hordas[1] = 5;
        hordas[2] = 5;
    }
}

void inicia_horda (void)
{
    if (horda_atual < 3) // Se ainda não derrotou a última horda
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText(TextFormat("Horda %d", horda_atual+1), 285, 200, 40, GREEN); // Aviso que uma nova horda está chegando (aparece apenas para segunda e terceira horda)
        EndDrawing();
        WaitTime(2.0);
        zumbis_horda = hordas[horda_atual];
        mostrando_horda = VERDADEIRO;
        tempo_horda = 0;
    }
}

void trata_clique_mouse (void) // O que acontece quando o jogador clica com o mouse
{
    int linha, coluna;

    if (IsMouseButtonPressed (MOUSE_LEFT_BUTTON))
    {
        Vector2 mouse = GetMousePosition();
        coluna = (mouse.x - offsetX) / LARG_CEL;
        linha = (mouse.y - offsetY) / ALT_CEL; // Linha e coluna em que vai plantar

        if (grid_plantas[linha][coluna] == 0)
        {
            if (planta_selecionada == DISPARA_ERVILHA && cont_sol >= 50) // Se estiver com dispara-ervilha selecionada e tiver pelo menos 50 sóis
            {
                grid_plantas[linha][coluna] = DISPARA_ERVILHA; // Planta dispara-ervilha
                cont_sol -= 50; // Que custa 50 sóis
            }
            else if (planta_selecionada == GIRASSOL && cont_sol >= 25) // Se estiver com girassol selecionado e tiver pelo menos 25 sóis
            {
                grid_plantas[linha][coluna] = GIRASSOL; // Planta girassol
                cont_sol -= 25; // Que custa 25 sóis
            }
        }
    }
}

void gera_sois (float delta) // Gera sóis aleatoriamente, a cada 4 segundos
{
    int i;
    int x, y; // Coluna e linha, respectivamente, do sol gerado

    tempo_sol += delta;

    if (tempo_sol >= 4.0f)
    {
        tempo_sol = 0;
        for (i = 0; i < MAX_SOIS; i++)
        {
            if (sois[i].ativo == FALSO) // Se o for encontrar um sol que está inativo
            {
                x = offsetX + (rand() % NCOLUNAS) * LARG_CEL + 16;
                y = offsetY + (rand() % NLINHAS) * ALT_CEL + 16; // Gera um sol em uma posição x e y aleatória do grid
                sois[i].posicao = (Vector2){x, y};
                sois[i].ativo = VERDADEIRO; // Ativa o sol
                break;
            }
        }
    }
}

void atualiza_girassois (float delta)
{
    int linha, coluna, i;
    int x, y;

    for (linha = 0; linha < NLINHAS; linha++) // For percorre todas as linhas e colunas
    {
        for (coluna = 0; coluna < NCOLUNAS; coluna++)
        {
            if (grid_plantas[linha][coluna] == GIRASSOL) // Se a planta for um girassol
            {
                tempo_girassol[linha][coluna] += delta;
                if (tempo_girassol[linha][coluna] >= 4.0f) // A cada 4 segundos
                {
                    tempo_girassol[linha][coluna] = 0;
                    for (i = 0; i < MAX_SOIS; i++) // Percorre o arranjo de sóis
                    {
                        if (sois[i].ativo == FALSO) // Na primeira posição que estiver vazia
                        {
                            x = offsetX + coluna * LARG_CEL + 16; // Emite um sol na mesma posição do girassol
                            y = offsetY + linha * ALT_CEL + 16;
                            sois[i].posicao = (Vector2){x, y};
                            sois[i].ativo = VERDADEIRO;
                            break;
                        }
                    }
                }
            }
            else
            {
                tempo_girassol[linha][coluna] = 0; // Senão, resetar caso tenha removido a planta
            }
        }
    }
}


void recolhe_sois(void)
{
    int i;

    for (i = 0; i < MAX_SOIS; i++)
    {
        if (sois[i].ativo)
        {
            sois[i].ativo = FALSO;
            cont_sol += 25;  // Para cada sol, recolhido, soma 25 no contador
        }
    }
}

void atualiza_disparos (float delta)
{
    int linha, coluna, i;

    for (linha = 0; linha < NLINHAS; linha++)
    {
        for (coluna = 0; coluna < NCOLUNAS; coluna++)
        {
            if (grid_plantas[linha][coluna] == DISPARA_ERVILHA) // Se tiver uma dispara-ervilha plantada
            {
                tempo_dispara[linha][coluna] += delta;
                if (tempo_dispara[linha][coluna] >= 4.0f)
                {
                    tempo_dispara[linha][coluna] = 0;
                    for (i = 0; i < MAX_PROJETEIS; i++)
                    {
                        if (projeteis[i].ativo == FALSO)
                        {
                            projeteis[i].ativo = VERDADEIRO; // Projétil se torna ativo
                            projeteis[i].linha = linha; // Cria um projétil nessa linha (não muda)
                            projeteis[i].velocidade = 200;
                            projeteis[i].x = offsetX + coluna * LARG_CEL + LARG_CEL / 2;
                            projeteis[i].y = offsetY + linha * ALT_CEL + ALT_CEL / 2;
                            break;
                        }
                    }
                }
            }
            else
            {
                tempo_dispara[linha][coluna] = 0;
            }
        }
    }
}

void move_projeteis (float delta)
{
    int i;

    for (i = 0; i < MAX_PROJETEIS; i++) // Percorre todo arranjo com os projéteis
    {
        if (projeteis[i].ativo == VERDADEIRO)
        {
            projeteis[i].x += projeteis[i].velocidade * delta; // A coordenada x do projétil aumenta (vai da esquerda para direita) de acordo com a velocidade * tempo delta (deslocamento)
            if (projeteis[i].x > 800)
            {
                projeteis[i].ativo = FALSO; // Desativa o projétil depois que ele sai de vista
            }
        }
    }
}

void desenha_projeteis (void)
{
    int i;

    for (i = 0; i < MAX_PROJETEIS; i++)
    {
        if (projeteis[i].ativo) // Desenha o projétil, se esse estiver ativo
        {
            DrawTexturePro(texturas.projetil, (Rectangle){0, 0, texturas.projetil.width, texturas.projetil.height}, (Rectangle){projeteis[i].x, projeteis[i].y,20, 20}, (Vector2){10, 10}, 0.0f, WHITE);
        }
    }
}

void gera_zumbis (float delta)
{
    int i;

    tempo_zumbi += delta;

    if (tempo_zumbi >= 6.0f) // Gera um zumbi a cada 6 segundos
    {
        tempo_zumbi = 0;
        for (i = 0; i < MAX_ZUMBIS; i++)
        {
            if (zumbis[i].ativo == FALSO)
            {
                zumbis[i].ativo = VERDADEIRO;
                zumbis[i].x = 800;                  // Começa na margem direita da tela
                zumbis[i].linha = rand() % NLINHAS; // Em uma linha aleatória
                zumbis[i].velocidade = 20;
                zumbis[i].vida = 100;
                break;
            }
        }
    }
}

void move_zumbis (float delta)
{
    int i;

    for (i = 0; i < MAX_ZUMBIS; i++)
    {
        if (zumbis[i].ativo == VERDADEIRO)
        {
            zumbis[i].x -= zumbis[i].velocidade * delta; // Zumbis perdem deslocamento no valor da coordenada x conforme o tempo delta
            if (zumbis[i].x < -LARG_CEL) // Zumbis param de se mexer quando sua coordenada for menor que -72
            {
                zumbis[i].ativo = FALSO;
            }
        }
    }
}

void trata_colisoes_projeteis (void)
{
    int i, j;

    for (i = 0; i < MAX_PROJETEIS; i++)
    {
        if (projeteis[i].ativo)
        {
            for (j = 0; j < MAX_ZUMBIS; j++)
            {
                if (zumbis[j].ativo == VERDADEIRO && projeteis[i].linha == zumbis[j].linha) // Se tiver um projétil e zumbi na mesma linha, eles VÃO se chocar
                {
                    Rectangle ret_projetil = {projeteis[i].x, offsetY + projeteis[i].linha * ALT_CEL, 20, 20};
                    Rectangle ret_zumbi = {zumbis[j].x, offsetY + zumbis[j].linha * ALT_CEL, LARG_CEL, ALT_CEL};

                    if (CheckCollisionRecs(ret_projetil, ret_zumbi)) // Quando eles se chocam
                    {
                        projeteis[i].ativo = FALSO; // Desativa o projétil
                        zumbis[j].vida -= 20; // Zumbi perde 20 de vida
                        if (zumbis[j].vida <= 0) // Se a vida do zumbi chegar em 0, ele morre
                        {
                            zumbis[j].ativo = FALSO;
                            hordas[horda_atual]--; // Morreu um zumbi da horda atual
                            pontos += 100; // Cada zumbi que morre, o jogador ganha 100 pontos
                            if (hordas[horda_atual] == 0) // Quando os zumbis da horda atual forem mortos, passa para próxima horda
                            {
                                horda_atual++;
                                if (horda_atual < 3)
                                {
                                    inicia_horda(); // Chama inicia_horda
                                }
                                else // Se a horda chegou em 3, não há mais hordas
                                {
                                    tela = VITORIA; // O jogador venceu
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}

void desenha_hud (TEXTURAS *ptr_texturas) // HUD = Head-up Display
{
    DrawRectangle(0, 0, 800, 120, DARKGREEN);
    DrawText(TextFormat("%d", cont_sol), 90, 35, 30, YELLOW); // Número de sóis disponíveis

    DrawText(TextFormat("%d pts", pontos), 390, 35, 30, WHITE); // Pontos do jogador

    DrawRectangle(180, 15, 64, 64, GREEN);
    DrawTexture(ptr_texturas->dispara_ervilha, 180, 15, WHITE);
    DrawText("50", 200, 85, 20, WHITE);

    DrawRectangle(270, 15, 64, 64, YELLOW);
    DrawTexture(ptr_texturas->girassol, 270, 15, WHITE);
    DrawText("25", 290, 85, 20, WHITE);

    DrawText("E volta ao menu", 600, 90, 20, RED);
}

void desenha_grid (void)
{
    int linha, coluna;
    int x, y;

    for (linha = 0; linha < NLINHAS; linha++)
    {
        for (int coluna = 0; coluna < NCOLUNAS; coluna++)
        {
            x = offsetX + coluna * LARG_CEL;
            y = offsetY + linha * ALT_CEL;
            DrawTexture(texturas.grama, x, y, WHITE);
            if (grid_plantas[linha][coluna] == DISPARA_ERVILHA) // Se a planta é dispara-ervilha, desenha dispara-ervilha
            {
            DrawTexturePro(texturas.dispara_ervilha, (Rectangle){0,0,texturas.dispara_ervilha.width,texturas.dispara_ervilha.height}, (Rectangle){x,y,LARG_CEL,ALT_CEL}, (Vector2){0,0}, 0.0f, WHITE);
            }
            else if (grid_plantas[linha][coluna] == GIRASSOL) // Se a planta é um girassol, desenha girassol
            {
            DrawTexturePro(texturas.girassol, (Rectangle){0,0,texturas.girassol.width,texturas.girassol.height}, (Rectangle){x,y,LARG_CEL,ALT_CEL}, (Vector2){0,0}, 0.0f, WHITE);
            }
            DrawRectangleLines(x, y, LARG_CEL, ALT_CEL, DARKGREEN); // Desenha contorno de cada célula
        }
    }
}

void desenha_zumbis (void)
{
    int i;
    int y_zumbi;

    for (i = 0; i < MAX_ZUMBIS; i++)
    {
        if (zumbis[i].ativo == VERDADEIRO) // Se o zumbi está ativo, desenha-o
        {
            y_zumbi = offsetY + zumbis[i].linha * ALT_CEL;
            DrawTexturePro(texturas.sprite_zumbi, (Rectangle){0,0,texturas.sprite_zumbi.width,texturas.sprite_zumbi.height}, (Rectangle){zumbis[i].x, y_zumbi, LARG_CEL, ALT_CEL}, (Vector2){0,0}, 0.0f, WHITE);
        }
    }
}

void trata_colisoes_zumbis_plantas (void)
{
    int i, coluna;
    int linha_zumbi;
    float x_planta;

    for (i = 0; i < MAX_ZUMBIS; i++)
    {
        if (zumbis[i].ativo == VERDADEIRO)
        {
            linha_zumbi = zumbis[i].linha; // Salva a linha do zumbi ativo

            for (coluna = 0; coluna < NCOLUNAS; coluna++)
            {
                if (grid_plantas[linha_zumbi][coluna] != 0)    // Se tem alguma planta na célula (pode ser girassol ou dispara-ervilha)
                {
                    x_planta = offsetX + coluna * LARG_CEL;

                    if (zumbis[i].x <= x_planta + LARG_CEL / 2) // Se o zumbi chegou "perto" (72 / 2 = 36 pixels) da planta
                    {
                        grid_plantas[linha_zumbi][coluna] = 0; // Zumbi mata planta
                    }
                }
            }
        }
    }
}

void desenha_sois (void)
{
    int i;

    for (i = 0; i < MAX_SOIS; i++) // Percorre o arranjo dos sóis, e desenha os que estiverem ativos
    {
        if (sois[i].ativo == VERDADEIRO)
        {
            DrawTextureEx(texturas.sprite_sol, sois[i].posicao, 0.0f, 0.2f, WHITE);
        }
    }
}

void checa_fim_de_jogo (void)
{
    int i;

    for (i = 0; i < MAX_ZUMBIS; i++)
    {
        if (zumbis[i].ativo && zumbis[i].x <= offsetX + 20) // Se um zumbi ativo tiver chegado na margem esquerda (20 pixels)
        {
            game_over = VERDADEIRO; // Ativa o estado game over
            break;
        }
    }
}

void reinicia_jogo(void)
{
    int linha, coluna, i;

    for (linha = 0; linha < NLINHAS; linha++)
        for (coluna = 0; coluna < NCOLUNAS; coluna++)
            grid_plantas[linha][coluna] = 0; // Limpa todo o grid

    for (i = 0; i < MAX_ZUMBIS; i++)
        zumbis[i].ativo = FALSO; // Reseta zumbis

    for (i = 0; i < MAX_SOIS; i++)
        sois[i].ativo = FALSO; // Reseta sois

    for (i = 0; i < MAX_PROJETEIS; i++)
        projeteis[i].ativo = FALSO; // reseta projeteis

    cont_sol = 100;
    pontos = 0;
    tempo_sol = 0;
    tempo_zumbi = 0;
    game_over = FALSO;
    planta_selecionada = 1;
    pontos = 0;
    horda_atual = 0;
    mostrando_horda = VERDADEIRO;
    tempo_horda = 0; // Reseta demais variáveis
}

void atualiza_jogo(void)
{
    float delta = GetFrameTime();
    trata_clique_mouse ( );
    gera_sois (delta);
    gera_zumbis (delta);
    move_zumbis (delta);
    atualiza_girassois (delta);
    atualiza_disparos (delta);
    move_projeteis (delta);
    trata_colisoes_projeteis ( );
    trata_colisoes_zumbis_plantas ( ); // Chama diversas funções auxiliares

    if (IsKeyPressed(KEY_S)) // Tecla S recolhe todos os sóis da tela
    {
        recolhe_sois();
    }

    if (IsKeyPressed(KEY_A)) // Tecla A muda a seleção de planta
    {
        if (planta_selecionada == DISPARA_ERVILHA)
        {
            planta_selecionada = GIRASSOL;
        }
        else planta_selecionada = DISPARA_ERVILHA;
    }

    if (IsKeyPressed(KEY_E))
    {
        tela = MENU; // Tecla E volta para o menu (não para nem reseta o jogo)
    }

    checa_fim_de_jogo ( );
    if (game_over == VERDADEIRO) // Se o jogador perdeu
    {
        DrawText("GAME OVER", 285, 200, 40, RED);
        DrawText(TextFormat("Voce nao sobreviveu a horda %d", horda_atual+1), 250, 270, 20, BLACK); // Exibe mensagem de derrota
        EndDrawing();
        WaitTime(4.0); // Por quatro segundos

        reinicia_jogo (); // E reinicia o jogo, voltando para o menu
        tela = MENU;
    }
}

void desenha_jogo(void)
{
    desenha_hud (&texturas);
    desenha_grid ( );
    desenha_zumbis ( );
    desenha_projeteis ( );
    desenha_sois ( ); // Chama as funções gráficas
}

void atualiza_ranking (void)
{
    FILE *arq;
    int i;

    arq = fopen("ranking.bin", "rb+");
    if (arq != NULL)
    {
        fread (ranking, sizeof(RECORDE), 5, arq);
        fclose (arq);
    }

    for (i = 0; i < TOP; i++)
    {
        if (pontos > ranking[i].pontos) // Se o jogador tiver pontuação suficiente para entrar no ranking
        {
            for (int j = 4; j > i; j--) // Move os que têm menos pontos para baixo
                ranking[j] = ranking[j-1];
            strcpy (ranking[i].nome, nome); // E insere o novo jogador
            ranking[i].pontos = pontos;
            break;
        }
    }
}

void insere_nome (void){
    RECORDE jogador;
    int key;
    int nome_len = strlen(nome);

    ClearBackground(BLUE);

    DrawText(TextFormat("Voce venceu com %d pontos! Digite seu nome:", pontos), 100, 100, 20, WHITE); // Mostra para o jogador quanto ele pontuou
    DrawRectangle(100, 150, 300, 40, LIGHTGRAY);
    DrawText(jogador.nome, 110, 160, 20, GREEN);
    DrawText("Pressione ENTER para confirmar", 100, 220, 20, WHITE);

    key = GetCharPressed();
    if ((key >= 32) && (key <= 125) && (nome_len < 31)) // Lê um por um caractere
    {
        jogador.nome[nome_len++] = (char)key;
        jogador.nome[nome_len] = '\0';
    }

    if (IsKeyPressed(KEY_BACKSPACE) && nome_len > 0) // Se apaga uma letra
    {
        jogador.nome[nome_len--] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && nome_len > 0)
    {
        tela = MENU;
        strcpy (nome, jogador.nome);
        atualiza_ranking ( );
    }
}

void desenha_ranking (void)
{
    int i;

    ClearBackground(BLUE);
    DrawText("LEADERBOARD", 50, 50, 30, WHITE);

    for (i = 0; i < TOP; i++)
    {
        DrawText(TextFormat("%d - %s: %d pts", i+1, ranking[i].nome, ranking[i].pontos), 50, 50 + ((i + 1) * 30), 20, WHITE);
    }

    if (IsKeyPressed(KEY_E))
    {
        tela = MENU; // Tecla E volta para o menu
    }
}

int main(void)
{
    srand(time(NULL));

    carrega_hordas ( );

    InitWindow(800, 600, "INF vs Zombies"); // Na tela Menu (por padrão)
    SetTargetFPS(60);

    texturas = carrega_texturas();

    botoes.jogar = (Rectangle){300, 200, 200, 50};
    botoes.ranking = (Rectangle){300, 280, 200, 50};
    botoes.sair = (Rectangle){300, 360, 200, 50};

    while (!WindowShouldClose()) // Laço principal do jogo
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (tela)
        {
        case MENU:
            desenha_menu(&botoes, &texturas);
            atualiza_menu(&botoes, &tela);
            break;
        case JOGO:
                atualiza_jogo ( );
                desenha_jogo ( );
                break;
        case RANKING:
            desenha_ranking ( );
        case VITORIA:
            insere_nome ( );
            break;
        }

        EndDrawing();
    }

    libera_texturas(&texturas);

    CloseWindow();

    return 0;
}

