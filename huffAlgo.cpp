#include <iostream>
#include <queue>
#include <unordered_map>
#include <string>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <filesystem>
using namespace std;

//node for huffman tree
struct Node{
    char ch;
    int freq;
    Node *left, *right;

    Node(char c, int f){
        ch = c;
        freq = f;
        left = right = nullptr;
    }
};

//comparator for priority queue to make it a min heap
struct Compare{
    bool operator()(Node* a, Node* b){
        return a->freq > b->freq;
    }
};

//building Huffman Tree
Node* buildTree(const string& text){
    unordered_map<char, int> freq; // stores elements as key value pairs with unique keys, freq can be used as object

    //Counts freqs
    for (char ch : text){ //uses val of ch for loop and traverses text
        freq[ch]++;//added to freq map
    }

    //Min heap
    priority_queue<Node*, vector<Node*>, Compare> pq;

    //create leaf nodes
    for (auto pair : freq){
        pq.push(new Node(pair.first, pair.second));
    }

    // Building trees
    while (pq.size() > 1){
        //sets up nodes via queue
        Node* left = pq.top(); pq.pop();
        Node* right = pq.top(); pq.pop();


        Node* parent = new Node ('\0', left->freq + right->freq);
        //iterates down the queue pushing new nodes
        parent->left = left;
        parent->right = right;

        pq.push(parent);//root
    }

    return pq.top();
}

void generateCodes(Node* root, string code, unordered_map<char, string>& huffmanCode){
    if (!root) return;

    //leaf Node
    if(!root->left && !root->right){
        huffmanCode[root->ch] = code;
    }

    //recursively iteraites through the tree creating codes for the node traversal
    generateCodes(root->left, code + "0", huffmanCode);
    generateCodes(root->right, code + "1", huffmanCode);    
}



//frees up memory
void deleteTree(Node* root){
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}




//new
//recursive function that writes a 0 or 1 to all nodes
void serializeTree(Node* root, ofstream& fout){
    if (!root) //if the node doesnt exist it returns
        return;
    if (!root -> left && !root -> right){
        fout.put('1');  //leaf marker
        fout.put(root -> ch); // the char its self
    } else {
        fout.put('0');
        serializeTree(root->left, fout);
        serializeTree(root->right, fout);
    }
}

//recursive function to get the chars back
Node* deserializeTree(ifstream& fin){
    char marker;
    fin.get(marker);

    if (marker == '1'){
        char ch;
        fin.get(ch);
        return new Node(ch, 0);
    }

    Node* node = new Node('\0', 0);
    node->left = deserializeTree(fin);
    node->right = deserializeTree(fin);

    return node;
}

//allows creation of file and converts 1's & 0's into bytes
class BitWriter{
    ofstream& out;
    unsigned char buffer;
    int bitCount;

    public:
        BitWriter(ofstream& o) : out(o), buffer(0), bitCount(0) {}

        void writeBit(bool bit){
            buffer <<= 1; // left shifts by one
            if (bit) buffer |= 1; //runs bit wise or function
                bitCount++;
            //outputs byte and resets counters
            if (bitCount == 8){
                out.put(buffer);
                buffer = 0;
                bitCount = 0;
            }
        }
        //Left shifts buffer by 8-bitCount and adds it to the file
        void flush(){
            if (bitCount > 0){
                buffer <<= (8-bitCount);
                out.put(buffer);
            }
        }
};

//allows reading of a file and taking in the 0's and 1's
class BitReader{
    ifstream& fin;
    unsigned char buffer;
    int bitCount;

    public:
        BitReader(ifstream& i) : fin(i), buffer(0), bitCount(0) {}

        //
        bool readBit(bool& bit){
            //when the the entire byte has been read the count will be reset
            if (bitCount == 0){
                char byte;
                if (!fin.get(byte)) 
                    return false;
                buffer = static_cast<unsigned char>(byte);
                bitCount = 8;
            }
            //takes in a bit
            bit = (buffer & 0x80);
            buffer <<= 1;
            bitCount--;
            return true;
        }
};

// Compresses the input file to the output file using Huffman coding
void compressToFile(const string& inputFile, const string& outputFile){
    ifstream fin(inputFile);
    stringstream buffer;

    //if it cannot be opened an errror will be thrown
    if (!fin){
        throw runtime_error("Cannot open file " + inputFile); 
    }

    buffer << fin.rdbuf();
    string text = buffer.str();

    Node* root = buildTree(text);

    unordered_map<char, string> codes;
    generateCodes(root, "", codes);

    ofstream out(outputFile, ios::binary);

    //writes to file the original size of the string
    int size = text.size();
    out.write((char*)&size, sizeof(size));

    //writes tree
    serializeTree(root, out);

    //writes bit packed encoded data 
    BitWriter writer(out);

    for (char ch : text){
        for (char bit : codes[ch]){
            writer.writeBit(bit=='1');
        }
    }

    writer.flush();
    deleteTree(root);
}

// Decompresses the input file to the output file using Huffman coding
void decompressFromFile(const string& inputFile, const string& outputFile){
    ifstream fin(inputFile, ios::binary);

    //read orignial size
    int size;
    fin.read((char*)&size, sizeof(size));

    //rebuilding the tree from the file
    Node* root = deserializeTree(fin);

    ofstream fout(outputFile);

    if(!fout){
        throw runtime_error("Could not create decoded file.");
    }

    BitReader reader(fin);
    Node* curr = root;

    int decodedCount = 0;
    bool bit;

    while (decodedCount < size && reader.readBit(bit)){
        if (bit)
            curr = curr ->right;
        else
            curr = curr->left;
        
        if (!curr->left && !curr->right){
            fout.put(curr->ch);
            curr = root;
            decodedCount++;
        }
    }

    deleteTree(root);
}

int main(){
    //These strings will store the inputted filename and the .txt file will be inserted into the variable
    string fileName = "";

    cout << "Enter file to compress: ";
    getline(cin, fileName);
    
    //if the the doesn't exist or cannot be opened an error will be caught, printed out, 
    //and then the program will restart
    try{
        compressToFile(fileName, "output.huff");
    }
    catch(const runtime_error& e){
        cerr << "Caught Error: " << e.what() << std::endl;
        main();
    }
    
    cout << "Compression complete. Output written to output.huff" << endl;

    //attempts to write new files with the results of the encoding and decoding process
    //if it doesnt work it will catch the error and exit the program
    try{
        decompressFromFile("output.huff", "recovered.txt");
    }catch(const runtime_error& e){
        cerr << "File Write Error: " << e.what() << endl;
        return 1;
    }

    cout << "Decompression complete. Output written to recovered.txt" << endl;

    return 0;
    
}