# Opciones de compilación
CFLAGS = -Wall -Wextra -lpthread -g  # -g para habilitar la información de depuración

#Librerías auxiliares
IO = Libs/IO/io.o
FRAME = Libs/Frame/frame.o
SOCKET = Libs/Socket/socket.o
STRING = Libs/String/string.o
FILE = Libs/File/file.o
DIR = Libs/Dir/dir.o
LOAD = Libs/Load/load_config.o
MONITOR = Libs/Monitor/monitor.o
COMM = Libs/Communication/communication.o
FLECK_LINKEDLIST = Libs/LinkedList/fleckLinkedList.o
WORKER_LINKEDLIST = Libs/LinkedList/workerLinkedList.o
SEMAPHORE = Libs/Semaphore/semaphore_v2.o
COMPRESSION = Libs/Compress/so_compression.o

#Modulos de Fleck
FLECK_CMD = Fleck/Modules/CMD/cmd.o
FLECK_EXIT = Fleck/Modules/Exit/exit.o
FLECK_COMM = Fleck/Modules/Communication/communication.o
FLECK_DIST = Fleck/Modules/Distortion/distortion.o

#Modulos de Gotham
GOTHAM_EXIT = Gotham/Modules/Exit/exit.o
GOTHAM_HANDLE = Gotham/Modules/Handle/handle.o
GOTHAM_MANAGE_CLIENT = Gotham/Modules/MC/manage_client.o
GOTHAM_COMM = Gotham/Modules/Communication/communication.o
GOTHAM_SRV = Gotham/Modules/Server/server.o

#Modulos de Worker
WORKER_EXIT = Worker/Modules/Exit/exit.o
WORKER_COMM = Worker/Modules/Communication/communication.o
WORKER_SRV = Worker/Modules/Server/server.o
WORKER_DIST = Worker/Modules/Distortion/distortion.o
WORKER_MANAGE_CLIENT = Worker/Modules/MC/manage_client.o
WORKER_CONTEXT = Worker/Modules/Context/context.o

#Ejecutables
FLECK = Fleck/Fleck.o
GOTHAM = Gotham/Gotham.o
HARLEY = Worker/Harley/Harley.o
ENIGMA = Worker/Enigma/Enigma.o

all: Fleck Gotham Harley Enigma

###############################################Librerías###############################################
# Librería io auxiliar
Libs/IO/io.o: Libs/IO/io.c Libs/IO/io.h
	gcc $(CFLAGS) -c Libs/IO/io.c -o Libs/IO/io.o

# Librería frame auxiliar
Libs/Frame/frame.o: Libs/Frame/frame.c Libs/Frame/frame.h
	gcc $(CFLAGS) -c Libs/Frame/frame.c -o Libs/Frame/frame.o

# Librería socket auxiliar
Libs/Socket/socket.o: Libs/Socket/socket.c Libs/Socket/socket.h
	gcc $(CFLAGS) -c Libs/Socket/socket.c -o Libs/Socket/socket.o

# Librearía string auxiliar
Libs/String/string.o: Libs/String/string.c Libs/String/string.h
	gcc $(CFLAGS) -c Libs/String/string.c -o Libs/String/string.o

# Libreria file auxiliar
Libs/File/file.o: Libs/File/file.c Libs/File/file.h
	gcc $(CFLAGS) -c Libs/File/file.c -o Libs/File/file.o

# Libreria dir auxiliar
Libs/Dir/dir.o: Libs/Dir/dir.c Libs/Dir/dir.h
	gcc $(CFLAGS) -c Libs/Dir/dir.c -o Libs/Dir/dir.o

# Libreria de load_config
Libs/Load/load_config.o: Libs/Load/load_config.c Libs/Load/load_config.h Fleck/typeFleck.h Gotham/typeGotham.h Worker/typeWorker.h
	gcc $(CFLAGS) -c Libs/Load/load_config.c -o	Libs/Load/load_config.o

# Libreria de monitor
Libs/Monitor/monitor.o: Libs/Monitor/monitor.c Libs/Monitor/monitor.h Libs/Structure/typeMonitor.h
	gcc $(CFLAGS) -c Libs/Monitor/monitor.c -o Libs/Monitor/monitor.o

#Libreria de comucacion
Libs/Communicaton/communication.o: Libs/Communication/communication.c Libs/Communication/communication.h
	gcc $(CFLAGS) -c Libs/Communication/communication.c -o Libs/Communication/communication.o

#Libreria de Listas para Fleck
Libs/LinkedList/fleckLinkedList.o: Libs/LinkedList/fleckLinkedList.c Libs/LinkedList/fleckLinkedList.h Libs/LinkedList/types.h
	gcc $(CFLAGS) -c Libs/LinkedList/fleckLinkedList.c -o Libs/LinkedList/fleckLinkedList.o

#Libreria de listas para Worker
Libs/LinkedList/workerLinkedList.o: Libs/LinkedList/workerLinkedList.c Libs/LinkedList/workerLinkedList.h Libs/LinkedList/types.h
	gcc $(CFLAGS) -c Libs/LinkedList/workerLinkedList.c -o Libs/LinkedList/workerLinkedList.o

#Llibreria de semaforos
Libs/Semaphore/semaphore_v2.o: Libs/Semaphore/semaphore_v2.c Libs/Semaphore/semaphore_v2.h
	gcc $(CFLAGS) -c Libs/Semaphore/semaphore_v2.c -o Libs/Semaphore/semaphore_v2.o

#####################################################################################################

#############################################FLECK MODULES###########################################
# Modulo de Cmd
Fleck/Modules/CMD/cmd.o: Fleck/Modules/CMD/cmd.c Fleck/Modules/CMD/cmd.h 
	gcc $(CFLAGS) -c Fleck/Modules/CMD/cmd.c -o Fleck/Modules/CMD/cmd.o

#Modulo de Exit
Fleck/Modules/Exit/exit.o: Fleck/Modules/Exit/exit.c Fleck/Modules/Exit/exit.h Fleck/typeFleck.h Libs/Structure/typeDistort.h
	gcc $(CFLAGS) -c Fleck/Modules/Exit/exit.c -o Fleck/Modules/Exit/exit.o

#Modulo de Communication
Fleck/Modules/Communication/communication.o: Fleck/Modules/Communication/communication.c Fleck/Modules/Communication/communication.h Fleck/typeFleck.h
	gcc $(CFLAGS) -c Fleck/Modules/Communication/communication.c -o Fleck/Modules/Communication/communication.o

#Modulo de Distorsion
Fleck/Modules/Distortion/distortion.o: Fleck/Modules/Distortion/distortion.c Fleck/Modules/Distortion/distortion.h Fleck/typeFleck.h Libs/Structure/typeDistort.h
	gcc $(CFLAGS) -c Fleck/Modules/Distortion/distortion.c -o Fleck/Modules/Distortion/distortion.o

#####################################################################################################

#############################################GOTHAM MODULES###########################################
#Modulo de Exit
Gotham/Modules/Exit/exit.o: Gotham/Modules/Exit/exit.c Gotham/Modules/Exit/exit.h Gotham/typeGotham.h
	gcc $(CFLAGS) -c Gotham/Modules/Exit/exit.c -o Gotham/Modules/Exit/exit.o

#Modulo de Handle
Gotham/Modules/Handle/handle.o: Gotham/Modules/Handle/handle.c Gotham/Modules/Handle/handle.h Gotham/typeGotham.h Libs/LinkedList/fleckLinkedList.h Libs/LinkedList/workerLinkedList.h
	gcc $(CFLAGS) -c Gotham/Modules/Handle/handle.c -o Gotham/Modules/Handle/handle.o

#Modulo de Manage Client
Gotham/Modules/MC/manage_client.o: Gotham/Modules/MC/manage_client.c Gotham/Modules/MC/manage_client.h Gotham/typeGotham.h
	gcc $(CFLAGS) -c Gotham/Modules/MC/manage_client.c -o Gotham/Modules/MC/manage_client.o

#Modulo de communication
Gotham/Modules/Communication/communication.o: Gotham/Modules/Communication/communication.c Gotham/Modules/Communication/communication.h Gotham/typeGotham.h
	gcc $(CFLAGS) -c Gotham/Modules/Communication/communication.c -o Gotham/Modules/Communication/communication.o

#Modulo del servidor
Gotham/Modules/Server/server.o: Gotham/Modules/Server/server.c Gotham/Modules/Server/server.h Gotham/typeGotham.h
	gcc $(CFLAGS) -c Gotham/Modules/Server/server.c -o Gotham/Modules/Server/server.o

#####################################################################################################

#############################################WORKER MODULES##########################################
#Modulo de exit
Worker/Modules/Exit/exit.o: Worker/Modules/Exit/exit.c Worker/Modules/Exit/exit.h Worker/typeWorker.h Libs/Structure/typeDistort.h 
	gcc $(CFLAGS) -c Worker/Modules/Exit/exit.c -o Worker/Modules/Exit/exit.o

#Modulo de communication
Worker/Modules/Communication/communication.o: Worker/Modules/Communication/communication.c Worker/Modules/Communication/communication.h Worker/typeWorker.h Libs/Structure/typeDistort.h
	gcc $(CFLAGS) -c Worker/Modules/Communication/communication.c -o Worker/Modules/Communication/communication.o

#Modulo de servidor
Worker/Modules/Server/server.o: Worker/Modules/Server/server.c Worker/Modules/Server/server.h Worker/typeWorker.h
	gcc $(CFLAGS) -c Worker/Modules/Server/server.c -o Worker/Modules/Server/server.o

#Modulo de distorsion
Worker/Modules/Distortion/distortion.o: Worker/Modules/Distortion/distortion.c Worker/Modules/Distortion/distortion.h Worker/typeWorker.h Libs/Structure/typeDistort.h
	gcc $(CFLAGS) -c Worker/Modules/Distortion/distortion.c -o Worker/Modules/Distortion/distortion.o

#Modulo de manage client
Worker/Modules/MC/manage_client.o: Worker/Modules/MC/manage_client.c Worker/Modules/MC/manage_client.h Worker/typeWorker.h
	gcc $(CFLAGS) -c Worker/Modules/MC/manage_client.c -o Worker/Modules/MC/manage_client.o

#Modulo de Context
Worker/Modules/Context/context.o: Worker/Modules/Context/context.c Worker/Modules/Context/context.h Worker/typeWorker.h Libs/Structure/typeDistort.h
	gcc $(CFLAGS) -c Worker/Modules/Context/context.c -o Worker/Modules/Context/context.o

#####################################################################################################

#############################################EXECUTABLES#############################################
# Ejecutable de Fleck
Fleck:  $(FLECK) $(IO) $(STRING) $(LOAD) $(FILE) $(DIR) $(SOCKET) $(MONITOR) $(FRAME) $(COMM) $(FLECK_CMD) $(FLECK_EXIT) $(FLECK_COMM) $(FLECK_DIST) Fleck/typeFleck.h Libs/Structure/typeDistort.h Libs/Structure/typeMonitor.h
	gcc $(CFLAGS) $(FLECK) $(IO) $(STRING) $(LOAD) $(FILE) $(DIR) $(SOCKET) $(MONITOR) $(FRAME) $(COMM) $(FLECK_CMD) $(FLECK_EXIT) $(FLECK_COMM) $(FLECK_DIST) -o Fleck/Fleck

# Ejecutable de Gotham
Gotham: $(GOTHAM) $(IO) $(STRING) $(LOAD) $(SOCKET) $(FRAME) $(FILE) $(COMM) $(FLECK_LINKEDLIST) $(WORKER_LINKEDLIST) $(GOTHAM_EXIT) $(GOTHAM_HANDLE) $(GOTHAM_MANAGE_CLIENT) $(GOTHAM_SRV) $(GOTHAM_COMM) Gotham/typeGotham.h 
	gcc $(CFLAGS) $(GOTHAM) $(IO) $(STRING) $(LOAD) $(SOCKET) $(FRAME) $(FILE) $(COMM) $(FLECK_LINKEDLIST) $(WORKER_LINKEDLIST) $(GOTHAM_EXIT) $(GOTHAM_HANDLE) $(GOTHAM_MANAGE_CLIENT) $(GOTHAM_SRV) $(GOTHAM_COMM) -o Gotham/Gotham

# Ejecutable de Harley
Harley: $(HARLEY) $(SEMAPHORE) $(IO) $(STRING) $(LOAD) $(FRAME) $(SOCKET) $(MONITOR) $(COMM) $(DIR) $(FILE) $(WORKER_EXIT) $(COMPRESSION) $(WORKER_COMM) $(WORKER_SRV) $(WORKER_DIST) $(WORKER_MANAGE_CLIENT) $(WORKER_CONTEXT) Worker/typeWorker.h
	gcc $(CFLAGS) $(HARLEY) $(SEMAPHORE) $(IO) $(STRING) $(LOAD) $(FRAME) $(SOCKET) $(MONITOR) $(COMM) $(DIR) $(FILE) $(WORKER_EXIT) $(COMPRESSION)  $(WORKER_COMM) $(WORKER_SRV) $(WORKER_DIST) $(WORKER_MANAGE_CLIENT) $(WORKER_CONTEXT) -o Worker/Harley/Harley -lm 

# Ejecutable de Enigma
Enigma: $(ENIGMA) $(SEMAPHORE) $(IO) $(STRING) $(LOAD) $(FRAME) $(SOCKET) $(MONITOR) $(COMM) $(DIR) $(FILE) $(WORKER_EXIT) $(COMPRESSION) $(WORKER_COMM) $(WORKER_SRV) $(WORKER_DIST) $(WORKER_MANAGE_CLIENT) $(WORKER_CONTEXT) Worker/typeWorker.h
	gcc $(CFLAGS) $(ENIGMA) $(SEMAPHORE) $(IO) $(STRING) $(LOAD) $(FRAME) $(SOCKET) $(MONITOR) $(COMM) $(DIR) $(FILE) $(WORKER_EXIT) $(COMPRESSION)  $(WORKER_COMM) $(WORKER_SRV) $(WORKER_DIST) $(WORKER_MANAGE_CLIENT) $(WORKER_CONTEXT) -o Worker/Enigma/Enigma -lm
#####################################################################################################

#############################################CLEAN###################################################
clean:
	rm -f $(IO) $(FRAME) $(SOCKET) $(STRING) $(FILE) $(DIR) $(LOAD) $(MONITOR) $(COMM) $(FLECK_LINKEDLIST) $(WORKER_LINKEDLIST) $(SEMAPHORE) \
	$(FLECK_CMD) $(FLECK_EXIT) $(FLECK_DIST) $(FLECK_COMM) \
	$(GOTHAM_EXIT) $(GOTHAM_HANDLE) $(GOTHAM_MANAGE_CLIENT) $(GOTHAM_COMM) $(GOTHAM_SRV) \
	$(WORKER_EXIT) $(WORKER_COMM) $(WORKER_SRV) $(WORKER_DIST) $(WORKER_MANAGE_CLIENT) $(WORKER_CONTEXT) \
	$(FLECK) $(GOTHAM) $(HARLEY) $(ENIGMA) 
