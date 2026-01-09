#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE 0x2000
#define IO_BASE 0xF000

uint16_t memoria[MEM_SIZE];
bool memoria_acessada[MEM_SIZE];
uint16_t mem_read(uint16_t addr)
{
    /* MEMÓRIA NORMAL */
    if (addr < MEM_SIZE)
    {
        memoria_acessada[addr] = true;
        return memoria[addr];
    }

    /* IO - ENTRADA */
    if (addr == 0xF000)
    { // CHAR IN
        char c;
        scanf(" %c", &c);
        return (uint16_t)c;
    }

    if (addr == 0xF002)
    { // INT IN
        int x;
        scanf("%d", &x);
        return (uint16_t)x;
    }

    return 0; // fora da faixa → 0
}

void mem_write(uint16_t addr, uint16_t value)
{
    /* MEMÓRIA NORMAL */
    if (addr < MEM_SIZE)
    {
        memoria[addr] = value;
        memoria_acessada[addr] = true;
        return;
    }

    /* IO - SAÍDA */
    if (addr == 0xF001)
    { // CHAR OUT
        printf("OUT <= %c\n", (char)value);
        return;
    }

    if (addr == 0xF003)
    { // INT OUT
        printf("OUT <= %d\n", (int16_t)value);
        return;
    }
}
void push(uint16_t *regs, uint16_t value)
{
    regs[14]--; // SP--
    memoria[regs[14]] = value;
    memoria_acessada[regs[14]] = true;
}

uint16_t pop(uint16_t *regs)
{
    uint16_t value = memoria[regs[14]];
    regs[14]++; // SP++
    return value;
}
/* ================= PESSOA 3 ================= */

/* LDR */
case 0x2:
{
    uint16_t endereco = regs[Rm] + Im; // zero-extension
    regs[Rd] = mem_read(endereco);
    break;
}

/* STR */
case 0x3:
{
    uint16_t endereco = regs[Rm] + Im;
    mem_write(endereco, regs[Rn]);
    break;
}

/* PUSH */
case 0xE:
{
    push(regs, regs[Rn]);
    break;
}

/* POP */
case 0xF:
{
    regs[Rd] = pop(regs);
    break;
}
