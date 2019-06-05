# UniTN - Operating Systems Lab project 2019

## Goal
Implement the emulation of a home automation system so that each device is represented by a single entity (process with possible sub-processes). The devices can be of two kinds and various types and they all have a "status", two-position "switches" (on / off) and a possible parameter register.


**Implementation with processes**: the individual devices are represented by processes children of a main process, each containing specific information. Controlled devices must be represented as children of the control process so that the tree of all processes represents the overall hierarchy.

#### Controllers:
* Control Unit
* Hub

#### Interaction devices:
* Bulb

##### Made by :
* Alessia Marcolini - 194274
* Matteo Slaviero - 193934
* Nicolò Gottardello - 187550

⚠️ WIP
