#include <stdio.h>
#include <dos.h> far
#include <setjmp.h>
/*#include <alloc.h>*/
#include <stdlib.h>

#define MAX_PILHA  1000


typedef struct {
    unsigned ss,sp;
    unsigned area[MAX_PILHA];
    unsigned base;
    }descritor;

typedef descritor *PTR_DESC;  /* TIPO PONTEIRO PARA TIPO DESCRITOR */

typedef struct {
    PTR_DESC p_origem,p_destino;
    int num_vetor;
    void interrupt (*int_anterior)();
    } estrutura_io;

typedef estrutura_io  *PTR_ESTR; /* tipo ponteiro para estrutura_io */

extern PTR_ESTR p_est;

extern estrutura_io est_io1;

extern void far transfer(PTR_DESC or,PTR_DESC dest);


extern void  far newprocess(void far(*proc)(),PTR_DESC end_desc);

extern PTR_DESC cria_desc();

extern void far iotransfer();



