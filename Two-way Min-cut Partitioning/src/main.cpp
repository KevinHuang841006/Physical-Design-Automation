/*
	./main cell.cells net.nets output
*/
#include<iostream>
#include<unistd.h>
#include<cstdlib>
#include<fstream>
#include<list>
#include<vector>
#include<string>
#include<map>
#include<algorithm>
#include<stack>
#include<cmath>
#include<cstring>
#include<sys/time.h>

using namespace std;

struct cell{
	string name;//cell name
	int size;//cell size
	int gain;//cell gain
	int pin;//cell pin
	bool state;//0 for free; 1 for lock;
	bool partition;//0 for partition A; 1 for partition B
    vector<int> net_list;
    cell* pre;
    cell* next;
	list<cell*>::iterator ptr;//point to the position in bucket list
};

struct net{
	string name;//net name
	int A, B;//cell num in A and B respectively
	vector<int> cell_list;
	
};
int special_flag = 10;
ifstream fh_in_cell, fh_in_net;
ofstream fh_out;
bool debug_flag, time_flag, sort_flag;
//get opt via execution

map<string, int> cell_dictionary, net_dictionary;
int cell_num, net_num;
vector<cell*> cell_vector;
vector<net*> net_vector;

vector <bool> best_group(500000,false);
vector <int> best_netA(500000,0);
vector <int> best_netB(500000,0);

//parse cell data
void parse_cell(istream &in){
	string tmp_name;
	int tmp_size;
	while(in >> tmp_name >> tmp_size){
		cell_dictionary[tmp_name] = cell_num;
		cell* tmp_cell = new cell();
		tmp_cell->name = tmp_name;
		tmp_cell->size = tmp_size;
        tmp_cell->pre = NULL;
        tmp_cell->next = NULL;
		cell_vector.push_back(tmp_cell);
		++cell_num;
	}
	if(debug_flag == true){
		cout << "[Initial cell]"<<endl;
		cout << "total cell num: " << cell_num <<endl;
	}
	return;
}

//parse net data
void parse_net(istream &in){
	string title;
	while(in >> title){
		string tmp_name;
		in >> tmp_name;
		net_dictionary[tmp_name] = net_num;
		net* tmp_net = new net();
		tmp_net->name = tmp_name;
		net_vector.push_back(tmp_net);
		string cell_name;
		while(in >> cell_name){
			if(cell_name[0] == '{'){
				continue;
			}
			if(cell_name[0] == '}'){
				break;
			}
			cell* &tmp_cell = cell_vector[cell_dictionary[cell_name]];
			tmp_cell->net_list.push_back(net_num);
			++(tmp_cell->pin);
			net_vector[net_num]->cell_list.push_back(cell_dictionary[cell_name]);
		}
		++net_num;
	}
	if(debug_flag == true){

		cout << "[Initial Net]"<<endl;
		cout << "total net num: " << net_num<<endl;
		
	}
	return;
}

//compare pin num between two cells
bool cmppin(cell* &x, cell* &y){
	return x->pin < y->pin;
}

int partA_num, partB_num;
int partA_size, partB_size;
//get initial_gain
void initial_partition(){
	int n = cell_num;
	list<cell*> heap;
	for(int i = 0; i < n; ++i){
		heap.push_back(cell_vector[i]);
	}
	//use customize sort in list
	if(sort_flag == true) heap.sort(cmppin);
	
	while(!heap.empty()){
		if(partA_size <= partB_size){
			cell* &tmp = heap.back();
			tmp -> partition = 0;
			partA_size += tmp->size;
			++partA_num;
			heap.pop_back();
		}
		else{
			cell* &tmp = heap.front();
			tmp -> partition = 1;
			partB_size += tmp->size;
			++partB_num;
			heap.pop_front();
		}
	}
	if(debug_flag == true){
		cout << "[Initial Partition]"<<endl;
		cout << "Partition A num: " << partA_num << "; size: "<< partA_size <<endl;
		cout << "Partition B num: " << partB_num << "; size: "<< partB_size <<endl;
	}
	return;
}

//get cell num in partition A&B for each net
void get_AB(){
	//cout<<"stuck getAB"<<endl;
    for (int i = 0; i < net_num; ++i){
        vector<int> &tmp_vector = net_vector[i]->cell_list;
        net_vector[i]->A = 0;
        net_vector[i]->B = 0;
        for (unsigned int j = 0; j < tmp_vector.size(); ++j){
            cell* tmp_cell = cell_vector[ tmp_vector[j] ];
            if(!(tmp_cell->partition))
				net_vector[i]->A++;
            else
				net_vector[i]->B++;
        }
    }
	return;
}

int cutsize;
//calculate cut size
void get_cutsize(){
	cutsize = 0;
	for(int i = 0; i < net_num; i++){
		if(net_vector[i]->A && net_vector[i]->B) cutsize++;
	}
	if(debug_flag == true){
		cout << "[Cutsize]"<<endl;
		cout << "cut size: " << cutsize<<endl;
	}
	return;
}

int constraint;
//get the upper/lower bound diff
void get_constraint(){
	constraint = 0;
	constraint = cell_num / 10;
	if(debug_flag == true){
		cout << "[Constraint]"<<endl;
		cout << "constraint: " << constraint<<endl;
	}
	return;
}

int maxpin;
//get the max pin
void get_pmax(){
	maxpin = 0;
	for(int i = 0; i < cell_num; ++i){
		int pin_num = cell_vector[i]->pin;
		if(maxpin < pin_num) maxpin = pin_num;
	}
	if(debug_flag == true){
		cout << "[MaxP]\n";
		cout << "Max pin num: " << maxpin <<endl;
	}
	return;
}

//initialize the gain in cell vector
void initial_gain(){
	for(int i = 0; i < cell_num; ++i){
		cell_vector[i]->gain = 0;
		cell_vector[i]->state = false;
	}
	for(int i = 0; i < cell_num; ++i){
		for(unsigned int j = 0; j < cell_vector[i]->net_list.size(); ++j){
			int id = cell_vector[i]->net_list[j];
			if(cell_vector[i]->partition == 0){
				if(net_vector[id]->A == 1){
					cell_vector[i]->gain++;
				}
				if(net_vector[id]->B == 0){
					cell_vector[i]->gain--;
				}
			}
			else{
				if(net_vector[id]->B == 1){
					cell_vector[i]->gain++;
				}
				if(net_vector[id]->A == 0){
					cell_vector[i]->gain--;
				}
			}
				
		}
	}
	return;
}

map<int, list<cell*> > bucketA, bucketB;
vector <cell *> bucket_list;

void print_bucket(){
    int count_p=0;
	//cout<<"maxpin "<<maxpin<<endl;
	for(int i = maxpin*2; i >=0 ; --i){
        int cc=0;
		cell* K = bucket_list[i];
		//cout<<"g: "<<i-maxpin<<" ";
		while(K!=NULL){
			//cout<<K->name<<" ";
			K = K->next;
            count_p++;
            cc++;
		}
		cout<<"g "<<i - maxpin<<" : "<<cc<<endl;
	}
    cout<<"size in bucket:  "<<count_p<<endl;
	return;
}


//construct the bucket list
void get_bucket_list(){
	//bucketA.clear();
	//bucketB.clear();
    bucket_list.clear();
    bucket_list.resize(2*maxpin + 1);
    bucket_list.shrink_to_fit();
    
	for(int i = 0; i <cell_num ; ++i){
		cell_vector[i]->pre = NULL;
		cell_vector[i]->next = NULL;
	}
	// Insert bucket_list
	for(int i = 0; i <cell_num ; ++i){
		int g = cell_vector[i]->gain + maxpin;
		if(g<0)
			cout<<"big error~~~"<<endl;
		//bool s = cell_vector[i]->partition;
		// Insert bucket_list
		if(bucket_list[g]==NULL){
			bucket_list[g] = cell_vector[i];
            cell_vector[i]->pre = NULL;
		}
		// start from head
		else{
			bucket_list[g]->pre = cell_vector[i];
			cell_vector[i]->next = bucket_list[g];
			bucket_list[g] = cell_vector[i];
            cell_vector[i]->pre = NULL;
		}
		
	}
	//print_bucket();
	return;
}

//get the max gain cell name in partition
string get_maxgain_cellname(bool partition){
	string cell_name("");
	//cout<<"stuck get_maxgain"<<endl;
	bool breakk_flag=false;
    //int max_cell_deep=0;
	
	for(int i = 2*maxpin ;i >= 0 ; i--){
		if(bucket_list[i]==NULL)
			continue;
		cell* j = bucket_list[i];
		while(j!=NULL){
			cell_name = j->name;
			if( cell_vector[cell_dictionary[cell_name]]->partition==false && abs(partA_size - partB_size - 2 * ( cell_vector[cell_dictionary[cell_name]]->size )) >= constraint  ){
				j=j->next;
                //max_cell_deep++;
				continue;
			}
            else if( cell_vector[cell_dictionary[cell_name]]->partition==true && abs(partA_size - partB_size + 2 * ( cell_vector[cell_dictionary[cell_name]]->size )) >= constraint ){
                j=j->next;
                //max_cell_deep++;
				continue;
            }
			breakk_flag=true;
			break;
		}
		if(breakk_flag==true)
			break;
	}
	if(cell_name == "")
		cout<<"error no max number"<<endl;
	/*if(max_cell_deep!=0)
        cout<<"cell: "<<cell_name<<" deep: "<<max_cell_deep<<endl;*/
    return cell_name;
    
}


//get other max gain cell name in partition


//remove cell in partition
int remove_size = 0;
void remove_cell(string tmp){
	int true_index = cell_dictionary[tmp];
    cell_vector[true_index]->state = true;
    
    if(cell_vector[true_index]->next != NULL) 
        cell_vector[true_index]->next->pre = cell_vector[true_index]->pre;
    if(cell_vector[true_index]->pre != NULL) 
        cell_vector[true_index]->pre->next = cell_vector[true_index]->next;
    if(bucket_list[ cell_vector[true_index]->gain + maxpin ] == cell_vector[true_index]) 
        bucket_list[ cell_vector[true_index]->gain + maxpin ] = cell_vector[true_index]->next;
    cell_vector[true_index]->pre = NULL;
    cell_vector[true_index]->next = NULL;
	
	remove_size++;
	return;
}

void new_remove_cell(string tmp, int g){
	int true_index = cell_dictionary[tmp];
	// one data
    g = g + maxpin;
    //cell_vector[true_index]->state = true;
    if(cell_vector[true_index]->next != NULL) 
        cell_vector[true_index]->next->pre = cell_vector[true_index]->pre;
    if(cell_vector[true_index]->pre != NULL) 
        cell_vector[true_index]->pre->next = cell_vector[true_index]->next;
    if(bucket_list[ g ] == cell_vector[true_index]) 
        bucket_list[ g ] = cell_vector[true_index]->next;
    cell_vector[true_index]->pre = NULL;
    cell_vector[true_index]->next = NULL;
	
	return;
}

//update gain from move a cell to another partition, also adjust the position of relative cell in bucket list
void update_gain(string tmp){
	//cout<<"stuck updata gain"<<endl;
	cell* &tmp_cell = cell_vector[cell_dictionary[tmp]];
	tmp_cell->state = true;
	if(!(tmp_cell->partition)){//from A to B
		// swap cell to group B
		tmp_cell->partition = true;
		int n = tmp_cell->net_list.size();
		for(int i = n-1; i >=0; --i){
			
			net* &tmp_net = net_vector[tmp_cell->net_list[i]];
			int m = tmp_net->cell_list.size();
			int origin_gain[m];//store the key for erase in bucket list 
			
			//record initial gain
			for(int j = 0; j != m; ++j){
				origin_gain[j] = cell_vector[tmp_net->cell_list[j]]->gain;
			}

			//before the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->B == 0){
					if(cell_for_update->state == false)	cell_for_update->gain++;
				}
				else if(tmp_net->B == 1){
					if((cell_for_update->partition == 1) && (cell_for_update->state == false)) cell_for_update->gain--;
				}
			}
			
			//F(n) = F(n) - 1; T(n) = T(n) + 1;
			tmp_net->A--;
			tmp_net->B++;

			//after the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->A == 0){
					if(cell_for_update->state == false)	cell_for_update->gain--;
				}
				else if(tmp_net->A == 1){
					if((cell_for_update->partition == 0) && (cell_for_update->state == false)) cell_for_update->gain++;
				}
			}
			
			
			for(int j = 0; j != m; ++j){
				if(cell_vector[tmp_net->cell_list[j]]->state == true){ continue; }
				//adjust position in bucketlist A
				cell* change_cell = cell_vector[ tmp_net->cell_list[j] ];
				new_remove_cell(change_cell->name, origin_gain[j]);
                change_cell->pre = NULL;
                change_cell->next = NULL;
				// insert cell to new place
                //cout<<"current gain: "<<change_cell->gain<<endl;
				if(bucket_list[ change_cell->gain + maxpin ]==NULL){
                    //change_cell->pre = NULL;
					bucket_list[ change_cell->gain + maxpin ] = change_cell;
                    change_cell->pre = NULL;
                    //cell_vector[tmp_net->cell_list[j]]->pre = NULL;
                    //cell_vector[tmp_net->cell_list[j]]->next = NULL;
				}
				// start from head
				else{
                    
                    if(bucket_list[ change_cell->gain + maxpin ]!=NULL && bucket_list[ change_cell->gain + maxpin ]->pre != NULL){
                        
                        cout<<"need to be NULL"<<endl;
                    }
					bucket_list[ change_cell->gain + maxpin ]->pre = change_cell;
					change_cell->next = bucket_list[ change_cell->gain + maxpin ];
					bucket_list[ change_cell->gain + maxpin ] = change_cell;
                    change_cell->pre  = NULL;
				}
				
				
			}
			
			
		}
	}
	else{//from B to A
		tmp_cell->partition = false;
		int n = tmp_cell->net_list.size();

		for(int i = 0; i <n; i++){
			net* &tmp_net = net_vector[tmp_cell->net_list[i]];
			int m = tmp_net->cell_list.size();
			int origin_gain[m];

			for(int j = 0; j != m; ++j){
				origin_gain[j] = cell_vector[tmp_net->cell_list[j]]->gain;

			}
			
			//before the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->A == 0){
					if(cell_for_update->state == false)	cell_for_update->gain++;
				}
				else if(tmp_net->A == 1){
					if((cell_for_update->partition == 0) && (cell_for_update->state == false)) cell_for_update->gain--;
				}
			}
			
			//F(n) = F(n) - 1; T(n) = T(n) + 1;
			tmp_net->B--;
			tmp_net->A++;
			
			//after the move
			for(int j = 0; j != m; ++j){
				cell* &cell_for_update = cell_vector[tmp_net->cell_list[j]];
				if(tmp_net->B == 0){
					if(cell_for_update->state == false)	cell_for_update->gain--;
				}
				else if(tmp_net->B == 1){
					if((cell_for_update->partition == 1) && (cell_for_update->state == false)) cell_for_update->gain++;
				}
			}
			

			for(int j = 0; j != m; ++j){

				if(cell_vector[tmp_net->cell_list[j]]->state == true){ continue; }
				//adjust position in bucketlist A
				cell* change_cell = cell_vector[ tmp_net->cell_list[j] ];
				new_remove_cell(change_cell->name, origin_gain[j]);
				// insert cell to new place
				if(bucket_list[ change_cell->gain + maxpin ]==NULL){
					bucket_list[ change_cell->gain + maxpin ] = change_cell;
                    change_cell->pre = NULL;
                    //cell_vector[tmp_net->cell_list[j]]->next = NULL;
				}
				// start from head
				else{
					bucket_list[ change_cell->gain + maxpin ]->pre = change_cell;
					change_cell->next = bucket_list[ change_cell->gain + maxpin ];
					bucket_list[ change_cell->gain + maxpin ] = change_cell;
                    change_cell->pre = NULL;
				}
				
			}
			
		}
	}

	//remove_cell(tmp);
	return;
}

//calculate the partA_num & partB_num
void get_partitionAB_num(){
	//cout<<"stuck get parttionAB"<<endl;
	int tmp_partA_num = 0;
	int tmp_partB_num = 0;
	int tmp_partA_size = 0;
	int tmp_partB_size = 0;
	for(int i = 0; i < cell_num; ++i){
		if(cell_vector[i]->partition == 0){
			tmp_partA_num++;
			tmp_partA_size+=cell_vector[i]->size;
		}
		else{
			tmp_partB_num++;
			tmp_partB_size+=cell_vector[i]->size;
		}
	}
	partA_num = tmp_partA_num;
	partB_num = tmp_partB_num;
	partA_size = tmp_partA_size;
	partB_size = tmp_partB_size;
	return;
}

void check_cell_null(){
    int not_clean = 0;
    for(int i=0;i<cell_num;i++){
        if(cell_vector[i]->pre!=NULL || cell_vector[i]->next!=NULL)
            not_clean++;
    }
    cout<<"not clean: "<<not_clean<<endl;
}

int lps_partA_num;
int lps_partB_num;
int break_flag = 0;
//FM partition body for one iteration
int FM_partition(){
	//vector <int> lock(10000000,0);
	
	bool flag = false;
	initial_gain();
	get_bucket_list();
	int count = cell_num;
	int free_A = partA_num;
	int free_B = partB_num;
	stack<int> record_cell;
	int sum = 0;
	int largest_partial_sum = 0;
	int iteration = 0;
	int lps_iteration = 0;
	
    
    for(int i = maxpin*2 ; i>=0 ; i--){
        if(bucket_list[ i ]!=NULL && bucket_list[ i ]->pre != NULL){            
			cout<<"out space need to be NULL"<<endl;
        }
    }
    
    int wrong_array[1000000]={0};
    
	remove_size = 0;
    bool swapAB=false;
	while(count--){
		if(flag == true){
			break;
		}
        string max_cell;
		max_cell = get_maxgain_cellname(swapAB);
        
        if(cell_vector[ cell_dictionary[max_cell] ]->gain<-2 && cell_num < 150000){
			if(debug_flag == true)
				cout<<" 1/2 gain break"<<endl;
            break;
        }
        if(cell_vector[ cell_dictionary[max_cell] ]->gain<-1&& cell_num >=150000){
            if(debug_flag == true)
				cout<<" 1/2 gain break"<<endl;
            break;
        }
		
        if( iteration> cell_num / 5 && cell_num > 100000){
            if(debug_flag == true)
				cout<<" bigger than half iteration break"<<endl;
            break;
        }
		
        if(max_cell=="")
            break;
        remove_cell(max_cell);
        wrong_array[cell_dictionary[max_cell]]++;
        if(wrong_array[cell_dictionary[max_cell]]>1){
			cout<<"Big Wrong~~~~~~~~"<<endl;
            break;
        }
		/*if( cell_vector[cell_dictionary[max_cell]]->gain < -2 )
            break;*/
		//cout<<"max cell id: "<<max_cell<<endl;
        
        
		if(abs(partA_size - partB_size  < constraint)){
			record_cell.push(cell_dictionary[max_cell]);
			if((cell_vector[cell_dictionary[max_cell]])->partition == false ){
				partA_size -= cell_vector[cell_dictionary[max_cell]]->size;
				partB_size += cell_vector[cell_dictionary[max_cell]]->size;
				free_A--;
			}
			else{
				partA_size += cell_vector[cell_dictionary[max_cell]]->size;
				partB_size -= cell_vector[cell_dictionary[max_cell]]->size;
				free_B--;
			}
			sum += cell_vector[cell_dictionary[max_cell]]->gain;
			update_gain(max_cell);
			//print_bucket();
		}
		else
			flag = true;
		
		iteration++;
		if(iteration%5000==0 && debug_flag == true)
            cout<<"  iter: "<<iteration<<endl;
        if((abs(partA_size - partB_size  ) >= constraint)){
			if(debug_flag == true)
				cout<<"unbalance break"<<endl;
            break;
        }
		if(largest_partial_sum <= sum){
			largest_partial_sum = sum;
			lps_iteration = iteration;
			lps_partA_num = partA_num;
			lps_partB_num = partB_num;
		}
		
		//print_bucket();
	}
    //print_bucket();
    //check_cell_null();
	if(debug_flag == true)
		cout<<"remove size: "<<remove_size<<endl;
	
	if(debug_flag == true){
		cout<<"partA_size: "<<partA_size<<endl;
		cout<<"partB_size: "<<partB_size<<endl;
    }
    
    
	int pop_num = iteration - lps_iteration;
	//reverse the moved cell
	for(int i = 0; i != pop_num; ++i){
		int num = record_cell.top();
		cell* &tmp_cell = cell_vector[num];
		tmp_cell->partition = !(tmp_cell->partition);
		record_cell.pop();
	}
	if(debug_flag == true)
		cout<<"lps iteration: "<<lps_iteration<<" / "<<iteration<<endl; 
	get_AB();
	get_partitionAB_num();
	//get_cutsize();
    if( abs(partA_size - partB_size >= constraint) )
		if(debug_flag == true)
			cout<<"unbalance~~~~"<<endl;
    /*if( abs(partA_size - partB_size < constraint) ){
        for(int i=0 ; i<cell_vector.size() ; i++){
            best_group[i] = cell_vector[i]->partition;
        }
        for(int i=0 ; i<net_vector.size() ; i++){
            best_netA[i] = net_vector[i]->A;
            best_netB[i] = net_vector[i]->B;
        }    
    }
    else{
        break_flag = 1;
    }*/
    
    //cout<<"end FM~~~"<<endl;
	return largest_partial_sum;
}

//FM partition looper
void partition_looper(){
	int iterations = 1;
	special_flag = 15;
    break_flag = 0;
	if(cell_num > 130000 && cell_num<173000)
		special_flag = 2;
    if(cell_num >= 173000)
        special_flag = 3;
	else if(cell_num <= 130000 && cell_num>=100000)
		special_flag = 4;
	if(debug_flag == true)
		cout << "[Partition Looper Summary]"<<endl;
	while(1){
		
		if(special_flag == 0) break;
		int sum = FM_partition();
		
		if(debug_flag == true){
			cout << "Iteration: " << iterations <<endl;
			cout << "LPS: " << sum <<endl;
			++iterations;
		}
		//if Gk <= 0 than terminate
		if(sum <= 0 || break_flag == 1){
			break;
		}
		special_flag--;
	}
}

//output the answer and write file
void output_answer(ostream &out){
		get_partitionAB_num();
		out << "cut_size " << cutsize <<endl;
		out << "A " << partA_num <<endl;
		for(int i = 0; i < cell_num; ++i){
			if(cell_vector[i]->partition == 0){
				out << cell_vector[i]->name <<endl;
			}
		}
		if(debug_flag == true)
			cout << "Print A complete"<<endl;
		out << "B " << partB_num <<endl;
		for(int i = 0; i < cell_num; ++i){
			if(cell_vector[i]->partition == 1){
				out << cell_vector[i]->name <<endl;
			}
		}
		if(debug_flag == true)
			cout << "Print B complete"<<endl;
	return;
}

//time report variable
struct timeval io_st, io_ed, out_st, out_ed, co_st, co_ed;
double io_time, comp_time;
double io_st1, io_ed1, io_st2, io_ed2, co_st1, co_ed1;
int main(int argc, char *argv[]){
	
	//parse_parameter(argc, argv);
	char* cell_name = argv[1];
	char* net_name = argv[2];
	char* outputname = argv[3];
    
	debug_flag=false, time_flag=true, sort_flag=false;
	
	fh_in_cell.open(cell_name);
	fh_in_net.open(net_name);
	fh_out.open(outputname);
	if(!fh_out.is_open()){
		cerr << "Error: Open output file failed...\n";
		exit(EXIT_FAILURE);
	}
	
	
	
	if(time_flag == true){
		gettimeofday(&io_st,NULL);
		io_st1 = io_st.tv_sec + (io_st.tv_usec/1000000.0);
	}
	
	// read cell, net 
	parse_cell(fh_in_cell);
	fh_in_cell.close();
	parse_net(fh_in_net);
	fh_in_net.close();
	
	if(time_flag == true){
		gettimeofday(&io_ed,NULL);
		io_ed1 = io_ed.tv_sec + (io_ed.tv_usec/1000000.0);
		io_time += (io_ed1 - io_st1);
	}

	if(time_flag == true){
		gettimeofday(&co_st,NULL);
		co_st1 = co_st.tv_sec + (co_st.tv_usec/1000000.0);
	}
	
	initial_partition();
	get_AB();
	get_cutsize();
	get_constraint();
	get_pmax();
	
	//start partition
	partition_looper();
	
    /*
    if(cell_num > 130000){
        //  best data~~~~~~~~~~~~
        for(int i=0 ; i<cell_vector.size() ; i++){
            cell_vector[i]->partition = best_group[i];
        }
        for(int i=0 ; i<net_vector.size() ; i++){
            net_vector[i]->A = best_netA[i];
            net_vector[i]->B = best_netB[i];
        }
    }*/
    /*for(int i=0 ; i<cell_vector.size() ; i++){
        cell_vector[i]->partition = best_group[i];
    }
    for(int i=0 ; i<net_vector.size() ; i++){
        net_vector[i]->A = best_netA[i];
        net_vector[i]->B = best_netB[i];
    }*/
    
	get_cutsize();
	
	if(time_flag == true){
		gettimeofday(&co_ed,NULL);
		co_ed1 = co_ed.tv_sec + (co_ed.tv_usec/1000000.0);
		comp_time += (co_ed1 - co_st1);
	}
	
	if(debug_flag == true)
		cout << "Cutsize after FM looper: " << cutsize <<endl;
	
	if(time_flag == true){
		gettimeofday(&out_st,NULL);
		io_st2 = out_st.tv_sec + (out_st.tv_usec/1000000.0);
	}
	output_answer(fh_out);
	fh_out.close();
	if(time_flag == true){
		gettimeofday(&out_ed,NULL);
		io_ed2 = out_ed.tv_sec + (out_ed.tv_usec/1000000.0);
		io_time += (io_ed2 - io_st2);
	}
	
	//time report
	if(time_flag == true){
		cout << "[Time Report]"<<endl;
		cout << "I/O time: " << io_time <<endl;
		cout << "Computing time: " << comp_time <<endl;
		cout << "Total execution time: " << io_time + comp_time <<endl;
	}
	return 0;
	
}