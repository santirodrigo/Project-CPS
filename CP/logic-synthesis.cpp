/*
 *  Author:
 *    Santiago Rodrigo <srodrigo@ac.upc.edu>
 *  Gecode model for logic synthesis problem, CPS project, april 2017
 */

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>
#include <math.h>
#include <vector>
#include <csignal>

using namespace Gecode;
using namespace std;

struct Solution {
  int size;
  vector<int> nodes;
};

Solution sol_str;

class LogicSynthesis : public Space {
private:
    int maxDepth;
    int nVars;
    vector<int> truthTable;

protected:
    IntVarArray nodes;
    IntVar size;
    BoolVarArray outputs, isNOR;

public:
    LogicSynthesis(int maxD, int nV, vector<int>& truthT) : 
      maxDepth(maxD), nVars(nV),  
      truthTable(truthT), 
      nodes(*this, (int) pow(2,maxDepth+1)-1, -2, nVars),
      size(*this, 1, (int) pow(2,maxDepth)-1),
      outputs(*this, (int) (nodes.size() * pow(2,nVars)), 0, 1),
      isNOR(*this, nodes.size(), 0, 1) {
        rel(*this, nodes[0] == -1);
        int input[nVars];
        for (int x = 0; x < nVars; x++) {
            input[x] = 0;
        }
        for (int i = 0; i < (int) pow(2, nVars); i++) {
            for (int n = 0; n < (int) (nodes.size()/2); n++) {
                rel(*this, (nodes[n] == -1) >> (outputs[n*pow(2,nVars)+i] == !(outputs[leftChild(n,maxDepth)*pow(2,nVars)+i] || outputs[rightChild(n,maxDepth)*pow(2,nVars)+i])));
                rel(*this, (nodes[n] == 0) >> (outputs[n*pow(2,nVars)+i] == 0));
                for (int v = 0; v < nVars; ++v) {
                    rel(*this, ((nodes[n] == v+1) >> (outputs[n*pow(2,nVars)+i] == input[v])));
                }
            }
            for(int n = (int) (nodes.size()/2); n < nodes.size(); n++) {
                rel(*this, ((nodes[n] == 0) >> (outputs[n*pow(2,nVars)+i] == 0)));
                for (int v = 0; v < nVars; ++v) {
                    rel(*this, ((nodes[n] == v+1) >> (outputs[n*pow(2,nVars)+i] == input[v])));
                }
            }
            rel(*this, (outputs[i] == truthTable[i]));
            updateInput(input, nVars);
        }
        for(int n = 0; n < (int) (nodes.size()/2); n++) {
            rel(*this, (nodes[n] != -1) >> (nodes[leftChild(n,maxDepth)] == -2 && nodes[rightChild(n,maxDepth)] == -2));
            rel(*this, (nodes[n] == -1) >> (nodes[leftChild(n,maxDepth)] > -2 && nodes[rightChild(n,maxDepth)] > -2));
            rel(*this, (nodes[n] == -1) == (isNOR[n] == 1));
        }
        for(int n = (int) (nodes.size()/2); n < nodes.size(); n++) {
            rel(*this, nodes[n] != -1);
            rel(*this, isNOR[n] == 0);
        }
        linear(*this, isNOR, IRT_EQ, size);
        branch(*this, nodes, INT_VAR_NONE(), INT_VAL_MIN());
    }
  
    LogicSynthesis(bool share, LogicSynthesis& s) : Space(share, s) {
        nodes.update(*this, share, s.nodes);
        outputs.update(*this, share, s.outputs);
        size.update(*this, share, s.size);
        isNOR.update(*this, share, s.isNOR);
        maxDepth = s.maxDepth;
        nVars = s.nVars;
        truthTable = s.truthTable;
    }
  
    virtual Space* copy(bool share) {
        return new LogicSynthesis(share,*this);
    }
    
    // constrain function
    virtual void constrain(const Space& _l) {
        const LogicSynthesis& l = static_cast<const LogicSynthesis&>(_l);
        rel(*this, size < l.size);
    }
    
    int leftChild(int parent, int maxDepth) {
        return std::min((int) pow(2,maxDepth+1), 2*(parent+1)-1);
    }
    
    int rightChild(int parent, int maxDepth) {
        return leftChild(parent, maxDepth)+1;
    }
    
    void updateInput(int* input, int nVars) {
        for (int x = nVars-1; x >= 0; x--) {
            if (input[x] == 0) {
                input[x] = 1;
                return;
            } else {
                input[x] = 0;
            }
        }
    }
    
    void print(void) const {
        cout << size.val() << endl;
        for (int i = 0; i < nodes.size(); i++) {
            if (nodes[i].val() != -2) {
                cout << i+1 << " ";
                cout << nodes[i].val() << " ";
                if (nodes[i].val() == -1) {
                    cout << 2*(i+1) << " ";
                    cout << 2*(i+1)+1 << endl;
                } else {
                    cout << 0 << " " << 0 << endl;
                }
            }
        }
    }
    
    Solution getSolution() {
        Solution newSol;
        newSol.size = size.val();
        vector<int> n(nodes.size());
        for (int i = 0; i < nodes.size(); i++) {
            n[i] = nodes[i].val();
        }
        newSol.nodes = n;
        return newSol;
    }
    
    
};

void printSolution(struct Solution& sol) {
    cout << sol.size << endl;
    for (int i = 0; i < sol.nodes.size(); i++) {
        if (sol.nodes[i] != -2) {
            cout << i+1 << " ";
            cout << sol.nodes[i] << " ";
            if (sol.nodes[i] == -1) {
                cout << 2*(i+1) << " ";
                cout << 2*(i+1)+1 << endl;
            } else {
                cout << 0 << " " << 0 << endl;
            }
        }
    }
}

void TERMHandler(int signal) {
    printSolution(sol_str);
    exit(124); // TIMEOUT
}

int main(int argc, char* argv[]) {
    try {
        int maxDepth, nVars;
        cin >> maxDepth;
        cin >> nVars;
        vector<int> truthTable((int) pow(2,nVars));
        for (int i = 0; i < truthTable.size(); i++) {
            cin >> truthTable[i];
        }
        cout << maxDepth << endl;
        cout << nVars << endl;
        for (int i = 0; i < truthTable.size(); i++) {
            cout << truthTable[i] << endl;
        }
        LogicSynthesis* l = new LogicSynthesis(maxDepth, nVars, truthTable);
        BAB<LogicSynthesis> e(l);
        delete l;
        int sol = 0;
        while (LogicSynthesis* l = e.next()) {
            sol_str = l->getSolution();
            sol = 1;
            // Code for SIGTERM gracefully exit, to print the best solution
            // found till then (saved in sol_str)
            signal(SIGTERM, TERMHandler);
            delete l;
        }
        if (!sol) cout << -1 << endl;
        else printSolution(sol_str);
    } catch (Exception e) {
        cerr << "Gecode exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}
