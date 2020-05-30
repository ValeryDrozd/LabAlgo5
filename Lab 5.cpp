#include <iostream>
#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <iomanip>
#define pi 3.14159265358979323846  
#define minChildren 8
#define maxChildren 16
using namespace std;
bool out = 0;
double min(double a, double b) {
    return a > b ? b : a;
}

double max(double a, double b) {
    return a < b ? b : a;
}

double getLen(int deg) {
    return 2 * pi * (sin(((90 - deg) * pi) / 180) * 6371);
}

pair<double, double> toKm(double lat, double lng) {
    pair <double, double> p;
    p.first = lat * 111.1;
    p.second = 1.0 * getLen(lat) / 360 * lng;
    return p;
}

struct Place {
    double lng, lat;
    string type, subtype, name, addr;
    Place(double lng, double lat) {
        this->lng = lng;
        this->lat = lat;
    }
    Place(vector <string> data) {
        pair <double, double> p = toKm(atof(data[0].c_str()), atof(data[1].c_str()));
        this->lat = p.first;
        this->lng = p.second;
        this->type = data[2];
        this->subtype = data[3];
        this->name = data[4];
        this->addr = data[5];
    }
    Place() {

    }
    double distanceToPoint(double x, double y) {
        return sqrt(pow(this->lng - x, 2) + pow(this->lat - y, 2));
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
        return this->left <= p.lng && this->right >= p.lng && this->down <= p.lat && this->up >= p.lat;
    }

    double distance(Place p) {
        if (in(p))return 0;
        return min(abs(this->left - p.lng), min(abs(this->right - p.lng), min(abs(this->up - p.lat), abs(this->down - p.lat))));
    }

    double ClosestDistance(double x0, double y0, double x1, double y1, double x2, double  y2)
    {
        return abs((y2 - y1) * x0 - (x2 - x1) * y0 + x2 * y1 - y2 * x1) / sqrt(pow(y2 - y1, 2.0) + pow(x2 - x1, 2.0));
    }

    int znak(double x, double y, double xs, double ys, double xf, double yf) {
        double xa = xf - xs, xb = x - xs, ya = yf - ys, yb = y - ys;
        if (xa * xb + ya * yb < 0)return -1;
        else return 1;

    }
    bool collideUtil(double xs, double ys, double xf, double yf, double xa, double ya, double r)
    {
        double xv1 = xa - xs, yv1 = ya - ys, xv2 = xf - xs, yv2 = yf - ys;
        double dist;
        if (znak(xa, ya, xs, ys, xf, yf) * znak(xa, ya, xf, yf, xs, ys) < 0) {
            dist = sqrt(min((pow(xa - xs, 2) + pow(ya - ys, 2)), (pow(xa - xf, 2) + pow(ya - yf, 2))));
        }
        else
            dist = ClosestDistance(xa, ya, xs, ys, xf, yf);
        if (dist <= r)return true;
        return false;
    }

    bool collide(double xs, double ys, double r) {
        return collideUtil(this->left, this->down, this->left, this->up, xs, ys, r) ||
            collideUtil(this->left, this->up, this->right, this->up, xs, ys, r) ||
            collideUtil(this->right, this->up, this->right, this->down, xs, ys, r) ||
            collideUtil(this->left, this->down, this->right, this->down, xs, ys, r) ||
            this->in(Place(xs, ys));
    }
};


class Rnode {
    friend class Rtree;
    int numChildren, numTops;
    vector <Place> Places;
    bool isLeaf;
    vector <Rnode* > children;
    Rect square;
public:
    Rnode() {
        this->isLeaf = true;
        numTops = 0;
        numChildren = 0;
        Places.clear();
        children.clear();
        this->square = Rect();
    }
    bool cmp(Rnode* a, Rnode* b) {
        if (a->square.up >= b->square.up)return true;
        return false;
    }
    void add(Place p) {
        this->updateSides(p);
        if (this->isLeaf)this->insert(p);
        else
        {
            this->findToInsert(p);
        }
    }

    void insert(Place p) {
        this->Places.push_back(p);
        this->numTops += 1;
    }

    void split(int idx) {
        Rnode* child = this->children[idx];
        Rnode* sibling = new Rnode;
        vector <double> comps;
        double mid;
        if (child->isLeaf) {
            for (int i = 0; i < child->numTops; i += 1)
                comps.push_back(child->Places[i].lat);
            sort(comps.begin(), comps.end());
            mid = (comps[ceil(maxChildren / 2)] + comps[ceil(maxChildren / 2) - 1]) / 2;
        }
        else {
            mid = child->children[1]->square.down;
        }
        sibling->square.down = child->square.down;
        child->square.down = mid;
        sibling->square.up = mid;
        sibling->square.left = child->square.left;
        sibling->square.right = child->square.right;
        vector <int> toDel;

        for (int i = child->numTops - 1; i >= 0; i--) {
            if (sibling->square.in(child->Places[i])) {
                toDel.push_back(i);
            }
        }
        for (int i = 0; i < toDel.size(); i += 1) {
            sibling->Places.push_back(child->Places[toDel[i]]);
            sibling->numTops += 1;
            child->Places.erase(child->Places.begin() + toDel[i]);
            child->numTops -= 1;
        }
        if (!child->isLeaf) {
            double t = child->children.back()->square.up;
            while (t <= mid)
            {

                sibling->children.push_back(child->children.back());
                sibling->numChildren += 1;
                child->numChildren -= 1;
                child->children.pop_back();
                t = child->children.back()->square.up;
            }

        }
        sibling->isLeaf = child->isLeaf;
        this->children.insert(this->children.begin() + idx + 1, sibling);
        for (int i = 0; i < this->children.size(); i += 1) {
            for (int j = 0; j < this->children.size() - 1; j += 1) {
                if (this->children[j]->square.up < this->children[j + 1]->square.up)swap(this->children[j], this->children[j + 1]);
            }
        }
        this->numChildren += 1;
    }

    void findToInsert(Place p) {
        int idx = 0;
        Rnode* minPlace = 0;
        double dist = INT_MAX;
        for (int i = 0; i < this->numChildren; i++) {
            if (this->children[i]->square.in(p)) {
                minPlace = this->children[i];
                idx = i; break;
            }
            else
                if (dist > this->children[i]->square.distance(p)) {
                    dist = this->children[i]->square.distance(p);
                    minPlace = this->children[i];
                    idx = i;
                }
        }
        bool splitted = false;
        if (minPlace->numChildren == maxChildren + 1 || minPlace->numTops >= maxChildren + 1) {
            splitted = 1;
            split(idx);
            this->findToInsert(p);
        }
        if (splitted && this->children[idx]->square.distance(p) > this->children[idx + 1]->square.distance(p)) {
            idx += 1;
        }
        this->children[idx]->add(p);

    }
    void updateSides(Place p) {
        this->square.left = min(this->square.left, p.lng);
        this->square.right = max(this->square.right, p.lng);
        this->square.up = max(this->square.up, p.lat);
        this->square.down = min(this->square.down, p.lat);
    }
    void find(double x, double y, double r) {
        if (this->isLeaf == true) {
            for (int i = 0; i < this->numTops; i += 1) {
                double t = this->Places[i].distanceToPoint(x, y);
                if (this->Places[i].distanceToPoint(x, y) <= r) {
                    out = 1;
                    cout <<"Name "<< this->Places[i].name << " Address "<<this->Places[i].addr<<" Distance "<<  this->Places[i].distanceToPoint(x, y) << endl;
                }
            }
        }
        else
        {
            for (int i = 0; i < numChildren; i += 1) {
                if (this->children[i]->square.collide(x, y, r)) { children[i]->find(x, y, r); }
            }
        }
    }
};

class Rtree {
    Rnode* root;
public:
    Rtree() {
        this->root = nullptr;
    }
    void add(Place p) {
        if (this->root == NULL)this->root = new Rnode;
        else
            if (this->root->numChildren == maxChildren || this->root->numTops == maxChildren) {
                Rnode* temp = new Rnode;
                temp->isLeaf = false;
                temp->children.push_back(this->root);
                temp->numChildren += 1;
                temp->square = root->square;
                temp->split(0);
                this->root = temp;

            }
        this->root->add(p);
    }
    void find(double lat, double lng, double r) {
        out = 0;
        pair <double, double> p = toKm(lat, lng);
        double xCoord = p.second;
        double yCoord = p.first;
        this->root->find(xCoord, yCoord, r);
        if (out == 0) { cout << "Places not found!\n"; }
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
    int j = 0;
    while (getline(file, s)) {
        j += 1;
        vector <string> words;
        stringstream line(s);
        while (getline(line, data, ';'))
        {
            words.push_back(data);
        }
        for (int i = words.size(); i < 6; i++) {
            words.push_back(" ");
        }
        toPush = Place(words);
     //   cout << j << ' ' << toPush.type << ' ' << toPush.subtype << ' ' << toPush.name << endl;
        m[toPush.type][toPush.subtype].add(toPush);

    }
    cout << "Reading done!" << endl;
}

int main() {
    readCsvFile();
    cout << "Enter type and subtype of object\n";
    string type, subType;
    cin >> type >> subType;
    double lat, lng, r;
    cout << "Enter center of circle and its radius\n";
    cin >> lat >> lng >> r;
    m[type][subType].find(lat, lng, r);
}