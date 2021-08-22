#include <system.h>
#include <stdio.h>

/* Criacao dos Ponteiros dos Descritores das corotinas associadas */
PTR_DESC dmain, dcorA, dcorB, descala;

void far corotina_A(){
  while(1){
    printf("COROTINA-A");
  }
}

void far corotina_B(){
  while(1){
    printf("COROTINA-B");
  }
}

void far escalador(){
  /* Inicializando variaveis de entrada do iotransfer() */
  p_est->p_origem = descala; /* Origem = Escalador   */
  p_est->p_destino = dcorA;  /* Destino = Corotina A */
  p_est->num_vetor = 8;      /* Timer = reg. 8 (quantum) */

  while(1){
    iotransfer(); /* Realiza a trasnferencia de p_origem para p_destino com base na interrupcao definida acima */
    disable();    /* Desativando as interrupcoes para evitar possiveis problemas de transferencia de destino */
    if(p_est->p_destino == dcorA){
      /* Trocando o destino para a Corotina B */
      p_est->p_destino = dcorB;
    }
    else{
      /* Trocando o destino para a Corotina A */
      p_est->p_destino = dcorA;
    }
    enable();     /* Habilitando as interrupcoes novamente para poder utilizar a do Timer (quantum) */
  }
}

main(){
  /* Inicializando descritores (ponteiros) */
  dmain = cria_desc();
  dcorA = cria_desc();
  dcorB = cria_desc();
  descala = cria_desc();

  /* Criando processos associados as corotinas. A do main ja eh realizada pelo SO. */
  newprocess(corotina_A, dcorA);
  newprocess(corotina_B, dcorB);
  newprocess(escalador, descala);

  /* Transferindo o controle da main para o escalador */
  transfer(dmain, descala);

  printf("\nFim Programa\n");
  system("pause");
}