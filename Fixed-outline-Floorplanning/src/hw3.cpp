/*
compile code:
	g++ -std=c++11 floorplan.cpp -o run

run code:
	./run -b ../testcase/n100.blocks -n ../testcase/n100.nets -p ../testcase/n100.pl -o n100.output -r 0.1 -s
	./run -b ../testcase/n300.blocks -n ../testcase/n300.nets -p ../testcase/n300.pl -o n3000.output -r 0.15 -d
    ./run ../testcase/n300.blocks ../testcase/n300.nets ../testcase/n300.pl n3000.output 0.15

Seed:
    *** first ***
    ** min area **
    first pack: 1542455358
    best first pack (50000 iter): 1542456275
    
    *** second ***
    ** min wire_len **
    wire_len   seed
    584235:    1542554275
    565460:    1542554888
    559648:    1542555342
    
    n100：  r 0.15
    
    exe time: 8.59484
    Best Wirelength: 219774
    seed: 1543121977
    seed1: 1543121977
    Final
    
    exe time: 9.15253
    Best Wirelength: 214282
    seed: 1543123020
    seed1: 1543123020
    Final
    
    n200：  r 0.15
    
    exe time: 61.5097
    Best Wirelength: 404662
    seed: 1543123935
    seed1: 1543123935
    Final
    
    exe time: 82.5239
    Best Wirelength: 393058
    seed: 1543119954
    seed1: 1543119954
    Final
    
    n300  r 0.15
    exe time: 128.656
    Best Wirelength: 556247
    seed: 1542456275
    seed1: 1543116405
    Final
    
    *********************************
    
    n100 r 0.1
    exe time: 8.26833
    Best Wirelength: 224143
    seed: 1543144685
    seed1: 1543144685
    Final
    
    n200 r 0.1
    exe time: 95.8795
    Best Wirelength: 405693
    seed: 1543141259
    seed1: 1543141259
    Final
    
    n300 r 0.1
    exe time: 237.657
    Best Wirelength: 579728
    seed: 1542456275
    seed1: 1543143309
    Final
    
*/
#include<sys/time.h>
#include<iostream>
#include<unistd.h>
#include<cstdlib>
#include<fstream>
#include<cstdio>
#include<string>
#include<vector>
#include<list>
#include<sstream>
#include<cmath>
#include<algorithm>
#include<ctime>
#include<cstdlib>
#include<cstring>

using namespace std;

class node{
public:
	int id;
	int area;
	bool rotate;
	int width, height, x_coordinate, y_coordinate;
	int parent;
	int lchild, rchild;
	// bool choose;
};

class net{
public:
	int degree;
	vector <int> block_list;
	vector <int> terminal_list;
};

class terminal{
public:
	int id;
	int x_coordinate, y_coordinate;
};

FILE *fh_in_block, *fh_in_net, *fh_in_terminal;
FILE *fh_out;
string output_file;
double ws_ratio=0.1;				// global white space ratio   defualt : 0.1
int seed = (unsigned)time(NULL);
int seeds = (unsigned)time(NULL);
int seeds1 = (unsigned)time(NULL);
bool debug_flag;
bool time_flag;
bool seed_flag;
int seed_num = 3;					// check which files : 100, 200, 300
int break_point;

int net_num, pin_num;				// total net number
int block_num;						// total block number
int terminal_num;					// total terminal number
int total_area;						// total area ( total area of the bounded box )

int max_border;						// max border to place the blocks
int* y_boundary;			        // Record current usefull bounder

int root=0;							// Default: root = 0
int local_root=0;
int best_root=0;



// vector <block> tree_net;

vector <node> btree;				// original btree, store every individual block area 
vector <node> local_btree;
vector <node> best_btree;

vector <terminal> terminal_array;

vector <net> net_array;


void parse_terminal(FILE* in){
	//terminal_array = new terminal[terminal_num]();
	for(int i=0; i< terminal_num ; i++){
		terminal tmp;
		terminal_array.push_back(tmp);
	}
	for(int i = 0; i < terminal_num; i++){
		fscanf(in, "p%d %d %d\n", &terminal_array[i].id, &terminal_array[i].x_coordinate, &terminal_array[i].y_coordinate);
	}
	/*if(debug_flag){
		cout<<"Terminal Num: "<< terminal_num << endl;
	}*/
	fclose(fh_in_terminal);
	return;
}

void parse_block(FILE* in){

	fscanf(fh_in_block, "NumHardRectilinearBlocks : %d\n", &block_num);
	fscanf(fh_in_block, "NumTerminals : %d\n\n", &terminal_num);
	//btree = new vector <node> [block_num]();
	//local_btree = new vector <node> [block_num]();
	//best_btree = new vector <node> [block_num]();
	for(int l=0 ; l<block_num ; l++){
		node tmp;
		btree.push_back(tmp);
		local_btree.push_back(tmp);
		best_btree.push_back(tmp);
	}
    /*for(int l=0 ; l<block_num+100 ; l++){
		block tmp;
		tree_net.push_back(tmp);
	}*/
	int count = block_num;
	int i = 0;
    int check_bloac_id=0;
	while(count--){
        
        int a, b, c, d, e, f;//ignore parameter
        fscanf(fh_in_block, "sb%d hardrectilinear 4 (%d, %d) (%d, %d) (%d, %d) (%d, %d)\n", &btree[i].id, &a, &b, &c, &d, &btree[i].width, &btree[i].height, &e, &f);
        int index_id=0, index_width=0, index_height=0;

        if(debug_flag){
            if( btree[i].id != check_bloac_id ){
                cout<<"block id error！！！！！！！！"<<endl;
            }
        }
        check_bloac_id++;
        // tree_net[ btree[i].id ].id = btree[i].id;
        // tree_net[ btree[i].id ].width = btree[i].width;
        // tree_net[ btree[i].id ].height = btree[i].height;
    
        /*
        if(tmp_block->width > tmp_block->height){
            tmp_block->rotate = true;
            int tmp_value = tmp_block->width;
            tmp_block->width = tmp_block->height;
            tmp_block->height = tmp_value;
        }
        */
        btree[i].area = (btree[i].width) * (btree[i].height);
        total_area += btree[i].area;
        i++;
	}
	if(debug_flag){
		
		cout<<"[FloorPlan Report]"<<endl;
		cout<<"Total Area: "<< total_area << endl;
		cout<<"Block Num: "<< block_num << endl;
	}
	//fclose(fh_in_block);
	return;
}

void parse_net(FILE* in){
	fscanf(in, "NumNets : %d\n", &net_num);
	fscanf(in, "NumPins : %d\n", &pin_num);
	
	int count = net_num;
	//net_array = new net[net_num]();
	for(int i=0;i<net_num;i++){
		net N;
		net_array.push_back(N);
	}
	
	for(int i = 0; i < net_num; ++i){
		
		int net_degree;
		fscanf(in, "NetDegree : %d", &net_degree);
		net_array[i].degree = net_degree;
		while(net_degree--){
			int tmp_id;
			char tmp_char[10];
			fscanf(in, "%s\n", &tmp_char);
			if(tmp_char[0] == 's'){
				string tmp_string(tmp_char);
				for(char &c : tmp_string) if(!isdigit(c)) c=' ';
				stringstream ss(tmp_string);
				ss >> tmp_id;
				net_array[i].block_list.push_back(tmp_id);
			}
			else{
				string tmp_string(tmp_char);
				for(char &c : tmp_string) if(!isdigit(c)) c=' ';
				stringstream ss(tmp_string);
				ss >> tmp_id;
				net_array[i].terminal_list.push_back(tmp_id);
			}
		}
		
	}

	if(debug_flag){
	
		cout<<"Net Num: "<< net_num <<endl;
	}
	fclose(fh_in_net);
	return;
}

void calculate_border(){
	max_border = (int)sqrt(total_area * (1 + ws_ratio)) ;
	if(debug_flag){
		cout<<"Max border : "<< max_border <<endl;
	}
	return;
}

void init_y_boundary(){
	// make the y boundary multi 5 times larger !!  Becareful don't out of range
	/*for(int i=0;i<max_border * 5;i++)
		y_boundary.push_back(0);*/
    y_boundary = new int[max_border * 5];
    memset(y_boundary,0,max_border * 5 *sizeof(int));
	return;
}

int count_y_coor(int curr_x, int width, int height){
	int max_height=0;
	int curr_bound = curr_x + width;
	//find max y_coordinate
	for(int i = curr_x; i < curr_bound; i++){
		if(y_boundary[i] > max_height) 
			max_height = y_boundary[i];
	}
	//update y_coordinate
	int update_h = max_height + height;
	for(int i = curr_x; i < curr_bound; i++){
		y_boundary[i] = update_h;
	}
	return max_height;
	
}

void init_floor_plan(){
	
	/* 
		initialize b* tree : parent, children, whether choose before
	   		parent : -1 (root)
	   		lchild or rchild : -1 (leaf)
	*/
	bool choose[block_num];
	for(int i=0;i<block_num;i++){
		choose[i]=false;
		btree[i].parent = -1;
		btree[i].lchild = -1;
		btree[i].rchild = -1;
	}
	/*
		Place x coordinate first:
		
		x - x - x - x - x 
		|
		x - x - x - x - x - x
		|
		x - x - x - x - x - x
	*/
	int curr_x=0;
	int last_node=0;
	int left_node=0;
	for(int curr_node=0 ; curr_node < block_num ; curr_node++){
		//cout<<"curr_node: "<<curr_node<<endl;
		// root
		
		if(curr_node==0){
			btree[curr_node].lchild = 1;
			btree[curr_node].x_coordinate=0;
			btree[curr_node].y_coordinate= count_y_coor( curr_x, btree[curr_node].width, btree[curr_node].height ) ;
			choose[curr_node]=true;
			
			curr_x = btree[curr_node].width;
			last_node=0;
			left_node=0;
		}
		if( choose[curr_node]== true)
			continue;
		else{
			// Put block horizontally  ->  ->  ->
			if(curr_x + btree[curr_node].width < max_border){
				btree[curr_node].parent = last_node;
				btree[last_node].lchild = curr_node;
				btree[curr_node].x_coordinate = curr_x;
				choose[curr_node]=true;
				btree[curr_node].y_coordinate = count_y_coor( curr_x, btree[curr_node].width, btree[curr_node].height );
					
				curr_x+=btree[curr_node].width;
				last_node = curr_node;
			}
			// swap up to the next floor of blocks
			else{
				btree[left_node].rchild = curr_node;
				btree[curr_node].parent = left_node;
				btree[curr_node].x_coordinate = 0;
				btree[curr_node].y_coordinate = count_y_coor( 0, btree[curr_node].width, btree[curr_node].height );
				choose[curr_node]=true;
				
				curr_x=btree[curr_node].width;
				left_node = curr_node;
				last_node = curr_node;
			}
		}
	}
	
	if(debug_flag){
		ofstream myfile;
		myfile.open("init.floorplan");
		myfile<<"Wirelength 75563\n";
		myfile<<"Blocks\n";
		for(int i=0;i<block_num;i++){
			myfile<<"sb"<<i<<" "<<btree[i].x_coordinate<<" "<<btree[i].y_coordinate<<" "<<"0\n";
		}
		myfile.close();
	}
	return;
}

void clear_y_boundary(){
	/*for(int i=0;i<max_border * 5;i++)
		y_boundary[i] = 0;*/
	memset(y_boundary,0,max_border * 5 *sizeof(int));
    return;
}

void Rotate(int cand1){
	node temp = btree[cand1];
	if(btree[cand1].rotate==false)
		btree[cand1].rotate = true;
	else
		btree[cand1].rotate = false;
	btree[cand1].width = temp.height;
	btree[cand1].height = temp.width;	
	return;
}

void Swap(int cand1, int cand2){
	
	if(cand1 == cand2)
		return;
	// Check root first. external change
	int cand1_p = btree[cand1].parent;
	int cand2_p = btree[cand2].parent;
	
	
	int cand1_r = btree[cand1].rchild;
	int cand1_l = btree[cand1].lchild;
	int cand2_r = btree[cand2].rchild;
	int cand2_l = btree[cand2].lchild;
	// cand1 is root
	if(cand1_p == -1){
		btree[cand2].parent = -1;
		btree[cand2].lchild = cand1_l;
		btree[cand2].rchild = cand1_r;
		
		btree[cand1].parent = cand2_p;
		btree[cand1].lchild = cand2_l;
		btree[cand1].rchild = cand2_r;
		
		if(cand1_l != -1)
			btree[ cand1_l ].parent = cand2;
		if(cand1_r != -1)
			btree[ cand1_r ].parent = cand2;
		if(cand2_l != -1)
			btree[ cand2_l ].parent = cand1;
		if(cand2_r != -1)
			btree[ cand2_r ].parent = cand1;
		
		if(cand2_p != -1){
			if(btree[cand2_p].lchild == cand2)
				btree[ cand2_p ].lchild = cand1;
			else
				btree[ cand2_p ].rchild = cand1;
		}
		root = cand2;
	}
	else if(cand2_p == -1){
		btree[cand1].parent = -1;
		btree[cand1].lchild = cand2_l;
		btree[cand1].rchild = cand2_r;
		
		btree[cand2].parent = cand1_p;
		btree[cand2].lchild = cand1_l;
		btree[cand2].rchild = cand1_r;
		
		if(cand2_l != -1)
			btree[ cand2_l ].parent = cand1;
		if(cand2_r != -1)
			btree[ cand2_r ].parent = cand1;
		
		if(cand1_l != -1)
			btree[ cand1_l ].parent = cand2;
		if(cand1_r != -1)
			btree[ cand1_r ].parent = cand2;
		if(cand1_p != -1){
			if( btree[ cand1_p ].lchild == cand1 )
				btree[ cand1_p ].lchild = cand2;
			else
				btree[ cand1_p ].rchild = cand2;
		}
		root = cand1;
		
	}
	// neither cand1 nor cand2 are root
	else{
		btree[cand1].parent = cand2_p;
		btree[cand1].lchild = cand2_l;
		btree[cand1].rchild = cand2_r;
		btree[cand2].parent = cand1_p;
		btree[cand2].lchild = cand1_l;
		btree[cand2].rchild = cand1_r;
		
		if(cand2_l != -1)
			btree[ cand2_l ].parent = cand1;
		if(cand2_r != -1)
			btree[ cand2_r ].parent = cand1;
		if(cand2_p != -1){
			if( btree[ cand2_p ].lchild == cand2 )
				btree[ cand2_p ].lchild = cand1;
			else
				btree[ cand2_p ].rchild = cand1;
		}
		if(cand1_l != -1)
			btree[ cand1_l ].parent = cand2;
		if(cand1_r != -1)
			btree[ cand1_r ].parent = cand2;
		if(cand1_p != -1){
			if( btree[ cand1_p ].lchild == cand1 )
				btree[ cand1_p ].lchild = cand2;
			else
				btree[ cand1_p ].rchild = cand2;
		}
	}
	
/*
	// Change root first
	if(cand1_p == -1){
		root = cand2;
	}

	else if(btree[cand1_p].lchild == cand1){
		btree[cand1_p].lchild = cand2;
	}
	else{
		btree[cand1_p].rchild = cand2;
	}
	
	// Change root
	if(cand2_p == -1){
		root = cand1;
	}
	else if(btree[cand2_p].lchild == cand2){
		btree[cand2_p].lchild = cand1;
	}
	else{
		btree[cand2_p].rchild = cand1;
	}
	
	//external change of left, right sub-group
	int cand1_l = btree[cand1].lchild;
	int cand2_l = btree[cand2].lchild;
	
	if(cand1_l != -1)
		btree[cand1_l].parent = cand2;
	if(cand2_l != -1)
		btree[cand2_l].parent = cand1;
	
	int cand1_r = btree[cand1].rchild;
	int cand2_r = btree[cand2].rchild;
	
	if(cand1_r != -1)
		btree[cand1_r].parent = cand2;
	if(cand2_r != -1)
		btree[cand2_r].parent = cand1;
	
	//internal change
	int tmp_parent = btree[cand1].parent;
	btree[cand1].parent = btree[cand2].parent;
	btree[cand2].parent = tmp_parent;
	
	int tmp_lchild = btree[cand1].lchild;
	btree[cand1].lchild = btree[cand2].lchild;
	btree[cand2].lchild = tmp_lchild;
	
	int tmp_rchild = btree[cand1].rchild;
	btree[cand1].rchild = btree[cand2].rchild;
	btree[cand2].rchild = tmp_rchild;
    */
	return;
}

void Move(int cand1, int cand2){
	if(cand1 == cand2) 
		return;
	if(btree[cand1].parent == cand2 || btree[cand2].parent == cand1) 
		return;
	
	Swap(cand1, cand2);
	
	int cand1_p = btree[cand1].parent;
	if(btree[cand1_p].lchild == cand1){
		btree[cand1_p].lchild = -1;
	}
	else{
		btree[cand1_p].rchild = -1;
	}
	
	int position = rand() % block_num;
	
	//find a position for insert
	while(position == cand1 || (btree[ position ].lchild != -1 && btree[ position ].rchild != -1) )
		position = rand() % block_num;
	
	btree[ cand1 ].parent = position;
	
	if(btree[ position ].lchild == -1)
		btree[ position ].lchild = cand1;
	else
		btree[ position ].rchild = cand1;
	
	return;
}

void perturbation(){
	int style = rand()%3;
	// Rotate
	if(style==0){
		int cand1 = rand() % block_num;
		Rotate(cand1);
	}
	// Swap
	else if(style==1){
		int cand1 = rand() % block_num;
		int cand2 = cand1;
		while( cand1 == cand2 || cand1 == btree[cand2].parent || cand1 == btree[cand2].lchild || cand1 == btree[cand2].rchild || cand2 == btree[cand1].parent || cand2 == btree[cand1].lchild || cand2 == btree[cand1].rchild )
			cand2 = rand() % block_num;
		Swap(cand1, cand2);
	}
    /*else{
        int cand1 = rand() % block_num;
		int cand2 = cand1;
		while( cand1 == cand2 || cand1 == btree[cand2].parent || cand1 == btree[cand2].lchild || cand1 == btree[cand2].rchild || cand2 == btree[cand1].parent || cand2 == btree[cand1].lchild || cand2 == btree[cand1].rchild )
			cand2 = rand() % block_num;
		Swap(cand1, cand2);
    }*/
	// flip
	else{
		int cand1 = rand() % block_num;
		while( btree[ cand1 ].lchild == -1 && btree[ cand1 ].rchild == -1 ){
			cand1 = rand() % block_num;
		}
		// find cand1's leaf child
		int cand2 = cand1;
		

        
		while( btree[cand2].lchild != -1 || btree[cand2].rchild != -1 ){
			int RorL = rand()%2;
			if( RorL==0 ){
				if(btree[cand2].lchild != -1)
					cand2 = btree[cand2].lchild;
				else
					cand2 = btree[cand2].rchild;
			}
			else{
				if(btree[cand2].rchild != -1)
					cand2 = btree[cand2].rchild;
				else
					cand2 = btree[cand2].lchild;
			}
			//if( btree[cand2].lchild == -1 && btree[cand2].rchild == -1 )
			//	break;
		}
		
		
		// Debug~
		if( cand2 == -1 || ( btree[cand2].lchild!=-1 || btree[cand2].rchild!=-1 ) ){
			cout<<"Error accur in pertubation stage 3~  "<<endl;
		}
		Move(cand1,cand2);
	}
	return;
}

void perturbation2(){
	int style = rand()%3;
	// Rotate
	if(style==0){
		int cand1 = rand() % block_num;
		Rotate(cand1);
	}
	// Swap
	else if(style==1){
		int cand1 = rand() % block_num;
		int cand2 = cand1;
		while( cand1 == cand2 || cand1 == btree[cand2].parent || cand1 == btree[cand2].lchild || cand1 == btree[cand2].rchild || cand2 == btree[cand1].parent || cand2 == btree[cand1].lchild || cand2 == btree[cand1].rchild )
			cand2 = rand() % block_num;
		Swap(cand1, cand2);
	}
    else{
        int cand1 = rand() % block_num;
		int cand2 = cand1;
		while( cand1 == cand2 || cand1 == btree[cand2].parent || cand1 == btree[cand2].lchild || cand1 == btree[cand2].rchild || cand2 == btree[cand1].parent || cand2 == btree[cand1].lchild || cand2 == btree[cand1].rchild )
			cand2 = rand() % block_num;
		Swap(cand1, cand2);
    }
	// flip
	/*else{
		int cand1 = rand() % block_num;
		while( btree[ cand1 ].lchild == -1 && btree[ cand1 ].rchild == -1 ){
			cand1 = rand() % block_num;
		}
		// find cand1's leaf child
		int cand2 = cand1;
		

        
		while( btree[cand2].lchild != -1 || btree[cand2].rchild != -1 ){
			int RorL = rand()%2;
			if( RorL==0 ){
				if(btree[cand2].lchild != -1)
					cand2 = btree[cand2].lchild;
				else
					cand2 = btree[cand2].rchild;
			}
			else{
				if(btree[cand2].rchild != -1)
					cand2 = btree[cand2].rchild;
				else
					cand2 = btree[cand2].lchild;
			}
			//if( btree[cand2].lchild == -1 && btree[cand2].rchild == -1 )
			//	break;
		}
		
		
		// Debug~
		if( cand2 == -1 || ( btree[cand2].lchild!=-1 || btree[cand2].rchild!=-1 ) ){
			cout<<"Error accur in pertubation stage 3~  "<<endl;
		}
		Move(cand1,cand2);
	}*/
	return;
}

// Recursive~   LR=0 : left     LR=1 : right
void Packing(int curr, int LR){
	// first search 
	if(curr == root){
		btree[curr].x_coordinate = 0;
		btree[curr].y_coordinate = count_y_coor(0, btree[curr].width, btree[curr].height);
	}
	// search left
	else if(LR == 0){
		int parent = btree[curr].parent;
		btree[curr].x_coordinate = btree[parent].x_coordinate + btree[parent].width;
		btree[curr].y_coordinate = count_y_coor(btree[curr].x_coordinate, btree[curr].width, btree[curr].height);

	}
	// search right
	else{
		int parent = btree[curr].parent;
		btree[curr].x_coordinate = btree[parent].x_coordinate;
		btree[curr].y_coordinate = count_y_coor(btree[curr].x_coordinate, btree[curr].width, btree[curr].height);
	}

	if(btree[curr].lchild != -1){
		Packing(btree[curr].lchild, 0);
	}
	if(btree[curr].rchild != -1){
		Packing(btree[curr].rchild, 1);
	}
	return;
}

double wire_len = 0;
double local_wire_len = 0;
double best_wire_len = 0;

void Debug_floorplan(string file_name){
	if(debug_flag){
		ofstream myfile;
		myfile.open(file_name);
		myfile<<"Wirelength "<<best_wire_len<<"\n";
		myfile<<"Blocks\n";
		for(int i=0;i<block_num;i++){
			if( best_btree[i].rotate==false )
				myfile<<"sb"<<i<<" "<<best_btree[i].x_coordinate<<" "<<best_btree[i].y_coordinate<<" "<<"0\n";
			else
				myfile<<"sb"<<i<<" "<<best_btree[i].x_coordinate<<" "<<best_btree[i].y_coordinate<<" "<<"1\n";
		}
		myfile.close();
	}
	return;
}

void write_floorplan(string output_file){
    ofstream myfile;
	myfile.open(output_file);
	myfile<<"Wirelength "<<best_wire_len<<"\n";
	myfile<<"Blocks\n";
	for(int i=0;i<block_num;i++){
		if( best_btree[i].rotate==false )
			myfile<<"sb"<<i<<" "<<best_btree[i].x_coordinate<<" "<<best_btree[i].y_coordinate<<" "<<best_btree[i].width<<" "<<best_btree[i].height<<" 0\n";
		else
			myfile<<"sb"<<i<<" "<<best_btree[i].x_coordinate<<" "<<best_btree[i].y_coordinate<<" "<<best_btree[i].width<<" "<<best_btree[i].height<<" 1\n";
	}
	myfile.close();
    return;
}

int check_btree=0;
void check_btree_thread(int curr){
	if(curr==root){
		check_btree=1;
	}
	if( btree[curr].lchild != -1 ){
		check_btree++;
		if( btree[ btree[curr].lchild ].parent != curr )
			cout<<"Error accur in parent thread~ "<<endl;
		check_btree_thread( btree[curr].lchild );
	}
	if( btree[curr].rchild != -1 ){
		check_btree++;
		if( btree[ btree[curr].rchild ].parent != curr )
			cout<<"Error accur in parent thread~ "<<endl;
		check_btree_thread( btree[curr].rchild );
	}
	if(curr==root && check_btree != block_num){
		cout<<"Thread break~ "<<check_btree<<endl;
	}
	return;
}



double count_wire_len(){
    double tmp_wire=0;
    for(int i=0;i<net_array.size();i++){
        int maxx=0;
        int maxy=0;
        int minx=5000000;
        int miny=5000000;
        for(const auto j:net_array[i].terminal_list){
            int tmp_x = terminal_array[j-1].x_coordinate;
			int tmp_y = terminal_array[j-1].y_coordinate;
			if(tmp_x > maxx)
                maxx = tmp_x;
			if(tmp_x < minx)
                minx = tmp_x;
			if(tmp_y > maxy)
                maxy = tmp_y;
			if(tmp_y < miny)
                miny = tmp_y;
        }
        for(const auto j:net_array[i].block_list){
            int tmp_x = ceil( btree[j].x_coordinate + (btree[j].width) / 2 );
			int tmp_y = ceil( btree[j].y_coordinate + (btree[j].height) / 2 );
			if(tmp_x > maxx)
                maxx = tmp_x;
			if(tmp_x < minx)
                minx = tmp_x;
			if(tmp_y > maxy)
                maxy = tmp_y;
			if(tmp_y < miny)
                miny = tmp_y;
        }
        tmp_wire+=((maxy - miny)+(maxx - minx));
    }
    return tmp_wire;
}

double cost=0;
double local_cost=0;
double best_cost=0;

void simulate_anealing(){
	
	// store local tree first
	local_btree = btree;
	local_root = root;
	best_btree = btree;
	best_root = root;
	
	
	clear_y_boundary();
	Packing( root, 0 );
	
	root=0;
	check_btree_thread(root);
	
	int count=0;
	
	int min_area=100000000;
	cost = min_area*10;
    local_cost = cost;
    int get_worst=0;
    
    // make it into block
    /*
        Go~~~~~~~~~~~~~~~~~
    */
    best_cost = 50000000;
    int jump_local=0;
    while(count<50000){
		count++;
		// Watch out! every time need to clear y boundary first
		clear_y_boundary();
		perturbation2();
		Packing( root, 0);
		
        //wire_len = count_wire_len();
		
		// Make sure the tree don't get out of bound
		int x1=0, y1=0;
		for(int i=0 ; i<block_num ; i++){
			if(btree[i].x_coordinate>x1)
				x1=btree[i].x_coordinate + btree[i].width;
			if(btree[i].y_coordinate>y1)
				y1=btree[i].y_coordinate + btree[i].height;
		}

        if(count<35000){
            if( x1*y1 > min_area || x1 > max_border*1.15 || y1 > max_border*1.15){
                //jump_local++;
                btree = local_btree;
                root = local_root;
            }
            else{
                //jump_local=0;
                best_btree = btree;
                best_root = root;
                local_btree = btree;
                local_root = root;
                min_area = x1*y1;
            }
        }
        else{
            if( x1*y1 > min_area || x1 > max_border*1.05 || y1 > max_border*1.05){
                //jump_local++;
                btree = local_btree;
                root = local_root;
            }
            else{
                //jump_local=0;
                //wire_len = count_wire_len();
                //cost = wire_len * 0.001;
                //if(cost<best_cost){
                    best_btree = btree;
                    best_root = root;
                    //best_cost = cost;
                    //best_wire_len = wire_len;
                //}
                local_btree = btree;
                local_root = root;
                min_area = x1*y1;
            }
        }
        
        
        
		// Make sure the btree is robust~~	
		//check_btree_thread(root);
	}
    
    //cout<<"best wire: "<<best_wire_len<<endl;
    wire_len = count_wire_len();
    local_wire_len = wire_len;
    best_wire_len = wire_len;
    
    srand(seeds1);
    
    
    
	while(count<1000000){
        
        
        local_btree = btree;
        local_root = root;
        local_wire_len = wire_len;
        local_cost = cost;
        
        
		count++;
		// Watch out! every time need to clear y boundary first
		clear_y_boundary();
		perturbation2();
		Packing( root, 0);
		
        wire_len = count_wire_len();
		//if(count%50000==0)
            //cout<<"iter: "<<count<<" wire_len: "<<wire_len<<endl;

		// Make sure the tree don't get out of bound
		int x1=0, y1=0;
		for(int i=0 ; i<block_num ; i++){
			if(btree[i].x_coordinate + btree[i].width>x1)
				x1=btree[i].x_coordinate + btree[i].width;
			if(btree[i].y_coordinate + btree[i].height>y1)
				y1=btree[i].y_coordinate + btree[i].height;
		}
        cost = 0;
        cost = wire_len * 0.001;
        int yy=0;
        if(x1 > max_border*1){
            //for(int k =0;k<)
            cost+=50000*(x1 - max_border);
        }
        if(y1 > max_border*1){
            cost+=50000*(y1 - max_border);
        }
        if(cost < best_cost){
            //cout<<"iter: "<<count<<" wire_len: "<<wire_len<<endl;
            
            best_btree = btree;
            best_root = root;
            best_wire_len = wire_len;
            best_cost = cost;
            
            local_btree = btree;
            local_root = root;
            local_wire_len = wire_len;
            local_cost = cost;
            
            get_worst=0;
            
            
        }
        else{
            btree=local_btree;
            root=local_root;
            wire_len=local_wire_len;
            cost=local_cost;
            get_worst++;
        }
        
        if(get_worst>=break_point){
            //for(int k=0;k<5;k++)
                //perturbation2();
            
            //clear_y_boundary();
            //Packing( root, 0);
            //get_worst=0;
            break;
        }
        
        //if(count<400000){
            //if(get_worst>=10){
                //btree = local_btree;
                //root = local_root;
                //get_worst=0;
            //}
        //}
        /*else{
            if(get_worst>=5){
                btree = local_btree;
                root = local_root;
                get_worst=0;
            }
        }*/
		// Make sure the btree is robust~~	
		//check_btree_thread(root);
	}
	
    
    
	// string file_name = "pack.floorplan";
	//Debug_floorplan(output_file);
	write_floorplan(output_file);
	/*int xx=0, yy=0;
	for(int i=0 ; i<block_num ; i++){
		if(btree[i].x_coordinate + btree[i].width >xx){
			xx = btree[i].x_coordinate + btree[i].width;
		}
		if(btree[i].y_coordinate + btree[i].height >yy){
			yy=btree[i].y_coordinate + btree[i].height;
		}
	}*/
	//cout<<"area ( Not too precision ) : "<<xx*yy<<endl;
	return;
}

void check_best_out(){
    int x1 = 0, y1 = 0;
    for(int i=0 ; i<block_num ; i++){
		if(best_btree[i].x_coordinate + best_btree[i].width>x1)
			x1=best_btree[i].x_coordinate + best_btree[i].width;
		if(best_btree[i].y_coordinate + best_btree[i].height>y1)
			y1=best_btree[i].y_coordinate + best_btree[i].height;
	}
    if(x1>max_border || y1>max_border)
        cout<<endl<<"Sorry~ out bourder~"<<endl<<endl;
    return;
}

int main(int argc, char** argv){
	
	struct timeval st, ed;
    double start, end;
    
    gettimeofday(&st,NULL);
	start = st.tv_sec + (st.tv_usec/1000000.0);
	// Load data
	//parse_parameter(argc, argv);
    
    fh_in_block = fopen( argv[1] , "r");
    if( !strcmp(argv[1],"../testcase/n100.hardblocks") ){
        break_point = 5000;
        if( atof(argv[5]) == 0.15 ){
            //seeds = 1543121977;
            //seeds1 = 1543121977;
            seeds = 1543123020;
            seeds1 = 1543123020;
        }
        else if( atof(argv[5]) == 0.1 ){
            seeds = 1543144685;
            seeds1 = 1543144685;
        }
    }
    else if( !strcmp(argv[1],"../testcase/n200.hardblocks") ){
        break_point = 10000;
        if( atof(argv[5]) == 0.15 ){
            seeds = 1543119954;
            seeds1 = 1543119954;
        }
        else if( atof(argv[5]) == 0.1 ){
            seeds = 1543141259;
            seeds1 = 1543141259;
        }
    }
    else if( !strcmp(argv[1],"../testcase/n300.hardblocks") ){
        break_point = 20000;
        if( atof(argv[5]) == 0.15 ){
            seeds = 1542456275;
            seeds1 = 1543116405;
            //1543116405
            
        }
        else if( atof(argv[5]) == 0.1 ){
            seeds = 1542456275;
            seeds1 = 1543143309;
        }
    }
    //cout<<argv[2]<<endl;
    fh_in_net = fopen(argv[2], "r");
    fh_in_terminal = fopen(argv[3], "r");
    fh_out = fopen(argv[4], "w");
    output_file = argv[4];
    ws_ratio = atof(argv[5]);
    debug_flag=false;
    //if(!strcmp(argv[6],"debug"))
    //    debug_flag = true;
    
    
	parse_block(fh_in_block);
	parse_terminal(fh_in_terminal);
	parse_net(fh_in_net);
    
	srand(seeds);
    
	// initialization
	calculate_border();
	init_y_boundary();
	init_floor_plan();
	
	simulate_anealing();
    
    gettimeofday(&ed,NULL);
	end = ed.tv_sec + (ed.tv_usec/1000000.0);
    
    //check_best_out();
    cout<<"exe time: "<<end-start<<endl;
    cout<<"Best Wirelength: "<<best_wire_len<<endl;
	cout<<"seed: "<<seeds<<endl;
    cout<<"seed1: "<<seeds1<<endl;
	cout<<"Final"<<endl;
	return 0;
}