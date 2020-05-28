#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include <cmath>
#include <fstream>
#include <iomanip>
#define pi 3.14159265358979323846  
using namespace std;
//const double merid = 40008.5;
const double parall[10] = { 40075, 39456, 37656, 34740, 30708, 25596, 20088, 13752, 7128, 0 };
map <int, int> paralls;

double getLen(int deg){
    return 2*pi*(sin(((90 - deg) * pi) / 180) * 6371);
}

void init() {
    for (int i = 0; i < 10; i++) {
        paralls[i] = parall[i];
    }
}   
    
double min(double a, double b) {
    return a > b ? b : a;
}   
    
double max(double a, double b) {
    return a < b ? b : a;
}
/*
int nearest(double deg) {
    int intedDeg = int(deg);
    if (intedDeg % 10 > 10 - intedDeg % 10)return paralls[intedDeg / 10 + 1];
    else
        return paralls[intedDeg / 10];
}*/
pair<double,double> toKm(double lat,double lng) {
    pair <double, double> p;
    p.first = lat * 111.1;
    p.second = 1.0 * getLen(lat) /360*lng;
    return p;
}

struct Place {
    double lng, lat;
    string type, subtype, name,addr;
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
    double distanceToPoint(double x,double y){
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
    bool collideUtil(double xs,double ys,double xf,double yf,double xa,double ya,double r)
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
    bool collide(double xs,double ys, double r) {
        return collideUtil(this->left,this->down,this->left,this->up,xs, ys, r) ||
               collideUtil(this->left, this->up, this->right, this->up, xs, ys, r) || 
               collideUtil(this->right, this->up, this->right, this->down, xs, ys, r) ||
               collideUtil(this->left, this->down, this->right, this->down, xs, ys, r) ||
               this->in(Place(xs,ys));
    }
};  
    
class Rnode {
    friend class Rtree;
    const int minChildren = 2;
    const int maxChildren = 4;
    Place* tops = new Place[maxChildren + 1];
    Rnode** children = new Rnode * [maxChildren + 1];
    int numTops;
    int numChildren;
    Rect square;
public:
    bool isLeaf;
    Rnode() {
        this->numChildren = 0;
        this->numTops = 0;
        this->isLeaf = true;
       // square = Rect();
    }
    void add(Place p) {
        if (this->isLeaf == true)this->addVal(p);
        else
        {
            this->chooseLeaf(p);
        }
        this->updateSides(p);
    }
    void find(double x,double y,double r) {
        if (this->isLeaf == true) {
            for (int i = 0; i < this->numTops; i += 1) {
                if (this->tops[i].distanceToPoint(x, y) <= r) {
                    cout << this->tops[i].name << endl;
                }
                //cout <<fixed<<setprecision(6)<< this->tops[i].lat<<' '<<this->tops[i].lng << endl;
            }
        }
        else
        {
            for (int i = 0; i < numChildren; i += 1) {
                if (this->children[i]->square.collide(x, y, r)) { children[i]->find(x, y, r); }
            }
        }
    }
    void split(int idx) {
        Rnode* child = this->children[idx];
        Rnode* sibling = new Rnode;
        double mid = (child->square.up + child->square.down) / 2;
        sibling->square.down = child->square.down;
        child->square.down = mid;
        sibling->square.up = mid;
        sibling->square.left = child->square.left;
        sibling->square.right = child->square.right;

        for (int i = child->numTops - 1; i >= 0; i--) {
            Place temp = child->tops[i];
            if (sibling->square.in(child->tops[i])) {
                sibling->tops[sibling->numTops] = child->tops[i];
                sibling->numTops += 1;
                for (int j = i; j < child->numTops; j += 1) {
                    child->tops[j] = child->tops[j + 1];
                }
                child->numTops -= 1;
            }
        }

        for (int i = this->numChildren + 1; i > idx; i -= 1) {
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
        for (int i = 0; i < this->numChildren; i++) {
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
        bool splitted = false;
        if (minPlace->numChildren == this->maxChildren || minPlace->numTops == this->maxChildren) {
            splitted = 1;
            split(ind);
        }
        if (splitted && this->children[ind]->square.distance(p) > this->children[ind + 1]->square.distance(p)) {
            ind += 1;
        }
        this->children[ind]->add(p);
    }
    void updateSides(Place p) {
        this->square.left = min(this->square.left, p.lng);
        this->square.right = max(this->square.right, p.lng);
        this->square.up = max(this->square.up, p.lat);
        this->square.down = min(this->square.down, p.lat);
    }
    void addVal(Place p) {
        this->tops[this->numTops] = p;
        this->numTops += 1;
    }

    int getNumberOfChildren() {
        return this->numChildren;
    }

    int getNumberOfTops() {
        return this->numTops;
    }
    void outAll() {
        if (this->isLeaf) {
            for (int i = 0; i < this->numTops; i++) {
                cout << this->tops[i].name << endl;
            }
        }
        else {
            for (int i = 0; i < this->numChildren; i += 1) {
                this->children[i]->outAll();
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
        if (root == nullptr) {
            root = new Rnode();
        }
        else
            if (this->root->numTops == this->root->maxChildren || this->root->numChildren == this->root->maxChildren) {
                Rnode* temp = new Rnode();
                temp->children[0] = root;
                temp->numChildren = 1;
                temp->split(0);
                this->root = temp;
                this->root->numTops = 0;
                this->root->isLeaf = false;
            }
        root->add(p);
    }
    void find(double lat,double lng,double r){
        pair <double, double> p = toKm(lat,lng);
        double xCoord = p.second;
        double yCoord = p.first;
      //  cout << "lat " << yCoord << endl;
       // cout << "lng " << xCoord << endl;
        this->root->find(xCoord, yCoord, r);
    }
    void outAll() {
        this->root->outAll();
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
        while (getline(line, data, ','))
        {
            words.push_back(data);
        }
        for (int i = words.size(); i < 6; i++) {
            words.push_back(" ");
        }
        toPush = Place(words);
        cout <<j<<' '<< toPush.type << ' ' << toPush.subtype <<' '<< toPush.name << endl;
        m[toPush.type][toPush.subtype].add(toPush);

    }
    cout << "Reading done!" << endl;
}

int main() {
    init();
    readCsvFile();
    cout << "Enter type and subtype of object\n";
    string type, subType;
    cin >> type >> subType;
 //   m[type][subType].outAll();
    double lat, lng, r;
    cout << "Enter radius and coords of circle\n";
    cin >> lat >> lng >> r;
    m[type][subType].find(lat, lng, r);
}