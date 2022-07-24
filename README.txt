
Für das Assignment3 wurde zusätzlich ein serielles Protokoll entwickelt (serialprotocol.h), welches
die Eingabe am UART handled.

Die 3 Producer Tasks speichern periodisch ihren Namen und den Wert einer globalen Variable, welche zyklisch erhöht wird,
in die outputmessageQueue.

Der monitoringTask nimmt die Werte aus der outputmessageQueue und gibt diese auf der Konsole (Putty) aus.


  ==============================================================================
                               ##### Start #####
  ==============================================================================					
	
Nach dem Upload auf den STM32 und den anschließenden start von Putty gibt der monitoringTask
periodisch die Werte der outputmessageQueueHandle aus.

Mit einem Tastendruck wird eine Eingabe gestartet. Das eingegebene Zeichen wird anschließend 
auf der Konsole ausgegeben. Zusätzlich wird die Ausgabe der producerTasks gestoppt.

Mit Backspace können Zeichen gelöscht werden und mit Enter wird die Eingabe bestätigt.

Um das Intervall des Tasks zu ändern muss folgende Kommandosyntax beachtet werden.

#befehltsname,taskname:zeit in Millisekunden

#cms,t1:1000   -> setzt die Invervallzeit des producerTask1 auf 1000ms
#cms,t2:2000   -> setzt die Invervallzeit des producerTask2 auf 2000ms
#cms,t3:3000   -> setzt die Invervallzeit des producerTask3 auf 3000ms					
			


Nach der erfolgreichen Eingabe wird die Eingabe bestätigt (Eingabe wird übernommen)
STM32-ACK -> #cms,t1:10000 => #a,t1=1000

Bei Eingaben, welche nicht einem Kommando entspechen wird folgendes Ausgeben (Eingabe wird nicht übernommen)			
STM32-NACK -> #cms,t4:10000
				

  ==============================================================================
   ##### genauere Beschreibung der Eingabe-Syntax auf der Konsole (Putty) #####
  ==============================================================================

##### Kommandosyntax #####

#befehlsname,Parameter1:Parameter2\r

Der Befehlname besteht aus 3 Buchstaben
Die Parameter können je nach Befehl maximal 5 Stellen (Text oder Zahl) besitzen
Die Bestätigung/Absenden des Befehls erfolgt mit der Enter-Taste(\r)

##### Beschreibung #####

SoF(Zeichen für den Befehlsbeginn)	 						#
CMD(Befehle bestehen aus 3 Buchstaben)						cms
SEP(Trennzeichen zwischen Befehl und Parameter) 			,
PARAMETER1 (Zahl oder Text)									t1
SEP(Trennzeichen zwischen den 2 Parametern) 				:
PARAMETER2 (Zahl oder Text)									1000 			(in ms)
EoF(Zeichen für das Begehlsende) 							\r    			(drücken der Enter-Taste)





