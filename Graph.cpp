#include "Graph.hpp"
#include "Renderer.hpp"
#include <iostream>
#include "Algorithm.hpp"
#include <math.h>
#include "GlobalRecources.hpp"

Graph::Graph(int i_ys, int i_xs, float i_accuracy) {
    accuracy = i_accuracy;
    this->graphNodeCount = 0;
    this->xCount = (float)i_xs * accuracy;
    this->yCount = (float)i_ys * accuracy;
    int lenght = this->yCount * xCount;

    //init everything with given sizes
    neighbourCount = shared_ptr<int[]>(new int[lenght]);
    neighbourIndices = new int*[lenght];//for every node there can be 8 neighbours, second dimension initialized in loop
    indexBoundXs = shared_ptr<int[]>(new int[lenght]);
    indexBoundYs = shared_ptr<int[]>(new int[lenght]);
    heapIndices = shared_ptr<int[]>(new int[lenght]);
    usedByMoveable = shared_ptr<bool[]>(new bool[lenght]);

    rawIndices = new int*[this->yCount];
    neighbourCosts = nullptr; //initialized later for every run of algorithm

    deactivatedX = vector<int>();
    deactivatedY = vector<int>();
}

Graph::~Graph() {
    for (int i = 0; i < graphNodeCount; i++) {
        delete[] neighbourIndices[i];
        delete[] neighbourCosts[i];
        if (i < yCount) {//very unlikely to be bigger than graphnodecount, caus that would mean less than a single row of the graph is filled
            delete[] rawIndices[i];
        }
    }
    if (yCount > graphNodeCount) {//were catching the "yCount > graphnodecount" issue anyway caus mem leaks are evil
        for (int i = graphNodeCount; i < yCount; i++) {
            delete[] rawIndices[i];
        }
    }
    delete[] neighbourCosts;
    delete[] neighbourIndices;
    delete[] rawIndices;
}

/* MEMO:
isUseable = new bool*[this->yCount];
for (int y = 0; y < this->yCount; y++) {
    isUseable[y] = new bool[this->xCount];
}*/

void Graph::generateWorldGraph(bool** isUseable) {
    for (int y = 0; y < yCount; y++) {
        //one y
        rawIndices[y] = new int[xCount];

        for (int x = 0; x < xCount; x++) {
            neighbourIndices[graphNodeCount] = new int[8];
            //all yCount and xCount have been created

            //this should be initialized in "makeRectNodesUnusable()" if it was declared unusable
            //we have to do one less iteration through everything if we ask that here :)
            if (isUseable[y][x] == true) {
                rawIndices[y][x] = graphNodeCount;
                usedByMoveable[graphNodeCount] = false;
                indexBoundXs[graphNodeCount] = x;
                indexBoundYs[graphNodeCount] = y;

                //isUseable is false if it has been initialized as such in "makeRectNodesUnusable()"
                neighbourCount[graphNodeCount] = 0;
                int xIndex;
                int yIndex;
                if (x > 0) {
                    xIndex = x - 1;
                    yIndex = y;
                    if (isUseable[yIndex][xIndex] == true) {
                        neighbourIndices[rawIndices[yIndex][xIndex]][neighbourCount[rawIndices[yIndex][xIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[yIndex][xIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yIndex][xIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0) {
                    xIndex = x;
                    yIndex = y - 1;
                    if (isUseable[yIndex][xIndex] == true) {
                        neighbourIndices[rawIndices[yIndex][xIndex]][neighbourCount[rawIndices[yIndex][xIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[yIndex][xIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yIndex][xIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0 && x > 0) {
                    xIndex = x - 1;
                    yIndex = y - 1;
                    if (isUseable[yIndex][xIndex] == true) {
                        neighbourIndices[rawIndices[yIndex][xIndex]][neighbourCount[rawIndices[yIndex][xIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[yIndex][xIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yIndex][xIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                if (y > 0 && x < xCount - 1) {
                    xIndex = x + 1;
                    yIndex = y - 1;
                    if (isUseable[yIndex][xIndex] == true) {
                        neighbourIndices[rawIndices[yIndex][xIndex]][neighbourCount[rawIndices[yIndex][xIndex]]] = graphNodeCount;
                        neighbourCount[rawIndices[yIndex][xIndex]]++;
                        neighbourIndices[graphNodeCount][neighbourCount[graphNodeCount]] = rawIndices[yIndex][xIndex];
                        neighbourCount[graphNodeCount]++;
                    }
                }
                graphNodeCount++;
            } //End one node
            else {
                rawIndices[y][x] = -1;
            }
        } //end one y
    } //end all yCount

    neighbourCosts = new int* [graphNodeCount];
    for (int i = 0; i < graphNodeCount; i++) {
        neighbourCosts[i] = new int[8];
    }
}

struct IntPoint{
    int y;
    int x;
    IntPoint(int i_y, int i_x) {
        y = i_y;
        x = i_x;
    }
};

int Graph::findNextUseableVertex(int y, int x, bool moveableRelevant) {
    //do (basically) breathfirstsearch: add 1 to yCount, then 1 to xCount. then add minus 1 to yCount and xCount. then +2 -2 etc. Till you 
    //found the next best useable node, which is also the nearest. 

    int cIndex = -1;

    bool** visited = new bool*[yCount];
    for (int y = 0; y < yCount; y++) {
        visited[y] = new bool[xCount];
        for (int x = 0; x < xCount; x++) {
            visited[y][x] = false;
        }
    }
    


    vector<IntPoint>* toVisit = new vector<IntPoint>();
    //while no suitable index found (rawIndices[tempY][tempX] is -1 if unuseable) and in bounds of window)
    IntPoint cPoint = IntPoint(y, x);
    toVisit->push_back(cPoint);
    while (cIndex == -1) {
        //those two commands are basically "dequeue" from a regular queue.
        cPoint = toVisit->at(0);
        toVisit->erase(toVisit->begin());
        int cY = cPoint.y;
        int cX = cPoint.x;


        visited[cY][cX] = true;//dont use node again
        for (int i = 0; i < 4; i++) {
            if (cY + 1 < yCount) {//bound check
                if (visited[cY + 1][cX] == false) {
                    toVisit->push_back(IntPoint(cY + 1, cX));
                    visited[cY + 1][cX] = true;//dont use node again
                }
            }
            if (cX + 1 < xCount) {//bound check
                if (visited[cY][cX + 1] == false) {
                    toVisit->push_back(IntPoint(cY, cX + 1));
                    visited[cY][cX + 1] = true;//dont use node again
                }
            }
            if (cY - 1 >= 0) {//bound check
                if (visited[cY - 1][cX] == false) {
                    toVisit->push_back(IntPoint(cY - 1, cX));
                    visited[cY - 1][cX] = true;//dont use node again
                }
            }
            if (cX - 1 >= 0) {//bound check
                if (visited[cY][cX - 1] == false) {
                    toVisit->push_back(IntPoint(cY, cX - 1));
                    visited[cY][cX - 1] = true;//dont use node again
                }
            }
        }

       

        cIndex = rawIndices[cY][cX];//check if index is valid now
        if (cIndex != -1) {
            if (moveableRelevant == true) {
                if (usedByMoveable[cIndex] == true) {
                    cIndex = -1;
                }
            }
        }
    }

    for (int y = 0; y < yCount; y++) {
        delete[] visited[y];
    }
    delete[] visited;
    delete toVisit;
    if (cIndex >= 0) {
        return cIndex;
    }
    else {
        cout << "error in findUseableVertex";
        exit(0);
    }
}

void Graph::findNextUseableCoords(int* io_x, int* io_y, bool moveableRelevant) {
    float x = (float) *io_x * accuracy;
    float y = (float) *io_y * accuracy;

    int cIndex = rawIndices[(int)y][(int)x];//check if index is valid now
    if (cIndex != -1) {
        if (moveableRelevant == true) {
            if (usedByMoveable[cIndex] == true) {
                cIndex = -1;
            }
        }
    }
    if (cIndex != -1) {
        return;//dont change coords
    }

    int index = findNextUseableVertex(y, x, moveableRelevant);
    *io_x = (float)indexBoundXs[index] / accuracy;
    *io_y = (float)indexBoundYs[index] / accuracy;
}

int Graph::getIndexFromCoords(int y, int x, bool moveableRelevant) {
    y = round(((float)y * accuracy));
    x = round(((float)x * accuracy));

    if (y < yCount && x < xCount) {
        if (moveableRelevant == true) {
            if (rawIndices[y][x] >= 0 && usedByMoveable[rawIndices[y][x]] == false) {
                return rawIndices[y][x];
            }
            else {
                return findNextUseableVertex(y, x, moveableRelevant);
            }
        }
        else {
            if (rawIndices[y][x] >= 0) {
                return rawIndices[y][x];
            }
            else {
                return findNextUseableVertex(y, x, moveableRelevant);
            }
        }
    }
    else {
        cout << "out of bounds y or x in 'Graph->getIndexFromCoords'";
        exit(0);
    }
}



bool debug = false;

void Graph::disableObjectBounds(int y, int x, int width, int height) {
    GlobalRecources::pfMtx->lock();
    y = (float) y * accuracy;
    x = (float) x * accuracy;
    width = (float) width * accuracy;
    height = (float) height * accuracy;

    int minY = y + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = x + 1;
    if (minX < 0) {
        minX = 0;
    }

    int maxX = x + width - 1;
    if (maxX >= xCount) {
        maxX = xCount - 1;
    }
    int maxY = y + height - 1;
    if (maxY >= yCount) {
        maxY = yCount - 1;
    }

    for (int y = minY; y <= maxY; y++) {
        for (int x = minX; x <= maxX; x++) {
            int index = getIndexFromCoords((float) y / accuracy, (float) x / accuracy, false);
            usedByMoveable[index] = true;
            if (debug == true) {
                deactivatedX.push_back((float) x / accuracy);
                deactivatedY.push_back((float) y / accuracy);
            }
        }
    }

    GlobalRecources::pfMtx->unlock();
}

void Graph::enableObjectBounds(int y, int x, int width, int height) {
    GlobalRecources::pfMtx->lock();

    y = (float) y * accuracy;
    x = (float) x * accuracy;
    width = (float) width * accuracy;
    height = (float) height * accuracy;

    int minY = y - height + 1;
    if (minY < 0) {
        minY = 0;
    }
    int minX = x - width + 1;
    if (minX < 0) {
        minX = 0;
    }   

    int maxX = x + width - 1;
    if (maxX >= xCount) {
        maxX = xCount - 1;
    }
    int maxY = y + height - 1;
    if (maxY >= yCount) {
        maxY = yCount - 1;
    }

    for (int y = minY; y <= maxY; y ++) {
        for (int x = minX; x <= maxX; x ++) {
            int index = getIndexFromCoords((float) y / accuracy, (float) x / accuracy, false);
            usedByMoveable[index] = false;

            if (debug == true) {
                for (int i = 0; i < deactivatedY.size(); i++) {
                    if (deactivatedY.at(i) == (float)y / accuracy && deactivatedX.at(i) == (float)x / accuracy) {
                        deactivatedX.erase(deactivatedX.begin() + i);
                        deactivatedY.erase(deactivatedY.begin() + i);
                    }
                }
            }
        }
    }

    GlobalRecources::pfMtx->unlock();
}

void Graph::moveObject(int y, int x, int oldY, int oldX, int width, int height) {
    enableObjectBounds(oldY, oldX, width, height);
    disableObjectBounds(y, x, width, height);
}



void Graph::debugDrawing() {
    debug = true;
    for (int i = 0; i < deactivatedX.size(); i++) {
        Renderer::drawRect(deactivatedX.at(i), deactivatedY.at(i), 2, 2, sf::Color(255, 255, 0, 255), false);
    }
}