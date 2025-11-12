# T.U.I.L.E

Which stands for:

- **T**ext-based
- **U**ser
- **I**nterface
- **L**ayout
- **E**ngine

T.U.I.L.E is a project that aims to:

1. Make life easier for people who mainly use the Linux terminal.
2. Provide a simple User Interface in an SSH terminal.
3. Be lightweight enough to be used as a substitute for any graphical user interface.

---

# Installation

### 1. Clone the repository

```bash
git clone https://github.com/EmileF67/Tuile.git
cd Tuile
```

### 2. Install dependencies

The main library used in this project is **ncurses**.

* **Debian / Ubuntu**:

```bash
sudo apt update && apt upgrade
sudo apt install libncurses-dev
```

* **Arch-based systems**:

```bash
sudo pacman -Syu
sudo pacman -S ncurses
```

### 3. Install icons

The icons used in this project are from the Noto Color Font Emoji

* **Debian / Ubuntu**:

```bash
sudo apt install fonts-noto-color-emoji
```

* **Arch-based systems**:

```bash
sudo pacman -S noto-fonts-emoji
```



### 4. Create the build directory

```bash
mkdir -p build
```

### 5. Compile the project

```bash
make
```

The compiled executable will be located at `Tuile/build/main`.

---

# Running T.U.I.L.E

```bash
./build/main
```

Enjoy your text-based UI in the terminal!

---

# Notes

* The project is designed for **Linux terminals** and may not work correctly on Windows without WSL or a compatible terminal emulator.

---
