#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include "cpu.h"

int main(int argc, char* argv[]) {
    CPU cpu;
    cpu_init(&cpu);
    
    if(argc < 2) {
        printf("Uso: %s <arquivo_binario>\n", argv[0]);
        return 1;
    }

    FILE *arquivo = fopen(argv[1], "r");
    if (arquivo == NULL) {
        fprintf(stderr, "Erro ao abrir o arquivo %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    int addr, content;
    char buffer[256]; // Buffer para ler o "lixo" (resto da linha)

    // O loop tenta ler 2 valores HEX: Endereço e Conteúdo
    // Retorna 2 se conseguir ler ambos com sucesso
    while (fscanf(arquivo, "%x %x", &addr, &content) == 2) {
        
        // Verifica se o endereço é válido dentro da memória
        if (addr >= 0 && addr < MEMORY_SIZE) {
            cpu.memory[addr] = (uint16_t)content;
        }

        // IMPORTANTE: fgets lê o resto da linha (ex: " MOV R0, #5 //...") e descarta
        fgets(buffer, sizeof(buffer), arquivo);
    }

    fclose(arquivo);
    
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
    //     fgets(buffer, sizeof(buffer), arquivo);
    // }
    // fclose(arquivo);
    
    // Ler os breakpoints dos argumentos da linha de comando
    qtd_breakpoints = 0;
    for (int i = 2; i < argc && qtd_breakpoints < MAX_BREAKPOINTS; i++) {
        breakpoints[qtd_breakpoints] = (uint16_t) strtol(argv[i], NULL, 16);
        qtd_breakpoints++;
    }

    cpu_run(&cpu);

    printf("Programa finalizado. PC: %04X\n", cpu.registers[15]);
    return 0;
}