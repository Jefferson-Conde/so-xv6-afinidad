// Información de planificación de un proceso, devuelta por getptable().
// Se comparte entre el kernel y los programas de usuario (taskset, cpustat...).
// Requiere haber incluido types.h y param.h antes.
struct pstat {
  int pid;
  int state;            // 0=UNUSED 1=USED 2=SLEEPING 3=RUNNABLE 4=RUNNING 5=ZOMBIE
  int priority;         // prioridad base
  int eff_priority;     // prioridad efectiva (con aging)
  uint affinity;        // máscara de CPUs permitidos
  int last_cpu;         // último CPU donde corrió (-1 = nunca)
  int migrations;       // cambios de CPU entre ejecuciones consecutivas
  uint online;          // máscara de CPUs encendidos en el sistema
  uint ticks_cpu[NCPU]; // ticks de reloj corridos en cada CPU
  char name[16];
};
