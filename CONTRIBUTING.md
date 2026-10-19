# Contributing to the Z35 Platform

First off, thank you for considering contributing to the Z35 Environmental Controller! It's people like you that make the open-source community such an amazing place to learn, inspire, and create.

## 📜 Code of Conduct

By participating in this project, you are expected to uphold a welcoming, respectful, and harassment-free environment for everyone.

## 🐛 How to Report a Bug

If you find a bug, please open an **Issue** on GitHub. Before doing so, please check if the issue has already been reported. When creating an issue, please include:
- A clear and descriptive title.
- Steps to reproduce the bug.
- Expected behavior vs. actual behavior.
- Your hardware setup (e.g., ST-Link version, PCB revision).

## 💡 How to Suggest an Enhancement

We welcome new ideas! If you have a suggestion (like a new feature for the Roadmap), please open an **Issue** with the tag `[Feature Request]`. Explain why this feature would be useful for the Z35 platform.

## 💻 How to Submit Code (Pull Requests)

1. **Fork** the repository and create your branch from `main`.
2. **Clone** your fork locally.
3. **Code your changes**. Please adhere to our coding standards (see below).
4. **Test your code** on actual hardware if possible. Ensure it compiles without warnings.
5. **Commit your changes** with a clear commit message (e.g., `feat: add I2C bus recovery` or `fix: resolve timer overflow`).
6. **Push** to your fork and submit a **Pull Request** to the `main` branch.
7. Wait for a review. We might ask for some changes before merging.

## 📏 Coding Standards

To keep the codebase clean and maintainable, please follow these rules:

- **C Standard:** C11 (`-std=gnu11`).
- **Style:** MISRA-C friendly.
- **Formatting:** 4-space indentation (no tabs).
- **Naming:**
  - Variables and functions: `camelCase` or `snake_case`.
  - Macros and constants: `UPPER_SNAKE_CASE`.
- **Documentation:** Use Doxygen-style comments (`/** ... */`) for all public functions and complex logic.
- **Safety:** Always check return values of HAL functions. Avoid dynamic memory allocation (`malloc`/`free`).

### ⚙️ Architecture Guidelines (Please Note)
The Z35 platform consists of two distinct firmware architectures. Please ensure your code matches the target board:
- **Main Board (STM32G431):** Built on **FreeRTOS**. Tasks must be non-blocking. Use `vTaskDelay()` instead of `HAL_Delay()`. Be mindful of task priorities and inter-task communication (Queues/Semaphores).
- **Sensor Board (STM32G030):** **Bare-metal** (No RTOS). The main loop must be highly optimized for power consumption and timing. Use hardware timers for precise delays.

---
*Thank you for helping us build a better Z35.*