#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/wait.h>
#define MAX_PROCS 34
#define FICHERO "crisis.dat"
#define FINAL "Programa acabado correctamente\n"
#define ERR_FALTA_PROC  "Error: falta el numero de procesos.\n"
#define ERR_SOBRAN "Error: sobran argumentos.\n"
#define ERROR_RANGO  "Error: procesos entre 3 y 34.\n"
#define ERR_VELOC    "Error: velocidad incorrecta.\n" \
                    "prueba con normal o veloz.\n"
#define ERROR_FICH "Error:En el fichero.\n"

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
    escribir_pid('M');
    if (soy_matriz) {
        close(fd);
        unlink(FICHERO);
        write(1, FINAL, strlen(FINAL));
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

int control_arg(int argc, char *argv[]){
    char modo;
    if(argc == 1){
        write(2,ERR_FALTA_PROC,strlen(ERR_FALTA_PROC));
        return -1;
    }
    if(argc >= 4){
        write(2,ERR_SOBRAN,strlen(ERR_SOBRAN));
        return -2;
    }
    int proc_max=atoi(argv[1]);
    if(argc == 2){
        if(proc_max < 3 || proc_max > 34){
            write(2,ERROR_RANGO,strlen(ERROR_RANGO));
            return -3;
        }
        return modo=0;
    }
    if(argc == 3){
        if(proc_max < 3 || proc_max > 34){
            write(2,ERROR_RANGO,strlen(ERROR_RANGO));
            return -4;
        }
        
        if((strcmp(argv[2], "normal") !=0)  && (strcmp(argv[2],"veloz") !=0)){
            write(2,ERR_VELOC,strlen(ERR_VELOC));
            return -5;
        }
        if((strcmp(argv[2],"normal") == 0)){
            modo=0;
        }else{
            modo=1;
        }
    }
    return modo;
    
}
void crear_filial(void)
{
    sigset_t antes;
    pid_t pid;
   bloquear_senales(&antes);
   int result =sumar_vivos(1);
   if(result == 0){
    sigprocmask(SIG_SETMASK,&antes,NULL);
    
   }
   if(result == 1){
    pid = fork();
    if(pid == -1){
        sumar_vivos(-1);
    }
    if(pid == 0){
        soy_matriz = 0;
        nhijos =0;
        struct sigaction saa;
        saa.sa_handler = SIG_IGN;
        saa.sa_flags=0;
        sigemptyset (&saa.sa_mask);
        sigaction(SIGINT,&saa,NULL);
        close(fd);
        fd= open(FICHERO, O_RDWR);
        
        
        srand(getpid());
        escribir_pid('V');

    }
    if(pid > 0){

        hijos[nhijos] = pid;
        nhijos++;
    }

   }
   sigprocmask(SIG_SETMASK,&antes,NULL);
   
}
int sueno(){
    return (1+rand() % 4);
}

int main(int argc, char *argv[]){
    int modo=control_arg(argc, argv);
    
    if(modo <0){
        return -1;
    }
    int proc_max=atoi(argv[1]);

    int num_proc_activos=1;
    fd = open ("crisis.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
        if(fd == -1){
            write(2, ERROR_FICH, strlen(ERROR_FICH));
            return -2;
        }
    
    int datos[2]={proc_max,num_proc_activos};
    write(fd, datos, sizeof(datos));

        soy_matriz = 1;
        nhijos=0;
    struct sigaction sa;
   sa.sa_flags=0;
   sigemptyset(&sa.sa_mask);
   sigaddset(&sa.sa_mask,SIGINT);
   sigaddset(&sa.sa_mask,SIGTERM);
   sigaddset(&sa.sa_mask,SIGCHLD);

   sa.sa_handler= morir;
   sigaction(SIGINT ,&sa, NULL);
   sigaction(SIGTERM, &sa, NULL);

    sa.sa_handler  = recoger_hijos;
    sigaction(SIGCHLD, &sa, NULL);
        
        srand(getpid());
        escribir_pid('V');
      
            if(modo == 0){
                while(1){
                    int Hypnos = sueno();
                    while (Hypnos > 0)
                        Hypnos = sleep(Hypnos);
                    if(soy_matriz ==1){
                        crear_filial();
                        
                    }
                    else{
                        int Two_Face = rand()%2;

                        if(Two_Face == 0){
                        crear_filial();
                        
                        }
                        if(Two_Face == 1){
                            sigset_t antes;
                            bloquear_senales(&antes);
                            morir(SIGTERM);
                        }
                    }
                }
            }
            else{
                while(1){
                    if(soy_matriz ==1){
                    crear_filial();
                
                    }
                    else{
                        int Two_Face = rand()%2;

                        if(Two_Face == 0){
                        crear_filial();
                        
                        }
                        if(Two_Face == 1){
                            sigset_t antes;
                            bloquear_senales(&antes);
                            morir(SIGTERM);
                        }
                    }
                }

            }
        
}
