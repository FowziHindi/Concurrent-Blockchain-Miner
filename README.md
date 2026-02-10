# Concurrent-Blockchain-Miner

### Project Overview
This project is a high-performance C++ simulation of a Bitcoin mining network. It demonstrates the implementation of multithreading and synchronization primitives to manage a shared transaction chain (blockchain) among multiple concurrent worker threads.

### Technical Highlights
* Concurrency Management: Utilized the std::thread library to spawn multiple "miner" threads that execute simultaneously to solve cryptographic puzzles.
* Thread Safety: Implemented std::mutex to protect the shared linked-list data structure from race conditions during concurrent transaction insertions.
* Proof-of-Work Logic: Developed a brute-force mining algorithm where threads compete to find a nonce that, when hashed with block data, meets a specific difficulty target.
* Resource Efficiency: Leveraged this_thread::yield() to manage thread scheduling and ensure balanced CPU utilization during high-intensity mining cycles.
* Dynamic Data Structures: Engineered a custom linked-list to serve as the transaction chain, ensuring data integrity and persistence across all execution threads.

### Implementation Details
* Language: C++
* Concurrency Model: C++ Standard Threading (pthreads-based)
* Synchronization: Mutex (Locking/Unlocking)
* Data Structure: Singly Linked List
* Search Strategy: Brute-force Nonce Discovery

### Input Data Specification
The simulation processes transaction data from text files using a specific format:
* The first line must contain an integer representing the total number of transactions.
* Each subsequent line must contain exactly one transaction string.

The following test files are included in the input_files/ directory:
* in30.txt: A baseline dataset containing 30 transactions.
* in100.txt: A larger dataset containing 100 transactions for stress-testing multithreaded performance.

### How to Run

1.  **Compile the source**:
 
   ```bash
   g++ source.cpp -o MinerSim -lpthread
  ```
2. **Run the program**:
   
Linux / macOS: ./MinerSim

Windows: MinerSim.exe

3. **Usage**:

When prompted, enter the path to the transaction file (e.g., input_files/in30.txt) and the number of miner threads to simulate.
