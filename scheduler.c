#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "process.h"
#include "queue.h"
#include "scheduler.h"

int num_algorithms() {
  return sizeof(algorithmsNames) / sizeof(char *);
}

int num_modalities() {
  return sizeof(modalitiesNames) / sizeof(char *);
}

size_t initFromCSVFile(char* filename, Process** procTable){
    FILE* f = fopen(filename,"r");
    
    size_t procTableSize = 10;
    
    *procTable = malloc(procTableSize * sizeof(Process));
    Process * _procTable = *procTable;

    if(f == NULL){
      perror("initFromCSVFile():::Error Opening File:::");   
      exit(1);             
    }

    char* line = NULL;
    size_t buffer_size = 0;
    size_t nprocs= 0;
    while( getline(&line,&buffer_size,f)!=-1){
        if(line != NULL){
            Process p = initProcessFromTokens(line,";");

            if (nprocs==procTableSize-1){
                procTableSize=procTableSize+procTableSize;
                _procTable=realloc(_procTable, procTableSize * sizeof(Process));
            }

            _procTable[nprocs]=p;

            nprocs++;
        }
    }
   free(line);
   fclose(f);
   return nprocs;
}

size_t getTotalCPU(Process *procTable, size_t nprocs){
    size_t total=0;
    for (int p=0; p<nprocs; p++ ){
        total += (size_t) procTable[p].burst;
    }
    return total;
}

int getCurrentBurst(Process* proc, int current_time){
    int burst = 0;
    for(int t=0; t<current_time; t++){
        if(proc->lifecycle[t] == Running){
            burst++;
        }
    }
    return burst;
}

int run_dispatcher(Process *procTable, size_t nprocs, int algorithm, int modality, int quantum){

    // Ordenamos procesos por llegada
    qsort(procTable, nprocs, sizeof(Process), compareArrival);

    init_queue();

    // Duración segura: sum(burst) + max(arrive_time) + margen
    int max_arrive = 0;
    for (size_t i = 0; i < nprocs; i++){
        if (procTable[i].arrive_time > max_arrive)
            max_arrive = procTable[i].arrive_time;
    }
    size_t duration = getTotalCPU(procTable, nprocs) + max_arrive + 2;

    // Inicializamos lifecycle y métricas
    for (size_t p = 0; p < nprocs; p++ ){
        procTable[p].lifecycle = malloc(duration * sizeof(int));
        for (size_t t = 0; t < duration; t++){
            procTable[p].lifecycle[t] = -1;
        }
        procTable[p].waiting_time  = 0;
        procTable[p].return_time   = 0;
        procTable[p].response_time = -1;   // -1 = aún no ha usado CPU
        procTable[p].completed     = false;
    }

    size_t finished = 0;
    size_t t = 0;

    Process *current = NULL;
    int qleft = quantum;   // quantum restante (solo RR)

    // Bucle principal de simulación
    while (finished < nprocs){

        // 1) llegan procesos a la cola
        for (size_t p = 0; p < nprocs; p++){
            if (procTable[p].arrive_time == (int)t){
                enqueue(&procTable[p]);
            }
        }

        // 2) selección de proceso según algoritmo
        if (algorithm == FCFS){

            // FCFS siempre nonpreemptive
            if (current == NULL){
                current = dequeue();
                if (current != NULL && current->response_time < 0){
                    current->response_time = (int)t - current->arrive_time;
                }
            }

        } else if (algorithm == RR){

            // Round Robin (preemptive con quantum)
            if (current == NULL){
                current = dequeue();
                qleft = quantum; // nuevo turno
                if (current != NULL && current->response_time < 0){
                    current->response_time = (int)t - current->arrive_time;
                }
            }

        } else {
            // continuad por aqui vuestro código (biel y pol)
            // - SJF (nonpreemptive)
            // - SRTF (preemptive)
            // - PRIORITIES (preemptive / nonpreemptive)
            //
            // Para no bloquear, si CPU libre usamos FCFS básico:
            if (current == NULL){
                current = dequeue();
                if (current != NULL && current->response_time < 0){
                    current->response_time = (int)t - current->arrive_time;
                }
            }
        }

        // 3) marcar estados en lifecycle en este tick t
        for (size_t p = 0; p < nprocs; p++){
            Process *P = &procTable[p];

            if (P->completed){
                P->lifecycle[t] = Finished;
            } else if ((int)t < P->arrive_time){
                P->lifecycle[t] = -1; // aún no ha llegado
            } else if (current == P){
                P->lifecycle[t] = Running;
            } else {
                P->lifecycle[t] = Ready;
            }
        }

        // 4) actualizar ejecución / fin / expulsión RR
        if (current != NULL){

            if (algorithm == RR){
                qleft--;  // consumimos quantum
            }

            int executed = getCurrentBurst(current, (int)t + 1);

            // ¿terminó el proceso?
            if (executed >= current->burst){
                current->completed   = true;
                current->return_time = (int)(t + 1) - current->arrive_time;
                finished++;
                current = NULL;
                qleft = quantum;

            } else if (algorithm == RR && qleft == 0){
                // no terminó y se acabó quantum por llo tanto expulsión
                enqueue(current);
                current = NULL;
                qleft = quantum;
            }
        }

        t++;
        if (t >= duration) break; // seguridad
    }

    // 5) calcular waiting_time como num de ticks en Ready
    for (size_t p = 0; p < nprocs; p++){
        int waiting = 0;
        for (size_t k = 0; k < t; k++){
            if (procTable[p].lifecycle[k] == Ready) waiting++;
        }
        procTable[p].waiting_time = waiting;
    }

    // 6) imprimir simulación y métricas
    printSimulation(nprocs, procTable, t);
    printMetrics(t, nprocs, procTable);

    // 7) liberar memoria
    for (size_t p = 0; p < nprocs; p++ ){
        destroyProcess(procTable[p]);
    }

    cleanQueue();
    return EXIT_SUCCESS;
}



void printSimulation(size_t nprocs, Process *procTable, size_t duration){

    printf("%14s","== SIMULATION ");
    for (int t=0; t<duration; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf ("|%4s", "name");
    for(int t=0; t<duration; t++){
        printf ("|%2d", t);
    }
    printf ("|\n");

    for (int p=0; p<nprocs; p++ ){
        Process current = procTable[p];
            printf ("|%4s", current.name);
            for(int t=0; t<duration; t++){
                printf("|%2s",  (current.lifecycle[t]==Running ? "E" : 
                        current.lifecycle[t]==Bloqued ? "B" :   
                        current.lifecycle[t]==Finished ? "F" : " "));
            }
            printf ("|\n");
        
    }


}

void printMetrics(size_t simulationCPUTime, size_t nprocs, Process *procTable ){

    printf("%-14s","== METRICS ");
    for (int t=0; t<simulationCPUTime+1; t++ ){
        printf("%5s","=====");
    }
    printf("\n");

    printf("= Duration: %ld\n", simulationCPUTime );
    printf("= Processes: %ld\n", nprocs );

    size_t baselineCPUTime = getTotalCPU(procTable, nprocs);
    double throughput = (double) nprocs / (double) simulationCPUTime;
    double cpu_usage = (double) simulationCPUTime / (double) baselineCPUTime;

    printf("= CPU (Usage): %lf\n", cpu_usage*100 );
    printf("= Throughput: %lf\n", throughput*100 );

    double averageWaitingTime = 0;
    double averageResponseTime = 0;
    double averageReturnTime = 0;
    double averageReturnTimeN = 0;

    for (int p=0; p<nprocs; p++ ){
            averageWaitingTime += procTable[p].waiting_time;
            averageResponseTime += procTable[p].response_time;
            averageReturnTime += procTable[p].return_time;
            averageReturnTimeN += procTable[p].return_time / (double) procTable[p].burst;
    }


    printf("= averageWaitingTime: %lf\n", (averageWaitingTime/(double) nprocs) );
    printf("= averageResponseTime: %lf\n", (averageResponseTime/(double) nprocs) );
    printf("= averageReturnTimeN: %lf\n", (averageReturnTimeN/(double) nprocs) );
    printf("= averageReturnTime: %lf\n", (averageReturnTime/(double) nprocs) );

}