#include <stdio.h> // sprintf (escribir_pid)
#include <stdlib.h> // exit (morir)
#include <string.h> // strlen (escribir_pid, morir)
#include <unistd.h> // write, read, lseek, lockf, getpid, close, unlink
#include <signal.h> // kill, sigset_t, sigemptyset, sigaddset, sigprocmask, SIGINT, SIGTERM, SIGCHLD
#include <sys/types.h> // pid_t
#include <sys/wait.h> // waitpid, WNOHANG
#define MAX_PROCS 34 // tamaño del array hijos[]
#define FICHERO "crisis.dat" // lo usa unlink en morir
#define FINAL "Programa acabado correctamente\n" // lo escribimos en la matriz en morir

static int fd;
static int soy_matriz;
static pid_t hijos[MAX_PROCS];
static int nhijos;

static void escribir_pid(char letra)
{
    char texto[32];

    sprintf(texto, "%c(%d)\n", letra, (int) getpid());
    write(1, texto, strlen(texto));
}

static int sumar_vivos(int cambio)
{
    int datos[2];
    int hecho = 0;

    lseek(fd, 0, SEEK_SET);
    lockf(fd, F_LOCK, 0);
    read(fd, datos, sizeof(datos));
    if (datos[1] + cambio <= datos[0]) {
        datos[1] = datos[1] + cambio;
        lseek(fd, 0, SEEK_SET);
        write(fd, datos, sizeof(datos));
        hecho = 1;
    }
    lseek(fd, 0, SEEK_SET);
    lockf(fd, F_ULOCK, 0);
    return hecho;
}

static void quitar_hijo(pid_t pid)
{
    int i;

    for (i = 0; i < nhijos; i++) {
        if (hijos[i] == pid) {
            hijos[i] = hijos[nhijos - 1];
            nhijos--;
            return;
        }
    }
}

static void recoger_hijos(int sig)
{
    pid_t pid;

    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        quitar_hijo(pid);
        sumar_vivos(-1);
    }
}

static void morir(int sig)
{
    int i;

    for (i = 0; i < nhijos; i++)
        kill(hijos[i], SIGTERM);
    for (i = 0; i < nhijos; i++) {
        waitpid(hijos[i], NULL, 0);
        sumar_vivos(-1);
    }
    if (soy_matriz) {
        close(fd);
        unlink(FICHERO);
        write(1, FINAL, strlen(FINAL));
    } else {
        escribir_pid('M');
    }
    exit(0);
}

static void bloquear_senales(sigset_t *antes)
{
    sigset_t senales;

    sigemptyset(&senales);
    sigaddset(&senales, SIGINT);
    sigaddset(&senales, SIGTERM);
    sigaddset(&senales, SIGCHLD);
    sigprocmask(SIG_BLOCK, &senales, antes);
}

//Controloacion de la velocidad y de los argumentos:(CORRECTO)
int control_arg(int argc, char *argv[]){
    char modo;
    if(argc == 1){
        fprintf(stderr,"Error, no me has pasado el numero de porcesoso, abortando programa.\n");
        return -1;
    }
    if(argc >= 4){
        fprintf(stderr,"Error, me has pasado argumentos demas.\n");
        return -2;
    }
    int proc_max=atoi(argv[1]);// pasamos de cadena de texto a int  por eso el atoi
    if(argc == 2){
        if(proc_max < 3 || proc_max > 34){
            fprintf(stderr,"Error el numero de porcesos no es valido (rango entre 3-34).\n");
            return -3;
        }
        return modo=0; // modo 0 significa q es velocidad norma asi  no estamos comparando con palabras
    }
    if(argc == 3){
        if(proc_max < 3 || proc_max > 34){
            fprintf(stderr,"Error el numero de porcesos no es valido (rango entre 3-34).\n");
            return -4;
        }
        
        if((strcmp(argv[2], "normal") !=0)  && (strcmp(argv[2],"veloz") !=0)){
            fprintf(stderr,"Error has introducido un nombre de velocidad incorrecta pureba con normal o veloz.\n");
            return -5;
        }
        // Comprobar q modo es
        if((strcmp(argv[2],"normal") == 0)){
            modo=0;// modo = 1 velocidad veloz
        }else{
            modo=1;
        }
    }
    return modo;
    
}

int main(int argc, char *argv[]){
    //argc guarda un numero con los argumentos q le pasamos
    
    int modo=control_arg(argc, argv);
    if(modo <0){
        return -1;
    }
    // Si has llegado hasta aqui has pasado todas las comprobaciones
    int proc_max=atoi(argv[1]);
    // ahora ya tenemos el  mdod del proceso y el numero de proceso max

    
    
    
    


}
