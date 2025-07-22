# Distributed File Compression & Recovery Simulator

Distributed File Compression & Recovery Simulator is an educational C project that models a distributed system for file compression (referred to as "distortion" in the codebase). The system is composed of several modules:

- **Gotham:** The central server that receives compression (distortion) requests from Fleck clients and delegates them to the main Worker.
- **Fleck:** The client module that submits file compression requests.
- **Worker(s):** Perform file compression. One main Worker handles tasks, while backup Workers monitor and can recover/resume in-progress compressions using shared memory if the main Worker fails.
- **Harley & Enigma:** Additional modules for monitoring, logging, or specialized processing.

## Key Features & Tools

- Multithreading (pthread)
- Semaphores for synchronization
- Shared memory for state recovery
- Process forking for parallelism
- Socket-based inter-module communication
- Mutexes for thread safety

**How it works:**  
Gotham coordinates the system, assigning compression tasks to Workers. If a Worker fails, backup Workers use shared memory to detect and resume incomplete compressions, ensuring reliability and fault tolerance.

---

## Project Structure

This project includes **four main programs** distributed in different directories. Each program requires a configuration file named `config.dat`, which must be present in the corresponding directory.

### Available Programs

- **Fleck:** located in the `Fleck/` folder  
- **Gotham:** located in the `Gotham/` folder  
- **Enigma:** located in `Worker/Enigma/`  
- **Harley:** located in `Worker/Harley/`  

---

## Execution Instructions

To run any of the programs, follow these steps:

1. Go to the directory of the program you want to run.  
2. Run the following command, replacing `<process_name>` with the appropriate program name (`Enigma`, `Harley`, `Fleck`, or `Gotham`):  
   `./<process_name> config.dat`

### Prerequisites

All folders and subfolders have already been created, but these are the requirements to create all necessary system folders for correct operation.

Currently, the following configuration files are adapted so that the different processes connect to the corresponding machine:

(Name  |  Config file | Server to connect to | Associated folder)
- Gotham -> config.dat -> (Montserrat) -> None
- Fleck -> config.dat  -> (Puigpedros) -> /arthur
- Fleck -> config2.dat -> (Matagalls)  -> /armand
- Fleck -> config3.dat -> (Montserrat) -> /alex
- Enigma -> config.dat -> (Matagalls)  -> /riddler
- Enigma -> donfig.dat -> (Matagalls)  -> /rizzler 
- Harley -> config.dat -> (Puigpedros) -> /riddler
- Harley -> donfig.dat -> (Puigpedros) -> /rizzler  

If you want to make any changes to the configuration files or the folders associated with a process, please follow these steps:

**Fleck:**  
Create the directory specified in the `config.dat` file with the following command:  
`mkdir <name_directory>`  
This directory must be created inside the `Fleck/` folder.

**Workers (Harley and Enigma):**  
1. Create the directory specified in the `config.dat` file inside the `Harley/` or `Enigma/` directory:  
   `mkdir <name_directory>`  
2. Create the `unfinished_distortions` directory (only necessary **once**) inside the `Worker/` directory:  
   `mkdir Worker/unfinished_distortions`

---

## Execution Examples

### **Run Fleck**
```sh
cd Fleck/
mkdir <name_directory>
./Fleck config.dat
```

### **Run Gotham**
```sh
cd Gotham/
./Gotham config.dat
```

### **Run Harley**
```sh
cd Worker/
mkdir unfinished_distortions
cd Harley/
mkdir <name_directory>
./Harley config.dat
```

### **Run Enigma**
```sh
cd Worker/Enigma/
mkdir <name_directory>
./Enigma config.dat
```

---

## Notes

- Ensure that the `config.dat` files are correctly configured for each program.
- The `unfinished_distortions` directory is shared by both **Enigma** and **Harley**, and only needs to be created once.
- It is essential that all **Fleck** instances have **unique names** defined in their configuration files.

