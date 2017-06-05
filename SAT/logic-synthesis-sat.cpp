#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <math.h>
#include <stdlib.h>

#define V +

using namespace std;

int maxDepth, nVars;
vector<int> truthTable;
int n_vars_sat;
int n_clauses;

ofstream cnf;
ifstream sol;

typedef string literal;
typedef string  clause;

literal operator-(const literal& lit) {
    if (lit[0] == '-') return lit.substr(1); 
    else               return "-" + lit;
}

int treeSize(int depth) {
    return pow(2,depth+1)-1;
}

int leftChild(int parent, int maxDepth) {
    return std::min((int) pow(2,maxDepth+1), 2*(parent+1)-1);
}
    
int rightChild(int parent, int maxDepth) {
    return leftChild(parent, maxDepth)+1;
}

void updateInput(int* input, int nVars) {
//We are considering input[0] as the value for 0 signal ==> always 0
    for (int x = nVars; x > 0; x--) {
        if (input[x] == 0) {
            input[x] = 1;
            return;
        } else {
            input[x] = 0;
        }
    }
}

string to_string(int i) {
    string res = "";
    while (i > 0) {
        res.insert(res.begin(), (char) (i%10 + '0'));
        i /= 10;
    }
    return res;
}

literal node(int i, int j) {
    assert(0 <= i and i < treeSize(maxDepth));
    assert(0 <= j and j < (nVars + 3));
    return to_string(i*(nVars + 3) + j + 1) + " ";
}

literal nodRightChild(int i, int j) {
    assert(0 <= i and i <= (int) (treeSize(maxDepth)/2));
    assert(0 <= j and j < (nVars + 3));
    return to_string(rightChild(i,maxDepth)*(nVars + 3) + j + 1) + " ";
}

literal nodLeftChild(int i, int j) {
    assert(0 <= i and i <= (int) (treeSize(maxDepth)/2));
    assert(0 <= j and j < (nVars + 3));
    return to_string(leftChild(i,maxDepth)*(nVars + 3) + j + 1) + " ";
}

literal output(int i, int j) {
    assert(0 <= i and i < treeSize(maxDepth));
    assert(0 <= j and j < pow(2,nVars));
    return to_string(i*pow(2,nVars) + j + (treeSize(maxDepth)*(nVars + 3)) + 1) + " ";
}

literal outRightChild(int i, int j) {
    assert(0 <= i and i <= (int) (treeSize(maxDepth)/2));
    assert(0 <= j and j < pow(2,nVars));
    return to_string(rightChild(i,maxDepth)*pow(2,nVars) + j + (treeSize(maxDepth)*(nVars + 3)) + 1) + " ";
}

literal outLeftChild(int i, int j) {
    assert(0 <= i and i <= (int) (treeSize(maxDepth)/2));
    assert(0 <= j and j < pow(2,nVars));
    return to_string(leftChild(i,maxDepth)*pow(2,nVars) + j + (treeSize(maxDepth)*(nVars + 3)) + 1) + " ";
}

void add_clause(const clause& c) {
  cnf << c << "0" << endl;
  ++n_clauses;
}


void add_amo(const vector<literal>& z) {
  int N = z.size();
  for (int i1 = 0; i1 < N; ++i1)
    for (int i2 = i1+1; i2 < N; ++i2)
      add_clause(-z[i1] V -z[i2]);
}


void write_CNF() {

    n_vars_sat = treeSize(maxDepth)*(nVars + 3) + treeSize(maxDepth)*pow(2,nVars);

    // The first node should be always a NOR gate
    add_clause(node(0,nVars+1));


//    // At most one queen for each descending diagonal.
//    for (int k = 1-n; k <= n-1; ++k) {
//    vector<literal> z;
//    for (int i = max(0, k); i <= min(n-1, n+k-1); ++i) {
//      int j = i - k;
//      z.push_back(x(i,j));
//    }
//    add_amo(z);
//    }

    int input[nVars+1];
    for (int x = 0; x < nVars+1; x++) {
        input[x] = 0;
    }
    
    for (int i = 0; i < (int) pow(2, nVars); i++) {
        for (int n = 0; n < (int) (treeSize(maxDepth)/2); n++) {
            // The two following constraints refer to the NOR behavior of the 
            /* outputs concatenation
            */
            add_clause(output(n,i) V -node(n,nVars+1) V outLeftChild(n,i) V outRightChild(n,i));
            add_clause(-output(n,i) V -node(n,nVars+1) V -outLeftChild(n,i) V -outRightChild(n,i));
            add_clause(-output(n,i) V -node(n,nVars+1) V outLeftChild(n,i) V -outRightChild(n,i));
            add_clause(-output(n,i) V -node(n,nVars+1) V -outLeftChild(n,i) V outRightChild(n,i));
            for (int v = 0; v < nVars+1; ++v) {
                // The two following constraints assign outputs of the nodes 
                /* acting as input vars or 0s with its corresponding value
                */
                if (input[v]) add_clause(-node(n,v) V output(n,i));
                else add_clause(-node(n,v) V -output(n,i));
            }
        }
        for(int n = (int) (treeSize(maxDepth)/2); n < treeSize(maxDepth); n++) {
            for (int v = 0; v < nVars+1; ++v) {
                // The two following constraints assign outputs of the nodes 
                /* acting as input vars or 0s with its corresponding value
                */
                if (input[v]) add_clause(-node(n,v) V output(n,i));
                else add_clause(-node(n,v) V -output(n,i));
            }
        }
        // The output of the first node should equate the truth table
        if (truthTable[i]) add_clause(output(0,i));
        else add_clause(-output(0,i));
        
        updateInput(input, nVars);
    }
    // TODO: OPTIMIZATION?
    // The following expression constraints a the size variable to be equal to
    /* the sum of NOR gates in the circuit
    */
    //vector<literal> sumNORs;
    
    for(int n = 0; n < (int) (treeSize(maxDepth)/2); n++) {
        // The following expression constraints a node to be assigned a unique
        /* value (either NOR gate, input var, 0 constant input or unassigned)
        */
        vector<literal> amoNodeValue;
        literal aloNodeValue = node(n, nVars+1) V node(n, nVars+2);
        amoNodeValue.push_back(node(n,nVars+1));
        amoNodeValue.push_back(node(n,nVars+2));
        for (int v = 0; v < nVars+1; ++v) {
            amoNodeValue.push_back(node(n,v));
            aloNodeValue += node(n,v);
            // The following two constraints require the nodes behind (childs) a
            /* certain node acting as an input var / 0 constant value to be kept
            /* unassigned
            */
            // left child unassigned when var
            add_clause(-node(n,v) V nodLeftChild(n, nVars+2));
            // right child unassigned when var
            add_clause(-node(n,v) V nodRightChild(n, nVars+2));
        }
        // The following two constraints require the nodes value behind (childs) a
        /* certain node acting as unassigned to be kept unassigned
        /* ATTENTION! It solves accuracy problems with case nlsp_4_3_36.inp, but
        /* causes another accuracy problem on case nlsp_4_3_131.inp
        */
        // left child unassigned when unassigned
        add_clause(-node(n, nVars+2) V nodLeftChild(n, nVars+2));
        // right child unassigned when unassigned
        add_clause(-node(n, nVars+2) V nodRightChild(n, nVars+2));
        add_amo(amoNodeValue);
        add_clause(aloNodeValue);
        // The following two constraints require the nodes behind (childs) a
        /* certain node acting as NOR gate to be assigned (NOR/var/0 constant)
        */
        // left child assigned when NOR
        add_clause(-node(n,nVars+1) V -nodLeftChild(n, nVars+2));
        // right child assigned when NOR
        add_clause(-node(n,nVars+1) V -nodRightChild(n, nVars+2));
        // TODO: OPTIMIZATION?
        //sumNORs += node(n, nVars+1);
    }
    
    for(int n = (int) (treeSize(maxDepth)/2); n < treeSize(maxDepth); n++) {
        vector<literal> amoNodeValue;
        literal aloNodeValue = node(n, nVars+1) V node(n, nVars+2);
        amoNodeValue.push_back(node(n,nVars+1));
        amoNodeValue.push_back(node(n,nVars+2));
        for (int v = 0; v < nVars+1; ++v) {
            amoNodeValue.push_back(node(n,v));
            aloNodeValue += node(n,v);
        }
        add_amo(amoNodeValue);
        add_clause(aloNodeValue);
        
        add_clause(-node(n,nVars+1));
        // TODO: OPTIMIZATION?
        //sumNORs += node(n, nVars+1);
    }
    // TODO: OPTIMIZATION?
//    add_clause(sumNORs == size);
//    // The objective function is simply to minimize size
//    add_clause(IloMinimize(env, size));

  cnf << "p cnf " << n_vars_sat << " " << n_clauses << endl;
}


void get_solution(vector<int>& nodes) {
    int lit, i = 0;
    // We are intereseted only in the nodes assignments
    while (sol >> lit and abs(lit) <= (treeSize(maxDepth)*(nVars + 3))) {
        nodes[i] = (lit > 0);
        i++;
    }
//        if (lit > 0 && lit <= (treeSize(maxDepth)*(nVars + 3))) {
//            int i = (lit-1) / (nVars + 3);
//            int j = (lit-1) % (nVars + 3);
//            nodes[i] = j;
//    }
}


void write_solution(const vector<int>& n) {
    // Printing out the input
    cout << maxDepth << endl;
    cout << nVars << endl;
    for (int i = 0; i < truthTable.size(); i++) {
        cout << truthTable[i] << endl;
    }
    // Now the solution, if any
    int size = 0;
    for (int i = 0; i < treeSize(maxDepth); i++)
        if (n[i*(nVars+3)+nVars+1]) size++;
    if (size != 0) {
        cout << size << endl;
        for (int i = 0; i < treeSize(maxDepth); i++) {
            if (n[i*(nVars+3)+nVars+2] == 0) {
                cout << i+1 << " ";
                for (int v = 0; v < nVars+1; ++v) {
                    if (n[i*(nVars+3)+v] == 1)
                        cout << v << " ";
                }
                if (n[i*(nVars+3)+nVars+1] == 1) {
                    cout << -1 << " ";
                    cout << 2*(i+1) << " ";
                    cout << 2*(i+1)+1 << endl;
                } else {
                    cout << 0 << " " << 0 << endl;
                }
            }
        }
    } else cout << -1 << endl;
}


int main() {
    cin >> maxDepth;
    cin >> nVars;
    truthTable = vector<int>((int) pow(2,nVars));
    for (int i = 0; i < truthTable.size(); i++) {
       cin >> truthTable[i];
    }

    cnf.open("tmp.rev");
    write_CNF();
    cnf.close();

    system("tac tmp.rev | lingeling | grep -E -v \"^c\" | tail --lines=+2 | cut --delimiter=' ' --field=1 --complement > tmp.out");

    vector<int> sol_nodes(treeSize(maxDepth)*(nVars + 3));
    sol.open("tmp.out");
    get_solution(sol_nodes);
    sol.close();

    write_solution(sol_nodes);
}
