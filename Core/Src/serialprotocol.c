/**
******************************************************************************
* @file serialprotocol.c
* @author Reiter Roman
* @brief mylib-Serielles Protokoll.
* Diese Datei bietet Funktionen zur Verwaltung der folgenden
* Funktionalitäten eines seriellen Protokolls in Verbindung mit dem UART2:
* + IO-Betriebsfunktionen
* + Zustands- und Fehlerfunktionen
*
@verbatim
  ==============================================================================
                     ##### How to use this driver #####
  ==============================================================================
   [..] Der SERIALPROT MYLIB-Treiber kann wie folgt verwendet werden:

	(#) Einbinden des UART2
		(+) Damit das serielle Protokoll verwendet werden kann muss der UART2 aktiviert werden.
			(++) Der UART2 muss auf den Pins PA4 (TX) und PA15(RX) liegen
			(++) Für den UART2 werden die Standardeinstellungen der IDE-CubeMX verwendet (Baudrate, Wortlänge, Stop-Bit, Parität, Prescaler-Wert..)
			(++) Zusätzlich muss der globale Interrupt in der NVIC-Konfiguration des UART2 aktiviert werden.
			(++) Das Empfangen der Daten erfolgt mit HAL_UART_Receive_IT ()
			(++) Das Senden der Daten erfolgt mit HAL_UART_Transmit ()
			(++) Da der Datenampfang per Interrupt erfolgt, muss die Callback-Methode HAL_UART_TxCpltCallback () definiert werden.

	(#) Verwenden des seriellen Protokolls
		(+) Für das serielle Protokoll wird der UART2 benötigt.
			(++) Die Daten werden mit dem UART2 per Interrupt zeichenweise empfangen. Dafür muss ein Empfangspuffer deklariert und initialisiert werden.
				(+++) z.B.: uint8_t RxBuffer[RxBuffer_SIZE]={0};
			(++) Der Funktion HAL_UART_Receive_IT () muss der Empfangspuffer übergeben werden.
				(+++) z.B.: HAL_UART_Receive_IT(&huart2, RxBuffer, 1)
			(++) Nach einem Zeichenempfang wird die HAL_UART_RxCpltCallback () aufgerufen
				(+++) Um die Eingabe mit dem seriellen Protokoll zu verknüpfen muss ein exchangePuffer angelegt werden,
				 welcher die Antworten zu den getätigten Eingaben in RxBuffer enthält.
					(++++) z.B.: uint8_t exchangedMessage[50] ={0};
				(+++) Als Schnittstelle für die Eingabe (RxBuffer) und der Ausgabe (exchangedMessage) muss die Funktion
					MYLIB_SERIALPROT_XCHANGE() aufgerufen werden.
					(++++) z.B.: MYLIB_SERIALPROT_XCHANGE(&hserialprot,RxBuffer,exchangedMessage);
				(+++) Die erstellte Antwortnachricht muss nun über den UART2 hinausgeschrieben werden HAL_UART_Transmit()
					(++++) z.B.: HAL_UART_Transmit(&huart2, exchangedMessage,(uint16_t)strlen(exchangedMessage), 100)
				(+++) Abschließend muss der Interrupt für den UART2-Empfang wieder aktiviert werden HAL_UART_Receive_IT()
					(++++) z.B.: HAL_UART_Receive_IT(&huart2, RxBuffer, RxBuffer_SIZE)

	(#) Verwenden der Callback-Funktion SERIALPROT_Command_GPO_Callback()
	 	(+) Die Funktion dient dazu, um z.B. GPIO's ansteuern oder andere zusätzliche Aktivitäten auszuführen.
	 		(++) Dazu wird die Callback-Funktion SERIALPROT_Command_XXX_Callback() in die main.c kopiert
	 		(++) Für die Abfragen bzw. das Festlegen der Kommandos können diese einfach in einer If-Schleige abgefragt werden.
	 			(+++) zur Vereinfachung der Abfrage wird das Makro __SERIALPROT_IS_COMMAND() zu verfügung gestellt,
	 		 		  wodurch die Eingabeparameter des letzten Kommandos abgefragt werden können
	 		 	(+++) Je nach Ergebnis muss beim erfüllen der Bedingung eine 0, anderfalls eine 1 zurückgegeben werden

  @endverbatim
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/

#include "stdint.h"
#include "stddef.h"
#include "serialprotocol.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

/** @addtogroup MyLibrary
  * @{
  */

/** @addtogroup SERIALPROTOCOL SERIALPROTOCOL
  * @brief SerialProtocol Module Driver
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Defines SERIALPROTOCOL Private Defines
  * @{
  */
#define collectionBuffer_SIZE 65 	/*!< Groesse des Sendepuffers */
#define STM32_ACK_STR "STM32-ACK -> "	/*!< Nachricht OK */
#define STM32_NACK_STR "STM32-NACK -> " /*!< Nachricht falsch oder nicht erkannt */
#define NEW_LINE_STR "\n\r"				/*!< Neue Zeile */
/**
  * @}
  */
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

static char CollectionBuffer[collectionBuffer_SIZE];


/* Private function prototypes -----------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Functions SERIALPROTOCOL Private Functions
  * @{
  */
static uint8_t is_number(char string[]);
static uint8_t is_alpha_string(char string[]);

static void cut_input(char * delimiter,char * string, char (*result)[collectionBuffer_SIZE]);
static uint8_t string_char_frequency(char * string, char * spanset);

static void wrong_message(char * TxBuffer);
static SERIALPROTCOL_StatusTypeDef SERIALPROT_CheckMessage(SERIALPROTOCOL_HandleTypeDef *hserialprot);

static void SERIALPROT_CreateMessage_NUMBER_NUMBER(SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer );
static void SERIALPROT_CreateMessage_TEXT_TEXT(SERIALPROTOCOL_HandleTypeDef *hserialprot, char * TxBuffer );
static void SERIALPROT_CreateMessage_TEXT_NUMBER(SERIALPROTOCOL_HandleTypeDef *hserialprot, char * TxBuffer );
static void SERIALPROT_CreateMessage_NUMBER_TEXT(SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer );

static void SERIALPROT_COMMAND_CMS(SERIALPROTOCOL_HandleTypeDef *hserialprot, char * TxBuffer );
/**
  * @}
  */
/* Exported functions --------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Exported_Functions SERIALPROTOCOL Exported Functions
  * @{
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group1 Initialization Functions
 *  @brief   Initialization Functions
 *
@verbatim
 ===============================================================================
             ##### Initialization Functions  #####
 ===============================================================================
    [..] Dienen fürs initialisieren des Rotary Modules

@endverbatim
  * @{
  */

/**
  * @brief  Funktion verarbeitet die einzel Eingegebenen Zeichen von RxBuffer und gibt dementsprechend die Antwort im TXBuffer zurück
  * @param  hserialprot SERIALPROT handle
  * @param  RxBuffer 	Ein-Zeichen-Empfangspuffer
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
uint8_t SERIALPROTOCOL_Transmit(SERIALPROTOCOL_HandleTypeDef *hserialprot, char * RxBuffer, char * TxBuffer ){

	uint8_t retval=0;
	/* Überprüfen eingegebene Zeichen zwischen 32 und 127 oder Enter-Taste sind */
	if((RxBuffer[0]>=32 && RxBuffer[0]<=127) || RxBuffer[0]=='\r' )
	{

		/* Wenn Puffergröße überschritten, dann OV ausgeben */
		if(strlen(CollectionBuffer)==collectionBuffer_SIZE)
		{
			memset(CollectionBuffer,0,strlen(CollectionBuffer));
			strcat(TxBuffer, " -> OV\n\r");
			strcat(TxBuffer, "Input> ");

		}
		else
		{
			/* Wenn Backspace-Taste gedrückt */
			if(RxBuffer[0]!='\177')
			{
				strcat(CollectionBuffer, RxBuffer);
				strcat(TxBuffer, RxBuffer);
			}
		}

		/* Wenn Backspace-Taste gedrückt */
		if(RxBuffer[0]=='\177')
		{
			/* Verhindern, dass "Input> " überschrieben wird */
			if(strlen(CollectionBuffer)>0){
				CollectionBuffer[strlen(CollectionBuffer)-1]=0;
				strcat(TxBuffer, "\177");
			}
			else
			{
				strcat(TxBuffer, "\32");
			}
		}

		/* Wenn Enter-Taste gedrückt */
		if(RxBuffer[0]=='\r')
		{
			retval=1;
			/* Eingabe überprüfen ob Kommando-Syntax */
			if(SERIALPROT_CheckMessage(hserialprot) == SERIALPROT_OK )
			{
				/* Nachrichtenarten abragen */
				if(hserialprot->MessageKind == MESSAGEKIND_NUMBER_NUMBER){
					SERIALPROT_CreateMessage_NUMBER_NUMBER(hserialprot,TxBuffer);
				}else if(hserialprot->MessageKind == MESSAGEKIND_TEXT_TEXT){
					SERIALPROT_CreateMessage_TEXT_TEXT(hserialprot,TxBuffer);
				}else if(hserialprot->MessageKind == MESSAGEKIND_TEXT_NUMBER){
					SERIALPROT_CreateMessage_TEXT_NUMBER( hserialprot,TxBuffer);
				}else{
					SERIALPROT_CreateMessage_NUMBER_TEXT(hserialprot,TxBuffer);
				}

			}else if(!strcmp(CollectionBuffer,"\r"))
			{
				strcat(TxBuffer, "\n\r");
			}else{
				wrong_message(TxBuffer);
			}
				memset(CollectionBuffer,0,strlen(CollectionBuffer));
		}
	}
	else
	{
		strcat(TxBuffer, "\32");
	}

	return retval;
}

/**
  * @}
  */

/** @addtogroup SERIALPROTOCOL_Exported_Functions_Group2 Callback Functions
 *  @brief   Callback Functions
 *
@verbatim
 ===============================================================================
                      #####  Callback Functions  #####
 ===============================================================================
    [..]  Dienen dazu, um die Funktionen in die main.c zum kopieren um Befehle abzufragen und diese zu Handeln

@endverbatim
  * @{
  */


/**
  * @brief  Funktion 	Callback, welche in die main.c kopiert werden kann um die Eingabe des "cms" Befehls abzufragen
  * @param  hserialprot SERIALPROT handle
  * @retval query
  */
__weak uint16_t SERIALPROT_Command_CMD_Callback(SERIALPROTOCOL_HandleTypeDef *hserialprot)
{
	/* Prevent unused argument(s) compilation warning */
	UNUSED(hserialprot);

	/* NOTE : This function should not be modified, when the callback is needed,
            	the SERIALPROT_Command_GPO_Callback could be implemented in the user file
	 */
	return 0;
}

/**
  * @}
  */

/**
  * @}
  */

/* Private functions----------------------------------------------------------*/
/** @addtogroup SERIALPROTOCOL_Private_Functions
  * @{
  */

/**
  * @brief  Funktion 	überprüft ob ein String eine Zahl enthält
  * @param  string[] 	der zu überprüfende String
  * @retval result 		Ergebnis
  */
static uint8_t is_number(char string[])
{
    for (uint8_t i = 0; string[i]!= '\0'; i++)
    {
        if (isdigit((int)string[i]) == 0)
              return 0;
    }
    return 1;
}

/**
  * @brief  Funktion 	überprüft ob ein String ein aus Kleinbuchstaben besteht
  * @param  string[] 	der zu überprüfende String
  * @retval result 		Ergebnis
  */
static uint8_t is_alpha_string(char string[])
{
    for (uint8_t i = 0; string[i]!= '\0'; i++)
    {
        if (isalpha((int)string[i]) == 0)
              return 0;
    }
    return 1;
}


/**
  * @brief  Funktion zerteilt den Inputstring "string" bei jedem Zeichen "delimiter" und speichert diese in "result"
  * @param  string		der zu zerteilende String
  * @param  delimiter	das Zeichen, nachdem der String geteilt werden soll
  * @param  result		zerteilte Strings
  * @retval none
  */
static void cut_input(char * delimiter,char * string, char (*result)[collectionBuffer_SIZE]){

    char *ptr;

    ptr = strtok(string, delimiter);
    {
    	uint8_t i =0;
        while(ptr != NULL)
        {
            strcpy(result[i++],ptr);
            ptr = strtok(NULL, delimiter);
        }
    }
}

/**
  * @brief  Funktion zählt die die vorkommenden Zeichen "spanset" eines Strings "string" und retourniert die Anzahl
  * @param  string 		String, wo die Zeichen gezählt werden sollen
  * @param  spanset 	Zeichen, welches gezählt werden soll
  * @retval count		Anzahl der Zeichen in dem String
  */
static uint8_t string_char_frequency(char * string, char * spanset){

    char *ptr = string;

    uint8_t count=0;
		for(uint8_t i=0; i<strlen(string);i++)
        {
            ptr = strpbrk(ptr, spanset);

            if(ptr++)
            	count++;
            else
                break;
        }

	return count;
}

/**
  * @brief  Funktion 	fügt dem TxBuffer "falsche Nachricht" hinzu
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void wrong_message(char * TxBuffer){

	strcat(TxBuffer, "\n\r");
	strcat(TxBuffer, "STM32-NACK -> ");
	strcat(TxBuffer, CollectionBuffer);
	strcat(TxBuffer, "\n\r");

}

/**
  * @brief  Funktion 	überprüft ob das eingegebene Kommando für "MESSAGEKIND_NUMBER_NUMBER" definiert ist
  * @param  hserialprot SERIALPROT handle
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void SERIALPROT_CreateMessage_NUMBER_NUMBER(SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer ){

	/* kein Kommando definiert */
	if(0){

	}else{
		wrong_message(TxBuffer);
	}
}

/**
  * @brief  Funktion 	überprüft ob das eingegebene Kommando für "MESSAGEKIND_TEXT_TEXT" definiert ist
  * @param  hserialprot SERIALPROT handle
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void SERIALPROT_CreateMessage_TEXT_TEXT( SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer ){
	/* kein Kommando definiert */
	if(0){

	}else{
		wrong_message(TxBuffer);
	}
}

/**
  * @brief  Funktion 	überprüft ob das eingegebene Kommando für "MESSAGEKIND_TEXT_NUMBER" definiert ist
  * @param  hserialprot SERIALPROT handle
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void SERIALPROT_CreateMessage_TEXT_NUMBER(SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer ){
	/* Überprüfen ob Kommando "cms" */
	if (__SERIALPROT_IS_COMMANDNAME(hserialprot,"cms")){
		SERIALPROT_COMMAND_CMS(hserialprot,TxBuffer);
	}else{
		wrong_message(TxBuffer);
	}
}

/**
  * @brief  Funktion 	überprüft ob das eingegebene Kommando für "MESSAGEKIND_NUMBER_TEXT" definiert ist
  * @param  hserialprot SERIALPROT handle
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void SERIALPROT_CreateMessage_NUMBER_TEXT(SERIALPROTOCOL_HandleTypeDef *hserialprot,char * TxBuffer ){

	/* kein Kommando definiert */
	if(0){

	}else{
		wrong_message(TxBuffer);
	}
}

/**
  * @brief  Funktion 	wertet den Rückgabewert der "SERIALPROT_Command_CMS_Callback" aus und erzeugt dementsprechend die Antwort im TxBuffer
  * @param  hserialprot SERIALPROT handle
  * @param  TxBuffer 	Sendepuffer/Antwortpuffer
  * @retval none
  */
static void SERIALPROT_COMMAND_CMS(SERIALPROTOCOL_HandleTypeDef *hserialprot, char * TxBuffer ){
	/* Erzeugen der Ausgangsnachricht falls Rückgabewerde der Callback 0 ist */
	if (!SERIALPROT_Command_CMS_Callback(hserialprot)){
		strcat(TxBuffer, NEW_LINE_STR);
				strcat(TxBuffer, STM32_ACK_STR);
				strcat(TxBuffer, CollectionBuffer);
				TxBuffer[strlen(TxBuffer)-1]=0;
				strcat(TxBuffer," => " );
				strcat(TxBuffer, "#a,");
				strcat(TxBuffer, hserialprot->Parameter1);
				strcat(TxBuffer, "=");
				strcat(TxBuffer, hserialprot->Parameter2);
				strcat(TxBuffer, NEW_LINE_STR);
	}else{
		wrong_message(TxBuffer);
	}

}

/**
  * @brief  Funktion 	Überprüft vorab ob die Eingabe eine "Kommando-Syntax" ist
  * @param  hserialprot SERIALPROT handle
  * @retval none
  */
static SERIALPROTCOL_StatusTypeDef SERIALPROT_CheckMessage(SERIALPROTOCOL_HandleTypeDef *hserialprot)
{

	char inputmessage[40] = {0};
	strcat(inputmessage, CollectionBuffer);

	uint8_t  pos_command_start = strcspn( inputmessage, "#" );
	uint8_t  pos_command_split = strcspn( inputmessage, "," );
	uint8_t  pos_command_end = strcspn( inputmessage, "\r" );

	if(pos_command_start==0 && pos_command_split==4  && pos_command_end == strlen(inputmessage)-1  && string_char_frequency(inputmessage,",") ==1 && string_char_frequency(inputmessage,":") ==1 ){

		inputmessage[strlen(inputmessage)-1]=0;

		char command_name[10];
		char command_param1[10];
		char command_param2[10];

		char result1[3][collectionBuffer_SIZE]={0};
		char result2[3][collectionBuffer_SIZE]={0};

		cut_input(",",inputmessage,result1);
		strcpy(command_name,result1[0]+1);
		cut_input(":",result1[1],result2);

		strcpy(command_param1, result2[0]);
		strcpy(command_param2, result2[1]);

		//uint8_t sd = strlen(command_name);
		//uint8_t sdd = strlen(command_param1);
		//uint8_t sdf = strlen(command_param2);
		if(strlen(command_name)<=3 && strlen(command_param1)<=4 && strlen(command_param2)<=5){

			strcpy(hserialprot->CommandName, command_name);
			strcpy(hserialprot->Parameter1, command_param1);
			strcpy(hserialprot->Parameter2, command_param2);

			if(is_alpha_string(command_name) && is_number(command_param1) && is_number(command_param2)){
				hserialprot->MessageKind = MESSAGEKIND_NUMBER_NUMBER;
				return SERIALPROT_OK;
			}else if(is_alpha_string(command_name) && is_number(command_param2)){
				hserialprot->MessageKind = MESSAGEKIND_TEXT_NUMBER;
				return SERIALPROT_OK;
			}else if(is_alpha_string(command_name) && is_number(command_param1) && is_alpha_string(command_param2)){
				hserialprot->MessageKind = MESSAGEKIND_NUMBER_TEXT;
				return SERIALPROT_OK;
			}else if(is_alpha_string(command_name) && is_alpha_string(command_param1) && is_alpha_string(command_param2)){
				hserialprot->MessageKind = MESSAGEKIND_TEXT_TEXT;
				return SERIALPROT_OK;
			}else{
				return SERIALPROT_ERROR;
			}
		}
		else
		{
			return SERIALPROT_ERROR;
		}

  	  }else{
  		  return SERIALPROT_ERROR;
  	  }
}

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
