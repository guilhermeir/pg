/*
** Algorithm 6: Recursive + Parallel masks test without interruption
**
*/

#pragma once

#include <stdint.h>
#include <vector>
#include <math.h>
#include <thread>
#include "Node.h"
#include "Link.h"
#include "Network.h"

class Algorithm6
{
private:
	bool intrf_test_end, intrf_test_val,
		 masks_test_end, masks_test_val,
		 result;
	uint64_t it, index, n, m;
	vector<Link*> current_set;
	vector<uint64_t> fsets;
	vector<uint64_t> masks; // or patterns?
	Network* network;
	
	void find_fsets(uint64_t);
	void add_link(uint64_t);
	double calculate_interference(Node*, Node*);
	void print_interf();
	bool is_feasible();
	void intrf_test();
	void masks_test();
	bool primary_test();
	bool secondary_test();
	double calculate_sinr(Link*);
	void del_link(uint64_t);
	
public:
	Algorithm6(Network*);
	vector<uint64_t> get_fsets();
	void print_fsets();
	void print_masks();
};