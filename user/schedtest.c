// schedtest [n]: batería de pruebas del planificador (prioridad + aging + afinidad).
//   schedtest      ejecuta las 8 pruebas
//   schedtest 6    ejecuta solo la prueba 6
// Requiere xv6 con 4 o más CPUs:  make qemu CPUS=4
// Cada prueba imprime: entrada, resultado esperado, resultado obtenido y PASS/FAIL.
#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/pstat.h"
#include "user/user.h"

static struct pstat tab[NPROC];
static uint online;     // máscara de CPUs encendidos
static int total, passed;
static volatile uint64 sink;

// ---------------------------------------------------------------- utilidades

// Trabajo de CPU puro (sin llamadas al sistema).
static void
spin(int n)
{
  int i;

  for (i = 0; i < n; i++)
    sink += i;
}

// Copia en *out la información de planificación del proceso pid.
static int
statof(int pid, struct pstat *out)
{
  int n = getptable(tab, NPROC), i;

  for (i = 0; i < n; i++) {
    if (tab[i].pid == pid) {
      *out = tab[i];
      return 0;
    }
  }
  return -1;
}

// Lee exactamente n bytes de un pipe (o menos si todos los escritores cerraron).
static void
readn(int fd, void *buf, int n)
{
  char *b = buf;
  int got = 0, r;

  while (got < n) {
    r = read(fd, b + got, n - got);
    if (r <= 0)
      break;
    got += r;
  }
}

// Durante `ticks` ticks anota en qué CPU se ejecuta el proceso (getcpu).
static void
sample(int ticks, int *cnt)
{
  int t0 = uptime(), c;

  for (c = 0; c < NCPU; c++)
    cnt[c] = 0;
  while (uptime() - t0 < ticks) {
    spin(2000);
    cnt[getcpu()]++;
  }
}

static void
head(int n, char *title, char *input, char *expected)
{
  printf("\n[T%d] %s\n", n, title);
  printf("  entrada : %s\n", input);
  printf("  esperado: %s\n", expected);
}

static void
result(int ok)
{
  total++;
  if (ok)
    passed++;
  printf("  resultado: %s\n", ok ? "PASS" : "FAIL");
}

// ------------------------------------------------------------------- pruebas

// T1: un proceso fijado a un solo CPU nunca corre en otro.
static void
t1(void)
{
  int p[2], cnt[NCPU], c, others = 0;

  head(1, "Afinidad dura a un CPU", "setaffinity(0, CPU1); 15 ticks muestreando getcpu()",
       "todas las muestras en CPU1");
  pipe(p);
  if (fork() == 0) {
    setaffinity(0, 1U << 1);
    sample(15, cnt);
    write(p[1], cnt, sizeof(cnt));
    exit(0);
  }
  close(p[1]);
  readn(p[0], cnt, sizeof(cnt));
  close(p[0]);
  wait(0);
  for (c = 0; c < NCPU; c++)
    if (c != 1)
      others += cnt[c];
  printf("  obtenido: muestras en CPU1=%d, en otros CPUs=%d\n", cnt[1], others);
  result(cnt[1] > 0 && others == 0);
}

// T2: con una máscara de varios CPUs, solo se usan esos CPUs.
static void
t2(void)
{
  int p[2], cnt[NCPU], tot[NCPU], i, c, outside = 0;

  head(2, "Mascara de varios CPUs", "3 procesos con afinidad {CPU1, CPU2}; 15 ticks cada uno",
       "muestras solo en CPU1 y CPU2; ninguna en el resto");
  for (c = 0; c < NCPU; c++)
    tot[c] = 0;
  pipe(p);
  for (i = 0; i < 3; i++) {
    if (fork() == 0) {
      setaffinity(0, (1U << 1) | (1U << 2));
      sample(15, cnt);
      write(p[1], cnt, sizeof(cnt));
      exit(0);
    }
  }
  close(p[1]);
  for (i = 0; i < 3; i++) {
    readn(p[0], cnt, sizeof(cnt));
    for (c = 0; c < NCPU; c++)
      tot[c] += cnt[c];
  }
  close(p[0]);
  for (i = 0; i < 3; i++)
    wait(0);
  for (c = 0; c < NCPU; c++)
    if (c != 1 && c != 2)
      outside += tot[c];
  printf("  obtenido: CPU1=%d CPU2=%d, fuera de la mascara=%d\n", tot[1], tot[2], outside);
  result(tot[1] + tot[2] > 0 && outside == 0);
}

// T3: argumentos inválidos se rechazan; los válidos se aceptan.
static void
t3(void)
{
  int r1, r2, r3, r4, r5, r6, r7, off, c, ok;

  head(3, "Validacion de argumentos",
       "mascara 0, CPU inexistente, pid inexistente, prioridad -1 y 10; luego casos validos",
       "los 5 invalidos devuelven -1; los 2 validos devuelven 0");
  r1 = setaffinity(0, 0);
  off = -1;
  for (c = 0; c < NCPU; c++)
    if (!(online & (1U << c)))
      off = c;
  r2 = (off >= 0) ? setaffinity(0, 1U << off) : -1; // sin CPUs apagados: no aplica
  r3 = setaffinity(30000, 1);
  r4 = setpriority(0, -1);
  r5 = setpriority(0, MAXPRIO + 1);
  r6 = setaffinity(0, online);
  r7 = setpriority(0, DEFPRIO);
  printf("  obtenido: mascara0=%d cpu_inexistente=%d pid_inexistente=%d prio(-1)=%d prio(%d)=%d | validos: %d %d\n",
         r1, r2, r3, r4, MAXPRIO + 1, r5, r6, r7);
  ok = r1 == -1 && r2 == -1 && r3 == -1 && r4 == -1 && r5 == -1 && r6 == 0 && r7 == 0;
  result(ok);
}

// T4: el hijo hereda la prioridad y la afinidad del padre (como en Linux).
static void
t4(void)
{
  int st, bad = 0;

  head(4, "Herencia en fork()", "padre: prioridad 7 y afinidad {CPU2}; luego fork()",
       "el hijo ve prioridad 7, afinidad 0x4 y se ejecuta en CPU2");
  setpriority(0, 7);
  setaffinity(0, 1U << 2);
  if (fork() == 0) {
    struct pstat me;
    int code = 0;

    if (statof(getpid(), &me) < 0)
      exit(8);
    if (me.priority != 7)
      code |= 1;
    if (me.affinity != (1U << 2))
      code |= 2;
    if (getcpu() != 2)
      code |= 4;
    exit(code);
  }
  wait(&st);
  setpriority(0, DEFPRIO);
  setaffinity(0, online);
  if (st & 1)
    bad++;
  if (st & 2)
    bad++;
  if (st & 4)
    bad++;
  printf("  obtenido: codigo de salida del hijo=%d (bit1=prioridad, bit2=afinidad, bit4=cpu)\n", st);
  result(st == 0 && bad == 0);
}

// Calibra cuántas llamadas a spin(1000) caben en un tick (con el sistema en reposo).
static int
calibrate(void)
{
  int t = uptime(), t0, it = 0;

  while (uptime() == t)
    ;
  t0 = uptime();
  while (uptime() - t0 < 3) {
    spin(1000);
    it++;
  }
  return it / 3;
}

// Lanza n procesos que hacen `work` iteraciones de spin(1000); el proceso i
// queda fijado a masks[i] (0 = sin afinidad). Devuelve los ticks hasta que
// terminan todos.
static int
run_load(int n, uint *masks, int work)
{
  int i, k, t0 = uptime();

  for (i = 0; i < n; i++) {
    if (fork() == 0) {
      if (masks[i])
        setaffinity(0, masks[i]);
      for (k = 0; k < work; k++)
        spin(1000);
      exit(0);
    }
  }
  for (i = 0; i < n; i++)
    wait(0);
  return uptime() - t0;
}

// T5: la misma carga tarda ~4 veces mas si todo se concentra en un CPU.
static void
t5(void)
{
  uint conc[4], rep[4];
  int i, rate, work, ta, tb;

  head(5, "Carga concentrada vs repartida",
       "4 procesos CPU-bound iguales: (A) todos fijos al CPU0, (B) uno por CPU",
       "A tarda al menos el doble que B (en teoria ~4x)");
  for (i = 0; i < 4; i++) {
    conc[i] = 1U << 0;
    rep[i] = 1U << i;
  }
  rate = calibrate();
  work = rate * 20; // ~20 ticks de CPU por proceso
  ta = run_load(4, conc, work);
  tb = run_load(4, rep, work);
  printf("  obtenido: A (concentrada)=%d ticks, B (repartida)=%d ticks\n", ta, tb);
  result(tb > 0 && ta >= 2 * tb);
}

// Devuelve el tick (relativo al inicio) en que un proceso de prioridad 0 logra
// ejecutarse en el CPU0 mientras otro de prioridad MAXPRIO lo ocupa 60 ticks.
static int
starve_run(int aging)
{
  int ready[2], res[2], d = -1, t0;
  char b = 'r';

  pipe(ready);
  pipe(res);
  setaging(aging);
  setaffinity(0, 1U << 1); // el padre en CPU1: no compite con los hijos
  t0 = uptime();
  if (fork() == 0) { // H: prioridad alta, ocupa el CPU0 hasta t0+60
    setpriority(0, MAXPRIO);
    setaffinity(0, 1U << 0);
    write(ready[1], &b, 1);
    while (uptime() - t0 < 60)
      spin(2000);
    exit(0);
  }
  read(ready[0], &b, 1); // H ya esta en el CPU0
  if (fork() == 0) { // L: prioridad 0; anota cuando consigue ejecutarse en CPU0
    setpriority(0, 0);
    setaffinity(0, 1U << 0);
    d = uptime() - t0;
    write(res[1], &d, sizeof(d));
    exit(0);
  }
  close(res[1]);
  readn(res[0], &d, sizeof(d));
  close(res[0]);
  close(ready[0]);
  close(ready[1]);
  wait(0);
  wait(0);
  setaging(1);
  setaffinity(0, online);
  return d;
}

// T6: sin aging un proceso de baja prioridad sufre inanicion; con aging no.
static void
t6(void)
{
  int off, on;

  head(6, "Aging evita la inanicion",
       "H(prio 9) ocupa el CPU0 60 ticks; L(prio 0) espera en CPU0. Se mide cuando corre L",
       "sin aging L corre >= 50 ticks (tras H); con aging L corre antes de 40 ticks");
  off = starve_run(0);
  on = starve_run(1);
  printf("  obtenido: L empieza en el tick %d sin aging y en el tick %d con aging\n", off, on);
  result(off >= 50 && on < 40 && on < off);
}

// T7: con afinidad dura no hay migraciones; sin afinidad si.
static void
t7(void)
{
  int p[2], i, m, mu = 0, mp = 0;
  struct pstat me;

  head(7, "Migraciones entre CPUs",
       "6 procesos CPU-bound 25 ticks sobre 4 CPUs: sin afinidad y fijos (i mod 4)",
       "sin afinidad migraciones > 0; con afinidad dura migraciones = 0 (tras fijar)");
  for (int pinned = 0; pinned < 2; pinned++) {
    pipe(p);
    for (i = 0; i < 6; i++) {
      if (fork() == 0) {
        int t0 = uptime(), base = -1;

        // Al fijarse, el proceso puede ceder su CPU actual y reaparecer en la
        // CPU pedida: esa unica migracion previa no cuenta. Se mide desde aqui.
        if (pinned)
          setaffinity(0, 1U << (i % 4));
        if (statof(getpid(), &me) == 0)
          base = me.migrations;
        while (uptime() - t0 < 25)
          spin(2000);
        m = (base >= 0 && statof(getpid(), &me) == 0) ? me.migrations - base : -1;
        write(p[1], &m, sizeof(m));
        exit(0);
      }
    }
    close(p[1]);
    for (i = 0; i < 6; i++) {
      readn(p[0], &m, sizeof(m));
      if (pinned)
        mp += m;
      else
        mu += m;
    }
    close(p[0]);
    for (i = 0; i < 6; i++)
      wait(0);
  }
  printf("  obtenido: migraciones sin afinidad=%d, con afinidad dura=%d\n", mu, mp);
  result(mu > 0 && mp == 0);
}

// T8: estres del planificador con 4 CPUs (el diseño anterior podia congelarse
// por un deadlock entre locks de procesos). Si el sistema se cuelga, esta prueba
// no termina.
static void
t8(void)
{
  uint seed = 12345, mask;
  int round, i, st, fails = 0, t0 = uptime(), done = 0;

  head(8, "Estres del planificador (sin deadlock)",
       "3 rondas x 16 procesos con prioridad y afinidad aleatorias, algunos duermen",
       "los 48 procesos terminan con codigo 0 y el sistema no se congela");
  for (round = 0; round < 3; round++) {
    for (i = 0; i < 16; i++) {
      seed = seed * 1103515245 + 12345;
      mask = 1 + ((seed >> 16) % 15); // 1..15: algun CPU de 0..3
      if (fork() == 0) {
        int t1 = uptime();

        setpriority(0, (seed >> 8) % (MAXPRIO + 1));
        if (setaffinity(0, mask) < 0)
          exit(1);
        if (i % 4 == 0)
          pause(1); // ejercita sleep/wakeup
        while (uptime() - t1 < 2)
          spin(2000);
        exit(0);
      }
    }
    for (i = 0; i < 16; i++) {
      wait(&st);
      done++;
      if (st != 0)
        fails++;
    }
  }
  printf("  obtenido: %d procesos terminados, %d con error, %d ticks\n", done, fails,
         uptime() - t0);
  result(done == 48 && fails == 0);
}

int
main(int argc, char **argv)
{
  struct pstat me;
  int which = 0, c, ncpu = 0;

  if (statof(getpid(), &me) < 0) {
    fprintf(2, "schedtest: getptable fallo\n");
    exit(1);
  }
  online = me.online;
  for (c = 0; c < NCPU; c++)
    if (online & (1U << c))
      ncpu++;
  if (ncpu < 4) {
    fprintf(2, "schedtest: se requieren 4 CPUs o mas (ejecute: make qemu CPUS=4)\n");
    exit(1);
  }
  if (argc > 1)
    which = atoi(argv[1]);

  printf("== schedtest: CPUs encendidos=%d (mascara 0x%x), AGING_TICKS=%d, MAXPRIO=%d ==\n",
         ncpu, online, AGING_TICKS, MAXPRIO);
  if (which == 0 || which == 1)
    t1();
  if (which == 0 || which == 2)
    t2();
  if (which == 0 || which == 3)
    t3();
  if (which == 0 || which == 4)
    t4();
  if (which == 0 || which == 5)
    t5();
  if (which == 0 || which == 6)
    t6();
  if (which == 0 || which == 7)
    t7();
  if (which == 0 || which == 8)
    t8();
  printf("\n== Resultado: %d/%d pruebas PASS ==\n", passed, total);
  exit(passed == total ? 0 : 1);
}
