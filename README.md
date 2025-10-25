# nxReplayer  
TAS Emulator for S.T.A.L.K.E.R.: Shadow of Chernobyl 

`nxReplayer` is a minimalist command-line utility that enables **Tool-Assisted Speedruns (TAS)** through scripted event replay in *S.T.A.L.K.E.R.: Shadow of Chernobyl*.  
It provides a simple scripting interface for automating in-game actions, timing sequences, and replays.

---

## ⚙️ Features

- Event scripting for player actions  
- Support for custom binds (`binds.txt`)  
- Load & run replay scripts (`main.txt`)  
- Minimal CLI interface for automation  
- Supports SoC versions **1.0000** and **1.0006**

---

## 🧩 Available Script Actions

| **Toggles** | **Triggers** | **Functions** |
|--------------|--------------|----------------|
| crouch | inventory | `wait <ms>` |
| moveleft | medkit | `yawspeed <value>` |
| moveright | bandage | `pitchspeed <value>` |
| forward | sprint | `run <fileName>` |
| back | drop |  |
| walk | savegame |  |
| left | loadgame |  |
| right | jump |  |
| up | knife |  |
| down | pistol |  |
| leanleft | rifle |  |
| leanright | nade |  |
| fire | binoc |  |
| zoom | bolt |  |
|  | reload |  |
|  | type |  |
|  | launcher |  |
|  | use |  |
|  | pause |  |
|  | escape |  |
|  | firemode |  |
|  | timeaccel |  |
|  | timedeccel |  |

---

## 🗂️ `binds.txt` Notes

A **default `binds.txt`** is provided for players using custom in-game keybinds.  
⚠️ **Do not**:
- change the order of the binds assignment
- map the same action twice
- map the same key twice

**Otherwise: default binds will be loaded instead**


---

## 💻 CLI Commands

| Command | Description |
|----------|-------------|
| `load` | Loads the script from `./scripts/main.txt` |
| `script` | Prints the loaded script (unconverted) |
| `data` | Prints the integer array representation of the loaded script |
| `run` | Runs `main.txt` (auto-loads before execution) |
| `start` | Like `run`, but begins executing your script when game reaches client sychronization stage |
| `menu` | Prints the main menu |
| `exit` | Exits the program |

---

## 🧾 Example Script (`main.txt`)

```text
wait 50;+forward;sprint;wait 400;run anotherfile;wait 200;-forward
