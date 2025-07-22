 **PràcticaSO**

## **Projecte: MR.J.System**  
Aquest projecte inclou **quatre programes principals** distribuïts en diferents directoris. Cada programa requereix un fitxer de configuració anomenat `config.dat`, que ha d'estar present al directori corresponent.

---

## **Programes disponibles**  
- **Fleck**: ubicat a la carpeta `Fleck/`  
- **Gotham**: ubicat a la carpeta `Gotham/`  
- **Enigma**: ubicat dins de `Worker/Enigma/`  
- **Harley**: ubicat dins de `Worker/Harley/`  

---

## **Instruccions per a l'execució**  
Per a la correcta execució de qualsevol dels programes, cal seguir aquests passos:

1. Accedeix al directori del programa que vols executar.  
2. Executa la comanda següent, substituint `<nom del procés>` pel nom del programa corresponent (`Enigma`, `Harley`, `Fleck` o `Gotham`):  
   `./<nom del procés> config.dat`

### **Requisits previs per a l'execució**  
Totes les carpetes i subcarpetes ja han estat creades, tot i així aquests son els requisits per a crear totes les carpetes del sistema necessaries per al correcte funcionament 
d'aquest.

Actualment tenim els seguents fitxers de configuració ja adapatats per a que es connectin els diferents processos a la màquina corresponent:

(Nom  |  Fitxer conf | Servidor al qeu ha d'estar connectat | Carpeta associada al procès)
Gotham -> config.dat ->           (Montserrrat)             ->        No hi ha
Fleck -> config.dat  ->           (Puigpedros)              ->        /arthur
Fleck -> config2.dat ->           (Matagalls)               ->        /armand
Fleck -> config3.dat ->           (Montserrat)              ->        /alex
Enigma -> config.dat ->           (Matagalls)               ->        /riddler
Enigma -> donfig.dat ->           (Matagalls)               ->        /rizzler 
Harley -> config.dat ->           (Puigpedros)              ->        /riddler
Harley -> donfig.dat ->           (Puigpedros)              ->        /rizzler  

En cas que vulgueu fer algun canvi respecte els fitxers de configuració o les carpetes associades a aquell procès, siusplau seguiu els següents passos:

**Fleck**:  
Cal crear el directori especificat al fitxer `config.dat` amb la comanda següent:  
`mkdir <name_directory>`  
Aquest directori s'ha de crear dins de la carpeta `Fleck/`.  

**Workers (Harley i Enigma)**:  
1. Crear el directori especificat al fitxer `config.dat` dins del directori `Harley/` o `Enigma/`:  
   `mkdir <name_directory>`  
2. Crear el directori `unfinished_distortions` (només és necessari fer-ho **una sola vegada**) dins del directori `Worker/`:  
   `mkdir Worker/unfinished_distortions`

---

## **Exemples d'execució**

**Executar Fleck**:  
cd Fleck/  
mkdir <name_directory>  
./Fleck config.dat  

**Executar Gotham**:  
cd Gotham/  
./Gotham config.dat  

**Executar Harley**:  
cd Worker/  
mkdir unfinished_distortions  
cd Harley/  
mkdir <name_directory>  
./Harley config.dat  

**Executar Enigma**:  
cd Worker/Enigma/   
mkdir <name_directory>  
./Enigma config.dat  

---

## **Observacions**  
- És important que els fitxers `config.dat` estiguin configurats correctament per a cada programa.  
- El directori `unfinished_distortions` s'utilitza de manera compartida per `Enigma` i `Harley`, i només cal crear-lo **una sola vegada**.  
- Es requisit indispensable que tots els Fleck's tinguin noms diferents en el seu fitxer de configuració

---