/*
 *  Authors:
 *    Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *    Christian Schulte, 2008-2013
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software, to deal in the software without restriction,
 *  including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sublicense, and/or sell copies of the software,
 *  and to permit persons to whom the software is furnished to do so, subject
 *  to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace Gecode;
using namespace std;

class QueensProblem : public Space {
private:
  int n;
protected:
  IntVarArray q;
public:
  QueensProblem(int cols) : n(cols), q(*this, cols*cols, 0, 1) {

    linear(*this, q, IRT_EQ, n);
    for(int i = 0; i < n; i++) {
        IntVarArgs row(n);
        for(int j = 0; j < n; j++) {
            row[j] = q[i*n+j];
        }
        linear(*this, row, IRT_LQ, 1);
    }
    for(int j = 0; j < n; j++) {
        IntVarArgs col(n);
        for(int i = 0; i < n; i++) {
            col[i] = q[i*n+j];
        }
        linear(*this, col, IRT_LQ, 1);
    }
    for(int d = 0; d < n; d++) {
        IntVarArgs diag_princ(n-d);
        int i = 0;
        for (int j = d; j < n; j++) {
            diag_princ[j-d] = q[i*n+j];
            i++;
        }
        linear(*this, diag_princ, IRT_LQ, 1);
    }
    for(int d = 1; d < n; d++) {
        IntVarArgs diag_princ(n-d);
        int j = 0;
        for (int i = d; i < n; i++) {
            diag_princ[i-d] = q[i*n+j];
            j++;
        }
        linear(*this, diag_princ, IRT_LQ, 1);
    }
    for(int d = 0; d < n; d++) {
        IntVarArgs diag_sec(d+1);
        int i = 0;
        for (int j = d; j >= 0; j--) {
            diag_sec[d-j] = q[i*n+j];
            i++;
        }
        linear(*this, diag_sec, IRT_LQ, 1);
    }
    for(int d = 1; d < n; d++) {
        IntVarArgs diag_sec(n-d);
        int j = n-1;
        for (int i = d; i < n; i++) {
            diag_sec[i-d] = q[i*n+j];
            j--;
        }
        linear(*this, diag_sec, IRT_LQ, 1);
    }
    branch(*this, q, INT_VAR_NONE(), INT_VAL_MAX());
  }
  
  QueensProblem(bool share, QueensProblem& s) : Space(share, s) {
    q.update(*this, share, s.q);
    n = s.n;
  }
  
  virtual Space* copy(bool share) {
    return new QueensProblem(share,*this);
  }
  
  void print(void) const {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (q[i*n+j].val() == 1) {
                cout << 'X';
            } else {
                cout << '.';
            }
        }
        cout << endl;
    }
  }
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 2) return 1;
        int cols = atoi(argv[1]);
        QueensProblem* q = new QueensProblem(cols);
        DFS<QueensProblem> e(q);
        delete q;
        if (QueensProblem* s = e.next()) { // To print all the solutions, simply substitute 'if' by 'while'
            s->print();
            delete s;
        }
    } catch (Exception e) {
        cerr << "Gecode exception: " << e.what() << endl;
        return 1;
    }
    return 0;
}
