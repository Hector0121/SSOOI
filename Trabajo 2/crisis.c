#include <stdio.h> // sprintf (escribir_pid)
#include <stdlib.h> // exit (morir)
#include <string.h> // strlen (escribir_pid, morir)
#include <unistd.h> // write, read, lseek, lockf, getpid, close, unlink
#include <signal.h> // kill, sigset_t, sigemptyset, sigaddset, sigprocmask, SIGINT, SIGTERM, SIGCHLD
#include <sys/types.h> // pid_t
#include <fcntl.h> // para el fichero
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
        write(2,"Error, no me has pasado el numero de procesos, abortando programa.\n",
             strlen("Error, no me has pasado el numero de procesos, abortando programa.\n"));
        return -1;
    }
    if(argc >= 4){
        write (2,"Error, me has pasado argumentos demas.\n",strlen("Error, me has pasado argumentos demas.\n"));
        return -2;
    }
    int proc_max=atoi(argv[1]);// pasamos de cadena de texto a int  por eso el atoi
    if(argc == 2){
        if(proc_max < 3 || proc_max > 34){
            write (2,"Error el numero de procesos no es valido (rango entre 3-34).\n", strlen("Error el numero de procesos no es valido (rango entre 3-34).\n"));
            return -3;
        }
        return modo=0; // modo 0 significa q es velocidad norma asi  no estamos comparando con palabras
    }
    if(argc == 3){
        if(proc_max < 3 || proc_max > 34){
            write(2,"Error el numero de procesos no es valido (rango entre 3-34).\n", strlen("Error el numero de procesos no es valido (rango entre 3-34).\n"));
            return -4;
        }
        
        if((strcmp(argv[2], "normal") !=0)  && (strcmp(argv[2],"veloz") !=0)){
            write(2,"Error has introducido un nombre de velocidad incorrecta prueba con normal o veloz.\n",strlen("Error has introducido un nombre de velocidad incorrecta prueba con normal o veloz.\n"));
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
void crear_filial(void)
{
    sigset_t antes;
    pid_t pid;
   bloquear_senales(&antes); // bloqueamos las señales por precaucion y evitar problemas
   int result =sumar_vivos(1);// sumamos 1 para comprobar cuanto proceos hay y si cabe reserva  un hueco en el fichero
   if(result == 0){
    //restaurmos las señales
    sigprocmask(SIG_SETMASK,&antes,NULL);
    
   }
   // si hay espacio
   if(result == 1){
    pid = fork();
    if(pid == -1){
        // un fallo
        sumar_vivos(-1);// eliminamos el hueco q habiamos guardado
    }
    if(pid == 0){
        // somos el hijo
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

        hijos[nhijos] = pid;// almaceno el pid de todos los hijos del padre
        nhijos++;
    }

   }
   sigprocmask(SIG_SETMASK,&antes,NULL);
   
}
int sueno(){
    return (1+rand() % 4);
}

int main(int argc, char *argv[]){
    //argc guarda un numero con los argumentos q le pasamos
    //--Control de argum---
    int modo=control_arg(argc, argv);
    
    if(modo <0){
        return -1;
    }
    // Si has llegado hasta aqui has pasado todas las comprobaciones
    int proc_max=atoi(argv[1]);
    // ahora ya tenemos el  mdod del proceso y el numero de proceso max

    //--Fichero---(CORECTO)
    //vamos a  la creacion del fichero y almacenamos el numero de proceso max y  1 q es el numero del proceso del padre
    int num_proc_activos=1;
    fd = open ("crisis.dat", O_RDWR | O_CREAT | O_TRUNC, 0600);
        if(fd == -1){
            write(2, "Error en el fichero.\n", strlen("Error en el fichero.\n"));
            return -2;
        }
    
    // Vmos a crear un vector para introducri los datos a la vez en el fichero !! recordar en la lectura necesitaremos un vector para leerlo
    // ojo el orden,  porc max, y porc activos
    int datos[2]={proc_max,num_proc_activos};
    write(fd, datos, sizeof(datos));

    //---Arrancamos la mtriz
        soy_matriz = 1;
        nhijos=0;
    //---Crear ficha
    struct sigaction sa;
   sa.sa_flags=0;
   sigemptyset(&sa.sa_mask);
   //Añadimos señales a la lista de mascara 
   sigaddset(&sa.sa_mask,SIGINT);
   sigaddset(&sa.sa_mask,SIGTERM);
   sigaddset(&sa.sa_mask,SIGCHLD);

   sa.sa_handler= morir; // funcion de Andres
   sigaction(SIGINT ,&sa, NULL);
   /*entrega la ficha para Ctrl+C. Los tres parámetros son:
    La señal a la que se aplica (SIGINT).
    La dirección de tu ficha (&sa).
    Dónde guardar la configuración anterior. Como no la necesitas, NULL.
    */
   sigaction(SIGTERM, &sa, NULL);// lo mismo pero para SIGTERM

   // cuando pulsemos Ctr C el la matrz ejecutara morir, mata a todos sus hijos y se envia el mensaje
    // cuando el programa reciva igterm del padre , ejecutara morir y este se lo pasara a sus hijos
    sa.sa_handler  = recoger_hijos;
    sigaction(SIGCHLD, &sa, NULL);
        /*Cuando un hijo muere, queda como "zombi" hasta que su padre lo recoge con waitpid.
         Además, el padre tiene que restar uno al contador del fichero y 
         quitar el PID de la lista hijos[]. La función recoger_hijos de tu compañero hace las tres cosas
        , y el aviso SIGCHLD es lo que la pone en marcha.*/
        
        //---Implementar azar
        srand(getpid());
        escribir_pid('V');
      
        // Cracion de hijos y nieto grueso del programa
        while(1){
            int Hypnos = sueno();
            if(modo == 0){
                sleep (Hypnos);
                //primera accion soy el padre y creo una filial dspues paso a valer 0 y comeinzo a crear subproceso
                if(soy_matriz ==1){
                    crear_filial();
                    
                }
                else{
                    // 50% de crear un proceso o eliminarlo y co el todos los subprcesos q tenga
                    int Two_Face = rand()%2;

                    if(Two_Face == 0){
                       crear_filial();
                    
                    }
                    if(Two_Face == 1){
                        raise(SIGTERM);
                    }
                }
            }
            else{
            // es lo mismo q aqui pero como era modo veloz no dormimos
                if(soy_matriz ==1){
                  crear_filial();
               
                }
                else{
                    int Two_Face = rand()%2;

                    if(Two_Face == 0){
                       crear_filial();
                     
                    }
                    if(Two_Face == 1){
                        raise(SIGTERM);
                    }
                }

            }
        }
}
