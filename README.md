# Go-Back-N Protocol

A C++ simulation program that models data link layer flow control and error handling mechanisms. This project provides a configurable simulation engine to compare the behavior of **Go-Back-N (GBN)** and **Selective-Repeat-like (Buffered Receiver)** sliding window protocols under varying network constraints and channel conditions.

---

## Features

- **Dynamic Sliding Window Engine:** Supports a runtime-configurable window size relative to a maximum circular sequence window space ($MAX\_SEQ = 7$).
- **Dual Protocols/Scenarios Implementation:**
  1. **Go-Back-N (Unbuffered Receiver):** Enforces strict in-order acceptance using cumulative ACKs. If a packet drops or times out, the entire sliding window pipeline is retransmitted.
  2. **Selective-Repeat-like Architecture (Buffered Receiver):** Allows out-of-order buffering within window boundaries and utilizes individual ACKs / negative acknowledgments (NACKs) to pinpoint missing frames.
- **Error & Corruption Modeling:** Implements circular sequence validation routines, dynamic packet tracking, and runtime stochastic checksum checking to simulate packet corruption.
- **Interactive Network Steps:** Allows step-by-step control of current time tick increments, manual frame delivery arrival entries, and specific feedback simulations.

---

## Technical Specifications & Constants

The underlying codebase adheres to standard sliding window mechanics defined by these fixed criteria:

- `MAX_SEQ`: Defined as `7`, establishing sequence wrap-around paths ($0, 1, 2, 3, 4, 5, 6, 7, 0, 1, \dots$).
- `N_SEQ`: Total unique sequence identifiers allocated (`MAX_SEQ + 1 = 8`).
- `TIMEOUT`: Fixed at `3` time ticks to monitor outstanding unacknowledged frames before triggering a bulk resend event (for unbuffered configurations).
- **Mathematical Bound Checkers:** Circular verification logic handled by a specialized boundary function.

---

## System Architecture & Components

### 1. Structure Definitions & Functions
* **`Frame` Struct:** Holds individual packet payloads, identifying sequence headers (`seq`), packet payload payload data strings (`data`), and computed integers (`checksum`).
* **`compute_checksum()`:** Adds frame sequences and character metrics together to evaluate the payload validity upon arrival.
* **`is_corrupted()`:** Introduces a random $20\%$ chance of checksum corruption to test receiver resiliency.

### 2. Receiver Class
Manages independent receiver states, tracking expected incoming frames through specific configurations:
* **Unbuffered Node Logic:** Compares incoming sequence numbers with `frame_expected`. If matching, the window slides by incrementing the expected index; otherwise, out-of-order frames are dropped.
* **Buffered Node Logic:** Accepts any out-of-order frames that fall safely within the active tracking window ($[\text{expected}, \text{expected} + \text{win\_size})$) and buffers them inside a map. The receipt boundary window automatically slides forward as soon as missing sequential packets arrive.
