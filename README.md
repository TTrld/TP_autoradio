# TP_autoradio

## Démarrage

Dans un premier temps, on suit les étapes de démarrage du projet. On teste les différents périphériques (LEDs, USART2, printf), et on fait en sorte que notre shell fonctionne.

## Configuration matérielle

### GPIO Expander

- **Composant** : MCP23S17-E/SO
- **Datasheet** : disponible dans le dossier `datasheet/`

### SPI

- **Bus SPI utilisé** : SPI3 (sur la STM32)

Pour faire fonctionner le GPIO Expander, on configure le SPI en **"Full-Duplex Master"**. On met une **Data Size** de **8 bits**, puis on change le **PSC** à **32** afin d'avoir un **baudrate** de **2.5Mb/s**.

### I2C

- **Pins utilisées** : PB10 et PB11
- **Bus I2C** : I2C2

## Tests

Avant de passer au SGTL5000, on réalise le **driver** pour les leds, ainsi que des **commandes** qu'on appelle avec le shell, avec lesquels on commande l'état de chaque LED indépendant des autres. 
Toutes ces étapes ont été vérifiées en classe par le professeur.
### TRAM DOUT

- **Interface de communication** pour la transmission de données
- Permet la sortie de données numériques vers les dispositifs externes

![TRAM DOUT](ressources/tram%20dout.png)
