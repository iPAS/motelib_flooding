#include <stdio.h>

#include "queue.h"

#define TEST_COUNT 1000000
queue_t q;

int main()
{
    char *messages[] =
    {
        "hello one",
        "two worlds",
        "Tree for three",
    };

    q_init(&q);

    int loop = TEST_COUNT;
    while (loop-- > 0)
    {
        printf("loop: %d\n", TEST_COUNT-loop);
        int i;

        for (i = 0; i < sizeof(messages)/sizeof(char *); i++)
        {
            q_enqueue(&q, messages[i], strlen(messages[i]));
            printf("In queue: '%s' -- queue len = %d\n", messages[i], q_length(&q));
        }

        char buffer[11];
        buffer[sizeof(buffer)-1] = '\0';
        uint8_t len;

        for (i = 0; i < sizeof(messages)/sizeof(char *); i++)
        {
            len = q_dequeue(&q, buffer, sizeof(buffer)-1);
            printf("Out queue: '%s' -- queue left = %d\n", buffer, q_length(&q));
        }
    }
    return 0;
}
