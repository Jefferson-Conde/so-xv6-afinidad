#define NPROC       64                // maximum number of processes
#define NCPU        8                 // maximum number of CPUs
#define NOFILE      16                // open files per process
#define NFILE       100               // open files per system
#define NINODE      50                // maximum number of active i-nodes
#define NDEV        10                // maximum major device number
#define ROOTDEV     1                 // device number of file system root disk
#define MAXARG      32                // max exec arguments
#define MAXOPBLOCKS 10                // max # of blocks any FS op writes
#define LOGBLOCKS   (MAXOPBLOCKS * 3) // max data blocks in on-disk log
#define NBUF        (MAXOPBLOCKS * 3) // size of disk block cache
#define FSSIZE      4000              // size of file system in blocks (antes 2000; sobra espacio para los programas nuevos)
#define MAXPATH     128               // maximum file path name
#define USERSTACK   1                 // user stack pages

// --- Extensión del planificador: prioridad + aging + afinidad de CPU ---
#define MAXPRIO     9                 // prioridad máxima (mayor número = más prioridad)
#define DEFPRIO     4                 // prioridad por defecto de todo proceso nuevo
#define AGING_TICKS 2                 // ticks esperando en RUNNABLE para subir 1 nivel
#define ALLCPUS     ((1U << NCPU) - 1) // máscara de afinidad con todos los CPUs
