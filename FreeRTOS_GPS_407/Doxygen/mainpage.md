Hoofdpagina {#mainpage}
----------- 
<span style="font-family:'Corbel'; font-size:10pt; color:green">**Over deze pagina.** Deze pagina (..\Doxygen\\<b>mainpage.md</b>) is een .md markdown-bestand, gemaakt met Typora (gratis download: https://typora.en.uptodown.com/windows/download), aangevuld met html (markdown kun je mixen met html). Zo kun je redelijk simpel zelf een hoofdpagina maken, die Doxygen dan gebruikt (ipv de standaard Doxygen-mainpage); dit wordt dan deze **index.html**.<br>
Het is geïnstalleerd via de Doxygen Wizard, tab [Expert mode]:<br>
&nbsp;&nbsp;&nbsp; INPUT                  = mainpage.md<br>
&nbsp;&nbsp;&nbsp; USE_MDFILE_AS_MAINPAGE = mainpage.md<br>   Bestand <b>..\Doxyfile</b> is de doxygen projectfile. De outputmap van doxygen is <b>..\Doxygen\html</b><br>NB. Als je deze pagina verandert, moet je uiteraard wel eerst doxygen runnen (via tab [Run]) om aanpassingen te zien.
</span>



<span style="font-family:'Corbel'; font-size:12pt;"> 
<!-- 
*(Corbel 12pt is nu het standaard font)* 
</span>

-->

<br>

<h1 style="font-family:'Corbel';">
FreeRTOS documentatie</h1>

Het realtime OS FreeRTOS hebben we gekozen omdat het gratis is, open source is, veel gebruikt wordt, relatief eenvoudig is én... omdat het uitgebreid gedocumenteerd is. Dat betekent dat het lesmateriaal grotendeels online te vinden is (en niet op powerpoints herhaald hoeft te worden).  De belangrijkste bronnen zijn: 

<ul style="font-family:'Corbel';">
<li>**FREERTOS hoofddocumentatie**: de [API-Reference](https://www.freertos.org/a00106.html). <u>Bij het ontwikkelen en de lessen moet je die er constant bij houden.</u> </li>
<li>[FreeRTOS boeken en pdf's](https://www.freertos.org/Documentation/RTOS_book.html)</li>
</ul>

<br>

<h1 style="font-family:'Corbel';">
Werking: wat doet de applicatie?</h1>
De applicatie freeRTOS_GPS_407 (geschikt voor ARM Cortex 407) is een werkende freeRTOS-applicatie, met de volgende functies:

<ul style="font-family:'Corbel';">
<li>**Locatie opslaan**. Het slaat de huidige locatie op door middel van een gps module. De inkomende data wordt vervolgends omgezet in coördinaten.</li>
<li>**Route zetten**. Het zet een route door middel van verschillende waypoints te zetten. Deze waypoints zijn voorzien van coördinaten, die zijn opgehaald door de GPS.</li>
<li>**Route rijden**. Het verwerkt de inkomende gps data om hiermee de rij-richting te bepalen, vervolgens wordt dat doorgestuurd naar de arduino UNO die op basis daarvan de motoren aanstuurt.</li>
</ul> 



Als het ARM-bordje met je laptop communiceert via een terminalprogramma, dan zie je dit:

<ul style="font-family:'Corbel';">
<li>Het display toont het versienummer (main.c, admin.c)</li>
<li>Het blauwe ledje wordt elke halve seconde aan- en uitgezet (admin.c).</li>
<li>De applicatie leest de ARM-keys in (ARM_keys.c, en stm32f4xx_it.c).</li>
<li>De applicatie heeft een user interface via de UART (main.c). Het menu geeft aan welke toetsen gebruikt kunnen worden (UART_keys.c). </li>
<ul  style="list-style-type:none;">
<li>**MENU:**</li>
<li>**0**:  *On/Off ALL test output.* Bij '0'\<enter\> krijg je of ALLE of GEEN testoutput te zien.</li>
<li>**1**:  *On/Off ARM_keys output.* Bij '2'\<enter\> gaan de ARM-keys-tasken WEL of GEEN output genereren.'</li>
<li>**2**:  *On/Off UART_keys output.* Bij '3'\<enter\> gaan de UART-keys-tasken WEL of GEEN output genereren.'</li> 
<li>**3**:  On/Off GPS-data output (mits een GPS-receiver aangesloten is).</li>
<li>**m**:  *show Menu*. Dit menu wordt getoond.</li> 
<li>**p**:  *change Priority of task*. Met 'p'[,tasknummer, prioriteit]\<enter\> kun je de prioriteit van een task aanpassen en zien wat er gebeurt. 
Voorbeeld: 'p,7,20' verandert de prioriteit van task 7 naar 20.</li>
<li>**t**:  *display Task-data.* Na 't'\<enter\> krijg je de gegevens per task te zien, zoals: nummer, prioriteit en geheugen (stack) gebruik. Zo kun je code optimaliseren en zien of een taak dreigt te weinig geheugen te krijgen...</li>
</ul>
</ul>

<br>

<h1 style="font-family:'Corbel';">
Design: overview van de applicatie</h1>
Hieronder zie je het *graphical design* van de applicatie (gemaakt in Miro).<br> 
Het doel van het grafisch design is om snel inzicht te krijgen in de samenhang en werking van de applicatie. Daarom zijn alleen de belangrijkste zaken weergegeven. 

<img src="https://raw.githubusercontent.com/KarmaMarazu/GPS-Leaphy-ARM/refs/heads/master/FreeRTOS_GPS_407/Doxygen/HLD_gps_LEAPHY_V2.jpg" alt="FreeRTOS_GPS design V2" width="100%"/>

De verschillende elementen:
<ul style="font-family:'Corbel';">
<li>*I/O-laag.* Links zie je de ARM-inputs (UART, ARM-toetsen en ultrasone sensor), rechts de ARM-outputs (LCD, RGBY-leds, leaphy-motoren, en UART). De applicatie gebruikt deze I/O's om te laten zien hoe multitasking werkt met de FreeRTOS-mechanismen. </li>
<li>*Tasks.* Elke task bestaat uit een gekleurd blokje met titel en 'sourcecode'. Bijbehorende tasks zijn per kleur gegroepeerd en per functie opgedeeld. Zoals je ziet zijn er 6 task-groepen:</li>
<ul>
<li>*Blauw.* Deze task-groep roept de ISR voor UART aan. </li>
<li>*Oranje.* Deze groep verwerkt inkomende GPS-data en zet het om naar leesbare data. </li>
<li>*Roze.* Deze groep berekent een gemiddelde van de verwerkte data en zet dit in een array wanneer er op een ARM-key gedrukt wordt. </li>
<li>*Groen.* Dit is de User Interface via de UART. Het leest en interpreteert de commando's die de gebruiker via een terminalprogramma intikt. Daarnaast schrijft het naar de gebruiker het menu en het programmaverloop.</li>
<li>*Grijs.* Deze groep geeft functies aan de ARM-keys. </li>
<li>*Paars.* Deze groep bepaalt de afstand tussen de leaphy en een object op basis van inkomende data van de ultrasone sensor.</li>
<li>*Geel.* Deze groep houdt zich bezig met het bepalen van de actuele locatie van de Leaphy. </li>
</ul>
<li>en 3 functie-groepen:</li>
<ul>
<li>*Locatie opslaan.* Deze groep haalt de locatie van de gps op. </li>
<li>*Route opslaan.* Deze groep slaat de locatie van de gps op als waypoint. </li>
<li>*Route volgen.* Deze groep zorgt ervoor dat de Leaphy de route veilig kan rijden. </li>
</ul>
<li>*FreeRTOS-mechanismen.* De inter-proces-communicatie (tussen de tasks dus) met FreeRTOS-functies is met gekleurde pijlen aangegeven. Zo zie je welke tasks welke data naar elkaar sturen. </li>
<li>*Source-files.* De gekleurde blokjes geven aan in welke c-files de tasks gecodeerd zijn. </li>
<li>*IRQ.* Niet alle blokjes zijn tasks, maar functies, in dit geval interrupt handlers. Deze functies worden door STM32 gegenereerd op het moment dat je op de processor (.ios) hardwarematig een interrupt definieert. Die functie is dan nog leeg, en aan de programmeur om verder 'in te vullen'. In deze applicatie zijn de ARM-toetsen en de UART-input aan interrupts gekoppeld.</li>
</ul>


<br>




De applicatie is bedoeld om blinden te helpen navigeren. In het design hierboven zie je zijn de tasks beschreven in hun c-functieprototypes. 

Ze zijn allemaal keurig beschreven op de FreeRTOS-site (de [API-Reference](https://www.freertos.org/a00106.html) en  [boeken en pdf's](https://www.freertos.org/Documentation/RTOS_book.html)). 

Laten we eerst eens kijken wat een multitasking OS is en hoe die werkt. NB. Ik ga hier nu niet deze referenties herhalen, maar ga alleen globaal in op de basis FreeRTOS-zaken als multitasking, synchronisatie- en communicatiemechanismen en bijbehorende functies. 

<br>

<h1 style="font-family:'Corbel';">
FreeRTOS: de mechanismen in de applicatie</h1>

<ul style="font-family:'Corbel';">
<li>**Algemeen.** Voor de synchronisatie en communicatie *tussen* tasks gebruiken we 4 mechanismen: *mutexes* voor <u>synchronisatie</u>, en *notifications, events* en *queues* voor <u>communicatie</u>.<br></li><br>

<li>**[Mutual Exclusion Semaphores (mutexes)](https://www.freertos.org/a00113.html).** <br>Met een Mutual Exclusion Semaphore, meestal **mutex** genoemd, zorg je ervoor dat een 'shared resource' (denk aan: printer of wc of ledjes) niet tegelijkertijd door meerdere processen gebruikt kan worden (hence: mutual exluded). Bij een printer wil je dat eerst de ene printjob afgehandeld wordt voordat de volgende printjob aan de beurt komt. Bij een wc wil je liever niet dat een ander van het toilet gebruik maakt terwijl jij er net op zit. Anders gezegd: je wil dat dit soort 'gedeelde resources' *gesynchroniseerd* kunnen worden. Bij een wc is dit het slotje op de deur: rood betekent 'bezet', groen betekent: 'vrij'.<br> 
<img style="margin-left:30px;" src="../vrij-bezet.png" alt="mutex" width="15%"/>
<br>In de applicatie wordt een *mutex* gebruikt om te ledjes te synchroniseren. Kijk in het design naar het groene gedeelte. Daar zie je dat meerdere tasks tegelijkertijd naar de ledjes willen schrijven. Om ervoor te zorgen dat ze niet allemaal dwars door elkaar gaan rammen, is er een mutex aangemaakt. Een task kan pas naar de leds schrijven als de 'deur op groen' staat. De task wacht op het OS tot hij erin mag. Zolang de mutex 'bezet' is door een andere task, geeft het OS de beurt aan een andere task. De wachtende task blijft dan [blocked](https://www.freertos.org/RTOS-task-states.html).</li>

<ul>
<li>*Aanmaken:* handle = [xSemaphoreCreateMutex();](https://www.freertos.org/CreateMutex.html)</li>
<li>*Aanvragen (ik wil erin...):* [xSemaphoreTake(handle);](https://www.freertos.org/a00122.html)</li>
<li>*Vrijgeven (ik ben klaar...):* [xSemaphoreGive(handle);](https://www.freertos.org/a00123.html)</li>
<li>*Zie:* admin.c, ledjes.c <br></li><br>
</ul>
<li>**[Notifications.](https://www.freertos.org/RTOS-task-notification-API.html)**<br>Met notifications maak je een virtuele verbinding naar een andere task. De ontvangende task wacht (en is dus *'blocked'*) op het bericht van de versturende taak. Dat bericht kan een signalering zijn (net als 'tikkertje'), of een waarde. Om een notificatie te doen, moet de task die stuurt eerst de taskhandle van de ontvanger verkrijgen. De ontvangende taak wacht gewoon, en aan de code kun je niet zien op welke task hij wacht.<br>
<img style="margin-left:30px;" src="../notify.jfif" alt="notification" width="20%"/>
<br>*Let op. Bij queues en events stuur je niet een bericht naar een task, maar naar een specifieke queue-handle of een event-handle; elke taak kan naar die handle sturen of van die handle ontvangen ('luisteren').*</li>
<ul>
<li>*Aanmaken:* hoeft niet, de handle die nodig is bestaat al: de handle van de task waarnaar verzonden wordt.</li>
<li>*Zenden en ontvangen met een waarde:*  [xTaskNotify(taskhandle)](https://www.freertos.org/xTaskNotify.html) en [xTaskNotifyWait()](https://www.freertos.org/xTaskNotifyWait.html)</li>
<li>*Op elkaar laten wachten:* [xTaskNotifyGive(taskhandle)](https://www.freertos.org/xTaskNotifyGive.html) en [ulTaskNotifyTake()](https://www.freertos.org/ulTaskNotifyTake.html)</li>
<li>*Zie:* handles.c, ledjes.c, ARM_keys.c <br></li><br>
</ul>
<li>**[Eventgroups (flags).](https://www.freertos.org/event-groups-API.html)** <br>Met een event group zet je een waarde - of eigenlijk: een rij bits - op de aangemaakte eventgroup (op de handle dus) - dat doe je met EventGroupSetBits(). Elke bit kun je zien als een event - hence 'event group'.  Bijvoorbeeld, bit 1 staat voor de motorstatus (aan of uit, 1 of 0), bit 2 voor de koplampen (aan of uit) etc. <br>
<img style="margin-left:30px;" src="../event.png" alt="eventgroup" width="20%"/>
<br>
Met EventGroupWaitBits() worden de bits uitgelezen. Je kunt alle bits uitlezen, of alleen op een specifieke bit (event) wachten, bijvoorbeeld: de motortask wacht tot de motor aangezet wordt, dus reageert op bit 1. Je kunt dan aan het OS aangeven of het bit gecleared moet worden of niet. De applicatie doet het simpeler: nadat een ARM-key gepressed is, wordt een interrupt gegeneerd. In de ISR wordt bepaald welke key gedrukt is, en die key-waarde (int) wordt op de eventgroup-handle gezet (met ...SendFromISR). In ARM_keys.c wordt die waarde in zijn geheel uitgelezen. Er wordt dus verder niks met afzonderlijke bits gedaan. </li>
<ul>
<li>*Aanmaken:* handle = [xEventGroupCreate()](https://www.freertos.org/xEventGroupCreate.html)</li>
<li>*Zenden:*  [xEventGroupSetBits(handle)](https://www.freertos.org/xEventGroupSetBits.html) en [xEventGroupSetBitsFromISR(handle)](https://www.freertos.org/xEventGroupSetBitsFromISR.html)</li>
<li>*Ontvangen:* [xEventGroupWaitBits(handle)](https://www.freertos.org/xEventGroupWaitBits.html)</li>
<li>*Zie:* admin.c, stm32f4xx_it.c, ARM_keys.c <br></li><br>
</ul>
<li>**[Queues. ](https://www.freertos.org/a00018.html)** <br>Een queue is een rij/array van elementen. Net als karretjes bij een kassa. Een element kan een char zijn, of een structure, of een pointer, whatever. Bij het aanmaken geef je aan hoeveel elementen de queue moet bevatten (hoeveel karretjes er in de rij kunnen), en hoe groot (hoeveel bytes) elk element is (hoeveel in elk karretje past). Daarmee weet het OS genoeg. Een queue werkt via FIFO (first in, first out: default) of LIFO. Met *send* zet je een element op de queue; het OS zorgt (bij FIFO) dat dat element keurig achter het vorige element terecht komt. 
<img style="margin-left:30px;" src="../queue.png" alt="queue" width="30%"/>
<br>
Bij *receive* wordt het eerst verstuurde element van de queue gehaald (wil je dat niet, dan moet je *peek* gebruiken). In de applicatie wordt een queue gebruikt om de toetsaanslagen op de UART op te vangen; dit wordt gedaan door de ISR (met ...SendFromISR in main.c). Met deze queue wordt een soort buffer gecreëerd, zodat er geen toetsaanslagen verloren gaan. De ontvangende kant (UART_keys.c) leest elke character (toets) direct uit, tot ie een linefeed  '\\n' tegenkomt - dan gaat ie de string naar een andere task versturen met xTaskNotify(). <ul>
<li>*Aanmaken:* handle = [xQueueCreate()](https://www.freertos.org/a00116.html) </li>
<li>*Op queue zetten:* [xQueueSend(handle)](https://www.freertos.org/a00117.html) en [xQueueSendFromISR(handle)](https://www.freertos.org/a00119.html)</li>
<li>*Ontvangen:* [xQueueReceive(handle)](https://www.freertos.org/a00118.html)</li>
<li>*Zie:* admin.c, main.c, UART_keys.c<br></li>
</ul>
</li>
</ul>

<br>
<h1 style="font-family:'Corbel';">
Sourcefiles: samenhang van de applicatie</h1>
Hieronder zie je de voor deze applicatie belangrijkste bestanden. De applicatie bevindt zich 
in de map **Core**, en wel hoofdzakelijk in de map **MyApp**, die weer verdeeld is in App en Ports. Daarnaast zie je in Core nog de map **Src**, die door STM gegenereerde bestanden bevat.

<table  border='1' style="margin-left:30px; font-family:'Corbel'; font-size:11pt; background-color: #FFFFFF; border-style:solid;">
    <tr>
        <td>**[Core/Src]**</td>
        <td>**Bevat de door STM gegenereerde bestanden**</td>
    </tr>
    <tr>
        <td>main.c</td>
        <td>ISR voor UART (UART_keys.c), start van default tasks en error handler</td>
    </tr>
    <tr>
        <td>stm32f4xx_it.c</td>
        <td>ISR voor ARM_keys.c</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td></td>
    </tr>
    <tr>
        <td>**[Core/MyApp/App]**</td>
        <td>**Bevat de applicatie-code**</td>
    </tr>
    <tr>
        <td>admin.c</td>
        <td>Initialisatieoutput naar uart en lcd-scherm<br>De functies voor de initialisatie van de handles<br>De functies voor opzet en handling van tasks</td>
    </tr>
    <tr>
        <td>admin.h</td>
        <td>Bevat #defines, structs, externals, etc</td>
    </tr>
        <tr>
        <td>ARM_keys.c</td>
        <td>Behandelt de communicatie met de ARM_Keys via ISR in stm32f4xx_it.c</td>
    </tr>
    <tr>
        <td>gps.c</td>
        <td>Verwerking van de inkomende GPS-data (NMEO-protocol) van UART1</td>
    </tr>
        <tr>
        <td>gps_parser.c</td>
        <td>Verwerkt de data die binnen komt in gps.c tot bruikbare data</td>
    </tr>
        <tr>
        <td>Positite_Bepaling.c</td>
            <td>Bepaalt welke richting de Leaphy op moet rijden<br>Deze file bevat ook de drive_task om te zorgen dat de Leaphy alleen rijdt wanneer wij dat willen</td>
    </tr>
    <tr>
        <td>UART_keys.c</td>
        <td>Afhandeling UART-input via ISR in main.c</td>
    </tr>
    <tr>
        <td>&nbsp;</td>
        <td></td>
    </tr>
    <tr>
        <td>**[Core/Myapp/Ports]**</td>
        <td>**Bevat ARM-port-functies**</td>
    </tr>
    <tr>
        <td>buzzer.c</td>
        <td>In deze file wordt de buzzer aan gestuurt</td>
    </tr>
        <tr>
        <td>HC-SR04.c</td>
        <td>Dit bestand bevat de functie en variabelen voor het aansturen van de ultrasonic afstandsensor</td>
    </tr>
        <tr>
        <td>keys.c</td>
        <td>Keyboard-driver for SpARM-board v2</td>
    </tr>
    <tr>
        <td>lcd.c</td>
        <td>Het LCD schrempje wordt aangestuurt door lcd.c met behulp van admin.c</td>
    </tr>
    <tr>
        <td>uart.c</td>
        <td>De bestuuring van de UART</td>
    </tr>
</table>


<br>
