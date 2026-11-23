// taskset: fija o consulta la afinidad de CPU de un proceso (como taskset de Linux).
//   taskset -c LISTA comando [args...]   ejecuta comando solo en los CPUs de LISTA
//   taskset -p PID LISTA                 cambia la afinidad de un proceso existente
//   taskset -p PID                       muestra la afinidad de un proceso
// LISTA: "0", "0,2", "1-3", "0,2-3"
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

static struct pstat tab[NPROC];

// Convierte "0,2-3" en la máscara 0b1101. Devuelve 0 si la lista es inválida.
static uint
parselist(char *s)
{
  uint mask = 0;
  int a, b;

  while (*s) {
    if (*s < '0' || *s > '9')
      return 0;
    a = 0;
    while (*s >= '0' && *s <= '9') {
      a = a * 10 + (*s - '0');
      s++;
    }
    b = a;
    if (*s == '-') {
      s++;
      if (*s < '0' || *s > '9')
        return 0;
      b = 0;
      while (*s >= '0' && *s <= '9') {
        b = b * 10 + (*s - '0');
        s++;
      }
    }
    if (a > b || b >= NCPU)
      return 0;
    for (; a <= b; a++)
      mask |= 1U << a;
    if (*s == ',')
      s++;
    else if (*s)
      return 0;
  }
  return mask;
}

static void
showaffinity(int pid)
{
  int n = getptable(tab, NPROC), i;

  for (i = 0; i < n; i++) {
    if (tab[i].pid == pid) {
      printf("pid %d (%s): afinidad = 0x%x\n", pid, tab[i].name, tab[i].affinity);
      return;
    }
  }
  fprintf(2, "taskset: no existe el pid %d\n", pid);
  exit(1);
}

int
main(int argc, char **argv)
{
  uint mask;

  if (argc >= 4 && strcmp(argv[1], "-c") == 0) {
    mask = parselist(argv[2]);
    if (mask == 0) {
      fprintf(2, "taskset: lista de CPUs invalida: %s\n", argv[2]);
      exit(1);
    }
    if (setaffinity(0, mask) < 0) {
      fprintf(2, "taskset: setaffinity fallo (CPU apagado o inexistente)\n");
      exit(1);
    }
    // La afinidad se conserva a través de exec(), igual que en Linux.
    exec(argv[3], &argv[3]);
    fprintf(2, "taskset: no pude ejecutar %s\n", argv[3]);
    exit(1);
  }
  if (argc == 3 && strcmp(argv[1], "-p") == 0) {
    showaffinity(atoi(argv[2]));
    exit(0);
  }
  if (argc == 4 && strcmp(argv[1], "-p") == 0) {
    mask = parselist(argv[3]);
    if (mask == 0 || setaffinity(atoi(argv[2]), mask) < 0) {
      fprintf(2, "taskset: no se pudo cambiar la afinidad\n");
      exit(1);
    }
    showaffinity(atoi(argv[2]));
    exit(0);
  }
  fprintf(2, "uso: taskset -c LISTA comando [args...]\n");
  fprintf(2, "     taskset -p PID [LISTA]\n");
  exit(1);
}
