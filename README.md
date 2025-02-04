# 🔷 HexThello AI - Tournament-Optimized Agent 🔷

## 📌 Overview

HexThello AI is an **intelligent agent** designed to play **HexThello (a hexagonal variant of Othello/Reversi)**.\
This AI is **optimized for tournament play**, using advanced techniques like **Minimax with Alpha-Beta Pruning, dynamic evaluation, and adaptive strategies** for both Black and White.

## 🏆 Key Features

- **Minimax Algorithm** with **Alpha-Beta Pruning** for efficient decision-making.
- **Custom Evaluation Function** that dynamically **adapts to the game phase** (Early, Mid, End).
- **Hexagonal Grid Handling** to ensure **accurate board evaluation**.
- **Move Ordering** to improve pruning efficiency.
- **Separate Strategies for Black & White** for maximum advantage.
- **Corner & Stability Prioritization** to ensure a strong board presence.
- **Parity Control for Endgame** to maximize final move advantage.

---

## 👉 How It Works

### **1️⃣ Board Representation**

The board is stored as a **15×15 2D array**, following a **hexagonal structure**:

- **Some positions are NULL (out-of-bounds)** due to the hexagonal shape.
- **The middle row is the widest**, with fewer tiles in the first and last rows.
- The logic follows **hexagonal mapping**:
  - **Corners are at six key positions**
  - **Edges are slanted, not straight like in square grids**

### **2️⃣ AI Decision-Making Process**

1. **Generates all possible legal moves** (with `countAvailableMoves()`).
2. **Evaluates moves using Minimax with Alpha-Beta Pruning** (avoiding unnecessary searches).
3. **Sorts moves by heuristic value** to **prune weak moves early**.
4. **Chooses the best move and updates the board**.

### **3️⃣ Evaluation Function Components**

- **Piece Difference** → Rewards having more discs than the opponent.
- **Mobility** → Prioritizes having more legal moves.
- **Corner Control** → Rewards capturing **unflippable** hexagonal corners.
- **Edge Stability** → Ensures that AI controls the **outer board areas**.
- **Frontier Control** → Penalizes unstable pieces.
- **Parity (Endgame Control)** → Ensures the AI gets the **last move advantage**.

---

## 🚀 Installation & Setup

### **🔹 Prerequisites**

- **C Compiler (GCC or Clang)**
- **Makefile (Optional)**
- **Linux/Mac/Windows with WSL**

### **🔹 Clone the Repository**

```sh
git clone https://github.com/YOUR_USERNAME/HexThello-AI.git
cd HexThello-AI
```

### **🔹 Compile the Code**

```sh
gcc -o hexthello_ai client.c board.c move.c -lm
```

### **🔹 Run the AI**

```sh
./hexthello_ai
```

---

## 🏆 AI Strategies for Tournament Play

### **📌 As White (Second Player)**

- **Restrict Black’s mobility early** to force bad moves.
- **Force Black to flip near corners**, then take them later.
- **Control endgame parity** to secure the **final move advantage**.

### **📌 As Black (First Player)**

- **Avoid being trapped with no legal moves early**.
- **Prioritize edge and stability before taking aggressive moves**.
- **Maintain move flexibility** while countering White’s aggressive start.

---

## 🛠️ Code Structure

```bash
💚 HexThello-AI/
🌍 client.c        # Main AI agent logic (Minimax, move selection)
🌍 board.c         # Handles board state, legal moves, and game rules
🌍 move.c          # Implements move generation and validation
🌍 board.h         # Defines board-related structures and functions
🌍 move.h          # Defines move-related structures and functions
🌍 global.h        # Global definitions (board size, colors, etc.)
🌍 README.md       # Project documentation (You are here!)
🌍 Makefile        # Compilation instructions (optional)
```

---

## 🛠️ Contributing

We welcome contributions! Follow these steps:

1. **Fork the repository**.
2. **Create a feature branch** (`git checkout -b feature-name`).
3. **Commit your changes** (`git commit -m "Description of changes"`).
4. **Push to your fork** (`git push origin feature-name`).
5. **Create a Pull Request**.

---

## 🌟 License

This project is licensed under the **MIT License**.\
Feel free to use, modify, and contribute to **improve the AI!**

---

## 👥 Credits & Acknowledgments

- **Your Name** - Lead Developer
- **University Name / Organization (if applicable)**
- Special thanks to **OpenAI & HexThello community** for research inspiration!

---

## 📩 Contact

For questions, suggestions, or collaborations, feel free to **open an issue** or reach out via:
📧 **Email:** [your.email@example.com](mailto\:your.email@example.com)\
🗲 **Twitter:** [@yourhandle](https://twitter.com/yourhandle)\
👥 **GitHub:** [Your Profile](https://github.com/YOUR_USERNAME)

```
https://github.com/npapoutsakis
```
