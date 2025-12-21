Markdown

# SCHSIM – Simulador de planificació de la CPU

## Descripció
**SCHSIM** és un simulador de planificació de processos desenvolupat en llenguatge **C** per a l’assignatura de **Sistemes Operatius**. 

L’objectiu del projecte és implementar i analitzar diferents algorismes de planificació de la CPU, observant el seu comportament i comparant les mètriques de rendiment obtingudes mitjançant una simulació detallada.

---

## Algorismes implementats
El simulador permet executar els següents planificadors:

* **FCFS (First Come First Served)**: Modalitat *non-preemptive*.
* **SJF (Shortest Job First)**: 
    * Modalitat *non-preemptive*.
    * Modalitat *preemptive* (coneguda com **SRT** – Shortest Remaining Time).
* **Round Robin (RR)**: Modalitat *preemptive* (basada en tall de temps o *quantum*).
* **Prioritats**:
    * Modalitat *non-preemptive*.
    * Modalitat *preemptive*.

---

## Característiques del simulador
* Cada procés té **una única ràfega de CPU**.
* No es contempla la suspensió de processos.
* El temps de **canvi de context** es considera nul.
* Només es simula un **únic processador**.
* Càlcul automàtic de mètriques de rendiment al final de cada execució.

---

## Estructura del projecte
```text
.
├── main.c           # Punt d'entrada del programa i gestió d'arguments
├── scheduler.c      # Lògica dels algorismes de planificació
├── scheduler.h      # Definicions i interfícies del planificador
├── process.c        # Gestió de l'estructura i dades dels processos
├── process.h        # Definició del TDA Process
├── queue.c          # Implementació de la cua de processos (Ready Queue)
├── queue.h          # Definició del TDA Queue
├── process.csv      # Fitxer de dades d'entrada
├── Makefile         # Automatització de la compilació
└── README.md        # Documentació del projecte

```


## Compilació i Execució

### Compilació
Per compilar el projecte i generar l'executable `main`, executa:

```bash
make
```


## Execució
S'ha d'especificar l'algorisme, la modalitat i el fitxer de dades:

```bash

./main -a <algorisme> -m <modalitat> -f <fitxer.csv>

```

```bash

Exemples d'ús:

FCFS: ./main -a fcfs -m nonpreemptive -f process.csv

SJF (Preemptive): ./main -a sjf -m preemptive -f process.csv

Round Robin: ./main -a rr -m preemptive -f process.csv

```

## Sortida del programa
1. SIMULATION (Diagrama de Gantt)
Es mostra una graella temporal on:

Files: Processos.

Columnes: Instants de temps.

Llegenda:

E: El procés està en execució.

F: El procés ha finalitzat.

: El procés està en espera (Ready).

2. METRICS
Es calculen els següents indicadors de rendiment:

Durada total de la simulació.

Percentatge d'ús de la CPU.

Throughput.

Temps mitjà d'espera (Average Waiting Time).

Temps mitjà de resposta (Average Response Time).

Temps mitjà de retorn (Average Turnaround Time).

# Autors
## Projecte realitzat per:

Biel Riba

Pol Serra

Luis Sejas

## Assignatura: Sistemes Operatius