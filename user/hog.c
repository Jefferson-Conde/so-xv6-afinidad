// hog <ticks>: consume CPU durante <ticks> ticks de reloj y, al terminar,
// muestra en qué CPUs corrió. Sirve para generar carga en las demostraciones:
//   taskset -c 0 hog 30 &     (fijo al CPU 0)
//   hog 30 &                   (sin afinidad: el planificador decide)
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

static struct pstat tab[NPROC];
static volatile uint64 sink;

int
main(int argc, char **argv)
{
  int t, start, n, i, c, me;

  if (argc != 2) {
    fprintf(2, "uso: hog <ticks>\n");
    exit(1);
  }
  t = atoi(argv[1]);
  start = uptime();
  while (uptime() - start < t)
    for (i = 0; i < 2000; i++)
      sink += i;

  me = getpid();
  n = getptable(tab, NPROC);
  for (i = 0; i < n; i++) {
    if (tab[i].pid != me)
      continue;
    printf("hog %d: prio=%d aff=0x%x migraciones=%d ticks por CPU:", me,
           tab[i].priority, tab[i].affinity & tab[i].online,
           tab[i].migrations);
    for (c = 0; c < NCPU; c++)
      if (tab[i].online & (1U << c))
        printf(" CPU%d=%d", c, tab[i].ticks_cpu[c]);
    printf("\n");
  }
  exit(0);
}
