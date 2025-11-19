// Generator Subplazczyzn.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>
#include <vector>
#include <cstdlib>
#include <unordered_set>
#include <format>
#include <unordered_map>
#include <glaze/glaze.hpp>
#include <fstream>
//Mozna uzyc Glaze do szybszej serializacji ale nie ma potrzeby tutaj, w celu prostszej implementacji uzyjemy nlohmann json

struct PairHash {
    std::size_t operator()(const std::pair<int, int>& p) const noexcept {
        return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1); //Hash to pary <int,int> prosty(zeby set mogl wyszukiwac>)
    }
};
static int GenerateNumber(int min, int max) {
    return rand() % (max - min + 1) + min;
}

static bool ChanceGenerate(int percentage) {
    int number = GenerateNumber(0, 100);
    return number < percentage;
}



struct Point {
public:
    uint8_t x;
    uint8_t  y;
    bool collision;

    Point(int x, int y, bool collision = false) : x(x), y(y), collision(collision) {};
    std::string toString() {
        return std::format("{},{}", this->x, this->y);
    }
};



struct Edge {
public:
    int idMapConnect = 0;
    Point* Grid1Point = nullptr;
    Point* Grid2Point = nullptr;
    Edge(int idMapConnect, Point* Grid1Point, Point* Grid2Point) : idMapConnect(idMapConnect), Grid1Point(Grid1Point), Grid2Point(Grid2Point) {};
};

class Grid {
public:
    std::vector<Point> points; //1D or 2D? x*ymax + y 
    std::vector<Edge> Edges;
    unsigned short  id = 0;
    uint8_t width = 0;
    uint8_t height = 0;

        Point& getPoint(int x, int y) {
            return points[ (x * height) + y];
        }
        void GenerateNewGrid(int minx,int maxx,int miny,int maxy, int chanceforcol) {
            //Random Number
            width = GenerateNumber(minx, maxx);
            height = GenerateNumber(miny, maxy);
            points.clear();
            points.reserve(width * height);
            for (int x = 0; x < width; x++) {
                for (int y = 0; y < height; y++) {
                    points.emplace_back(Point(x, y, ChanceGenerate(chanceforcol)));
                }
            }
        }
        Point* getRandomPoint(bool collision, std::unordered_set<std::string>& PointsonGridTaken ) {
            int maxretry = this->points.size();
            while (maxretry>0) {
                int x = GenerateNumber(0, this->width-1);
                int y = GenerateNumber(0, this->height-1);
                Point& point = this->getPoint(x, y);
                if (point.collision == false) {
                    std::string pointtostring = point.toString();
                    if (PointsonGridTaken.contains(pointtostring)) {
                        continue;
                    }
                    return &point;
                }
                maxretry--;
            }
            return nullptr;
        }
       
        Grid(int id = 0, int width = 1, int height = 1) : id(id),width(width),height(height) {}
};


class Graph {
    public:
        //Vector Siatek i generowanie miedzy nimi przejsc?
        std::vector<Grid>Grids;
        Graph(int sizemap = 10, int maxconnonmaps = 5,int minx=0,int maxx=100,int miny=0,int maxy=100, int chanceofcoll=20) {
            Grids.reserve(sizemap);
            for (int i = 0; i < sizemap; i++) {
                Grid tempgrid = Grid(i, maxx-minx, maxy-miny);
                tempgrid.GenerateNewGrid(minx, maxx, miny, maxy, chanceofcoll);
                Grids.emplace_back(tempgrid);
            }
            //Pora na generowanie wezlow granicznych
            //std::uno<std::pair<int>> AlreadyEdges;
            std::unordered_set<std::pair<int, int>,PairHash> AlreadyEdges;
            std::unordered_map<int, std::unordered_set<std::string>> PointsonGridTaken; //Punkty na mapie ktore maja krawedz

            for (int i = 0; i < sizemap; i++) {
                //Indexy powinny byc rowne z id w vectorze
				int max = Grids[i].points.size() / 2 > Grids.size() ? Grids.size() / 2 : Grids[i].points.size() / 2;
                int CountConnect = GenerateNumber(1, max); //Ilosc polaczen miedzy mapa X a Mapami //uwzgledniam kolzije teoretycznie powinnismy wyliczyc mozliwe punkty, ale przyblizenie moze byc
                CountConnect -= Grids[i].Edges.size(); //Odejujemy juz krawedzie ktore mamy bo generujemy krawedzie symetrycznie!

                for (int j = 0; j < CountConnect; j++) {
                    int ConnectTO = GenerateNumber(0, Grids.size() - 1); //ID mapy do ktorej sie laczymy
                    //Musimy wygenerować Punkt graniczny na mapie nr.1 i mapie nr.2, który nie jest kolizja

                    if (ConnectTO == i) {
                        j--;
                        continue;
                    } //Ta sama mapa to nie bedziemy sie laczyc :D sam ze soba


                    //Sprawdzmy czy mamy takie polaczenie O(1) bo to jest SET
                    std::pair<int, int> EdgeCheck1 = std::make_pair(i, ConnectTO);
                    std::pair<int, int> EdgeCheck2 = std::make_pair(ConnectTO,i);
                    //Duplikat bo trzeba byloby napisać wlasny komparator?

                    if (AlreadyEdges.contains(EdgeCheck1)) {
                        j--;
                        continue;
                    } //jezeli mamy taka krawedz to wywalamy

                    AlreadyEdges.insert(EdgeCheck1);
                    AlreadyEdges.insert(EdgeCheck2);
                    //Krawdzecie do setu dodane, tera Trzeba koordynaty wygenerowac z jednej i drugiej mapy ktore nie sa kolizja i nie są także granicznym wezlem
                    Point* PointinGrid1 = Grids[i].getRandomPoint(false, PointsonGridTaken[Grids[i].id]);
                    Point* PointinGrid2 = Grids[ConnectTO].getRandomPoint(false, PointsonGridTaken[Grids[ConnectTO].id]);
                    if (PointinGrid1 == nullptr || PointinGrid2 == nullptr) { //jezeli jakis nullptr to powinnismy to jakos obsluzyc narazie pomijam tutaj powinno byc throw
                        j--;
                        continue;

                    }
                    //Dobra finalnie dodajemy krwadzie
                    Grids[i].Edges.emplace_back(Edge(Grids[ConnectTO].id,PointinGrid1,PointinGrid2));
                    Grids[ConnectTO].Edges.emplace_back(Edge(Grids[i].id, PointinGrid2, PointinGrid1)); //odwotnosc punktow dajemy ze w 1 miejscu punkt w naszej plazczyznie
                }

            }
        }

    
};

template<>
struct glz::meta<Point> {
    using T = Point;
    static constexpr auto value = object(
        "x",&T::x,
        "y",&T::y,
        "collision",&T::collision

    );
};
template<>
struct glz::meta<Edge> {
    using T = Edge;
    static constexpr auto value = object(
        "idMapConnect",&T::idMapConnect,
        "Grid1Point",&T::Grid1Point,
        "Grid2Point",&T::Grid2Point
    );
};

template<>
struct glz::meta<Grid> {
    using T = Grid;
    static constexpr auto value = object(
        "id",&T::id,
        "width",&T::width,
        "points",&T::points,
        "Edges", &T::Edges,
        "height",& T::height
    );
};
template<>
struct glz::meta<Graph> {
    using T = Graph;
    static constexpr auto value = object(
        "Grids", &T::Grids
    );
};
int main()
{
    //Okay Przerabiamy tak zeby byly nie losowe dane, lecz wredne dane?
    //Przypadki brzegowe?
    //1->2->3->4->5 (i zawsze po przekatnej jednej i drugiej w sumie mozna 4 takie przekatne?


    srand(time(NULL));
    Graph testgraph = Graph(10, 5, 5, 100, 5, 100);
    size_t totalSize = sizeof(testgraph);
    totalSize += testgraph.Grids.capacity() * sizeof(Grid);
    for (auto& grid : testgraph.Grids) {
        totalSize += grid.points.capacity() * sizeof(Point);
        totalSize += grid.Edges.capacity() * sizeof(Edge);
    }
    std::cout << "Size of TestGraph: " << totalSize;
    std::string stringjson = glz::write_json(testgraph).value_or("errorjson");
    std::ofstream writeFile("generated_subspaces.json");
    writeFile << stringjson;
    writeFile.close();
    
    return 0;

}
