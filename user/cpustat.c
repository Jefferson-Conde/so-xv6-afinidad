// cpustat [pid]: tabla de planificación de los procesos: estado, prioridad base
// y efectiva, afinidad, último CPU, migraciones y ticks consumidos en cada CPU.
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

static struct pstat tab[NPROC];
static char *states[] = {"unused", "used", "sleep", "runble", "run", "zombie"};

static int
digits(int v)
{
  int d = 1;

  if (v < 0) {
    d++;
    v = -v;
  }
  while (v >= 10) {
    d++;
    v /= 10;
  }
  return d;
}

// Imprime un entero alineado a la derecha en un campo de ancho w.
static void
num(int v, int w)
{
  int i;

  for (i = digits(v); i < w; i++)
    printf(" ");
  printf("%d", v);
}

// Imprime un texto alineado a la izquierda en un campo de ancho w.
static void
str(char *s, int w)
{
  int i;

  printf("%s", s);
  for (i = strlen(s); i < w; i++)
    printf(" ");
}

int
main(int argc, char **argv)
{
  int n, i, c, only = -1;
  uint online;

  if (argc == 2)
    only = atoi(argv[1]);
  n = getptable(tab, NPROC);
  if (n <= 0) {
    fprintf(2, "cpustat: getptable fallo\n");
    exit(1);
  }
  online = tab[0].online;

  printf("PID NOMBRE     ESTADO PRIO EFEC AFIN  ULT MIGR |");
  for (c = 0; c < NCPU; c++)
    if (online & (1U << c))
      printf(" CPU%d", c);
  printf("   (ticks de reloj corridos en cada CPU)\n");

  for (i = 0; i < n; i++) {
    if (only >= 0 && tab[i].pid != only)
      continue;
    num(tab[i].pid, 3);
    printf(" ");
    str(tab[i].name, 10);
    printf(" ");
    str(states[tab[i].state], 6);
    printf(" ");
    num(tab[i].priority, 4);
    printf(" ");
    num(tab[i].eff_priority, 4);
    printf(" ");
    printf("0x%x", tab[i].affinity & online);
    printf(" ");
    num(tab[i].last_cpu, 3);
    printf(" ");
    num(tab[i].migrations, 4);
    printf(" |");
    for (c = 0; c < NCPU; c++)
      if (online & (1U << c)) {
        printf(" ");
        num(tab[i].ticks_cpu[c], 4);
      }
    printf("\n");
  }
  exit(0);
}
