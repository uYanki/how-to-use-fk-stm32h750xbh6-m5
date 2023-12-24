#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

typedef struct {
    uint8_t buff[5];
    uint8_t reader;  // cur
    uint8_t writer;  // next
    uint8_t size;
    uint8_t cap;
} loop_t;

loop_t loop = {{0}, 0, 0, 0, 5};

void push(uint8_t n)
{
    loop.buff[loop.writer] = n;

    loop.reader = 0;
    loop.writer++;
    loop.writer %= loop.cap;

    if (loop.size < loop.cap)
    {
        loop.size++;
    }
}

#define INDEX(n) ((loop.cap + n) % loop.cap)

void disp()
{
    printf("----------\n");

    for (uint8_t i = 0; i < loop.size; ++i)
    {
        uint16_t n = INDEX(loop.writer - loop.size + i);

        printf("%d. %d\n", i, loop.buff[n]);
    }

    printf("----------\n");
}

void read_prev()
{
    if (loop.reader == loop.size)
    {
        return;
    }

    loop.reader++;

    printf("%d\n", loop.buff[INDEX(loop.writer - loop.reader)]);
}

void read_next()
{
    if (loop.reader > 0)
    {
        loop.reader--;

        if (loop.reader == 0)
        {
        }
        else
        {
            printf("%d\n", loop.buff[INDEX(loop.writer - loop.reader)]);
        }
    }
}

int main()
{
    push(0);
    push(1);
    push(2);
    disp();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    push(3);
    push(4);
    push(5);
    push(6);
    push(7);
    push(8);
    disp();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_prev();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    read_next();
    disp();

    return -1;
}