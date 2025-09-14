#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

// Data structure for a node in the Dancing Links algorithm
struct Node {
    Node *left, *right, *up, *down;
    int rowID, colID;
    int nodeCount; // Number of 1s in the column (for header nodes)
    Node* colHeader;     // Pointer to the column header (for data nodes)

    Node(int r = -1, int c = -1) : rowID(r), colID(c), nodeCount(0) {
        left = right = up = down = colHeader = this; // Initialize pointers to self
    }
};

// Function to link two nodes horizontally
void linkHorizontal(Node* leftNode, Node* rightNode) {
    leftNode->right = rightNode;
    rightNode->left = leftNode;
}

// Function to link two nodes vertically
void linkVertical(Node* upperNode, Node* lowerNode) {
    upperNode->down = lowerNode;
    lowerNode->up = upperNode;
}

// Function to create linked list of header nodes
vector<Node*> createHeaderList(int numColumns) {
    vector<Node*> headers(numColumns);
    for (int i = 0; i < numColumns; ++i) {
        headers[i] = new Node(-1, i); // Header nodes have rowID -1
    }
    // Link header nodes in a circular doubly linked list
    for (int i = 0; i < numColumns; ++i) {
        linkHorizontal(headers[i], headers[(i + 1) % numColumns]);
    }
    return headers;
}

void createNodes(const vector<string>& data, const vector<Node*>& headers) {
    int numRows = data.size();
    if (numRows == 0) return;
    int numColumns = headers.size();

    // Iterate over each row definition
    for (int r = 0; r < numRows; ++r) {
        Node* rowHead = nullptr; // This will be the first node in the current row

        // Iterate over each column for the current row
        for (int c = 0; c < numColumns; ++c) {
            // Check if a node should exist at this position (r, c)
            if (data[r][2 * c] == '1') { // Assuming space-separated values
                // 1. Create the new node
                Node* newNode = new Node(r, c);
                newNode->colHeader = headers[c];

                // 2. Link the new node vertically into its column list
                // It's inserted right above the column header
                Node* lastNodeInCol = headers[c]->up;
                lastNodeInCol->down = newNode;
                newNode->up = lastNodeInCol;
                newNode->down = headers[c];
                headers[c]->up = newNode;
                headers[c]->nodeCount++;

                // 3. Link the new node horizontally into its row list
                if (rowHead == nullptr) {
                    // This is the first node in the row
                    rowHead = newNode;
                    // A single-node list points to itself
                    newNode->left = newNode;
                    newNode->right = newNode;
                } else {
                    // This is a subsequent node, insert it just before the row head
                    Node* lastNodeInRow = rowHead->left;
                    lastNodeInRow->right = newNode;
                    newNode->left = lastNodeInRow;
                    newNode->right = rowHead;
                    rowHead->left = newNode;
                }
            }
        }
    }
}


// Function for inserting a node into an ordered vector based on rowID
void insert(vector<Node*>& vec, Node* newNode) {
    auto it = vec.begin();
    while (it != vec.end() && (*it)->rowID <= newNode->rowID) {
        ++it;
    }
    vec.insert(it, newNode);
}

// Function for printing a row
void printRow(Node* rowNode, vector<int>& activeColumns) {
    Node* current = rowNode;
    for (int c : activeColumns) {
        if (current->colID == c) {
            std::cout << "1 ";
            current = current->right;
        } else {
            cout << "0 ";
        }
    }
}

// Function for printing the matrix as seen in example.txt (for debugging)
void printMatrix(Node* h) {
    if (h->right == h) {
        cout << "Matrix is empty." << endl;
        return;
    }
    Node* header = h->right; // First column header
    vector<Node*> queue; // Queue with one node per row, ordered by rowID
    vector<int> activeColumns;
    // Loop through all column headers
    for (Node* col = header; ; col = col->right) {
        activeColumns.push_back(col->colID);
        for (Node* row = col->down; ; row = row->down) {
            insert(queue, row);
            if (row->rowID >= row->down->rowID) break; // Stop if we've looped back to the header
        }
        if (col->right == h) break; // Stop if we've looped back to the header
    }
    
    int previousRowID = -1;
    int index = 0;
    for (Node* n : queue) {
        if (n->rowID == previousRowID) continue; // Skip duplicate rows

        previousRowID = n->rowID;
        printRow(n, activeColumns);
        // cout << "Node (rowID, colID): (" << n->rowID << ", " << n->colID << ")";
        cout << endl;
    }
}

// Function for vertical unlinking of a node
void unlinkVertically(Node* node) {
    node->up->down = node->down;
    node->down->up = node->up;
}

// Function for horizontal unlinking of a node
void unlinkHorizontally(Node* node) {
    node->left->right = node->right;
    node->right->left = node->left;
}

// Function for covering column
void cover(Node* col) {
    // unlink column header horizontally
    unlinkHorizontally(col);

    // for each row in this column
    for (Node* row = col->down; row != col; row = row->down) {
        // for each node in that row
        for (Node* n = row->right; n != row; n = n->right) {
            unlinkVertically(n);
            n->colHeader->nodeCount--; // Decrement the count of 1s in the column
        }
    }
}

// Function for uncovering column
void uncover(Node* col) {
    // relink column header horizontally
    col->left->right = col;
    col->right->left = col;

    // reverse of cover: relink nodes vertically
    for (Node* row = col->up; row != col; row = row->up) {
        for (Node* n = row->left; n != row; n = n->left) {
            n->colHeader->nodeCount++; // Increment the count of 1s in the column
            n->up->down = n;
            n->down->up = n;
        }
    }
}

// Function for the dancing links algorithm
void solve(Node* h, vector<int>& solution, int& numSolutions, bool printSolutions) {
    if (h->right == h) {
        // cout << "Solution found: ";
        numSolutions++;
        if (printSolutions) {
            for (size_t i = 0; i < solution.size(); i++) {
                cout << solution[i];
                if (i + 1 < solution.size()) cout << " ";
            }
            cout << "\n";
        }
        return;
    }

    // Choose column with the smallest nodeCount
    Node* col = h->right;
    for (Node* c = h->right; c != h; c = c->right) {
        if (c->nodeCount < col->nodeCount) {
            col = c;
        }
    }

    cover(col);

    for (Node* row = col->down; row != col; row = row->down) {
        solution.push_back(row->rowID);
        for (Node* n = row->right; n != row; n = n->right) {
            cover(n->colHeader);
        }

        solve(h, solution, numSolutions, printSolutions);
        solution.pop_back();
        
        for (Node* n = row->left; n != row; n = n->left) {
            uncover(n->colHeader);
        }
    }

    uncover(col);
}

void countSolutions(Node* h, int& numSolutions) {
    if (h->right == h) {
        // cout << "Solution found: ";
        numSolutions++;
        return;
    }

    // Choose column with the smallest nodeCount
    Node* col = h->right;
    for (Node* c = h->right; c != h; c = c->right) {
        if (c->nodeCount < col->nodeCount) {
            col = c;
        }
    }

    cover(col);

    for (Node* row = col->down; row != col; row = row->down) {
        for (Node* n = row->right; n != row; n = n->right) {
            cover(n->colHeader);
        }

        countSolutions(h, numSolutions);
        
        for (Node* n = row->left; n != row; n = n->left) {
            uncover(n->colHeader);
        }
    }

    uncover(col);
}

// Function for converting a string of space separated '0's and '1's, which represent bits, to an integer
int binaryStringToInt(const string& binaryStr) {
    int result = 0;
    size_t pos = 0, prev = 0;
    while ((pos = binaryStr.find(' ', prev)) != string::npos) {
        result = (result << 1) | (binaryStr[prev] - '0');
        prev = pos + 1;
    }
    // Process the last bit
    if (prev < binaryStr.size()) {
        result = (result << 1) | (binaryStr[prev] - '0');
    }
    return result;
}

// Function for reading and parsing a text file
vector<string> readFile(const string& filePath) {
    ifstream file(filePath);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filePath << endl;
        return {};
    }

    // Read first line to get the number of columns
    int numColumns;
    file >> numColumns;
    // cout << "Number of columns: " << numColumns << endl;
    file.ignore(); // Ignore the newline character after the number

    // Read the rest of the file line by line, storing them as integers in a vector
    vector<string> data;
    string line;
    while (getline(file, line)) {
        // Process each line
        // The first line is a number indicating the number of columns
        // The subsequent lines contain space-separated binary values (0s and 1s)
        // which should be read as integer bits.
        data.push_back(line);
    }

    file.close();
    data.push_back(to_string(numColumns)); // Remove the last element if it was added by mistake
    return data;
}

void printColumnCounts(Node* h) {
    for (Node* c = h->right; c != h; c = c->right) {
        cout << "Col " << c->colID << " count=" << c->nodeCount << endl;
    }
}



int main(int argc, char* argv[]) {
    // cout << "Dancing links!" << endl;
    
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <input_file>" << endl;
        return 1;
    }

    string filePath;
    bool printSolutions = false;
    bool countOnly = false;

    // Parse args: e.g. ./dlx file.txt -p
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-p") {
            printSolutions = true;
        } else if (arg == "-c") {
            countOnly = true;
        } else {
            filePath = arg;
        }
    }

    vector<string> data = readFile(filePath);
    int numSolutions = 0;

    int numColumns = stoi(data.back()); // Last element is the number of columns
    data.pop_back(); // Remove the last element as it's not part of the matrix
    int numRows = data.size();

    // for (string& row : data) {
    //     cout << row << endl;
    // }
    // cout << "Number of rows: " << numRows << endl;

    Node* h = new Node(-1, -1); // Master header node
    linkHorizontal(h, h); // Initialize circular linking

    vector<Node*> headers = createHeaderList(numColumns);
    linkHorizontal(h, headers[0]);
    linkHorizontal(headers.back(), h);
    createNodes(data, headers);
    // printMatrix(h);
    // Node* temp = h->right;
    // cover(temp); // Example of covering the first column
    // cout << "After covering first column:" << endl;
    // printMatrix(h);
    // uncover(temp); // Uncovering it back
    // cout << "After uncovering first column:" << endl;
    // printMatrix(h);

    vector<int> solution;
    if (countOnly) {
        countSolutions(h, numSolutions);
        cout << "Total number of solutions found: " << numSolutions << endl;
        return 0;
    } else {
        cout << "Finding all solutions..." << endl;
        solve(h, solution, numSolutions, printSolutions);
    }
    cout << "Total number of solutions found: " << numSolutions << endl;
    return 0;
}


