
typedef struct {
    void *first_block;
    unsigned int max;
} sparsecounter_t;

void *sc_init(
        const unsigned int max);

void sc_set_max(sparsecounter_t* me, const unsigned int max);

void sc_free(
        void *ra);

int sc_get_num_blocks(
        const sparsecounter_t * prog);

void sc_mark_complete(
        sparsecounter_t * prog,
        const unsigned int offset,
        const unsigned int len);

void sc_mark_incomplete(
        sparsecounter_t * prog,
        const unsigned int offset,
        const unsigned int len);

void sc_mark_all_incomplete(sparsecounter_t * me);

int sc_is_complete(
        const sparsecounter_t * prog);

void sc_get_incomplete(
        const sparsecounter_t * prog,
        unsigned int *offset,
        unsigned int *len,
        const unsigned int max);

unsigned int sc_get_nbytes_completed(
        const sparsecounter_t * prog);

int sc_have(
        const sparsecounter_t * prog,
        const unsigned int offset,
        const unsigned int len);

void sc_print_contents(
        const sparsecounter_t * prog);
