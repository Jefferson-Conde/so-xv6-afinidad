# documento_TrabajoFinal

Documentación del Trabajo Final de **Sistemas Operativos (IS-380)**, UNSCH, 2026-II.
Tema: *Afinidad de CPU y estadísticas por núcleo sobre un planificador con prioridades y aging en xv6 multinúcleo*.

## Contenido

| Elemento | Archivo o carpeta |
|---|---|
| Trabajo monográfico completo (formato APA 7) | `Informe_xv6_afinidad_APA7.docx` y `Informe_xv6_afinidad_APA7.pdf` |
| Presentación en PowerPoint | `Presentacion_xv6_afinidad.pptx` |
| Evidencias (capturas de compilación, ejecución, pruebas y errores corregidos) | `evidencias/` |
| Documentación complementaria | `documentacion_complementaria/` |

`documentacion_complementaria/` contiene:

- `guia_reproduccion.md`: pasos para compilar, ejecutar y repetir las pruebas, y cómo reconstruir el proyecto desde un xv6 limpio.
- `parches/`: los cuatro parches (`paso1` a `paso4`) y `todo-junto.patch`, aplicables sobre el commit base `9e3161a`.
- `publicacion_linkedin.md`: texto de la publicación del proyecto.

El código fuente completo y el `README.md` con las instrucciones de instalación, compilación y ejecución están en la raíz del repositorio.

## Evidencias y figuras del informe

| Figura del informe | Archivo en `evidencias/` | Qué muestra |
|---|---|---|
| Figura 2 | `01-compilacion.png` | Compilación y arranque de xv6 con cuatro CPUs |
| Figura 3 | `11-github.png`, `11b-commits.png` | Repositorio y lista de commits |
| Figura 4 | `02-cpustat.png` | Tabla de procesos con `cpustat` |
| Figura 5 | `03-taskset.png` | Proceso fijado a la CPU 1 con `taskset` |
| Figura 6 | `04-demo-carga.png` | Carga mixta: dos procesos fijos a la CPU 0 y uno libre |
| Figuras 7 a 14 | `T1.png` a `T8.png` | Una captura por cada caso de prueba (entrada, esperado, obtenido, PASS) |
| Figura 15 | `09-resumen.png` | Ejecución completa de la batería: 8/8 pruebas PASS |
| Figura 16 | `10-error-corregido.png` | Planificador previo frente al planificador nuevo (bloqueo corregido) |

## Errores encontrados y corregidos

La Tabla 7 del informe documenta tres errores: el riesgo de deadlock del planificador previo (E1), un criterio incorrecto
en la prueba T7 sobre las migraciones de procesos fijados (E2) y una máscara de afinidad mal mostrada por `hog` (E3).
