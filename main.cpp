//FOWZI HINDI 33587
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <mutex>
#include <vector>
#include <random> // For generating random numbers used in proof-of-work computations
#include <atomic> // For atomic operations, ensuring thread-safe incrementing of shared counters
#include <cmath>  // For mathematical operations, such as calculating the work factor
#include <set>    // For ensuring unique transactions during mining to avoid duplicates


using namespace std;

typedef unsigned int uint;

// Represents a single transaction in the blockchain
struct transaction {
    uint current;           // Current transaction ID
    uint prev;              // Previous transaction hash
    uint rand;              // Random value used for proof of work
    thread::id miner_id;    // ID of the miner thread that mined this transaction
    transaction* next;      // Pointer to the next transaction in the chain
    transaction* previous;  // Pointer to the previous transaction in the chain

    // Constructor to initialize a transaction
    transaction(uint c, uint p, uint r, thread::id id)
        : current(c), prev(p), rand(r), miner_id(id), next(nullptr), previous(nullptr) {}
};

// Represents the entire transaction chain (blockchain)
struct transactionChain {
    transaction* head;      // Pointer to the first transaction in the chain
    transaction* tail;      // Pointer to the last transaction in the chain
    mutex chain_mutex;      // Mutex to ensure thread-safe access to the chain

    transactionChain() : head(nullptr), tail(nullptr) {}

    // Adds a new transaction to the end of the chain
    void add(transaction* new_transaction) {
        lock_guard<mutex> lock(chain_mutex);
        if (!head) {
            head = new_transaction;
            tail = new_transaction;
        } else {
            tail->next = new_transaction;
            new_transaction->previous = tail;
            tail = new_transaction;
        }
    }
};

// Computes a hash for a given transaction based on its attributes
uint hashTransaction(const transaction& tran) {
    uint result = (tran.current << 16) ^ (tran.prev << 8) ^ (tran.rand << 1);
    hash<uint> uint_hash;
    return uint_hash(result);
}

// Validates the entire transaction chain
bool transactionValidator(transactionChain &tChain, uint threshold) {
    lock_guard<mutex> lock(tChain.chain_mutex);
    if (!tChain.head) return true; // An empty chain is considered valid

    transaction* current = tChain.head->next;
    transaction* previous = tChain.head;

    while (current != nullptr) {
        // Check if the transaction hash exceeds the given threshold
        if (hashTransaction(*current) > threshold) {
            return false;
        }
        // Ensure the chain is properly linked
        if (current->previous != previous) {
            return false;
        }
        previous = current;
        current = current->next;
    }
    return true;
}

// Represents the mining process for a single thread
void mine(transactionChain& tChain, const vector<uint>& transaction_ids, uint threshold, mutex& print_mutex,
          atomic<bool>& start_flag, atomic<int>& current_transaction_index, mutex& chain_add_mutex,
          vector<pair<thread::id, uint>>& miner_bitcoins, uint difficulty) {
    uint bitcoins = 0; // Count of bitcoins mined by this thread
    thread::id tid = this_thread::get_id();

    // Generate a numeric representation for the thread ID
    uint thread_numeric_id = static_cast<uint>(hash<thread::id>()(tid) % 50000 + 10000);

    mt19937 rng(hash<thread::id>()(tid));
    // Random number generator for proof of work, `mt19937` is ideal for the mining simulation as it provides fast, uniform random number generation
    uniform_int_distribution<uint> dist(0, UINT_MAX);

    auto work_factor = pow(10, difficulty - 1); // Adjust mining difficulty based on user input
    set<uint> processed_transactions;          // Keep track of transactions already processed by this thread

    while (!start_flag); // Wait until mining starts

    while (true) {
        int index = current_transaction_index.fetch_add(1);
        if (index >= transaction_ids.size()) {
            break; // Stop mining when all transactions are processed
        }

        uint current_id = transaction_ids[index];
        uint prev_hash = 0;
        {
            lock_guard<mutex> lock(tChain.chain_mutex);
            if (tChain.tail) {
                prev_hash = hashTransaction(*tChain.tail); // Get the hash of the last transaction
            }
        }

        while (true) {
            uint random_value = dist(rng);
            uint hashValue = (current_id << 16) ^ (prev_hash << 8) ^ (random_value << 1);

            if (hashValue <= threshold) {
                for (uint i = 0; i < work_factor; ++i) {
                    random_value = dist(rng); // Simulate extra computational work
                }

                {
                    lock_guard<mutex> lock(chain_add_mutex);
                    hashValue = (current_id << 16) ^ (prev_hash << 8) ^ (random_value << 1);
                    if (hashValue > threshold) {
                        continue; // Ignore invalid hashes
                    }

                    // Avoid duplicate transactions
                    if (processed_transactions.find(current_id) != processed_transactions.end()) {
                        break;
                    }

                    // Add the new transaction to the chain
                    processed_transactions.insert(current_id);
                    transaction* new_tran_ptr = new transaction(current_id, prev_hash, random_value, tid);
                    tChain.add(new_tran_ptr);
                    bitcoins++;
                    break;
                }
            }
        }
    }

    {
        lock_guard<mutex> lock(print_mutex);
        cout << "Thread " << thread_numeric_id << " has " << bitcoins << " bitcoin(s)" << endl;
    }

    // Update the miner's total bitcoin count
    for (auto& miner_info : miner_bitcoins) {
        if (miner_info.first == tid) {
            miner_info.second = bitcoins;
            return;
        }
    }
}

int main() {
    int miners; // Number of mining threads
    uint difficulty; // Difficulty level for mining
    string filename; // Name of the input file containing transaction IDs

    // Get user input for difficulty level
    cout << "Enter difficulty level (1-10): ";
    while (!(cin >> difficulty) || difficulty < 1 || difficulty > 10) {
        cout << "Invalid input. Please enter a difficulty level between 1 and 10: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    uint threshold = (1LL << (32 - difficulty)) - 1; // Compute mining threshold
    cout << "Threshold: " << threshold << endl;

    // Get user input for the filename
    cout << "Enter the filename of the input file: ";
    cin.ignore();
    getline(cin, filename);
    if (filename.empty()) {
        cerr << "Error: Filename cannot be empty." << endl;
        return 1;
    }

    // Get user input for the number of miners
    cout << "Enter the number of miners: ";
    while (!(cin >> miners) || miners <= 0) {
        cout << "Invalid input. Please enter a positive number of miners: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "----------START----------" << endl;

    // Read transaction IDs from the input file
    ifstream inputFile(filename);
    if (!inputFile.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }

    int num_transactions;
    if (!(inputFile >> num_transactions) || num_transactions <= 0) {
        cerr << "Error reading number of transactions from file or invalid value." << endl;
        return 1;
    }

    vector<uint> transaction_ids(num_transactions);
    for (int i = 0; i < num_transactions; ++i) {
        if (!(inputFile >> transaction_ids[i])) {
            cerr << "Error reading transaction ID from file." << endl;
            return 1;
        }
    }

    if (inputFile.peek() != EOF) {
        cerr << "Error: File contains more transaction IDs than specified." << endl;
        return 1;
    }

    inputFile.close();

    // Initialize the transaction chain with a genesis block
    transactionChain tChain;
    transaction* genesis_block = new transaction(0, 0, 0, this_thread::get_id());
    tChain.add(genesis_block);

    vector<thread> threads;
    mutex print_mutex;
    atomic<bool> start_flag = false;
    atomic<int> current_transaction_index = 0;
    mutex chain_add_mutex;
    vector<pair<thread::id, uint>> miner_bitcoins(miners);

    // Create miner threads
    for (int i = 0; i < miners; ++i) {
        threads.emplace_back(mine, ref(tChain), cref(transaction_ids), threshold, ref(print_mutex), ref(start_flag), ref(current_transaction_index), ref(chain_add_mutex), ref(miner_bitcoins), difficulty);
    }

    for (int i = 0; i < miners; ++i) {
        miner_bitcoins[i].first = threads[i].get_id();
        miner_bitcoins[i].second = 0;
    }

    start_flag = true; // Signal miners to start

    // Wait for all miners to finish
    for (auto& th : threads) {
        th.join();
    }

    // Validate the transaction chain
    bool isValid = transactionValidator(tChain, threshold);
    cout << "\nThe transaction chain is " << (isValid ? "valid" : "invalid") << endl;

    return 0;
}
