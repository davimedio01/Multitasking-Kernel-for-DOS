/* Implementation of some processes, with simple printf function, 
for testing the Kernel Scheduler */

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_TIME 10000

void far process1(){
  int i = 0;
  while(i < MAX_TIME){
    printf("1");
    i++;
  }

  /* Terminando o Processo...*/
  terminateProcess();
}

void far process2(){
  int i = 0;
  while(i < MAX_TIME){
    printf("2");
    i++;
  }

  /* Terminando o Processo...*/
  terminateProcess();
}

void far process3(){
  int i = 0;
  while(i < MAX_TIME){
    printf("3");
    i++;
  }

  /* Terminando o Processo...*/
  terminateProcess();
}

void far process4(){
  int i = 0;
  while(i < MAX_TIME){
    printf("4");
    i++;
  }

  /* Terminando o Processo...*/
  terminateProcess();
}

void far process5(){
  int i = 0;
  while(i < MAX_TIME){
    printf("5");
    i++;
  }

  /* Terminando o Processo...*/
  terminateProcess();
}

main(){
  /* Iniciando a Fila de Processos como Vazia */
  initiateProcessQueue();

  /* Criacao dos Processos */
  createProcess(process1, "P1");
  createProcess(process2, "P2");
  createProcess(process3, "P3");
  createProcess(process4, "P4");
  createProcess(process5, "P5");

  /* Transferindo o controle para o Escalonador */
  activateScheduler();
}