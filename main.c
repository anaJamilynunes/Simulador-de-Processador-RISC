#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

int main(int argc, char *argv[])
{
    CPU cpu;
    cpu_init(&cpu);

    if (argc < 2)
    {
        printf("Uso: %s <arquivo_binario>\n", argv[0]);
        return 1;
    }

    FILE *arquivo = fopen(argv[1], "r");
    if (arquivo == NULL)
    {
        fprintf(stderr, "Erro ao abrir o arquivo %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    // --- MUDANÇA NECESSÁRIA PARA LER ARQUIVOS COM ---
    char linha[256];
    int addr, content;

    // Usamos fgets para ler a linha inteira. Se a linha for texto (ex: ),
    // o sscanf vai falhar em ler hexadecimais e o if será ignorado,
    // garantindo que o programa continue rodando sem erro.
    while (fgets(linha, sizeof(linha), arquivo) != NULL)
    {

        // Tenta ler dois hexadecimais: Endereço e Conteúdo
        if (sscanf(linha, "%x %x", &addr, &content) == 2)
        {

            // Verifica se o endereço é válido dentro da memória
            if (addr >= 0 && addr < MEMORY_SIZE)
            {
                cpu.memory[addr] = (uint16_t)content;
            }
        }
    }

    fclose(arquivo);

    // --- SEUS COMENTÁRIOS ORIGINAIS MANTIDOS ABAIXO ---

    // // Variáveis para leitura
    // int linha_ignore; // Ignorar o número da linha
    // int addr;         // Endereço (hex)
    // int content;      // Conteúdo/Instrução (hex)
    // char buffer[256]; // Para ler o resto da linha (comentários)

    // // Loop de leitura conforme formato do PDF (Ex: 1 0000 00B4 ...)
    // // Lê: Inteiro (lixo), Hex (endereço), Hex (conteúdo)
    // while (fscanf(arquivo, "%d %x %x", &linha_ignore, &addr, &content) == 3) {

    //     // Verifica se o endereço é válido
    //     if (addr >= 0 && addr < MEMORY_SIZE) {
    //         cpu.memory[addr] = (uint16_t)content;
    //     }

    //     // Consome o resto da linha (comentários)
    //     // fgets(buffer, sizeof(buffer), arquivo);
    // }
    // fclose(arquivo);

    // Ler os breakpoints dos argumentos da linha de comando
    qtd_breakpoints = 0;
    for (int i = 2; i < argc && qtd_breakpoints < MAX_BREAKPOINTS; i++)
    {
        breakpoints[qtd_breakpoints] = (uint16_t)strtol(argv[i], NULL, 16);
        qtd_breakpoints++;
    }

    cpu_run(&cpu);

    printf("Programa finalizado. PC: %04X\n", cpu.registers[15]);
    return 0;
}