# Publicación de LinkedIn

Cada integrante debe publicar o compartir este contenido desde su propia cuenta. Reemplazar los textos entre corchetes
por las menciones reales de LinkedIn de los compañeros y adjuntar de 3 a 4 capturas de la carpeta `evidencias/`
(sugeridas: `09-resumen.png`, `04-demo-carga.png`, `01-compilacion.png`).

---

**Extendimos el planificador de xv6 con afinidad de CPU, prioridades y aging**

Para el trabajo final de Sistemas Operativos (UNSCH) modificamos xv6-riscv, el sistema operativo didáctico del MIT, para que
funcione mejor en equipos con varios núcleos.

**El problema:** el planificador original no permite elegir en qué núcleo corre un proceso, no tiene prioridades y no
registra qué hizo cada CPU. Además, la versión con prioridades que veníamos usando en el laboratorio tenía riesgo de bloqueo
(deadlock), porque retenía un lock mientras pedía otros.

**La solución:**
- Afinidad de CPU dura (una máscara de núcleos permitidos por proceso).
- Prioridades de 0 a 9 con aging, para que ningún proceso quede esperando para siempre.
- Estadísticas por núcleo: ticks por CPU, última CPU y migraciones.
- Un planificador de dos fases que nunca retiene más de un lock de proceso a la vez.
- 5 llamadas al sistema, 3 herramientas (`taskset`, `cpustat`, `hog`) y 8 pruebas automáticas que pasan en QEMU con 4 CPUs.

**Conocimientos de Sistemas Operativos aplicados:** planificación de la CPU, inanición y aging, multiprocesamiento y afinidad,
sincronización con spin locks y condiciones de deadlock de Coffman. Comparamos el diseño con `taskset` y `sched_setaffinity` de Linux.

Código, informe y evidencias: https://github.com/KengySamir27/so-xv6-afinidad

Equipo: [@Kengy Samir Oré Mischa] [@Jefferson Conde Flores] [@Frank Felix Huicho Solier]
Docente: Ing. Lizbeth Jaico Quispe https://pe.linkedin.com/in/lizbeth-jaico-quispe-19a604170/

#SistemasOperativos #xv6 #GitHub #IngenieriaDeSoftware #UNSCH
