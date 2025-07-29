# All Data

## Table of contents

- [All Data](#all-data)
  - [Table of contents](#table-of-contents)
  - [Introdução](#introdução)

## Introdução

Este diretório é dedicado para o versionamento dos dados utilizados na pesquisa

As mensagens são compostas por três tipos principais: firstMessages, updateMessages e requestMessages.

firstMessages representa um vetor de mensagens de diferentes objetos.

updateMessages representa um vetor de vetores de tamanho 4 de mensagens de diferentes objetos.

Tanto firstMessages quanto updateMessages são indexados da seguinte maneira

| índice | Objeto |
|--------|--------|
|0| "cot:Computer"  |
|1| "cot:SmartMirror"  |
|2| "cot:SmartTelevision"  |
|3| "cot:SmartSpeaker"  |
|4| "cot:SmartCamera"  |
|5| "cot:SmartCabinet"  |
|6| "cot:SmartCabinet"  |
|7| ["cot:SmartMirror", "cot:SmartCabinet"]  |
|8| "cot:SmartAirConditioner"  |
|9|  "cot:SmartRing"  |
|10| "cafeteira"  |
|11| "cot:SmartShower"  |
|12| "cot:SmartCar"  |
|13| "cot:SmartNecklace"  |
|14| "cot:SmartCollar"  |
|15| "cot:SmartFeeder"  |
|16| "toothbrush"  |
|17| "cot:SmartStove"  |
|18| "cot:SmartRefrigerator"  |
|19| "cot:SmartIrrigationController"  |
|20| ["cot:SmartCurtain", "cot:SmartWindow"]  |
|21| "cot:SmartLightBulb"  |
|22| "cot:SmartLightSwitch"  |
|23| "cot:SmartLaundryMachine"  |
|24| "cot:SmartCookware"  |
|25| "piso"  |
|26| "cot:SmartDoor"  |
|27| "cot:SmartWatch"  |
|28| "Robo aspirador de po"  |
|29| "jardim"  |
|30| "cot:SmokeSensor"  |
|31| "cot:SmartPowerOutlet"  |

requestMessages representa mensagens de requisição de diversas aplicações, que podem ser indexadas da seguinte maneira:

| índice | Aplicação |
|--------|--------|
|0 |"fallDetection" |
|1 |"microControl" |
|2 |"petCare" |
|3 |"energyManegement" |
|4 |"waterManegement" |
|5 |"security" |
|6 |"localization" |
|7 |"gasSec" |
|8 |"healthCare" |
|9 |"lightControl" |
|10 |"tempControl" |
|11 |"smartCleaning" |
|12 |"garden" |
|13 |"mobility" |
|14 |"SmartCooking" |
|15 |"inventoryManagement" |
