#include <ilcplex/ilocplex.h>
#include <vector>
ILOSTLBEGIN

const int bigM = 10000;
const int DEBUG = 0;
int maxDepth, nVars;

IloNumVarArray nodes;
IloNumVar size;
IloNumVarArray outputs;
//IloNumVarArray isNOR;

int leftChild(int parent, int maxDepth) {
    return std::min((int) pow(2,maxDepth+1), 2*(parent+1)-1);
}
    
int rightChild(int parent, int maxDepth) {
    return leftChild(parent, maxDepth)+1;
}

IloNumVar out(int n, int i) { return outputs[n*pow(2,nVars)+i]; }
IloNumVar outRightChild(int n, int i) { return outputs[rightChild(n,maxDepth)*pow(2,nVars)+i]; }
IloNumVar outLeftChild(int n, int i) { return outputs[leftChild(n,maxDepth)*pow(2,nVars)+i]; }

IloNumVar nod(int n, int v) { return nodes[n*(nVars+3)+v]; }

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

int treeSize(int depth) {
    return pow(2,depth+1)-1;
}

int main() {
    IloEnv env;
    IloModel model(env);
    
    cin >> maxDepth;
    cin >> nVars;
    vector<int> truthTable((int) pow(2,nVars));
    for (int i = 0; i < truthTable.size(); i++) {
       cin >> truthTable[i];
    }
    //nodes = IloNumVarArray(env, treeSize(maxDepth), -1, nVars, ILOINT);
    //* nodes[n][v] represents wether the node n is corresponding to an input var
    /* v. v=0 represents the constant 0 signal, v=nVars+1 represents a NOR gate
    /* and v=nVars+2 represents unassigned */
    nodes = IloNumVarArray(env, treeSize(maxDepth) * (nVars + 3), 0, 1, ILOBOOL);
    size = IloNumVar(env, 1, treeSize(maxDepth-1), ILOINT);
    outputs = IloNumVarArray(env, treeSize(maxDepth) * pow(2,nVars), 0, 1, ILOBOOL);
    //isNOR = IloNumVarArray(env, treeSize(maxDepth), 0, 1, ILOBOOL);
    
    // The first node should be always a NOR gate
    model.add(nod(0,nVars+1) == 1);
    
    int input[nVars+1];
    for (int x = 0; x < nVars+1; x++) {
        input[x] = 0;
    }
    
    for (int i = 0; i < (int) pow(2, nVars); i++) {
        for (int n = 0; n < (int) (treeSize(maxDepth)/2); n++) {
            // The two following constraints refer to the NOR behavior of the 
            /* outputs concatenation
            */
            model.add((1-out(n,i)) - (1-nod(n,nVars+1)) <= outLeftChild(n,i) + outRightChild(n,i));
            model.add(outLeftChild(n,i) + outRightChild(n,i) <= bigM*(1-out(n,i)+(1-nod(n,nVars+1))));
            for (int v = 0; v < nVars+1; ++v) {
                // The two following constraints assign outputs of the nodes 
                /* acting as input vars or 0s with its corresponding value
                */
                model.add(nod(n,v)*input[v] <= out(n,i));
                model.add(out(n,i) <= nod(n,v)*input[v] + (1-nod(n,v)));
            }
        }
        for(int n = (int) (treeSize(maxDepth)/2); n < treeSize(maxDepth); n++) {
            for (int v = 0; v < nVars+1; ++v) {
                // The two following constraints assign outputs of the nodes 
                /* acting as input vars or 0s with its corresponding value
                */
                model.add(nod(n,v)*input[v] <= out(n,i));
                model.add(out(n,i) <= nod(n,v)*input[v] + (1-nod(n,v)));
            }
        }
        // The output of the first node should equate the truth table
        model.add(out(0,i) == truthTable[i]);
        updateInput(input, nVars);
    }
    // The following expression constraints a the size variable to be equal to
    /* the sum of NOR gates in the circuit
    */
    IloExpr sumNORs(env);
    
    for(int n = 0; n < (int) (treeSize(maxDepth)/2); n++) {
        // The following expression constraints a node to be assigned a unique
        /* value (either NOR gate, input var, 0 constant input or unassigned)
        */
        IloExpr uniqueNodeValue(env);
        uniqueNodeValue = nod(n,nVars+1) + nod(n,nVars+2);
        for (int v = 0; v < nVars+1; ++v) {
            uniqueNodeValue += nod(n,v);
            // The following two constraints require the nodes behind (childs) a
            /* certain node acting as an input var / 0 constant value to be kept
            /* unassigned
            */
            // left child unassigned when var
            model.add(nod(n,v) + nod(leftChild(n, maxDepth), nVars+2) >= 2*(nod(n,v)));
            // right child unassigned when var
            model.add(nod(n,v) + nod(rightChild(n, maxDepth), nVars+2) >= 2*(nod(n,v)));
        }
        // The following two constraints require the nodes value behind (childs) a
        /* certain node acting as unassigned to be kept unassigned
        /* ATTENTION! It solves accuracy problems with case nlsp_4_3_36.inp, but
        /* causes another accuracy problem on case nlsp_4_3_131.inp
        */
        // left child unassigned when unassigned
        model.add(nod(n, nVars+2) + nod(leftChild(n, maxDepth), nVars+2) >= 2*(nod(n,nVars+2)));
        // right child unassigned when unassigned
        model.add(nod(n, nVars+2) + nod(rightChild(n, maxDepth), nVars+2) >= 2*(nod(n,nVars+2)));
        model.add(uniqueNodeValue == 1);
        uniqueNodeValue.end();
        // The following two constraints require the nodes behind (childs) a
        /* certain node acting as NOR gate to be assigned (NOR/var/0 constant)
        */
        // left child assigned when NOR
        model.add(nod(n,nVars+1) + nod(leftChild(n, maxDepth), nVars+2) <= 1);
        // right child assigned when NOR
        model.add(nod(n,nVars+1) + nod(rightChild(n, maxDepth), nVars+2) <= 1);
        sumNORs += nod(n, nVars+1);
    }
    
    for(int n = (int) (treeSize(maxDepth)/2); n < treeSize(maxDepth); n++) {
        IloExpr uniqueNodeValue(env);
        uniqueNodeValue = nod(n,nVars+1) + nod(n,nVars+2);
        for (int v = 0; v < nVars+1; ++v) {
            uniqueNodeValue += nod(n,v);
        }
        model.add(uniqueNodeValue == 1);
        uniqueNodeValue.end();
        
        model.add(nod(n,nVars+1) == 0);
        sumNORs += nod(n, nVars+1);
    }
    model.add(sumNORs == size);
    // The objective function is simply to minimize size
    model.add(IloMinimize(env, size));
    
    IloCplex cplex(model);
    // To avoid logging messages by CPLEX on screen
    cplex.setOut(env.getNullStream());
    IloBool solutionAvailable;
    solutionAvailable = cplex.solve();
    // Printing out the input
    cout << maxDepth << endl;
    cout << nVars << endl;
    for (int i = 0; i < truthTable.size(); i++) {
        cout << truthTable[i] << endl;
    }
    // Now the solution, if any
    if (solutionAvailable == IloFalse) {
        cout << -1 << endl;
    } else {
        cout << cplex.getObjValue() << endl;
        IloNumArray n(env);
        cplex.getValues(n, nodes);
        if (DEBUG) {
            // For debugging the case 4_3_36.inp
            IloNumArray o(env);
            cplex.getValues(o, outputs);
            for (int i = 0; i < (int) pow(2, nVars); i++) {
                cout << o[6*pow(2,nVars)+i] << " "; // 6, 14
            }
            cout << endl;
            for (int i = 0; i < (nVars+3); i++) {
                cout << n[6*(nVars+3)+i] << " ";
            }
            cout << endl;
        }
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
    }
    env.end();
}
