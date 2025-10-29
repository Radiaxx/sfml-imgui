# Visualizzazione di dati raster 2D

Il lavoro prevede lo sviluppo di un visualizzatore, sviluppato in C++ con l'uso delle librerie SFML e Dear ImGui, che supporti le operazioni seguenti:
- Lettura di un campo scalare 2D campionato su una griglia a celle quadrate, da un file in formato ASC.
- Lettura di dati aggiuntivi da un file CSV. Tali dati consistono di punti sparsi e linee poigonali, definite in un sistema di riferimento 2D consistente con quello dei dati. 
- Visualizzazione 2D del campo scalare sotto forma di heatmap, utilizzando in alternativa diverse funzioni per tradurre i valori di campo in colori (color maps).
- Visualizzazione a richiesta dei dati aggiuntivi in sovraimpressione alla heatmap.
- Funzioni di query interattive sui valori di campo e gli attributi associati ai dati aggiuntivi.

Nel seguito vengono forniti ulteriori dettagli per le prime fasi del lavoro.

## Struttura generale del programma
Il programma dovrà presentare un GUI realizzata in Dear ImGui e un'area di visualizzazione utilizzata come canvas per SFML. Su entrambe le aree dovrà essere gestita l'interazione utente mediante mouse o tastiera. 

## Caricamento dati
Il formato ASC è descritto nel documento `Formato_ASC.md`. Per la gestione di tale formato, si potranno usare in alternativa o le classi fornite nei file `dem.hh` e `dem.cc` o sviluppare il parser ex-novo.

I file `aletsch_32T.asc` e `gausshills_1.asc` forniscono due esempi: il primo è un terreno reale, contenente dati di elevazione in metri e coordinate geografiche assolute; il secondo è un dataset sintetico. 

La struttura del file CSV contenente i dati secondari è in fase di definizione e sarà comunicata in seguito. In una prima fase, questi potranno essere tralasciati.

## Visualizzazione dati mediante heatmap
Una heatmap è un'immagine che rappresenta in falsi colori i dati scalari immagazzinati in un raster. Per poterla ottenere, è necessario convertire i valori dei dati in colori. Ciò è possibile tramite funzioni dette *color map* che hanno un prototipo simile al seguente

`sf::Color ColorMapX(double minv, double maxv, double v)`

dove `minv` e `maxv` sono i valori estremi dell'intervallo coperto dai dati che devono essere tradotti in valori e `v` è il valore da tradurre. Qualora `v` sia fuori dall'intervallo `[minv,maxv]`, la funzione deve fare clamp di tale valore all'intervallo stesso, ossia: tutti i valori minori o uguali a `minv` saranno mappati sullo stesso colore di `minv` e analogamente per quelli maggiori o uguali a `maxv`. 

Il programma deve prevedere in alternativa l'uso di diverse color map, che possono essere cambiate al volo da GUI. Il modo più efficiente per visualizzare heatmap con queste caratteristiche è descritto nel file `Shader.md`. La spiegazione è generata da un'AI, l'ho verificata sommariamente e sembra corretta, ma non ne sono totalmente sicuro. Consiglio di seguire l'Opzione B che è più flessibile e di implementare inizialmente solo le due color map (blue-to-red e greyscale) descritte nel documento stesso.  

In seguito, implementare almeno le seguenti altre color map (vedi sotto come si generano):
- Jet: definita da 
`float jet_srgb_floats[5][3] = {{0,0,1},{0,1,1},{0,1,0},{1,1,0},{1,0,0}`
- Turbo: vedere [qui](https://gist.github.com/mikhailov-work/6a308c20e494d9e0ccc29036b28faa7a)
- Viridis (con eventuali varianti plasma, inferno, magma): vedere [qui](https://github.com/BIDS/colormap/blob/master/colormaps.py) per l'implementazione Python da cui si possono facilmente estrarre i dati per l'equivalente implementazione C++.

Tutte queste color map sono definite da array di colori. Nell'esempio di Jet l'array contiene 5 triple, ciascuna rappresentante un colore in RGB, nell'ordine: blu, ciano, verde, giallo, rosso. Il primo colore è il valore mappato da `minv`, l'ultimo quello mappato da `maxv`. Il resto dell'intervallo $[vmin,vmax]$ va suddiviso in $N-1$ intervalli dove $N$ è la dimensione dell'array (4 intervalli per Jet). Per ogni valore dato $v$, bisogna individuare in che sotto-intervallo si trova (semplice divisione) e quindi ottenerne il colore interpolando tra i due colori codificati dall'array all'estremo di tale intervallo. Le altre color map oltre Jet hanno semplicemente array molto più lunghi, quindi dividono in molti più intervalli, ma il codice è identico. 

Tabelle delle precedenti e altre color map in formato CSV si possono anche scaricare da [questo sito](https://www.kennethmoreland.com/color-advice/). Queste tabelle forniscono già oltre ai colori anche i limiti degli intervalli. 
  
La libreria Python [Matplotlib](https://matplotlib.org/stable/gallery/color/colormap_reference.html) fornisce numerose altre colormap. Cercando nella libreria stessa è possibile trovare i dati che specificano ciascuna di queste, in un formato che però non è identico a quello utilizzato per le precedenti e che richiede quindi una conversione. Potrebbe essere utile avere a disposizione le color map `gist_earth` e `terrain`. 

## Opzioni di visualizzazione
Oltre alla scelta della color map, l'utente dovrà avere a disposizione le seguenti operazioni interattive:
- zoom e pan sulla porzione di dominio dei dati visualizzata. Per queste operazioni, agire sulla `sf::View` (qualche suggerimento alla Sezione 3 del file `Shader.md`)
- clamp automatico della color map al range di valori nella porzione visualizzata: per fare questo è sufficiente calcolare al volo i valori massimo e minimo all'interno della porzione visualizzata e passare tali valiro come parametri `minv` e `maxv` alla color map
- clamp manuale su un range di valori scelto dall'utente attraverso un cursore che setti i valori desiderati per `minv` e `maxv`. Scegliere il cursore adatto tra quelli messi a disposizione da Dear ImGui
- qualdo lo zoom è sufficientemente elevato (ad esempio ogni cella della griglia occupa almeno 30x30 pixel) disegno in sovraimpressione della griglia mediante linee sottili nere; a richiesta, nelle stesse condizioni, disegno in sovraimpressione del valore di campo in ogni cella
- a qualunque livello di zoom, tooltip che fornisce il valore di campo della cella corrispondente al puntatore del mouse in corrispondenza di un click.

## Visualizzazione dei dati aggiuntivi

I dati aggiuntivi sono contenuti in un file CSV e possono essere punti, linee e aree, estesi con attributi, nel formato seguente (esempio, specifica sotto):

`id;name;type;life;misc;geom`  
`0;L0;LINE-ASCENDING;1.292975;"mm-line|L0|flow:U|fS:1|fE:0|S:9|E:1|UE:1|USs:0,60";"LINESTRING (6.000000 45.109167,6.000000 45.110000)"`  
`1;L1;LINE-ASCENDING;1.749708;"mm-line|L1|flow:U|fS:1|fE:0|S:17|E:48|UE:48|USs:0,81";"LINESTRING (6.005833 45.048333,6.006667 45.048333,6.006667 45.047500,6.007500 45.047500,6.007500 45.046667)"`  
`2;L2;LINE-DESCENDING;7.976668;"mm-line|L2|flow:D|fS:0|fE:0|S:0|E:55|UE:55|USs:0";"LINESTRING (6.000000 45.090000,6.000833 45.090000,6.001667 45.090000,6.002500 45.090000,6.003333 45.090000,6.004167 45.090000,6.005000 45.090000,6.005833 45.090000,6.006667 45.090000,6.007500 45.090000,6.008333 45.090000,6.009167 45.090000,6.010000 45.090000)"`  
`3;L3;LINE-DESCENDING;7.976668;"mm-line|L3|flow:D|fS:0|fE:0|S:0|E:4294967295|UE:4294967295|USs:0";"LINESTRING (6.000000 45.090000,5.999167 45.089167)"`
`4;L4;LINE-ASCENDING;1.292975;"mm-line|L4|flow:U|fS:1|fE:0|S:70|E:3|UE:3|USs:2,159";"LINESTRING (6.000000 45.151667,6.000000 45.152500)"`  

- La prima riga consiste di metadati coi nomi dei vari campi ed è sempre uguale
- Ciascuna delle righe successive codifica un'entità
- L'identificatore `id` è un numero progressivo 
- Il nome `name` dell'entità è una stringa arbitraria (qui è semplicemente L seguito dall'id, ma non è detto che sia sempre così)
- Il tipo `type` può essere uno dei seguenti valori: MAXIMUM, MINIMUM, SADDLE, LINE-ASCENDING, LINE-DESCENDING, AREA
- Il campo `life` contiene un valore floating point che può avere diversi significati (vedi di seguito)
- Il campo `misc` è una stringa di lunghezza arbitraria che contiene informazioni aggiuntive non meglio specificate
- Il campo `geom` specifica la geometria del punto secondo il formato [WKT](https://www.giswiki.ch/GeoCSV) specificato di seguito.

Il testo seguente dovrebbe essere sufficiente per la comprensione del WKT, altrimenti consultare il sito:

> In GeoCSV (and other geospatial contexts), POINT represents a single location with an X,Y coordinate, LINESTRING is an ordered sequence of two or more points representing a path like a road or river, and MULTILINESTRING is a collection of multiple LineStrings, useful for grouping together disconnected linear features into one geometry. These formats are typically written using Well-Known Text (WKT) syntax within a single string column. 
>
> POINT
Definition: A single coordinate pair (X, Y), representing a discrete location. 
Example: POINT (8.8249 47.2274). 
Use Case: Representing cities on a world map, or specific data points like subway stations. 
>
> LINESTRING 
Definition: An ordered collection of two or more points, forming a continuous path.
Example: LINESTRING (10 10, 20 20, 10 40).
Use Case: Representing linear features such as roads, rivers, or cables.
>
>MULTILINESTRING
Definition: A collection of one or more LineStrings, grouped into a single geometric object. 
Example: MULTILINESTRING ((10 10, 20 20), (30 30, 40 40)). 
Use Case: Grouping several disconnected roads or river segments under a single feature. 
>
>How they are used in GeoCSV
GeoCSV uses a single string column to store geometries. 
The geometry type is defined by a constructor, such as POINT, LINESTRING, or MULTILINESTRING. 
Coordinates are typically represented in lon/lat (longitude, latitude) order within the WKT format. 
These geometry types are human-readable and can be parsed by GIS software and databases that support WKT. 

Le coordinate utilizzate nel campo `geom` sono coerenti con quelle  del relativo file `.asc`, quindi le geometrie andranno disegnate nello stesso sistema di riferimento. 

### Parser

Per prima cosa sarà necessario definire le strutture dati in cui immagazzinare questi dati nel programma. Potresti definire una classe generica per l'entità e poi specializzarla per i vari tipi, ma fai come preferisci. Poi fai semplicemente un vector di entità.

Quindi bisogna realizzare un parser per caricare questo formato. Puoi farti aiutare da un'AI: se le passi la spiegazione e la specifica della struttura dati, dovrebbe essere in grado di confezionare le funzioni di parsing, che comunque vanno poi controllate. 

### GUI

Una volta caricati i dati nel programma, devi estendere la GUI per gestirne la visualizzazione:

- Selezione delle entità da visualizzare (anche più di un tipo contemporaneamente): gestisci i punti (MAXIMUM, MINIMUM, SADDLE) in modo da poterli selezionare separatamente o tutti quanti insieme. Stessa cosa per le linee (LINE-ASCENDING e LINE-DESCENDING). Le aree sono per ora di un solo tipo, quindi stanno per conto loro. In questa prima fase (vedi sotto), manterrei la visualizzazione delle aree in alternativa a quella delle linee (quindi o aree o linee; se linee, allora si può scegliere se un tipo o entrambi)
- Scelta del colore da usare per ogni tipo di entità: il colore deve essere poi visualizzato in GUI per ogni tipo
- Possibilità di scalare i marker dei punti e lo spessore delle linee (oppure il colore, vedi sotto) secondo il valore del campo `life`
- Possibilità di filtrare le entità da visualizzare secondo il campo `life`: in fase di caricamento devi memorizzare il valore minimo e massimo di questo campo e poi inserire in GUI uno slider doppio che abbia gli estremi tra tali valori; l'utente può spostare i due cursori dello slider e il programma deve visualizzare solo le entità la cui `life` è compresa nell'intervallo scelto dall'utente (default: tutto). 

### Disegno

I punti dovranno essere disegnati in sovraimpressione ai dati, nella posizione specificata, rappresentanti con marker di forma e colore dipendente dalla categoria di ciascuno: 
- Triangolo con la punta in su per MAXIMUM; colore di default rosso
- Triangolo con al punta in giù per MINIMUM; colore di default blu
- X (crocetta) per SADDLE; colore di default verde

La grandezza può essere standard uguale per tutti, oppure, su scelta della GUI, scalata in base al valore di `life`. Sarebbe bene da GUI poter controllare le dimensioni massime e minime dei marker perché la scelta dipende da quanto sono densi nel dato. 

Le linee sono polyline (spezzate) e dovranno essere rappresentante con colore, stile e spessore dipendenti dalla categoria di ciascuna (default: ascendenti rosse e discendenti blu). Poiché il rendering in SFML di linee più spesse di un pixel non è banale, prova inizialmente con linee semplici e se si vedono troppo male prova a costruirti la tua linea con un rettangolo allungato e posizionato in modo che gli endpoint del segmento stiano a metà dei lati corti del rettangolo. Con questa tecnica è facile gestire lo spessore, ma sulle giunte il rendering non è ottimale. Per avere il risultato ottimale, bisogna disegnare anche cerchietti in corrispondenza dei giunti, il cui raggio è pari a metà dello spessore della linea. Se lavorare sullo spessore fosse troppo complicato, limitati a linee sottili e eventualmente usa la luminosità della linea per rappresentare la `life`, se richiesto dall'utente: nero per valori bassi e progressivamente più intenso per valori alti, fino alla saturazione piena del colore (può convenire convertire il colore avanti e indietro tra RGB e HSV oppure HSL - vedi se SFML ha supporto per questi formati colore, altrimenti fai i conti a mano) 

Per le aree si disegna semplicemente il contorno, che è dato da un insieme di polyline, quindi vale quanto sopra, ma le aree avranno il loro colore, quindi se si disegnano le aree si usa un colore diverso. 

### Pick

Aggiungi un meccanismo che ti permetta, cliccando sulle entità disegnate, di attivare un tooltip che visualizza le informazioni memorizzate per l'entità selezionata (`id, name, type, life, misc`, ma non `geom`).

Il picking può essere problematico da rendere efficiente. Il modo più semplice è il seguente:
1. al click del mouse nell'area di visualizzazione, ne catturi le coordinate e le riscali in coordinate del mondo (quelle usate per visualizzare i dati)
1. per ogni punto, linea e area visualizzata, misuri la distanza di tutti i punti che descrivono l'entità dal mouse e tieni traccia del più vicino e dell'entità a cui appartiene. A parità "vincono i punti": basta che li guardi per primi e aggiorni solo se la distanza è strettamente minore. 
1. L'entità corrispondente al punto più vicino è quella selezionata.

Questo approccio ha diversi problemi:
1. è lento perché guarda sempre tutte le entità; in dati veri, i punti da guardare possono facilmente essere dell'ordine di centinaia di migliaia; questo può causare lag
2. non misura le distanze vere tra punto e segmenti, ma confida nel fatto che i segmenti che definiscono le linee siano "corti" e quindi basti confrontarsi coi giunti delle polyline
3. Non permette di discriminare tra linee e aree, quindi "vinceranno" sempre quelle che guardi per prime.

Per ovviare al primo problema, si può integrare una libreria di indici spaziali (tipo quadtree o bounding volume hierarchy) e preprocessare i dati in modo da costruire l'indice in fase di caricamento e usarlo in fase di query per selezionare le sole intità con le quali ha senso fare il confronto. Puoi, in un secondo tempo, cercare in rete se ci sono librerie facili da usare adatte allo scopo. Se ne trovi, contattami prima di usarle; se non ne trovi, fammi sapere che ti aiuto.   

Per ovviare al secondo, si può implementare un algoritmo che calcola esattamente la distanza punto segmento. Vedi [questo sito](https://www.realtimerendering.com/intersections.html) e cerca l'algoritmo appropriato. Ma solo dopo aver risolto il problema precedente, altrimenti diventa tutto lentissimo. 

Per ovviare al terzo problema, puoi misurare quanti pixel dista il mouse dal punto più vicino in spazio immagine: a parità di distanza, se dista pochi pixel vince la linea, se dista molti vince l'area. 
