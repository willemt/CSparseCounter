#include <stdbool.h>
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "sparse_counter.h"

typedef struct var_block_s var_block_t;

struct var_block_s
{
    int offset;
    int len;
    var_block_t *next;
};

static int __capmax(
    int val,
    int max
)
{
    if (max < val)
    {
        return max;
    }
    else
    {
        return val;
    }
}

void *sparsecounter_init(
    const int max
)
{
    sparsecounter_t *prog;

    prog = calloc(1, sizeof(sparsecounter_t));
    prog->max = max;
    prog->first_block = NULL;
    return prog;
}

void sparsecounter_free(
    void *ra
)
{
    sparsecounter_t *prog = ra;
    var_block_t *blk, *prev;

    blk = prog->first_block;

    while (blk)
    {
        prev = blk;
        blk = blk->next;
        free(prev);
    }

    free(prog);
}

/**
 * @return number of non-contiginous blocks */
int sparsecounter_get_num_blocks(
    sparsecounter_t * prog
)
{
    var_block_t *block;
    int num;

    block = prog->first_block;

    for (num = 0; block; num++)
    {
        block = block->next;
    }

    return num;
}

/**
 * mark this block as complete */
void sparsecounter_mark_complete(
    sparsecounter_t * prog,
    const int offset,
    const int len
)
{
    var_block_t *this = NULL, *prev;

    if (!prog->first_block)
    {
        var_block_t *block;

        block = prog->first_block = malloc(sizeof(var_block_t));
        block->len = len;
        block->offset = offset;
        block->next = NULL;
        return;
    }

    /* get 1st block that has a higher offset than us */

    prev = prog->first_block;

    if (offset < prev->offset)
    {
        var_block_t *block;

        block = prog->first_block = malloc(sizeof(var_block_t));
        block->len = len;
        block->offset = offset;
        block->next = this = prev;
        prev = block;
    }
    else
    {
        /*  find a place where our offset is equal or greater */
        while (prev->next && (prev->next->offset <= offset))
        {
            prev = prev->next;
        }

        this = prev->next;

        /* The left block eats/touches this new one...
         * |00000LLN00000|;
         * |00000LLNL0000|
         * Combine the old with new */
        if (offset <= prev->offset + prev->len)
        {
            /* completely ate this one!
             * |00000LLNL0000| */
            if (offset + len <= prev->offset + prev->len)
            {

            }
            else
            {
                /* increase coverage */
                prev->len = (offset + len) - prev->offset;
                /* might eat another... */
            }
        }
        else
        {
            var_block_t *blk;

            blk = malloc(sizeof(var_block_t));
            blk->offset = offset;
            blk->len = len;
            blk->next = this;
            prev->next = blk;
            prev = blk;
        }

    }

    /* prev will feast! */
    if (this && this->offset <= prev->offset + prev->len)
    {
        do
        {
            /* not eaten */
            if (!(this->offset + this->len <= prev->offset + prev->len))
            {
                prev->len = (this->offset + this->len) - prev->offset;
            }

            prev->next = this->next;
            free(this);
            this = prev->next;
        }
        while (this && this->offset <= prev->offset + prev->len);
    }
}

bool sparsecounter_is_complete(
    sparsecounter_t * prog
)
{
    var_block_t *block;

    block = prog->first_block;

    return block && !block->next && block->len == prog->max;
}

/**
 * Get an incompleted block  */
void sparsecounter_get_incomplete(
    const sparsecounter_t * prog,
    int *offset,
    int *len,
    const int max
)
{
    const var_block_t *block;

    *offset = *len = 0;
    block = prog->first_block;

    if (!block)
    {
        *offset = 0;
        *len = max;
    }
    else
    {
        if (block->offset != 0)
        {
            *offset = 0;

            if (block->next)
            {
                *len = block->next->offset - block->offset;
            }
            else
            {
                *len = block->offset;
            }
        }
        else if (!block->next)
        {
            *offset = block->len;
            *len = max;
        }
        else
        {
            *offset = 0 + block->len;
            *len = block->next->offset - block->len;
        }
    }

    *len = __capmax(*len, max);

//    printf("%d %d %d %d\n", prog->max, *len, max, *offset + *len);

    /*  make sure we aren't going over the boundary */
    if (prog->max < *offset + *len)
    {
        *len = prog->max - *offset;
    }
}

int sparsecounter_get_nbytes_completed(
    const sparsecounter_t * prog
)
{
    const var_block_t *block;

    block = prog->first_block;

    int nbytes = 0;

    while (block)
    {
        nbytes += block->len;
        block = block->next;
    }

    return nbytes;
}

/**
 * @return 1 if we have this block, 0 otherwise */
int sparsecounter_have(
    sparsecounter_t * prog,
    const int offset,
    const int len
)
{
    const var_block_t *block;

    block = prog->first_block;

    while (block)
    {
        if (offset < block->offset)
        {

        }
        else if (block->offset <= offset)
        {
            if (offset + len <= block->offset + block->len)
            {
                return 1;
            }
        }
        else
        {
            return 0;
        }

        block = block->next;
    }

    return 0;
}
