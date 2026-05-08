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

//Encode text
string encode(const string& text, unordered_map<char, string>& huffmanCode){
    string encoded = "";
    for (char ch : text){
        encoded += huffmanCode[ch];
    }
    return encoded;
}

//decode Text
string decode(Node* root, const string& encoded){
    string decoded = "";
    Node* curr = root;

    //for each char in encoded it will be set to bit
    for (char bit : encoded){
        if (bit == '0')
            curr = curr->left; //traverses tree
        else
            curr = curr->right;

        //leaf node
        if(!curr->left && !curr->right){
            decoded += curr->ch;//adds character to string
            curr = root;//resets pointer
        }
    }
    return decoded;
}

//frees up memory
void deleteTree(Node* root){
    if (!root) return;
    deleteTree(root->left);
    deleteTree(root->right);
    delete root;
}

void fillTable(string& fileName, string& text){
    cout << "What is the name of file you would like to compress?\n";
    getline(cin, fileName);
    //creates an infsteam object called fin with the inputted filename
    //as the file thats searched for
    ifstream fin(fileName);

    //if it cannot be opened an errror will be thrown
    if (!fin){
        throw runtime_error("Cannot open file " + fileName); 
    }

    //reads the entire file and places the text into a string
    stringstream buffer;
    buffer << fin.rdbuf();
    text = buffer.str();  
}

//takes in the string and removes the .txt part
string getBaseFileName(const string& path){
    size_t dot = path.find_last_of('.');
    //size_t slash = path.find_last_of('/');

    if (dot == string::npos)
        return path;
    //if (slash == string::npos)
    //    return path;
    return path.substr(0, dot);
}

// writes encoded binary string to a file
void writeEncoded(const string& filename, const string& encoded){
    //ofsteam allows exporting to a file
    //this file is created near the inputted file
    ofstream fout(filename);
    
    if (!fout){
        throw runtime_error("Could not create encoded file.");
    }

    fout << encoded;

    fout.close();

    cout << "\nEncoded data written to: " << filename << endl;
}

// writes dencoded binary string to a file
void writeDecoded(const string&filename, const string& decoded){
    //ofsteam allows exporting to a file
    //this file is created near the inputted file
    ofstream fout(filename);

    if(!fout){
        throw runtime_error("Could not create decoded file.");
    }

    fout << decoded;

    fout.close();

    cout << "Decoded data written to: " << filename << endl;
}

int main(){
    //These strings will store the inputted filename and the .txt file will be inserted into the variable
    string fileName = "", text = "";
    
    //if the the doesn't exist or cannot be opened an error will be caught, printed out, 
    //and then the program will restart
    try{
        fillTable(fileName, text);
    }
    catch(const runtime_error& e){
        cerr << "Caught Error: " << e.what() << std::endl;
        main();
    }

    //builds tree
    Node* root = buildTree(text);

    //generates codes
    unordered_map<char, string> huffmanCode;
    generateCodes(root, "", huffmanCode);

    //prints huffman codes next to the corresponding char
    cout << "\nHuffman Codes:\n";
    for (auto pair : huffmanCode){
        cout << pair.first << ": " << pair.second << endl;
    }

    // Encodes the text and puts it into the encoded str
    string encoded = encode(text, huffmanCode);

    cout << "\nEncoded;\n" << encoded << endl;

    //decodes the text and puts the output into decoded
    string decoded = decode(root, encoded);

    cout << "\nDecoded:\n" << decoded << endl;

    //creates output filenames
    string baseName = getBaseFileName(fileName);
    string encodedFile = baseName + "_encoded.txt";
    string decodedFile = baseName + "_decoded.txt";

    //attempts to write new files with the results of the encoding and decoding process
    //if it doesnt work it will catch the error and exit the program
    try{
        writeEncoded(encodedFile, encoded);
        writeDecoded(decodedFile, decoded);
    }catch(const runtime_error& e){
        cerr << "File Write Error: " << e.what() << endl;
    }

    deleteTree(root);
    return 0;
    
}