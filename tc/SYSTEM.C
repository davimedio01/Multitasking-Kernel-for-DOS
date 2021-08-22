/***********************************************************************/
/*                                                                     */
/* ARQUIVO SYSTEM: contem os tipos e funcoes basicas direcionadas para */
/*                 suporte de multiprogramacao                         */
/***********************************************************************/

#include <stdio.h>
#include <dos.h> far
#include <setjmp.h>
#include <alloc.h>
#include <stdlib.h>

#define MAX_PILHA  1000      /* tamanho maximo da pilha do contexto */

typedef struct {             /* Estrutura usada para armazenar o contexto */
    unsigned ss,sp;          /* Apontador de segmento  de pilha e do topo */
    unsigned area[MAX_PILHA];/* Area reservada para pilha                 */
    unsigned base;           /* Base da pilha                             */
    }descritor;

typedef descritor *PTR_DESC;  /* TIPO PONTEIRO PARA TIPO DESCRITOR */

typedef struct {                    /* Estrutura usada pelo iotransfer()  */
    PTR_DESC p_origem,p_destino;    /* Endereco do chamador e do processo */
                                    /* destino do iotransfer()            */
    int num_vetor;                  /* Numero do vetor onde sera instalada*/
                                    /* a rotina de retorno                */
    void interrupt (*int_anterior)(); /* Endereco da rotina antiga        */
    } estrutura_io;

typedef estrutura_io  *PTR_ESTR; /* tipo ponteiro para estrutura_io */

estrutura_io est_io1;            /* declara a variavel global  usada pelo */
                                 /* iotransfer()                          */

PTR_ESTR  p_est=&est_io1;        /* Inicializa o ponteiro para estrutura  */
                                 /* usada pelo iotransfer()               */

PTR_DESC daux;                   /* ponteiro usado por newprocess()       */
PTR_DESC origem;                 /* apontadores da co-rotina chamadora e  */
PTR_DESC destino1;               /* destino em um transfer()              */

jmp_buf env_buf;                 /* buffer usado por setjmp() e longjmp() */
                                 /* usados nas funcoes newprocess()       */



/********************** ROTINA TRANSFER1 (CHAMADA POR TRANSFER) ***********/

void interrupt transfer1()
{
 disable();
 origem->ss=_SS;   /* guarda contexto da co-rotina origem do transfer()*/
 origem->sp=_SP;

 _SP=destino1->sp; /* guarda contexto da co-rotina destino do transfer() */
 _SS=destino1->ss;

 p_est->p_destino = destino1;  /* como houve um transfer mudou o destino */
                               /* do iotransfer() (caso ele esteja sendo */
                               /* usado ou nao)                          */
 enable();
}

/************************* ROTINA TRANSFER ********************************/
/*  Muda controle da co-rotina apontada por "or" para co-rotina apontada  */
/*   por "dest"                                                           */

void far transfer(or,dest)
PTR_DESC or,dest;
{
 origem=or;
 destino1=dest;
 transfer1();
}

/********** ROTINA AUXILIAR DA NEWPROCESS **********************************/

void interrupt inicia_pilha(bp,di,si,ds,es,dx,cx,bx,ax,ip,cs,flags)

{
 daux->ss=_SS;         /* inicia os apontadores de pilha do contexto */
 daux->sp=_SP;
 ip=ax;                /* faz ip (do contexto) apontar para co-rotina*/
                       /* que esta sendo criada                      */

 longjmp(env_buf,1);   /* Continua a partir de setjmp() em newprocess() */
}

/**************************** ROTINA NEWPROCESS ****************************/
/*  Inicializa o descritor do contexto de uma co-rotina                    */

void far newprocess(proc,end_desc)
void far (*proc)();
PTR_DESC end_desc;
{
int i;

daux=end_desc;
for(i=0;i<MAX_PILHA;i++)        /* zera vetor da pilha do contexto */
   daux->area[i]=0;

if(!setjmp(env_buf))
  {
  disable();
  _SS=_DS;                      /* inicializa campos dos apontadores de */
  _SP=(unsigned)&daux->base;    /* pilha                                */
  enable();
  _AX=(unsigned)proc;
  inicia_pilha();              /* chama rotina que inicializa a pilha   */
  }                            /* associada `a co-rotina                */
}

/******************** ROTINA CRIA_DESC() *******************************/
/* Cria um descritor dinamicamente, e retorna um ponteiro para o mesmo */

PTR_DESC cria_desc()
{
 PTR_DESC d;

 if((d=(descritor*)malloc(sizeof(descritor)))==NULL)
   {
   printf("\n\tMemoria Insuficiente para alocacao de descritor\n");
   exit(1);
   }
 return d;
}

/****************** ROTINA TRANSFER_IO_RET() ******************************/
/*  Rotina auxiliar do iotransfer(): e' instalada no local da rotina de   */
/*  de interrupcao ativada pelo iotrasnsfer().                            */

void interrupt transfer_io_ret()
{

 disable();
 p_est->p_destino->ss=_SS;    /* guarda o contexto do processo interrompido*/
 p_est->p_destino->sp=_SP;

 _SP= p_est->p_origem->sp;  /* muda o controle para rotina que ativou o   */
 _SS= p_est->p_origem->ss;  /* iotransfer().                              */
 enable();
 p_est->int_anterior();     /* salta para rotina antiga de interrupcao    */
}

/****************** ROTINA TRANSFER_IO_IDA() ******************************/
/*  Rotina auxiliar do iotransfer(): guarda o contexto do chamador do     */
/*  iotransfer() e passa o controle para a rotina destino.                */

void interrupt transfer_io_ida()
{

 disable();
 p_est->p_origem->ss=_SS;     /* guarda contexto do chamador  */
 p_est->p_origem->sp=_SP;

 _SP= p_est->p_destino->sp;   /* transfere controle para processo destino */
 _SS= p_est->p_destino->ss;
 enable();
}

/****************** ROTINA IOTRANSFER() ***********************************/
/*  Transfere controle da co-rotina origem para co-rotina destino, e      */
/*  instala a co-rotina origem como rotina de interrupcao                 */

void far iotransfer()
{
static int aux_io=0;

 if (!aux_io)       /* se iotransfer()  foi ativada pela primeira vez    */
  {
  aux_io=1;
  p_est->int_anterior=getvect(p_est->num_vetor); /* obtem endereco da rotina*/
                                                 /* de interrupcao original */
  setvect(p_est->num_vetor,transfer_io_ret); /* instala rotina de retorno   */
  }                                          /* como rotina de interrupcao  */

transfer_io_ida();  /* transfere controle para rotina destino */
}

/***************************************************************************/

