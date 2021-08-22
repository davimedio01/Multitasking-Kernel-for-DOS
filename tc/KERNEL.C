#include <system.h>

#define MAX_NAME_PROCESS 35 /* Nome (ID) maximo de um processo */
#define INTERRUPT_BIT    8  /* Numero do bit de interrupcao = TIMER (bit 8) */

/* ************************************************************* */
/* Definicao dos Registradores do Intel_8086 para Regiao Critica */
/* ************************************************************* */
typedef struct registers_8086{
  unsigned bx1, es1; /* Representa os registradores da flag de servicos do DOS */
}REG_8086;

/* ******************************************************* */
/* Definicao da Regiao Critica, com base nos Registradores */
/* ******************************************************* */

/* Uso de uma "union" para juntar ambas as partes, utilizando um ponteiro para indicar o valor */
typedef union critical_region{
  REG_8086 reg;     /* Representa o resultado dos registradores BX e ES (flag de servicos do DOS) */
  char far *value_reg; /* Representa um ponteiro para o resultado da flag de servicos do DOS */
}PTR_CR;

/* Variavel para identificacao na Regiao Critica do DOS */
PTR_CR crDOS;

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

/* ***************** */
/* Variaveis Globais */
/* ***************** */

PTR_PCB head_process_queue; /* Cabeca da Fila dos Processos PRONTOS para Execucao */
PTR_DESC context_scheduler; /* Contexto do Escalonador de Processos */

/* ***************************************** */
/* Funcoes Basicas do Nucleo Multiprogramado */
/* ***************************************** */

/* Voltar o controle do sistema para o DOS */
void far returnDOS(){
  disable();                                   /* Desabilitar as interrupcoes */
  setvect(INTERRUPT_BIT, p_est->int_anterior); /* Retornando o estado das interrupcoes para padrao */
  enable();                                    /* Habilitar as interrupcoes */
  exit(0);                                     /* Retornando para o DOS */
}

/* Funcao auxiliar para iniciar a Fila dos Processos PRONTOS para Execucao como vazia */
void far initiateProcessQueue(){
  head_process_queue = NULL;
}

/* Criacao de um Processo (Alocar BCP e inserir no fim da Fila) */
void far createProcess(void far (*p_address)(), char p_name[MAX_NAME_PROCESS]){
  /* Alocacao do BCP = espaco em memoria (malloc) e informacoes */
  PTR_PCB aloc_process = (PTR_PCB)malloc(sizeof(struct pcb));
  strcpy(aloc_process->name, p_name);
  aloc_process->state = activated;
  aloc_process->context = cria_desc();
  newprocess(p_address, aloc_process->context);
  aloc_process->semaphoreQueue = NULL;
  aloc_process->next_pcb = NULL;

  /* Insercao no fim da Fila de Processos Ativos (prontos para executar) */

  /* Caso a Fila esteja vazia, insere elemento unico */
  if(head_process_queue == NULL){
    aloc_process->next_pcb = aloc_process; /* Apontar para o processo */
    head_process_queue = aloc_process;     /* Atribuir o processo na Fila */
  }
  /* Do contrario, percorre ate o fim da Fila e insere elemento */
  else{
    PTR_PCB aux = head_process_queue; /* Atribuir Fila atual para percorrer */
    while(aux->next_pcb != head_process_queue){
      /* Percorre a Fila de Processos Ativos */
      aux = aux->next_pcb;
    }
    /* Insercao do processo no fim da Fila */
    aux->next_pcb = aloc_process;                /* Apontar para o processo */
    aloc_process->next_pcb = head_process_queue; /* Apontar para o inicio da Fila */
  }
}

/* Encerrar um Processo*/
void far terminateProcess(){
  disable();                            /* Desabilitar as interrupcoes */
  head_process_queue->state = finished; /* Colocar o processo como terminado */
  enable();                             /* Ativar as interrupcoes */
  while(1);                             /* Gastar o resto de fatia de tempo do processo */
}

/* Procura e retorna, se existente, o proximo Processo Ativo da Fila */
PTR_PCB returnNextActivated(){
  /* Auxiliar para percorrer a Fila de Processos Ativos */
  PTR_PCB next_activated = head_process_queue->next_pcb; /* Iniciando a partir do proximo elemento */

  /* Percorrendo a Fila de Processos */
  while(next_activated != head_process_queue){
    /* Caso haja algum processo ativo, retorna-o */
    if(next_activated->state == activated){
      return next_activated;
    }
    next_activated = next_activated->next_pcb;
  }

  /* Caso nao haja nenhum processo ativo, retorna NULL */
  return NULL;
}

/* Escalonador de Processos */
void far scheduler(){
  /* Atribuindo valores iniciais nas variaveis globais do Escalonador (p_est) para o iotransfer() */
  p_est->p_origem  = context_scheduler;           /* Origem = Escalonador  */
  p_est->p_destino = head_process_queue->context; /* Destino = head_process_queue->context (Inicio da Fila de Processos) */
  p_est->num_vetor = INTERRUPT_BIT;               /* Numero do Bit de Interrupcao = Timer (8)  */

  /* Iniciando variaveis para identificacao na Regiao Critica do DOS */

  /* Carrega o registrador AX com 0x34 */
  _AH=0x34; /* Iniciando o Registrador AH com o End. 0x34 (associado ao End. de um flag de pilha 'servicos' do DOS) */
  _AL=0x00; /* Iniciando o Registrador AL */

  geninterrupt(0x21);  /* Gera uma interrupcao associada ao End. 0x21 (aciona servico inicial do DOS) */

  crDOS.reg.es1 = _ES; /* Salvando registrador ES (associado ao End. do segmento "mais significativo" de 16 bits do flag) */
  crDOS.reg.bx1 = _BX; /* Salvando registrador BX (associado ao End. do deslocamento "menos significativo" de 16 bits do flag) */

  /* Realizando o controle e mudanca de processos */
  while(1){
    iotransfer(); /* Realizando controle por interrupcao de tempo (dar uma fatia ao processo) */
    disable();    /* Desabilitando as interrupcoes para mudanca de processo */

    /*
      Verificando se o processo NAO esta na Regiao Critica, para troca.
      Para tanto, a partir do registradores ES-BX, o valor do flag de servicos eh apontado por "value_reg".
    */

    if(!(*crDOS.value_reg)){
      /* Retornando o proximo processo ativo, caso exista */
      if ((head_process_queue = returnNextActivated()) == NULL){
        /* Se nao existir, retorna ao DOS */
        returnDOS();
      }

      /* Se existir, o processo eh colocado em execucao pelo escalonador */
      p_est->p_destino = head_process_queue->context;
    }

    enable(); /* Habilitando as interrupcoes para dar uma fatia de tempo ao novo processo */
  }
}

/* Ativar o Escalador (Processo Principal) */
void far activateScheduler(){

  /* Criacao dos Descritores */
  PTR_DESC aux_activate = cria_desc();
  context_scheduler = cria_desc();

  /* Inicia o processo do escalador no seu descritor associado */
  newprocess(scheduler, context_scheduler);

  /* Transfere o controle atual para o escalador */
  transfer(aux_activate, context_scheduler);
}

/* ************************************* */
/* Funcoes para Utilizacao dos Semaforos */
/* ************************************* */

/* Inicializar o Semaforo */
void far initiateSemaphore(SEMAPHORE *user_semaphore, int size_semaphore){
  user_semaphore->value = size_semaphore; /* Iniciando o valor com o tamanho dado pelo usuario */
  user_semaphore->blocked_queue = NULL;   /* Iniciando a fila de bloqueados com nulo */
}

/* Primitiva P (Down): decrementar o valor do semaforo. Se for zero, o processo eh colocado na Fila de Bloqueados */
void far downSemaphore(SEMAPHORE *user_semaphore){
  disable(); /* Desabilitando as interrupcoes para manipulacao de variavel compartilhada (semaforos) */

  /* ************************ */
  /* Condicoes da Primitiva P */
  /* ************************ */

  /* Verifica se a regiao critica esta sendo utilizada */
  if(user_semaphore->value > 0){
    /* Decrementa o semaforo e NAO bloqueia o processo */
    user_semaphore->value--;
  }
  else{
    /* Auxiliar de Ponteiro de BCP */
    PTR_PCB p_aux;

    /* Mudando o estado do processo ativo (head_process_queue) para bloqueado */
    head_process_queue->state = blocked;
    
    /* Bloquear o processo corrente e adiciona-lo a fila do semaforo */
      /* Verifica se a fila de bloqueados do semaforo esta vazia */
    if(user_semaphore->blocked_queue == NULL){ 
      /* Se estiver, entao insere o processo */
      user_semaphore->blocked_queue = head_process_queue; /* Insere o primeiro processo na fila */
    }
    else{
      /* Cria BCP auxiliar e utiliza ele para percorrer a fila de processos bloqueados */
      PTR_PCB aux;
      aux = user_semaphore->blocked_queue;

      /* Percorre a fila de processos bloqueados ate o ultimo existente */
      while(aux->semaphoreQueue != NULL){
        aux = aux->semaphoreQueue;
      }

      /* Salva o processo atual no fim da fila de processos bloqueados */
      aux->semaphoreQueue = head_process_queue;
    }

    /* Atribui o semaforo do processo atual com nulo */
    head_process_queue->semaphoreQueue = NULL;
    
    /* Salvar as informacoes do BCP atual (bloqueado) em uma auxiliar */
    p_aux = head_process_queue; 

    /* Procurar o proximo processo ativo, se existir */
    if((head_process_queue = returnNextActivated()) == NULL){
      /* Retornar o controle para o DOS, ja que o processo atual se bloqueou e nao ha ativos... */
        /* Situacao de DEADLOCK!! */
      returnDOS();
    }

    /* Do contrario, transferir o controle da auxiliar->contexto para o novo processo ativo */
    transfer(p_aux->context, head_process_queue->context);
  }

  enable(); /* Habilita as interrupcoes novamente para o Escalonador */
}

/* Primitiva V (Up): colocar o processo como ativo se o semaforo estiver nulo e tiver algum processo na Fila. Do contrario, decrementa o valor do semaforo */
void far upSemaphore(SEMAPHORE *user_semaphore){
  
  disable(); /* Desabilitando as interrupcoes para manipulacao de variavel compartilhada (semaforos) */

  /* ************************ */
  /* Condicoes da Primitiva V */
  /* ************************ */

  /* Verifica se a fila de bloqueados do semaforo esta nula */
  if(user_semaphore->blocked_queue == NULL){
    /* Apenas incrementa na variavel do semaforo, para indicacao de uso de uma regiao critica */
    user_semaphore->value++;
  }
  else{
    /* Auxiliar de Ponteiro de BCP */
    PTR_PCB p_prox;

    /* Recupera o primeiro elemento da Fila de Bloqueados */
    p_prox = user_semaphore->blocked_queue;

    /* Avanca a cabeca da fila para o prpximo processo bloqueado */
    user_semaphore->blocked_queue = p_prox->semaphoreQueue;

    /* Remove o processo, novamente ativo, da fila de bloqueados */
    p_prox->semaphoreQueue = NULL;

    /* Mudar o estado do processo bloqueado para ativo novamente */
    p_prox->state = activated;
  }

  enable(); /* Habilita as interrupcoes novamente para o Escalonador */
}