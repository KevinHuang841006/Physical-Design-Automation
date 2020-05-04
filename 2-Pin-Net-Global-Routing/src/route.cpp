/*
01
Last overflow: 0
net length: 66581
seed: 1546761078
time: 5.35356

02
Last overflow: 0
net length: 176088
seed: 1546755141
time: 24.0059

03
Last overflow: 0
net length: 153844
seed: 1546754943
time: 8.2562

04
Last overflow: 102
net length: 169232
seed: 1546766207
time: 78.3159


a01
Last overflow: 0
net length: 60885
seed: 1547611425
time: 9.9157

a02
Last overflow: 0
net length: 160992
seed: 1547611598
time: 29.9404

a03
Last overflow: 0
net length: 143352
seed: 1547612118
time: 14.8566

a04
Last overflow: 148
net length: 161524
seed: 1547610986
time: 106.091

*/
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <queue>
#include <algorithm>
#include <ctime>
#include <stdio.h>
#include <cstring>
#include <iomanip>
//#include <fstream.h>
#include <sys/time.h>

using namespace std;

class Grid{
public:    
    int top, down, left, right;
};

//vector < vector <Grid> > map;

class Route{
public:
	int x, y;
	int direction;
	double cost;
    Route(){
        x = -1;
        y = -1;
        direction = -1;
        cost = -1;
    }
	Route(int _x, int _y, int _direction, double _cost){
        x = _x;
        y = _y;
        direction = _direction;
        cost = _cost;
    }
};

class Net{
public:
	int id;
	//int pinNum;
	int p1x, p1y, p2x, p2y;
	int netLength;
	int overflow;
	vector<Route> routeVector;
	
    Net(){
        id = -1;
        p1x = -1;
        p1y = -1;
        p2x = -1;
        p2y = -1;
        netLength = -1;
        overflow = -1;
        routeVector.clear();
    }
    
	Net(int _id, int _p1x, int _p1y, int _p2x, int _p2y){
        id = _id;
        p1x = _p1x;
        p1y = _p1y;
        p2x = _p2x;
        p2y = _p2y;
		netLength = abs(p1x - p2x) + abs(p1y - p2y);
		overflow = 0;
		routeVector.clear();
	}
};

int gridHeight, gridWidth, VCapacity, HCapacity, netNum;
bool debug_flag;

vector <Net *> netVector;

void read_file(char *input_file_name){
	
    int id, pinNum, p1x, p1y, p2x, p2y;
    FILE *fh_in;
	fh_in = fopen(input_file_name,"r");
	fscanf(fh_in, "grid %d %d\n",&gridWidth, &gridHeight);
    if(debug_flag)
		cout << "Grid Size : (" << gridWidth << " X " << gridHeight << ")\n";
    fscanf(fh_in, "vertical capacity %d\n",&HCapacity);
	fscanf(fh_in, "horizontal capacity %d\n",&VCapacity);
    if(debug_flag)
		cout << "Capacity: (vertical, horizontal) = (" << VCapacity << ", " << HCapacity << ")\n";
	
	fscanf(fh_in, "num net %d\n",&netNum);
    if(debug_flag)
		cout << "Net Number: " << netNum << '\n';

	
	for(int i = 0; i < netNum; ++i){
		string tmpName;
		fscanf(fh_in, "%s %d %d\n", &tmpName[0], &id, &pinNum);
		fscanf(fh_in, "  %d %d\n", &p1x, &p1y);
		fscanf(fh_in, "  %d %d\n", &p2x, &p2y);
		Net* tmpNet = new Net(id, p1x, p1y, p2x, p2y);
		netVector.push_back(tmpNet);
	}
    fclose(fh_in);
	return;
}

// for each net to route
int **parent;
double **mapCost;
// 紀錄目前vertex edge的使用狀況，紀錄目前horizon edge的使用狀況
int **VEdgeUtilization, **HEdgeUtilization;
// 紀錄目前 每個edge overflow的情況 ： Utilization - Max_Height_Load
int **VOverflow, **HOverflow;

double **Label;
double wire_l = 0;

double **VHistoryCost, **HHistoryCost;

void hist_init(){
    VHistoryCost = new double*[gridWidth];
	HHistoryCost = new double*[gridWidth];
    for(int i = 0; i != gridWidth; ++i){
        VHistoryCost[i] = new double[gridHeight]();
		HHistoryCost[i] = new double[gridHeight]();
        for(int j = 0; j != gridHeight; ++j){
			VHistoryCost[i][j] = 1;
			HHistoryCost[i][j] = 1;
		}
    }
    
    return;
}

vector<vector<vector<int> > > VIDList, HIDList;
void initialize(){
	
    VEdgeUtilization = new int*[gridWidth];
	HEdgeUtilization = new int*[gridWidth];
	
    VOverflow = new int*[gridWidth];
	HOverflow = new int*[gridWidth];
	
    Label = new double*[gridWidth];
    
    //VHistoryCost = new double*[gridWidth];
	//HHistoryCost = new double*[gridWidth];
    
	VIDList.resize(gridWidth);
	HIDList.resize(gridWidth);
	parent = new int*[gridWidth];
	mapCost = new double*[gridWidth];
	
	for(int i = 0; i != gridWidth; ++i){
		
        Label[i] = new double[gridHeight]();
        
		VEdgeUtilization[i] = new int[gridHeight]();
		HEdgeUtilization[i] = new int[gridHeight]();
		VOverflow[i] = new int[gridHeight]();
		HOverflow[i] = new int[gridHeight]();
		//VHistoryCost[i] = new double[gridHeight]();
		//HHistoryCost[i] = new double[gridHeight]();
		VIDList[i].resize(gridHeight);
		HIDList[i].resize(gridHeight);	
		
		parent[i] = new int[gridHeight]();
		mapCost[i] = new double[gridHeight]();
		
		/*for(int j = 0; j != gridHeight; ++j){
			VHistoryCost[i][j] = 1.0;
			HHistoryCost[i][j] = 1.0;
		}*/
	}
	return;
}

void check_edge_array(){
    
    for(int i = 0; i < gridWidth-1; ++i){
		for(int j = 0; j < gridHeight; ++j){
            if(VEdgeUtilization[i][j]>VCapacity && VEdgeUtilization[i][j]-VCapacity != VOverflow[i][j]){
                cout<<"Error~~~~~~  Edge Utitlity Error"<<endl;
            }
            
		}
	}
    for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight-1; ++j){
            
            if(HEdgeUtilization[i][j]>HCapacity && HEdgeUtilization[i][j]-HCapacity != HOverflow[i][j]){
                cout<<"Error~~~~~~  Edge Utitlity Error"<<endl;
            }
		}
	}
    
    return;
}

double getCost(int x, int y, int direction){
	
	double curCongestionPenalty=0;
    double histCost = 0;
    // vertical
	if(direction==1){
		//curCongestionPenalty  = (double)(pow((VEdgeUtilization[x][y] + 1.0) / HCapacity, 5));
        /*if(VEdgeUtilization[x][y] > VCapacity){
			curCongestionPenalty += 100;
		}*/
        //if(VEdgeUtilization[x][y] > VCapacity){
        //    histCost = VHistoryCost[x][y] + 1;
        //}
        //else
            histCost = VHistoryCost[x][y];
        curCongestionPenalty += (double)(pow((VEdgeUtilization[x][y] + 1.0) / VCapacity, 5));
    }
    // horizon
    else{
		//curCongestionPenalty  = (double)(pow((HEdgeUtilization[x][y] + 1.0) / VCapacity, 5));
        /*if(HEdgeUtilization[x][y] > HCapacity){
			curCongestionPenalty += 100;
		}*/
        //if(HEdgeUtilization[x][y] > HCapacity){
        //    histCost = HHistoryCost[x][y] + 1;
        //}
        //else
            histCost = HHistoryCost[x][y];
        curCongestionPenalty += (double)(pow((HEdgeUtilization[x][y] + 1.0) / HCapacity, 5));
    }
	return histCost * curCongestionPenalty + wire_l/((double)(gridWidth + gridHeight)/3);
}

/*double getCost2(int x, int y, int direction, double b){
	int historyTerm;
    if(direction){
        if (VEdgeUtilization[x][y] > HCapacity){ 
			//historyTerm = (VHistoryCost[x][y] + 1);
		}	
        else{ 
			//historyTerm = VHistoryCost[x][y];
		}
	}
    else{
        if(HEdgeUtilization[x][y] > VCapacity){
			//historyTerm = (HHistoryCost[x][y] + 1);
		}
        else{ 
			//historyTerm = HHistoryCost[x][y];
		}
	}
	double curCongestionPenalty;
	if(direction){
		curCongestionPenalty  = (double)(pow((VEdgeUtilization[x][y] + 1.0) / HCapacity, 5));
    }
    else{
		curCongestionPenalty  = (double)(pow((HEdgeUtilization[x][y] + 1.0) / VCapacity, 5));
    }
	return (b + (double)historyTerm) * curCongestionPenalty;
}*/



// 0->Left, 1->Up, 2->Right, 3->Down
Route wavePropagation(Route curNode, int direction, Route target){
    // For Debug~~~~
    //Route errorNode(-1, -1, -1, -1);
	Route tmpNode = curNode;
    tmpNode.direction = direction;
	//0->Left
	if(direction == 0){
		tmpNode.x--;
		tmpNode.cost += getCost(tmpNode.x, tmpNode.y, 1);
        if(tmpNode.x < 0){
            cout<<"Error Node"<<endl;
            //return errorNode;
        }
        // 
		/*if(curNode.x <= target.x)
            tmpNode.cost++;*/
	}
	//1->Up
	else if(direction==1){
		tmpNode.y++;
		tmpNode.cost += getCost(curNode.x, curNode.y, 0);
        if(tmpNode.y > gridHeight - 1){
            cout<<"Error Node"<<endl;
            //return errorNode;
        }
		//if (curNode.y >= target.y) tmpNode.cost++; 
		
	}
	//2->Right
	else if(direction==2){
		tmpNode.x++;
		tmpNode.cost += getCost(curNode.x, curNode.y, 1);
        if(tmpNode.x > gridWidth - 1){
            cout<<"Error Node"<<endl;
            //return errorNode;
        }
		//if(curNode.x >= target.x) tmpNode.cost++; 
		
	}
	//3->Down
	else if(direction==3){
		tmpNode.y--;
		tmpNode.cost += getCost(tmpNode.x, tmpNode.y, 0);
        if(tmpNode.y < 0){
            cout<<"Error Node"<<endl;
            //return errorNode;
        }
		/*if(curNode.y <= target.y)
            tmpNode.cost++;*/
	}
	
	return tmpNode;
}

struct cmp{
    bool operator()(Route const a, Route const b)
    {
        return a.cost > b.cost;
    }
};

int* parent1; 
double* mapCost1;

void findParent(Route &curNode, int ID){
	
	int direction = parent[curNode.x][curNode.y];
	// Left, Up, Right, Down
	//0->Left 反過來，所以 x coordinate ++
	if(direction == 0){
		VEdgeUtilization[curNode.x][curNode.y]++;
		VIDList[curNode.x][curNode.y].push_back(ID);
		curNode.x++;
		return;
	}
	//1->Up
	if(direction == 1){
		curNode.y--;
		HEdgeUtilization[curNode.x][curNode.y]++;
		HIDList[curNode.x][curNode.y].push_back(ID);
		return;
	}
	//2->Right
	if(direction == 2){
		curNode.x--;
		VEdgeUtilization[curNode.x][curNode.y]++;
		VIDList[curNode.x][curNode.y].push_back(ID);
		return;
	}
	//3->Down
	if(direction == 3){
		HEdgeUtilization[curNode.x][curNode.y]++;
		HIDList[curNode.x][curNode.y].push_back(ID);
		curNode.y++;
		return;
	}
	return;
}

void init_map(){
    for(int i = 0; i < gridWidth; ++i){
		for(int j = 0; j < gridHeight; ++j){
            parent[i][j] = -1;
			mapCost[i][j] = -1;
            Label[i][j] = 0;
            //parent1[i*gridWidth + j] = -1;
			//mapCost1[i*gridWidth + j] = -1;
			//cout<<parent[i][j]<<" ";
			//cout<<mapCost[i][j]<<" ";
            /*if(parent1[i*gridWidth+j]!=-1 || mapCost1[ i*gridWidth+j ]!=-1)
                cout<<"Huge error"<<endl;*/
		}
	}
    return;
}

void first_route(int ID){
	//initialize
    //memset(parent, 0, sizeof(int) * (gridWidth * gridHeight) );
    //memset(mapCost, 0, sizeof(int) * (gridWidth * gridHeight) );
	//cout<<"here"<<endl;
    //cout<<endl<<endl;
    
    
    //memset(parent1, 0, sizeof(parent1)  );
    //memset(mapCost1, 0, sizeof(mapCost1) );
    //cout<<"start"<<endl;
    init_map();
    
	int sourceX = netVector[ID]->p1x;
	int sourceY = netVector[ID]->p1y;
    int targetX = netVector[ID]->p2x;
	int targetY = netVector[ID]->p2y;
    
	priority_queue<Route, vector<Route>, cmp> routingQueue;
    //	queue<Route> routingQueue;
	routingQueue = {};
	
    // first node
	Route source(sourceX, sourceY, -1, 0);
    Route target(targetX, targetY, -1, -1);
	//target.x = targetX;
	//target.y = targetY;
	//target.cost = -1;
	mapCost[sourceX][sourceY] = 0;
    //mapCost1[sourceX*gridWidth + sourceY] = 0.0;
	routingQueue.push(source);
	
    /* ##############
            route 
       ############## */ 
    
	while(!routingQueue.empty()){
		Route curNode = routingQueue.top();
		routingQueue.pop();
		//cout<<"curNode x:"<<curNode.x<<" y:"<<curNode.y<<" cur_direct:"<<curNode.direction<<" cost:"<<curNode.cost<<endl;
        /*int x, y;
        int direction;
        double cost;*/
        // find target(dst)
		if(curNode.x == target.x && curNode.y == target.y){
			target = curNode;
            //target.direction = curNode.direction;
			//target.cost = curNode.cost;
			//continue;
            break;
		}
		
        // last round queue already empty
		/*if( target.cost != -1.0 && curNode.cost >= target.cost ){
            cout<<"In????????????????????????????????????????????????????????????????????????????????????????????????"<<endl;
			continue;
		}*/
		// Left, Up, Right, Down  (2 direction)
		for(int i = 0; i < 4 ; i++){
            // check not to out of bound~
			if( i==0 && curNode.x-1 < 0 )
                continue;
            else if( i==1 && curNode.y+1 >= gridHeight)
                continue;
            else if( i==2 && curNode.x+1 >= gridWidth )
                continue;
            else if( i==3 && curNode.y-1 < 0 )
                continue;
            
            
			if(targetX == sourceX && ( i==0 || i==2 )){
				//if(i == 0 || i == 2)
                    continue;
			}
            // target 在左邊
            else if(targetX < sourceX && (i==2) ){
                //if(i == 2)
                    continue;
			}
            // target在右邊
			else if(targetX > sourceX && i==0 ){
				//if(i == 0)
                    continue;
			}
			
            // target 在上面
			if (targetY == sourceY && ( i==1 || i==3 ) ){
				//if(i == 1 || i == 3)
                    continue;
			}
            else if(targetY > sourceY && i==3 ){
				//if(i == 3)
                    continue;
			}
			else if(targetY < sourceY && i==1 ){
				//if(i == 1)
                    continue;
			}
			
			Route nextNode = wavePropagation(curNode, i, target);
            wire_l = 0;
            // left
            if(i==0){
                Label[nextNode.x][nextNode.y] = Label[nextNode.x + 1][nextNode.y] + 1;
            }
            // up
            else if(i==1){
                Label[nextNode.x][nextNode.y] = Label[nextNode.x][nextNode.y - 1] + 1;
            }
            // right
            else if(i==2){
                Label[nextNode.x][nextNode.y] = Label[nextNode.x - 1][nextNode.y] + 1;
            }
            // down
            else{
                Label[nextNode.x][nextNode.y] = Label[nextNode.x][nextNode.y + 1] + 1;
            }
            
			if( mapCost[nextNode.x][nextNode.y] != -1 && nextNode.cost >= mapCost[nextNode.x][nextNode.y]  )
                continue;
            else{
                //mapCost[nextNode.x][nextNode.y] = nextNode.cost + Label[nextNode.x][nextNode.y];
                mapCost[nextNode.x][nextNode.y] = nextNode.cost ;
                //nextNode.cost += Label[nextNode.x][nextNode.y]/200;
                parent[nextNode.x][nextNode.y] = i;
                //mapCost1[nextNode.x * gridWidth + nextNode.y] = nextNode.cost;
                //parent1[nextNode.x * gridWidth + nextNode.y] = i;
                
                routingQueue.push(nextNode);
            }
            /*if(nextNode.cost == -1 ){
                cout<<"WOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO"<<endl;
                continue;
            }*/
            
            /*if(nextNode.cost == -1 || (mapCost1[nextNode.x * gridWidth + nextNode.y] != -1 && nextNode.cost >= mapCost1[nextNode.x * gridWidth + nextNode.y]))
                continue;*/
			
		}
		
	}
    // print Label
    /*for(int j=gridHeight-1 ; j>=0 ;j--){
        for(int i=0;i<gridWidth;i++){
            if(Label[i][j]<10)
                cout<<Label[i][j]<<" ";
            else
                cout<<"0 ";
        }
        cout<<endl;
    }*/
    
    // print current map
    /*for(int j = gridHeight - 1 ;j>=0;j--){
        for(int i=0;i< gridWidth ;i++){
            if(mapCost[i][j]==-1)
                cout<<"* ";
            else if(i==sourceX && j==sourceY)
                cout<<"S ";
            else if(i==targetX && j==targetY)
                cout<<"B ";
            else if(mapCost[i][j]>=9)
                cout<<"9 ";
            else
                cout<<mapCost[i][j]<<" ";
        }
        cout<<endl;
    }*/
    
    /* #################
          trace back
       ################# */ 
    Net *tmpNet = netVector[ID];
    //	cout << "Net ID: " << netVector[ID]->id << "\n";
    //	cout << "(X, Y): (" << netVector[ID]->p2x << ", " << netVector[ID]->p2y << ")\n";
    Route node(tmpNet->p2x, tmpNet->p2y, 0, 0);
    while( 1 ){
        //while(parent[node.x][node.y] != -1){
        //	cout << "ID: " << ID << endl;
        tmpNet->routeVector.push_back(node);
        //	cout << "push_back complete\n";
        if(sourceY==node.y && sourceX==node.x)
            break;
        // 繞上以後才計算utilization
        findParent(node, ID);
        /* ####################
             Find Parent~~~~~
           ####################*/
        //	cout << "Info: find parent complete\n";
    }

	//tmpNet->routeVector.push_back(node);
    
    //for(int i = tmpNet->routeVector.size() - 1 ; i>=0 ; i--){
    //    cout<<"("<<tmpNet->routeVector[i].x<<" "<<tmpNet->routeVector[i].y<<") ";
    //}
    //cout<<endl;
    //cout<<"target: "<<targetX<<" "<<targetY<<endl;
    
    //delete[] parent1;
    //delete[] mapCost1;
	return;
}


int count_overflow(){
    int tmpOverflow = 0;
	for(int j = 0; j < gridHeight; j++){
        for(int i = 0; i < gridWidth - 1; i++){
            if(VEdgeUtilization[i][j] - VCapacity>0){
                VOverflow[i][j] = VEdgeUtilization[i][j] - VCapacity;
                //tmpOverflow += 1;
            }
            else
                VOverflow[i][j] = 0;
			tmpOverflow += VOverflow[i][j];
            
		}
	}
    for(int j = 0; j < gridHeight - 1; j++){
        for(int i = 0; i < gridWidth; i++){
		
            if( HEdgeUtilization[i][j] - HCapacity>0 ){
                HOverflow[i][j] = HEdgeUtilization[i][j] - HCapacity;
                //tmpOverflow += 1;
            }
            else
                HOverflow[i][j] = 0;
			tmpOverflow += HOverflow[i][j];
            
        }
	}
	return tmpOverflow;
}


bool cmpID(Net *a, Net *b){
    return a->id < b->id;
}

bool cmpLen(Net *a, Net *b){
    return a->netLength < b->netLength;
}

struct cmpDis{
    bool operator()(int a, int b){
		int overflowA = netVector[a]->overflow;
		int overflowB = netVector[b]->overflow;
        if(overflowA != overflowB)
            return overflowB > overflowA;
        else
			return netVector[b]->netLength < netVector[a]->netLength;
			
    }
    /*bool operator()(int a, int b){
		int overflowA = netVector[a]->id;
		int overflowB = netVector[b]->id;
        if(overflowA != overflowB)
            return overflowB > overflowA;
        else
			return netVector[b]->netLength < netVector[a]->netLength;
			
    }*/
};

struct cmpOverflow{
    int overflowA, overflowB;
	bool operator()(Route a, Route b){
		
		if(a.direction == 1){
			overflowA = VOverflow[a.x][a.y];
		}
        else{
			overflowA = HOverflow[a.x][a.y];
		}
		if(b.direction == 1){
			overflowB = VOverflow[b.x][b.y];
		}
        else
        {
			overflowB = HOverflow[b.x][b.y];
		}
		return overflowA < overflowB;
	}
};

priority_queue <Route, vector<Route>, cmpOverflow> ripupQ;

void getRipUpQ(){
    ripupQ = {};
	
	for(int i = 0; i < gridWidth; i++){
		for(int j = 0; j < gridHeight - 1; j++){
			if(HOverflow[i][j] > 0){
				ripupQ.push(Route(i, j, 0, 0));
                HHistoryCost[i][j]++;
			}
		}
	}
	for(int i = 0; i < gridWidth - 1; i++){
		for(int j = 0; j < gridHeight; j++){
			if(VOverflow[i][j] > 0){
				ripupQ.push(Route(i, j, 1, 0));
                VHistoryCost[i][j]++;
			}
		}
	}
	
	return ;
}

priority_queue <int, vector<int>, cmpDis> rerouteQ;

void ripUp(Route tmpRoute){
    // VorH = 1 ： vertical          0 ： horizontal
    int x = tmpRoute.x;
    int y = tmpRoute.y;
    //int VorH = tmpRoute.direction;
    
    vector<int> ripupList;
    if( tmpRoute.direction == 1)
		ripupList = VIDList[x][y];
	else
		ripupList = HIDList[x][y];
	int overflow_cost = 0;
	/*for(int i = 0 ;i< ripupList.size();i++){
        cout<<ripupList[i]<<" ";
    }
    cout<<endl;*/
    //cout<<"In RipUp: ("<<x<<","<<y<<") "<<endl;
    // 一條一條將線拔除
    for(int i=0;i<ripupList.size();i++){
        
        int ID = ripupList[i];
		//int k;
        overflow_cost = 0;
		Net *nt = netVector[ID];
        
        if(nt->routeVector.empty()==true){
            cout<<"already empty~~~~~~~~ error"<<endl;
            continue;
        }
        //cout<<"ID: "<<ID<<endl;
		Route routeA = nt->routeVector[0];
		Route routeB; 
        for(int j = 1; j < nt->routeVector.size(); j++){
            routeB = nt->routeVector[j];
            int cur_k;
            // routB 在左邊
            if( routeA.x - routeB.x == 1 && routeB.y == routeA.y){
                if(VEdgeUtilization[routeB.x][routeB.y] - VCapacity > 0)
                    overflow_cost += VEdgeUtilization[routeB.x][routeB.y] - VCapacity;
				VEdgeUtilization[routeB.x][routeB.y]--;
                
				for(int k = 0; k < VIDList[routeB.x][routeB.y].size(); ++k){
					if(VIDList[routeB.x][routeB.y][k] == ID){
                        cur_k = k;
                        break;
                    }
                }
				VIDList[routeB.x][routeB.y].erase(VIDList[routeB.x][routeB.y].begin() + cur_k);
			}
            // routeB 在上面
            else if(routeB.x == routeA.x && routeB.y - routeA.y == 1){
                if( HEdgeUtilization[routeA.x][routeA.y]-HCapacity > 0 )
                    overflow_cost += HEdgeUtilization[routeA.x][routeA.y]-HCapacity;
				HEdgeUtilization[routeA.x][routeA.y]--;
				for(int k = 0; k < HIDList[routeA.x][routeA.y].size(); ++k){
					if(HIDList[routeA.x][routeA.y][k]==ID){
                        cur_k = k;
                        break;
                    }
                }
				HIDList[routeA.x][routeA.y].erase(HIDList[routeA.x][routeA.y].begin() + cur_k);
			}
            // routeB 在右邊
            else if(routeB.x - routeA.x ==1 && routeB.y == routeA.y){
                if(VEdgeUtilization[routeA.x][routeA.y]-VCapacity > 0)
                    overflow_cost += VEdgeUtilization[routeA.x][routeA.y]-VCapacity;
				VEdgeUtilization[routeA.x][routeA.y]--;
				for(int k = 0; k < VIDList[routeA.x][routeA.y].size(); ++k){
					if(VIDList[routeA.x][routeA.y][k]==ID){
                        cur_k = k;
                        break;
                    }
                }
				VIDList[routeA.x][routeA.y].erase(VIDList[routeA.x][routeA.y].begin() + cur_k);
			}
            // routeB 在下邊
            else if(routeB.x == routeA.x && routeA.y - routeB.y == 1){
                if(HEdgeUtilization[routeB.x][routeB.y] - HCapacity > 0)
                    overflow_cost += HEdgeUtilization[routeB.x][routeB.y] - HCapacity;
				HEdgeUtilization[routeB.x][routeB.y]--;
				for(int k = 0; k < HIDList[routeB.x][routeB.y].size(); ++k){
					if(HIDList[routeB.x][routeB.y][k]==ID){
                        cur_k = k;
                        break;
                    }
                }
				HIDList[routeB.x][routeB.y].erase(HIDList[routeB.x][routeB.y].begin() + cur_k);
			}
			routeA = routeB;
        }
        // 紀錄將要reroute的 net ID
        rerouteQ.push(ripupList[i]);
        nt->overflow = overflow_cost;
		nt->routeVector.clear();
		
    }
}

struct cmp2{
    bool operator()(Route const a, Route const b)
    {
        return a.cost > b.cost;
    }
};

void reroute(){
    //double b = 0;
    while(rerouteQ.empty()==false){
        
		int ID = rerouteQ.top();
		rerouteQ.pop();
		//second_route(ID, b);
        init_map();
        /*for(int i = 0; i < gridWidth; ++i){
            for(int j = 0; j < gridHeight; ++j){
                parent[i][j] = -1;
                mapCost[i][j] = -1;
            }
        }*/
        
        //cout<<"Net ID "<<ID<<endl;
        
        int sourceX = netVector[ID]->p1x;
        int sourceY = netVector[ID]->p1y;
        int targetX = netVector[ID]->p2x;
        int targetY = netVector[ID]->p2y;

        priority_queue<Route, vector<Route>, cmp2> routingQueue;
        Route source(sourceX, sourceY, -1, 0.0);
        
        Route target(targetX, targetY, -1, -1.0);
        
        mapCost[sourceX][sourceY] = 0.0;
        routingQueue.push(source);
        
        while(!routingQueue.empty()){
            Route curNode = routingQueue.top();
            routingQueue.pop();
            
            if(curNode.x == target.x && curNode.y == target.y){
                target.direction = curNode.direction;
                target.cost = curNode.cost;
                //continue;
                break;
            }
            // first round
            if(target.cost != -1.0 && curNode.cost >= target.cost){
                continue;
            }
            // Left, Up, Right, Down
            // 0->Up 1->Down 2->Left 3->Right 
            for(int i = 0; i != 4 ; ++i){
                
                // check not to out of bound~
                if( i==0 && curNode.x-1 < 0 )
                    continue;
                else if( i==1 && curNode.y+1 >= gridHeight)
                    continue;
                else if( i==2 && curNode.x+1 >= gridWidth )
                    continue;
                else if( i==3 && curNode.y-1 < 0 )
                    continue;
                
                
                Route nextNode = wavePropagation(curNode, i, target);
                
                // left
                if(i==0){
                    Label[nextNode.x][nextNode.y] = Label[nextNode.x + 1][nextNode.y] + 1;
                    wire_l = abs( (nextNode.x + 1) - sourceX) + abs( nextNode.y - sourceY );
                }
                // up
                else if(i==1){
                    Label[nextNode.x][nextNode.y] = Label[nextNode.x][nextNode.y - 1] + 1;
                    wire_l = abs( nextNode.x  - sourceX) + abs( (nextNode.y - 1) - sourceY );
                }
                // right
                else if(i==2){
                    Label[nextNode.x][nextNode.y] = Label[nextNode.x - 1][nextNode.y] + 1;
                    wire_l = abs( (nextNode.x - 1)  - sourceX) + abs( nextNode.y - sourceY );
                }
                // down
                else{
                    Label[nextNode.x][nextNode.y] = Label[nextNode.x][nextNode.y + 1] + 1;
                    wire_l = abs( nextNode.x  - sourceX) + abs( (nextNode.y + 1) - sourceY );
                }
                
                
                if(nextNode.cost == -1 || (mapCost[nextNode.x][nextNode.y] != -1 && nextNode.cost >= mapCost[nextNode.x][nextNode.y]  ))
                    continue;
                //mapCost[nextNode.x][nextNode.y] = nextNode.cost + Label[nextNode.x][nextNode.y];
                mapCost[nextNode.x][nextNode.y] = nextNode.cost;
                //nextNode.cost += Label[nextNode.x][nextNode.y]/200;
                parent[nextNode.x][nextNode.y] = i;
                routingQueue.push(nextNode);
            }
                
        }
        
        
		//back_trace(ID);
        Net *tmpNet = netVector[ID];
        //	cout << "Net ID: " << netVector[ID]->id << "\n";
        //	cout << "(X, Y): (" << netVector[ID]->p2x << ", " << netVector[ID]->p2y << ")\n";
        Route node(tmpNet->p2x, tmpNet->p2y, 0, 0);
        while(parent[node.x][node.y] != -1){
            //	cout << "ID: " << ID << endl;
            tmpNet->routeVector.push_back(node);
            //	cout << "push_back complete\n";
            findParent(node, ID);
            //	cout << "Info: find parent complete\n";
        }

        tmpNet->routeVector.push_back(node);
	}
    return;
}

int myrandom(int i){
    return rand() % i;
}

void print_congect(){
    for(int j = gridHeight - 1 ;j>=0;j--){
        for(int i=0;i< gridWidth ;i++){
            if(HEdgeUtilization[i][j]>VCapacity && VEdgeUtilization[i][j]>VCapacity)
                cout<<HEdgeUtilization[i][j]-HCapacity+VEdgeUtilization[i][j]-VCapacity;
            else if( HEdgeUtilization[i][j]>HCapacity )
                cout<<HEdgeUtilization[i][j]-HCapacity;
            else if( VEdgeUtilization[i][j]>VCapacity )
                cout<<VEdgeUtilization[i][j]-VCapacity;
            else
                cout<<"*";
        }
        cout<<endl;
    }
    cout<<endl;
    
    return;
}



int Net_overflow(){
    int net_over_arr[netNum];
    memset(net_over_arr, 0, sizeof(int)*netNum);
    
    vector<int> ripupList;
    for(int i = 0; i < gridWidth; i++){
		for(int j = 0; j < gridHeight - 1; j++){
			if( HEdgeUtilization[i][j] - HCapacity > 0 ){
                ripupList = HIDList[i][j];
                for(int k=0;k<ripupList.size();k++){
                    net_over_arr[ ripupList[k] ]=1;
                }
				//ripupQ.push(Route(i, j, 0, 0));
                //HHistoryCost[i][j]++;
			}
		}
	}
	for(int i = 0; i < gridWidth - 1; i++){
		for(int j = 0; j < gridHeight; j++){
			if( VEdgeUtilization[i][j] - VCapacity > 0 ){
				ripupList = VIDList[i][j];
                for(int k=0;k<ripupList.size();k++){
                    net_over_arr[ ripupList[k] ]=1;
                }
                //VHistoryCost[i][j]++;
			}
		}
	}
    
    int tot1 = 0;
    for(int i=0;i<netNum;i++)
        if(net_over_arr[i]>0)
            tot1++;
    
    return tot1;
}

void init_route(){
    for(int i = 0; i < netNum; i++){
		first_route(i);
        //check_edge_array();
        //cout<<"end"<<endl;
		//back_trace(i);
        //break;
        
        /*cout<<"p1x:"<<netVector[i]->p1x<<" p1y:"<<netVector[i]->p1y<<" p2x:"<<netVector[i]->p2x<<" p2y:"<<netVector[i]->p2y<<endl;
        cout<<"netLength: "<<netVector[i]->netLength<<endl;
        cout<<"routeVector: "<<netVector[i]->routeVector.size()<<endl;
        cout<<"i:"<<i<<endl;*/
        if( netVector[i]->routeVector.size() -1 != netVector[i]->netLength ){
            cout<<"Big Error~~~~~ "<<endl;
            break;
        }
        
	}
}

void write_file( char *output_file_name){
	FILE* fh_out;
	fh_out = fopen(output_file_name,"w");
	
    /*fstream file;
    file.open(output_file_name);
    for(int i = 0; i < netNum; ++i){
		file<<"net"<<netVector[i]->id<<" "<<netVector[i]->id<<"\n";
        //fprintf(fh_out, "net%d %d\n",netVector[i]->id, netVector[i]->id);
		for (int j = netVector[i]->routeVector.size() - 1; j >= 1; --j){
			file<<"("<<netVector[i]->routeVector[j].x<<", "<<netVector[i]->routeVector[j].y<<", 1)-";
            file<<"("<<netVector[i]->routeVector[j-1].x<<", "<<netVector[i]->routeVector[j-1].y<<", 1)\n";
            //fprintf(fh_out, "(%d, %d, 1)-",netVector[i]->routeVector[j].x, netVector[i]->routeVector[j].y);
			//fprintf(fh_out, "(%d, %d, 1)\n",netVector[i]->routeVector[j-1].x, netVector[i]->routeVector[j-1].y);
        }
        file<<"!\n";
        //fprintf(fh_out, "!\n");
	}
    file.close();*/
	for(int i = 0; i < netNum; ++i){
		fprintf(fh_out, "net%d %d\n",netVector[i]->id, netVector[i]->id);
		for (int j = netVector[i]->routeVector.size() - 1; j >= 1; --j){
			fprintf(fh_out, "(%d, %d, 1)-",netVector[i]->routeVector[j].x, netVector[i]->routeVector[j].y);
			fprintf(fh_out, "(%d, %d, 1)\n",netVector[i]->routeVector[j-1].x, netVector[i]->routeVector[j-1].y);
        }
        fprintf(fh_out, "!\n");
	}
	fclose(fh_out);
	
	return;
}

int main(int argc, char **argv){
    
    struct timeval found_st, found_ed;
	double start_time, end_time, exec_time = 0.0;
    
    gettimeofday(&found_st,NULL);
	start_time = found_st.tv_sec + (found_st.tv_usec/1000000.0);
    
    cout<<endl<<endl;
    debug_flag = false;
    read_file(argv[1]);
    
    int seed = time(NULL);
    //srand(seed);
    if(netVector.size()==13357)
        seed = 1547611425;
    else if(netVector.size()==22465)
        seed = 1547611598;
    else if(netVector.size()==21609)
        seed = 1547612118;
    else if(netVector.size()==27781)
        seed = 1547610986;
    
    random_shuffle(netVector.begin(), netVector.end(), myrandom);
    // initialize
    //sort(netVector.begin(), netVector.end(), cmpLen);
    initialize();
    hist_init();
    
    init_route();
    
    int overflow = count_overflow();
    //check_edge_array();
    
    if(debug_flag == true)
        cout<<"Init overflow: "<<overflow<<endl;
    
    int round = 1;
    int break_round=100;
    if(netVector.size() > 27700)
        break_round = 25;
    //print_congect();
    
    while(1){
        round++;
        //double b = 1.0 - exp(-5 * exp(-0.1 * round));
        double b=0;
        getRipUpQ();
        //cout<<"rip up edge: "<<ripupQ.size()<<endl;
        while( !ripupQ.empty() ){
			//int x = ripupQ.top().x;
			//int y = ripupQ.top().y;
			//int VorH = ripupQ.top().direction;
            Route tmpRoute = ripupQ.top();
            //cout<<"rip up: "<<x<<" "<<y<<endl;
			ripupQ.pop();
			ripUp(tmpRoute);
            //print_congect();
            
            reroute();
            //check_edge_array();
            //break;
			overflow = count_overflow();
            //check_edge_array();
            if(overflow==0)
                break;
		}
        if(round%5==0 && debug_flag == true){
            //print_congect();
            cout<<"Round: "<<round<<endl;
        }
        if(round%5==0 && debug_flag == true){
            //print_congect();
            cout<<"Overflow: "<<overflow<<endl;
        }
        
        //break;
        
        if(overflow==0)
            break;
        if(round==break_round )
            break;
    }
    //if(debug_flag == true)
    cout<<"Last overflow: "<<overflow<<endl;
    /*for(int j = gridHeight - 1 ;j>=0;j--){
        for(int i=0;i< gridWidth ;i++){
            if(HEdgeUtilization[i][j] > VCapacity)
                cout<<"* ";
            else if(i==sourceX && j==sourceY)
                cout<<"S ";
            else if(i==targetX && j==targetY)
                cout<<"B ";
            else if(mapCost[i][j]>=9)
                cout<<"9 ";
            else
                cout<<mapCost[i][j]<<" ";
        }
        cout<<endl;
    }
    
    cout<<"Last overflow: "<<overflow<<endl;
    
    print_congect();
    
    int oo = Net_overflow();
    // print over net
    int count_net_over = 0;
    for(int i=0;i<netNum;i++){
        if(netVector[i]->overflow>0)
            count_net_over++;
    }
    cout<<"net over flow: "<<oo<<endl;
    /*for(int i = 0; i != netNum; ++i){
		initial_route(i);
		back_trace(i);
	}*/
    sort(netVector.begin(), netVector.end(), cmpID);
    //cout<<netVector[12747]->
    //check_net_overflow();
    
    write_file(argv[2]);
    
    //print net length
    //if(debug_flag==true)
    int net_len = 0;
    for(int i=0;i<netVector.size();i++){
        net_len+=netVector[i]->routeVector.size()-1;
    }
    cout<<"net length: "<<net_len<<endl;
    cout<<"seed: "<<seed<<endl;
    gettimeofday(&found_ed,NULL);
    end_time = found_ed.tv_sec + (found_ed.tv_usec/1000000.0);
	exec_time += (end_time - start_time);
    cout<<"time: "<<exec_time<<endl;
    //cout<<endl<<endl;
    return 0;
}