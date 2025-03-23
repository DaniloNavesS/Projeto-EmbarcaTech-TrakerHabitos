// Bibliotecas padroes
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <string.h>
// Configuracao de rede Wi-fi
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"
#include "lwip/opt.h"
#include "lwip/tcp.h"
// Biblioteca de matriz
#include "ws2818b.pio.h"
// Biblioteca do Display Oled
#include "ssd1306.h"
// Definicao de Numero de Leds e pino da matriz
#define LED_COUNT 25
#define LED_PIN 7
// Cores para matriz
#define AGUA 0
#define EXERCICIO 1
#define ALIMENTACAO 2
#define POMODORO 4
// Definicao do OLED
ssd1306_t oled;
// Leds para indicacao de tarefa
#define LED_VERDE 11
#define LED_AZUL 12
#define LED_VERMELHO 13
// Botoes
#define BUTTON_A 5
#define BUTTON_B 6
// Joystick
#define JOYSTICK_X 26
#define JOYSTICK_Y 27
// Configuracao display oled
#define I2C_SDA 14
#define I2C_SCL 15
// Buzzer
#define BUZZER_PIN 21
// Configuracao Buzzer
#define BUZZER_FREQUENCY 100
// Variaveis
volatile bool alarme_ativo_agua = false;
volatile bool alarme_ativo_exercicio = false;
volatile bool alarme_ativo_alimentacao = false;

volatile bool sistema_pausado = false;
volatile bool pomodoro_status = false;

volatile bool pomodoro_concluido = false;

struct repeating_timer timer_agua;
struct repeating_timer timer_exercicio;
struct repeating_timer timer_alimentacao;
struct repeating_timer timer_pomodoro;
// Inicializa contadores de habitos
uint contador_agua = 0;
uint contador_exercicio = 0;
uint contador_alimentacao = 0;
volatile uint contador_pomodoro = 0;
uint contador_sessao_pomodoro = 0;
// Buffer para conversao
char buffer[16];
// Configuracao de intervalos
#define POMODORO_INTERVALO 3000
#define AGUA_INTERVALO 15000
#define ALIMENTACAO_INTERVALO 60000
#define EXERCICIO_INTERVALO 25000

// Funcoes para requisicoes assincronas
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("Chamou recv!\n");
    if (!p)
    {
        // Conexão fechada pelo servidor
        tcp_close(tpcb);
        printf("Conexão fechada pelo servidor.\n");
        return ERR_OK;
    }

    // Processa os dados recebidos (exemplo: imprime na depuração)
    printf("Recebido: %.*s\n", p->len, (char *)p->payload);

    // Libera o buffer recebido
    pbuf_free(p);
    return ERR_OK;
}

// Callback chamada quando a conexão é estabelecida
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    printf("Chamou Connect!\n");
    if (err != ERR_OK)
    {
        printf("Erro na conexão: %d\n", err);
        return err;
    }

    // Cria a requisição HTTP POST
    char request[256];
    char body[256];

    int result = snprintf(body, sizeof(body),
             "{\"contador_agua\": %u, \"contador_exercicio\": %u, \"contador_alimentacao\": %u, \"contador_sessao_pomodoro\": %u}",
             contador_agua, contador_exercicio, contador_alimentacao, contador_sessao_pomodoro);

    printf("Result: %d\n", result);
    printf("Body: %s\n", body);

    snprintf(request, sizeof(request),
             "POST /habitos/sicronizar HTTP/1.1\r\n"
             "Host: 192.168.15.14:3000\r\n" // Altere para o IP do seu servidor
             "Content-Type: application/json\r\n"
             "Content-Length: %d\r\n"
             "\r\n"
             "%s",
             (int)strlen(body), body);

    // Envia a requisição
    err = tcp_write(tpcb, request, strlen(request), TCP_WRITE_FLAG_COPY);
    if (err != ERR_OK)
    {
        printf("Erro no tcp_write: %d\n", err);
        return err;
    }
    tcp_output(tpcb);

    // Registra o callback para receber dados
    tcp_recv(tpcb, tcp_client_recv);

    printf("Requisição enviada.\n");
    return ERR_OK;
}

// Função para iniciar o envio do HTTP POST usando a API raw
int send_http_post_raw()
{
    ip_addr_t dest_ip;
    IP4_ADDR(&dest_ip, 192, 168, 15, 14); // Exemplo: 192.168.1.100

    printf("Chamou!\n");
    struct tcp_pcb *pcb = tcp_new();
    if (pcb == NULL)
    {
        printf("Erro ao criar PCB.\n");
        return -1;
    }
    printf("Chamou 2!\n");

    // Conecta ao servidor na porta 3000
    err_t err = tcp_connect(pcb, &dest_ip, 3000, tcp_client_connected);
    if (err != ERR_OK)
    {
        printf("Erro ao conectar: %d\n", err);
        tcp_close(pcb);
    }
}

// Funcoes para controle das matriz de leds

struct pixel_t
{
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};

typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Inicializa a máquina PIO para controle da matriz de LEDs.
void npInit(uint pin)
{

    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0)
    {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Atribui uma cor RGB a um LED.
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Limpa o buffer de pixels.
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

// Escreve os dados do buffer nos LEDs.
void npWrite()
{
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    // sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}
// Fim das funcoes para controle da matriz de leds

// Inicio funcoes para controle o buzzer

// Definição de uma função para inicializar o PWM no pino do buzzer
void pwm_init_buzzer(uint pin)
{
    // Configurar o pino como saída de PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);

    // Obter o slice e canal do PWM associado ao pino
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);

    // Configuração do PWM
    pwm_config config = pwm_get_default_config();
    float divider = clock_get_hz(clk_sys) / (BUZZER_FREQUENCY * 4096);
    pwm_config_set_clkdiv(&config, divider);
    pwm_config_set_wrap(&config, 4095); // 12-bit PWM

    // Inicializar o PWM
    pwm_init(slice_num, &config, true);

    // Garante que o PWM começa desligado
    pwm_set_chan_level(slice_num, channel, 0);
}

// Função para emitir um beep por um determinado tempo
void beep(uint pin, uint duration_ms)
{
    // Obter o slice e canal do PWM
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint channel = pwm_gpio_to_channel(pin);

    // Ativar o PWM com duty cycle de 50%
    pwm_set_chan_level(slice_num, channel, 2048); // 50% de 4096
    pwm_set_enabled(slice_num, true);

    // Espera o tempo do beep
    sleep_ms(duration_ms);

    // Desativar o PWM corretamente
    pwm_set_enabled(slice_num, false);
}

// Fim das funcoes do controle do buzzer

void set_calcula_index(uint x, uint y, uint8_t r, uint8_t g, uint8_t b)
{
    uint index;
    // Calcula o índice na matriz invertida
    if (y % 2 == 0)
    {
        index = (4 - y) * 5 + (4 - x); // Inverte a linha e a posição dentro dela
    }
    else
    {
        index = (4 - y) * 5 + x; // Apenas inverte a linha
    }
    npSetLED(index, r, g, b); // Define a cor do LED
}

void reseta_coluna_leds(uint coluna)
{
    for (int i = 0; i < 5; i++)
    {
        set_calcula_index(i, coluna, 0, 0, 0);
    }
}

void contador_habitos(uint x, uint contador, uint tipo)
{
    if (tipo == 0)
    {
        set_calcula_index(x, contador, 0, 0, 25);
    }
    if (tipo == 1)
    {
        set_calcula_index(x, contador, 25, 0, 0);
    }
    if (tipo == 2)
    {
        set_calcula_index(x, contador, 0, 25, 0);
    }
}

bool pomodoro_funcao(struct repeating_timer *t)
{
    if (contador_pomodoro == 5)
    {
        printf("Parou no 5\n\n");
        pomodoro_status = false;
        contador_sessao_pomodoro++;
        pomodoro_concluido = true;
        return false;
    }

    printf("inicio\n");
    printf("Contador pomodoro: %d\n", contador_pomodoro);
    set_calcula_index(contador_pomodoro, POMODORO, 0, 0, 0);
    npWrite();
    contador_pomodoro++;
    return true;
}

bool acao_alarme_agua(struct repeating_timer *t)
{
    alarme_ativo_agua = true;
    return false;
}

bool acao_alarme_exercicio(struct repeating_timer *t)
{
    alarme_ativo_exercicio = true;
    return false;
}

bool acao_alarme_alimentacao(struct repeating_timer *t)
{
    alarme_ativo_alimentacao = true;
    return false;
}

bool acao_alarme_pomodoro(struct repeating_timer *t)
{
    pomodoro_status = true;
}

void display_menu(ssd1306_t *oled, int selected_option)
{
    ssd1306_clear(oled);
    // Primeira linha - Bebi água
    ssd1306_draw_string(oled, 10, 10, 1, selected_option == 0 ? "> Con. Agua" : "  Con. Agua");
    // Segunda linha - Levantei da cadeira
    ssd1306_draw_string(oled, 10, 20, 1, selected_option == 1 ? "> Exercicio" : "  Exercicio");
    // Terceira linha - Alimentação
    ssd1306_draw_string(oled, 10, 30, 1, selected_option == 2 ? "> Refeicao" : "  Refeicao");
    // Quarta linha - Desligar sistema
    ssd1306_draw_string(oled, 10, 40, 1, selected_option == 3 ? "> Pausa/Reiniciar" : "  Pausa/Reiniciar");
    // Quinta linha - Desligar
    ssd1306_draw_string(oled, 10, 50, 1, selected_option == 4 ? "> Enviar Dados" : "  Enviar Dados");
    ssd1306_show(oled);
}

void menu_interativo(ssd1306_t *oled)
{
    static int option = 0;

    display_menu(oled, option);

    uint16_t joystick_val = adc_read();

    if (joystick_val < 1000)
    {
        option = (option - 1 + 5) % 5; // Garante que não fique negativo
        sleep_ms(200);
    }
    else if (joystick_val > 3000)
    {
        option = (option + 1) % 5; // Agora temos 4 opções (0 a 3)
        sleep_ms(200);
    }

    if (gpio_get(BUTTON_A) == 0)
    {
        switch (option)
        {
        case 0:
            printf("Opcao 1");
            // Reseta caso ultrapasse tela do display
            if (contador_agua >= 5)
            {
                contador_agua = (contador_agua) % 5;
                reseta_coluna_leds(AGUA);
            }
            // Apaga led que indica contagem
            gpio_put(LED_AZUL, 0);
            // Adiciona contador e matriz
            contador_agua++;
            contador_habitos(contador_agua - 1, AGUA, AGUA);
            // Converter variavel para string
            sprintf(buffer, "%d vezes", contador_agua);
            // Animacao de acao
            ssd1306_clear(oled);
            ssd1306_draw_string(oled, 25, 10, 1, "Agua consumida");
            ssd1306_draw_string(oled, 25, 20, 2, buffer);
            // Apresenta acao
            ssd1306_show(oled);
            npWrite();
            sleep_ms(1000);
            cancel_repeating_timer(&timer_agua);
            add_repeating_timer_ms(AGUA_INTERVALO, acao_alarme_agua, NULL, &timer_agua);
            break;
        case 1:
            printf("Opcao 2\n");
            // Reseta caso ultrapasse tela do display
            if (contador_exercicio >= 5)
            {
                contador_exercicio = 0;
                reseta_coluna_leds(EXERCICIO);
            }
            // Apaga led que indica contagem
            gpio_put(LED_VERMELHO, 0);
            // Adiciona contador e matriz
            contador_exercicio++;
            contador_habitos(contador_exercicio - 1, EXERCICIO, EXERCICIO);
            // Converter variavel para string
            sprintf(buffer, "%d vezes", contador_exercicio);
            // Animacao de acao
            ssd1306_clear(oled);
            ssd1306_draw_string(oled, 25, 10, 1, "Exercicios Feitos");
            ssd1306_draw_string(oled, 25, 20, 2, buffer);
            // Apresenta acao
            ssd1306_show(oled);
            npWrite();
            sleep_ms(1000);
            cancel_repeating_timer(&timer_exercicio);
            add_repeating_timer_ms(EXERCICIO_INTERVALO, acao_alarme_exercicio, NULL, &timer_exercicio);
            break;
        case 2:
            printf("Opcao 3\n");
            // Reseta caso ultrapasse tela do display
            if (contador_alimentacao >= 5)
            {
                contador_alimentacao = 0;
                reseta_coluna_leds(ALIMENTACAO);
            }
            // Apaga led que indica contagem
            gpio_put(LED_VERDE, 0);
            // Adiciona contador e matriz
            contador_alimentacao++;
            contador_habitos(contador_alimentacao - 1, ALIMENTACAO, ALIMENTACAO);
            // Converter variavel para string
            sprintf(buffer, "%d vezes", contador_alimentacao);
            // Animacao de acao
            ssd1306_clear(oled);
            ssd1306_draw_string(oled, 30, 10, 1, "Voce comeu");
            ssd1306_draw_string(oled, 25, 20, 2, buffer);
            // Apresenta acao
            ssd1306_show(oled);
            npWrite();
            sleep_ms(1000);
            cancel_repeating_timer(&timer_alimentacao);
            add_repeating_timer_ms(ALIMENTACAO_INTERVALO, acao_alarme_alimentacao, NULL, &timer_alimentacao);
            break;
        case 3:
            printf("Opcao 4\n");

            ssd1306_clear(oled);
            ssd1306_draw_string(oled, 36, 10, 1, "Sistema");
            ssd1306_draw_string(oled, 36, 20, 1, "pausado");
            ssd1306_draw_string(oled, 25, 30, 1, "Pressione A");
            ssd1306_draw_string(oled, 25, 40, 1, "Para retornar");
            ssd1306_show(oled);

            sleep_ms(1000);

            if (!sistema_pausado)
            {
                sistema_pausado = true;
            }
            break;
        case 4:
            ssd1306_clear(oled);
            ssd1306_draw_string(oled, 30, 30, 1, "Sincronizado");
            ssd1306_show(oled);
            sleep_ms(2000);
            int response = send_http_post_raw();
            
            break;
        }

        sleep_ms(500);

        sleep_ms(100);
    }
}

void iniciar_pomodoro()
{
    contador_pomodoro = 0;
    pomodoro_status = true;
    for (int i = 5; i >= 0; i--)
    {
        set_calcula_index(i, POMODORO, 130, 130, 23);
    }
    npWrite();
    add_repeating_timer_ms(POMODORO_INTERVALO, pomodoro_funcao, NULL, &timer_pomodoro);
}

int main()
{
    // Inicializa depuracao USB
    stdio_init_all();
    // Inicializar Wifi
    if (cyw43_arch_init())
    {
        printf("Wi-Fi falhou\n");
        return -1;
    }
    else
    {
        printf("Driver CYW43 inicializado com sucesso.\n");
    }

    // Conexão WiFi
    cyw43_arch_enable_sta_mode();
    printf("Tentando conectar à rede WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("VIVOFIBRA-WIFI6-A2F1", "QT3HRPiPAdx5aaL", CYW43_AUTH_WPA2_AES_PSK, 10000))
    {
        printf("Falha ao conectar. Tentando novamente...\n");
        sleep_ms(1000); // Aguarda 1 segundo antes de tentar novamente.
    }
    else
    {
        printf("Conectado à rede WiFi!\n");
        // Exibe o endereço IP
        uint8_t *ip_address = (uint8_t *)&(cyw43_state.netif[0].ip_addr.addr);
        printf("IP address %d.%d.%d.%d\n", ip_address[0], ip_address[1], ip_address[2], ip_address[3]);
    }
    // Inicializa os LEDs Coloridos
    gpio_init(LED_AZUL);
    gpio_set_dir(LED_AZUL, GPIO_OUT);

    gpio_init(LED_VERMELHO);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);

    gpio_init(LED_VERDE);
    gpio_set_dir(LED_VERDE, GPIO_OUT);
    // Inicializa os botoes
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);

    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);

    // Inicializa Analogicos
    adc_init();
    adc_gpio_init(JOYSTICK_X);
    adc_select_input(1);
    // Inicializa display OLED
    i2c_init(i2c1, 100 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // Inicializa matriz de leds
    npInit(LED_PIN);
    npClear();
    npWrite();
    // Inicializa o buzzer e inicializa o pwn do pino do buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    pwm_init_buzzer(BUZZER_PIN);

    // Verifica sem existe erro no display
    if (!ssd1306_init(&oled, 128, 64, 0x3C, i2c1))
    {
        printf("Parou aqui\n");
        return 1;
    }
    // Menu inicial
    while (gpio_get(BUTTON_A) == 1)
    {
        ssd1306_clear(&oled);
        ssd1306_draw_string(&oled, 25, 10, 1, "Pressione A");
        ssd1306_draw_string(&oled, 22, 20, 1, "Para iniciar!");
        ssd1306_draw_string(&oled, 20, 35, 1, "TRACKER HABITOS");
        ssd1306_show(&oled);
    }

    ssd1306_clear(&oled);
    ssd1306_draw_string(&oled, 25, 10, 1, "Iniciando...");
    ssd1306_show(&oled);
    sleep_ms(2000);

    ssd1306_clear(&oled);
    sprintf(buffer, "%d SEGs", (AGUA_INTERVALO / 1000));
    ssd1306_draw_string(&oled, 25, 10, 1, "Agua:");
    ssd1306_draw_string(&oled, 25, 20, 2, buffer);
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    sprintf(buffer, "%d SEGs", (EXERCICIO_INTERVALO / 1000));
    ssd1306_draw_string(&oled, 25, 10, 1, "Exercicios:");
    ssd1306_draw_string(&oled, 25, 20, 2, buffer);
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    sprintf(buffer, "%d SEGs", (ALIMENTACAO_INTERVALO / 1000));
    ssd1306_draw_string(&oled, 25, 10, 1, "Refeicoes:");
    ssd1306_draw_string(&oled, 25, 20, 2, buffer);
    ssd1306_show(&oled);
    sleep_ms(3000);

    ssd1306_clear(&oled);
    sprintf(buffer, "%d SEC", ((POMODORO_INTERVALO / 1000)) * 5);
    ssd1306_draw_string(&oled, 25, 10, 1, "Pomodoro:");
    ssd1306_draw_string(&oled, 25, 20, 2, buffer);
    ssd1306_show(&oled);
    sleep_ms(3000);

    // Iniciar contadores
    gpio_put(LED_AZUL, 0);
    gpio_put(LED_VERMELHO, 0);
    gpio_put(LED_VERDE, 0);

    add_repeating_timer_ms(AGUA_INTERVALO, acao_alarme_agua, NULL, &timer_agua);
    add_repeating_timer_ms(EXERCICIO_INTERVALO, acao_alarme_exercicio, NULL, &timer_exercicio);
    add_repeating_timer_ms(ALIMENTACAO_INTERVALO, acao_alarme_alimentacao, NULL, &timer_alimentacao);

    while (1)
    {
        menu_interativo(&oled);

        if (pomodoro_concluido)
        {
            sprintf(buffer, "%d sessao", contador_sessao_pomodoro);
            // Animacao de acao
            ssd1306_clear(&oled);
            ssd1306_draw_string(&oled, 25, 10, 1, "Voce concluiu");
            ssd1306_draw_string(&oled, 20, 20, 2, buffer);
            ssd1306_draw_string(&oled, 25, 40, 1, "Pomodoro");

            beep(BUZZER_PIN, 1000);
            // Apresenta acao
            ssd1306_show(&oled);
            sleep_ms(10000);

            pomodoro_concluido = false;
        }

        if (sistema_pausado)
        {
            ssd1306_clear(&oled);
            ssd1306_draw_string(&oled, 36, 10, 1, "Sistema");
            ssd1306_draw_string(&oled, 36, 20, 1, "pausado");
            ssd1306_draw_string(&oled, 25, 30, 1, "Pressione A");
            ssd1306_draw_string(&oled, 25, 40, 1, "Para retornar");
            ssd1306_show(&oled);

            cancel_repeating_timer(&timer_agua);
            cancel_repeating_timer(&timer_alimentacao);
            cancel_repeating_timer(&timer_exercicio);

            while (!gpio_get(BUTTON_A) == 0)
            {
                sleep_ms(100);
            }

            sleep_ms(1000);
            sistema_pausado = false;

            add_repeating_timer_ms(20000, acao_alarme_agua, NULL, &timer_agua);
            add_repeating_timer_ms(30000, acao_alarme_exercicio, NULL, &timer_exercicio);
            add_repeating_timer_ms(10000, acao_alarme_alimentacao, NULL, &timer_alimentacao);
        }

        if (alarme_ativo_agua)
        {
            gpio_put(LED_AZUL, 1);
            beep(BUZZER_PIN, 500);
            alarme_ativo_agua = false;
            cancel_repeating_timer(&timer_agua);
        }

        if (alarme_ativo_exercicio)
        {
            gpio_put(LED_VERMELHO, 1);
            beep(BUZZER_PIN, 500);
            alarme_ativo_exercicio = false;
            cancel_repeating_timer(&timer_exercicio);
        }

        if (alarme_ativo_alimentacao)
        {
            gpio_put(LED_VERDE, 1);
            beep(BUZZER_PIN, 500);
            alarme_ativo_alimentacao = false;
            cancel_repeating_timer(&timer_alimentacao);
        }

        if (gpio_get(BUTTON_B) == 0 && !pomodoro_status)
        {
            printf("Botao Pressionado\n");
            // Pomodoro Iniciado
            ssd1306_clear(&oled);
            ssd1306_draw_string(&oled, 35, 10, 1, "Pomodoro");
            ssd1306_draw_string(&oled, 35, 20, 1, "Iniciado");
            ssd1306_show(&oled);
            sleep_ms(1500);
            iniciar_pomodoro();
            sleep_ms(150);
        }

        sleep_ms(100);
    }

    return 0;
}
