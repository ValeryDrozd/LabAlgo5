#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <fstream>
using namespace std;

double min(double a, double b) {
    return a > b ? b : a;
}

double max(double a, double b) {
    return a < b ? b : a;
}

struct Place {
    double lng, lat;
    string type, subtype, name;
    Place(vector <string> data) {
        this->lat = atof(data[0].c_str());
        this->lng = atof(data[1].c_str());
        this->type = data[2];
        this->subtype = data[3];
        this->name = data[4];
    }
    Place() {

    }
};

struct Rect
{
    double left;
    double right;
    double up;
    double down;
    Rect() {
        this->left = INT_MAX;
        this->right = INT_MIN;
        this->up = INT_MIN;
        this->down = INT_MAX;
    }
    bool in(Place p) {
        return this->left<p.lng&& this->right>p.lng && this->down<p.lng&& this->up>p.lng;
    }
    double distance(Place p) {
        if (in(p))return 0;
        return min(abs(this->left - p.lng), min(abs(this->right - p.lng), min(abs(this->up - p.lat), abs(this->down - p.lat))));
    }
};

class Rnode {
    friend class Rtree;
    const int minChildren = 2;
    const int maxChildren = 4;
    Place* tops = new Place[maxChildren + 1];
    Rnode** children = new Rnode * [maxChildren + 1];
    int number;
    int numChildren;
    Rect square;
public:
    bool isLeaf;
    Rnode() {
        this->number = 1;
        this->number = 0;
        this->isLeaf = true;
    }
    void add(Place p) {
        if (this->isLeaf == true)this->addVal(p);
        else
        {
            this->chooseLeaf(p);
        }
    }

    void split(int idx) {
        Rnode* child = this->children[idx];
        Rnode* sibling = this->children[idx] + 1;
        double mid = (child->square.up - child->square.down) / 2;
        sibling->square.down = child->square.down;
        child->square.down = mid;
        sibling->square.up = mid;
        sibling->square.left = child->square.left;
        sibling->square.right = child->square.right;
        for (int i = this->number - 1; i >= 0; i--) {
            if (sibling->square.in(this->tops[i])) {
                sibling->tops[sibling->number] = tops[i];
                sibling->number += 1;
                for (int j = i; j >= 0; j -= 1) {
                    child->tops[i] = child->tops[i + 1];
                }
            }
        }
        child->numChildren -= sibling->numChildren;
        for (int i = this->number + 1; i > idx; i -= 1) {
            this->children[i] = this->children[i - 1];
        }
        this->children[idx + 1] = sibling;
        this->numChildren += 1;
    }

    void chooseLeaf(Place p) {
        Rnode* minPlace = new Rnode;
        bool appended = false;
        double dist = INT_MAX;
        int ind = 1;
        for (int i = 0; i < this->number; i++) {

            if (this->children[i]->square.in(p)) {
                minPlace = this->children[i];
                ind = i;
                break;
            }
            else {
                if (dist > this->children[i]->square.distance(p)) {
                    dist = this->children[i]->square.distance(p);
                    minPlace = this->children[i];
                    ind = i;
                }
            }
        }
        if (minPlace->children[ind]->number == this->maxChildren || this->children[ind]->numChildren == this->maxChildren) {
            split(ind);
        }

        if (this->children[ind]->square.distance(p) <= this->children[ind + 1]->square.distance(p)) {
            this->children[ind]->add(p);
        }
        else
            this->children[ind + 1]->add(p);
    }

    void addVal(Place p) {
        this->tops[this->number] = p;
        this->numChildren += 1;
        this->square.left = min(this->square.left, p.lng);
        this->square.right = max(this->square.right, p.lng);
        this->square.up = max(this->square.up, p.lat);
        this->square.down = min(this->square.down, p.lat);
    }

    bool getNumberOfChildren() {
        return this->numChildren;
    }
};

class Rtree {
    Rnode* root;
public:
    Rtree() {
        this->root = nullptr;
    }
    void add(Place p) {
        if (root == nullptr) {
            root = new Rnode();
        }
        else
            if (this->root->numChildren == this->root->maxChildren) {
                Rnode* temp = new Rnode();
                temp->children[0] = root;
                temp->split(0);
                this->root = temp;
                this->root->number = 2;
                this->root->isLeaf = false;
            }
        root->add(p);
    }
};

map <string, map <string, Rtree> > m;

void readCsvFile()
{
    char filename[256];
    cout << "Enter path to the file\n";
    gets_s(filename);
    ifstream file(filename);
    if (!file) {
        cout << "Error during reading the file\n";
        exit(2);
    }
    int n;
    string s, data;
    Place toPush;
    char place[1024];
    while (getline(file, s)) {
        vector <string> words;
        stringstream line(s);
        while (getline(line, data, ','))
        {
            words.push_back(data);
        }
        toPush = Place(words);
        m[toPush.type][toPush.subtype].add(toPush);
    }
}

int main() {
    readCsvFile();
}
