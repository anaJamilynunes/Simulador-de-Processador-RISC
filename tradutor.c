#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_LABELS 256
#define MAX_LINE 256

/* Tabela de Labels (Símbolos) */
// Estrutura para salvar o nome da label e o endereço dela
typedef struct
{
    char name[64];
    uint16_t address;
} Label;

Label labels[MAX_LABELS];
int label_count = 0;

/* --- Funções Auxiliares --- */

// Função TRIM: remove espaços em branco do começo e do fim
void trim(char *str)
{
    char *start = str;
    // Avança o ponteiro enquanto for espaço em branco
    while (isspace((unsigned char)*start))
        start++;

    // Move o conteúdo útil para o início da string
    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }

    // Remove os espaços do final da string
    if (*str == 0)
        return;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
}

// Procura o endereço de uma label pelo nome
int get_label_address(const char *name)
{
    for (int i = 0; i < label_count; i++)
    {
        if (strcmp(labels[i].name, name) == 0)
        {
            return labels[i].address;
        }
    }
    return -1; // Retorna -1 se não encontrar
}

// Identifica o registrador e retorna o índice dele
int parse_reg(const char *str)
{
    // Se começar com R ou r, pega o número logo depois
    if (str[0] == 'R' || str[0] == 'r')
    {
        return atoi(str + 1);
    }
    // Tratamento para SP e PC que são especiais
    if (strcasecmp(str, "SP") == 0)
        return 14;
    if (strcasecmp(str, "PC") == 0)
        return 15;
    return 0;
}

// Faz o parse de valores imediatos (Hexa, Char ou Decimal)
int parse_imm(const char *str, uint16_t current_pc)
{
    // Pula o # se tiver
    const char *p = (str[0] == '#') ? str + 1 : str;

    // Verifica se é Hexadecimal
    if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
    {
        return (int)strtol(p, NULL, 16);
    }

    // Verifica se é um Char literal ('A', 'B'...)
    if (p[0] == '\'' || (unsigned char)p[0] > 127)
    {
        int k = 0;
        // Pula aspa inicial
        if ((unsigned char)p[k] > 127)
        {
            while (p[k] && (unsigned char)p[k] > 127)
                k++;
        }
        else
        {
            k = 1;
        }

        // Tratamento de caracteres de escape como \0 e \n
        if (p[k] == '\\')
        {
            if (p[k + 1] == '0')
                return 0; // Caractere nulo
            if (p[k + 1] == 'n')
                return 10; // Quebra de linha
            return p[k + 1];
        }
        return (int)p[k];
    }

    // Se for label, busca o endereço na tabela
    if (isalpha(p[0]))
    {
        int addr = get_label_address(p);
        if (addr != -1)
            return addr;
        return 0;
    }

    // Se não for nada disso, assume que é decimal
    return atoi(p);
}

/* --- Montagem das Instruções --- */
// Essa função monta a linha e devolve o código de máquina
uint16_t assemble_line(char *line, uint16_t pc)
{
    char instr[16], op1[32], op2[32], op3[32];
    instr[0] = op1[0] = op2[0] = op3[0] = '\0';

    // Separa a instrução e os operandos
    sscanf(line, "%s %[^,], %[^,], %s", instr, op1, op2, op3);

    // Converte para maiúsculo para facilitar a comparação
    for (int i = 0; instr[i]; i++)
        instr[i] = toupper(instr[i]);

    if (strcmp(instr, "HALT") == 0)
        return 0xFFFF;

    if (strcmp(instr, "JMP") == 0)
    {
        int addr = parse_imm(op1, pc);
        // Se for label, precisa calcular o offset relativo
        if (get_label_address(op1 + (op1[0] == '#' ? 1 : 0)) != -1)
            addr = addr - (pc + 1);
        return (0x0 << 0) | ((addr & 0xFFF) << 4);
    }

    // Verifica qual tipo de pulo condicional
    int cond = -1;
    if (strcmp(instr, "JEQ") == 0)
        cond = 0;
    else if (strcmp(instr, "JNE") == 0)
        cond = 1;
    else if (strcmp(instr, "JLT") == 0)
        cond = 2;
    else if (strcmp(instr, "JGE") == 0)
        cond = 3;

    if (cond != -1)
    {
        int addr = parse_imm(op1, pc);
        if (get_label_address(op1 + (op1[0] == '#' ? 1 : 0)) != -1)
            addr = addr - (pc + 1);
        return (0x1) | ((addr & 0x3FF) << 4) | (cond << 14);
    }

    if (strcmp(instr, "MOV") == 0)
    {
        int rd = parse_reg(op1);
        int imm = parse_imm(op2, pc);
        return (0x4) | ((imm & 0xFF) << 4) | (rd << 12);
    }

    // Instruções de memória (LDR)
    if (strcmp(instr, "LDR") == 0)
    {
        // Parsing manual pra lidar com os colchetes
        char *ptr_rm = strchr(op2, '[');
        if (!ptr_rm)
            ptr_rm = op2;
        else
            ptr_rm++;
        char *ptr_im = op3;
        char *end_im = strchr(ptr_im, ']');
        if (end_im)
            *end_im = '\0';
        int rd = parse_reg(op1);
        int rm = parse_reg(ptr_rm);
        int imm = parse_imm(ptr_im, pc);
        return (0x2) | ((imm & 0xF) << 4) | (rm << 8) | (rd << 12);
    }

    // Instrução STR (Store)
    if (strcmp(instr, "STR") == 0)
    {
        char *ptr_rm = strchr(op2, '[');
        if (!ptr_rm)
            ptr_rm = op2;
        else
            ptr_rm++;
        char *ptr_im = op3;
        char *end_im = strchr(ptr_im, ']');
        if (end_im)
            *end_im = '\0';
        int rn = parse_reg(op1);
        int rm = parse_reg(ptr_rm);
        int imm = parse_imm(ptr_im, pc);
        return (0x3) | (rn << 4) | (rm << 8) | ((imm & 0xF) << 12);
    }

    // Operações de Pilha
    if (strcmp(instr, "PUSH") == 0)
        return (0xE) | (parse_reg(op1) << 4);
    if (strcmp(instr, "POP") == 0)
        return (0xF) | (parse_reg(op1) << 12);

    // Operações Aritméticas e Lógicas
    int opcode = -1;
    int is_imm = 0;
    if (strcmp(instr, "ADD") == 0)
        opcode = 0x5;
    else if (strcmp(instr, "ADDI") == 0)
    {
        opcode = 0x6;
        is_imm = 1;
    }
    else if (strcmp(instr, "SUB") == 0)
        opcode = 0x7;
    else if (strcmp(instr, "SUBI") == 0)
    {
        opcode = 0x8;
        is_imm = 1;
    }
    else if (strcmp(instr, "AND") == 0)
        opcode = 0x9;
    else if (strcmp(instr, "OR") == 0)
        opcode = 0xA;
    else if (strcmp(instr, "SHR") == 0)
    {
        opcode = 0xB;
        is_imm = 1;
    }
    else if (strcmp(instr, "SHL") == 0)
    {
        opcode = 0xC;
        is_imm = 1;
    }
    else if (strcmp(instr, "CMP") == 0)
        opcode = 0xD;

    if (opcode != -1)
    {
        if (opcode == 0xD)
        {
            return (opcode) | (parse_reg(op2) << 4) | (parse_reg(op1) << 8);
        }
        else
        {
            int rd = parse_reg(op1);
            int rm = parse_reg(op2);
            // Verifica se o terceiro operando é imediato ou registrador
            int val_n = is_imm ? parse_imm(op3, pc) : parse_reg(op3);
            return (opcode) | ((val_n & 0xF) << 4) | (rm << 8) | (rd << 12);
        }
    }

    return (uint16_t)parse_imm(line, pc);
}

int main(int argc, char **argv)
{
    // Valida os argumentos de entrada
    if (argc != 3)
    {
        printf("Uso: %s <input.asm> <output.hex>\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    if (!fin)
    {
        printf("Erro ao abrir %s\n", argv[1]);
        return 1;
    }

    char line[MAX_LINE];
    uint16_t pc = 0;

    // PASS 1: Identificar todas as labels
    // Precisa fazer isso antes pra não dar erro de referência
    while (fgets(line, MAX_LINE, fin))
    {
        // Remove comentários
        char *comment = strstr(line, "//");
        if (comment)
            *comment = '\0';
        trim(line);
        if (strlen(line) == 0)
            continue;

        // Se tiver label, salva o endereço
        char *colon = strchr(line, ':');
        if (colon)
        {
            *colon = '\0';
            strcpy(labels[label_count].name, line);
            labels[label_count].address = pc;
            label_count++;

            // Verifica se tem instrução na mesma linha da label
            char *rest = colon + 1;
            trim(rest);
            if (strlen(rest) > 0)
                pc++;
        }
        else
        {
            pc++;
        }
    }

    // Reseta o arquivo para ler do início
    rewind(fin);
    FILE *fout = fopen(argv[2], "w");

    // PASS 2: Gerar o código de máquina
    pc = 0;
    while (fgets(line, MAX_LINE, fin))
    {
        char *comment = strstr(line, "//");
        if (comment)
            *comment = '\0';
        trim(line);
        if (strlen(line) == 0)
            continue;

        char *colon = strchr(line, ':');
        char *instruction_part = line;

        // Pula a label se tiver, pega só a instrução
        if (colon)
        {
            instruction_part = colon + 1;
            trim(instruction_part);
            if (strlen(instruction_part) == 0)
                continue;
        }

        uint16_t code = assemble_line(instruction_part, pc);
        fprintf(fout, "%04X %04X\n", pc, code);
        pc++;
    }

    fclose(fin);
    fclose(fout);
    printf("Montagem concluida com sucesso!\n");
    return 0;
}