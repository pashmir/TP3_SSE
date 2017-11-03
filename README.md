# TP3_SSE
Trabajo Práctico 3 Sistemas Embebidos

Solo archivos con código fuente, de Yakindu, y archivos importantes.

## GUÍA PARA INSTALAR EL REPOSITORIO
1. Descargar el archivo con los ejemplos de aplicación: [campus.fi.uba.ar/mod/resource/view.php?id=79379](campus.fi.uba.ar/mod/resource/view.php?id=79379)
2. Abrir el LpcXpresso e importar todos los archivos del ejemplo de aplicación
3. Cerrar LpcXpresso
4. Crear el directorio donde se quiera clonar el repositorio
5. Clonar el repositorio:
```bash
	git clone https://github.com/pashmir/TP3_SSE
```
6. Copiar los archivos del workspace donde importamos el ejemplo de aplicación sobre el workspace del repo
	IMPORTANTE: No sobrescribir ningun archivo!
    
## GUÍA PARA HACER CAMBIOS
1. De preferencia trabajar en un branch para poder hacer un merge con los cambios
		Ejecutar en línea de comandos en la carpeta del repositorio:
		Para crear una rama:
```bash
		git branch MiNuevaRama
```
		Para ver las ramas existentes:
```bash
		git branch list
```
		Para seleccionar una rama:
```bash
		git branch checkout LaRamaQueQuiero
```
	De otra forma se trabajará sobre el master
2. Luego de realizar los cambios deseados sobre los archivos
	Para subir archivos de código fuente, sct y cfg:
```bash
	git add \*.c \*.h \*.sct \*.cfg \*.-sct
```
	Para subir algun otro archivo:
```bash
	git add archivo
```
	Para borrar:
```bash
	git rm archivo
```
3. Se puede ver que cambios hacen los distintos miembros del grupo si se crean remotes:
```bash
	git remote add NombreDelRemote DireccionLocalDelRepoEnLaPcDeMiCompañero
```
	Para ver los cambios:
```bash
	fetch NombreDelRemote
```
	Si ven que se estan pisando en algo se puede ver que hacer
4. Hacer un pull con los cambios de otros antes de subir los tuyos. Esto va hacer un fetch y un merge:
```bash
	git pull . remotes/NombreDelRemote/Rama
```
5. Para subir los archivos crear un commit y hacer push:
```bash
	git commit -m 'Una descripción de lo que se cambió'
	push origin RamaALaQueQuieroSubirTodo
```

Fuente: tutorial de git en la consola:
```bash
git help tutorial
```
