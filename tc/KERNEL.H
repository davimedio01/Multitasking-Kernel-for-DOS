#include <system.h>

#define MAX_NAME_PROCESS 35
#define INTERRUPT_BIT    8

/* ************************************************************ */
/* Definicao do BCP = Bloco de Controle de Processo (Descritor) */
/* ************************************************************ */
typedef struct pcb
{
  char name[MAX_NAME_PROCESS]; /* Armazernar o identificador do processo */
  enum
  {
    activated,
    blocked,
    finished
  } state;                    /* Armazenar o estado corrente do processo */
  PTR_DESC context;           /* Armazenar o contexto do processo */
  struct pcb *semaphoreQueue; /* Fila de Processos Bloqueados por um Semaforo */
  struct pcb *next_pcb;       /* Fila de Processos em forma de Lista Circular */
} PCB;

/* Ponteiro para o Bloco de Controle de Processo (Alocacao Dinamica) */
typedef PCB *PTR_PCB;

/* ************************************************** */
/* Definicao dos Semaforos, para Processos Bloqueados */
/* ************************************************** */
typedef struct{
  int value;             /* Valor do Semaforo */
  PTR_PCB blocked_queue; /* Fila de Processos Bloqueados */
} SEMAPHORE;


/* ************************************************************* */
/* Funcoes Basicas do Nucleo Multiprogramado VISIVEIS AO USUARIO */
/* ************************************************************* */

/* Funcao auxiliar para iniciar a Fila dos Processos PRONTOS para Execucao como vazia */
void far initiateProcessQueue();

/* Criacao de um Processo (Alocar BCP e inserir no fim da Fila) */
void far createProcess(void far (*p_address)(), char p_name[MAX_NAME_PROCESS]);

/* Encerrar um Processo*/
void far terminateProcess();

/* Ativar o Escalador (Processo Principal) */
void far activateScheduler();


/* ************************************* */
/* Funcoes para Utilizacao dos Semaforos */
/* ************************************* */

/* Inicializar o Semaforo */
void far initiateSemaphore(SEMAPHORE *user_semaphore, int size_semaphore);

/* Primitiva P (Down): decrementar o valor do semaforo. Se for zero, o processo eh colocado na Fila de Bloqueados */
void far downSemaphore(SEMAPHORE *user_semaphore);

/* Primitiva V (Up): colocar o processo como ativo se o semaforo estiver nulo e tiver algum processo na Fila. Do contrario, decrementa o valor do semaforo */
void far upSemaphore(SEMAPHORE *user_semaphore);

