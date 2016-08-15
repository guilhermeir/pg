/*
** Algorithm 2: Iterative + Sequential masks test
**
*/

#pragma once

#include <stdint.h>
#include <vector>
#include <math.h>
#include "Node.h"
#include "Link.h"
#include "Network.h"

class Algorithm2
{
private:
	uint64_t it, index, n, m;
	vector<Link*> current_set;
	vector<uint64_t> feasible_sets;
	vector<uint64_t> masks;
	Network* network;
	
	void find_fsets();
	void decode_int(uint64_t);
	void update_interference();	
	double calculate_interference(Node*, Node*);
	bool is_feasible();
	bool masks_test();
	bool primary_test();
	bool secondary_test();
	double calculate_sinr(Link*);				
	void clr_currset();
	void print_currset();
		
public:
	Algorithm2(Network*);
	vector<uint64_t> get_fsets();
	void print_fsets();
};

