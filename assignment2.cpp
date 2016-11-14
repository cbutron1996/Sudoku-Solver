// Christian Butron
// CISC 3410, Program 2
// Language: C++

#include <iostream>
#include <vector>
#include <fstream>
#include <time.h>
using namespace std;

struct Square
{
    int var; //variables
    int row, col;
    vector<int> dom; //domains
    Square(int, int, int);
};

struct Arc
{
    Square *left;
    Square *right;
    Arc(Square *a, Square *b)
    {
        left = a;
        right = b;
    }
};

class CSP
{
private:
    Square *square[9][9];
    int success;
    int failure;
    vector<Square *> Q;
    vector<Arc *> arcQ; //arcs for ac3
    ofstream outfile;
public:
    bool constraint(int x, int y) { return x != y; } //constraint
    void read(int);
    void print();
    void fillArcQ();
    void fillArcQ(Square* &);
    bool AC3();
    bool removeInc(Square* &, Square* &);
    bool Backtrack();
    void sort();
    bool check();
    bool isSafe(int, int, int);
    void fillDomains();
};

Square::Square(int x, int r, int c)
{
    var = x;
    row = r;
    col = c;
}

void CSP::fillDomains()
{
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
        {
            while(!square[i][j]->dom.empty())
                square[i][j]->dom.erase(square[i][j]->dom.begin());
            if(square[i][j]->var == 0)
            {
                for(int k = 1; k <= 9; k++)
                    if(isSafe(k, i, j))
                        square[i][j]->dom.push_back(k);
            }
        }
    }
}

void CSP::read(int choice)
{
    success = 0;
    failure = 0;
    ifstream infile("sudokus.txt");
    outfile.open("sudokus_solved.txt");
    string line;
    if(choice == 1)
        cout << "AC3" << endl;
    else if(choice == 2)
        cout << "Backtracking w/ MRV and Forward Checking" << endl;
    while(infile >> line)
    {
        int k = 0;
        int state[9][9];
        for(int i = 0; i < 9; i++)
        {
            for(int j = 0; j < 9; j++)
            {
                square[i][j] = new Square(line[k] - '0', i, j);
                state[i][j] = line[k] - '0';
                k++;
            }
        }
        if(choice == 1)
        {
            if(AC3())
            {
                outfile << "Original" << endl;
                for(int i = 0; i < 9; i++)
                {
                    for(int j = 0; j < 9; j++)
                        outfile << state[i][j] << " ";
                    outfile << endl;
                }
                outfile << "Solution" << endl;
                success++;
                print();
                outfile << endl;
            }
            else
                failure++;
        }
        else if(choice == 2)
        {
            if(Backtrack())
            {
                outfile << "Original" << endl;
                for(int i = 0; i < 9; i++)
                {
                    for(int j = 0; j < 9; j++)
                        outfile << state[i][j] << " ";
                    outfile << endl;
                }
                outfile << "Solution" << endl;
                success++;
                print();
                outfile << endl;
            }
            else
                failure++;
        }
        while(!Q.empty())
            Q.erase(Q.begin());
    }
    cout << "Successes: " << success << endl;
    cout << "Failures: " << failure << endl;
}

void CSP::fillArcQ()
{
    for(int r = 0; r < 9; r++)
        for(int i = 0; i < 9; i++)
            for(int j = 0; j < 9; j++)
                if(square[r][i] != square[r][j])
                    arcQ.push_back(new Arc(square[r][i], square[r][j]));
    for(int c = 0; c < 9; c++)
        for(int i = 0; i < 9; i++)
            for(int j = 0; j < 9; j++)
                if(square[i][c] != square[j][c])
                    arcQ.push_back(new Arc(square[i][c], square[j][c]));
    for(int r = 0; r < 3; r++)
        for(int c = 0; c < 3; c++)
            for(int i = 0; i < 9; i++)
                for(int j = 0; j < 9; j++)
                {
                    int x1 = (i % 3) + r * 3;
                    int y1 = (i / 3) + c * 3;
                    int x2 = (j % 3) + r * 3;
                    int y2 = (j / 3) + c * 3;
                    if(square[x1][y1] != square[x2][y2])
                    	arcQ.push_back(new Arc(square[x1][y1], square[x2][y2]));
                }
}

void CSP::fillArcQ(Square* &curr)
{
    int r = curr->row;
    int c = curr->col;
    for(int i = 0; i < 9; i++)
        if(square[r][c] != square[r][i])
            arcQ.push_back(new Arc(square[r][i], square[r][c]));
    for(int i = 0; i < 9; i++)
        if(square[r][c] != square[i][c])
            arcQ.push_back(new Arc(square[i][c], square[r][c]));
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
        {
            int x = (r - r%3) + i;
            int y = (c - c%3) + j;
            if(square[r][c] != square[x][y])
                arcQ.push_back(new Arc(square[x][y], square[r][c]));
        }
}

bool CSP::AC3()
{
    fillDomains();
    fillArcQ();
    while(!arcQ.empty())
    {
        Arc *curr = arcQ.front();
        arcQ.erase(arcQ.begin());
        if(removeInc(curr->left, curr->right))
            fillArcQ(curr->left);
    }
    for(int i = 0; i < 9; i++)
        for(int j = 0; j < 9; j++)
            if(square[i][j]->dom.size() == 1)
                square[i][j]->var = square[i][j]->dom.front();
    if(check())
        return true;
    return false;
}

bool CSP::removeInc(Square* &left, Square* &right)
{
    if(left->dom.empty() || right->dom.empty())
        return false;
    int target = left->dom.front();
    for(int i = 0; i < left->dom.size(); i++)
    {
        bool satisfy = false;
        for(int j = 0; j < right->dom.size(); j++)
        {
            //if(left->dom[i] == right->dom[j])
            if(constraint(left->dom[i], right->dom[j]))
            	satisfy = true;
        }
        if(!satisfy)
        {
            left->dom.erase(left->dom.begin() + i);
            return true;
        }
    }
    return false;
}

bool CSP::Backtrack()
{
    fillDomains();
    while(!Q.empty())
        Q.erase(Q.begin());
    for(int i = 0; i < 9; i++)
        for(int j = 0; j < 9; j++)
            if(square[i][j]->var == 0)
                Q.push_back(square[i][j]);
    if(Q.empty())
        return true;
    sort();
    Square *curr = Q.front();
    Q.erase(Q.begin());
    for(int i = 0; i < curr->dom.size(); i++)
    {
        int k = curr->dom[i];
        if(isSafe(k, curr->row, curr->col))
        {
            curr->var = k;
            if(Backtrack())
                return true;
            curr->var = 0;
            fillDomains();
        }
    }
    return false;
}

void CSP::sort()
{
    for(int i = 0; i < Q.size(); i++)
        for(int j = 0; j < Q.size(); j++)
            if(Q[i]->dom.size() < Q[j]->dom.size())
                swap(Q[i], Q[j]);
}

bool CSP::check()
{
    for(int i = 0; i < 9; i++)
        for(int j = 0; j < 9; j++)
            if(square[i][j]->var == 0)
                return false;
    return true;
}

bool CSP::isSafe(int k, int r, int c)
{
    for(int i = 0; i < 9; i++)
    {
        //if(square[r][i]->var == k)
        if(!constraint(square[r][i]->var, k))
            return false;
        //if(square[i][c]->var == k)
        if(!constraint(square[i][c]->var, k))
            return false;
    }
    int startRow = r - r % 3;
    int startCol = c - c % 3;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            //if(square[i+startRow][j+startCol]->var == k)
            if(!constraint(square[i+startRow][j+startCol]->var, k))
                return false;
    return true;
}

void CSP::print()
{
    for(int i = 0; i < 9; i++)
    {
        for(int j = 0; j < 9; j++)
            outfile << square[i][j]->var << " ";
        outfile << endl;
    }
}

int main()
{
    cout << "This is a Sudoku Solver program." << endl;
    cout << "When you choose a method, the number of successes and failures are printed on screen." << endl;
    cout << "The finished puzzles are printed in sudokus_solved.txt." << endl;
    cout << "Type 1 to solve Sudoku puzzles with AC3." << endl;
    cout << "Type 2 to solve Sudoku puzzles with Backtracking w/ MRV and Forward Checking." << endl;
    cout << "Type anything else to leave." << endl;
    int choice;
    cin >> choice;
    while(choice == 1 || choice == 2)
    {
        clock_t start, finish, total;
        start = clock();
        CSP csp;
        csp.read(choice);
        finish = clock();
        total = (finish - start) / (CLOCKS_PER_SEC / 1000);
        cout << "Time: " << total << " milliseconds" << endl;
        cin >> choice;
    }
    return 0;
}
