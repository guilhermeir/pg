#include "Algorithm3.h"

// private member functions

void Algorithm3::find_fsets()
{
	//cout << "Finding feasible sets..." << endl << endl;
	uint64_t inc;
	uint64_t limit = (uint64_t)pow(2, m);
	for(it = 1; it < limit; it++)
	{
		index = 0;
		//cout << "Decoding integer " << it << "..." << endl;
		decode_int(it);
		update_interference();
		//cout << "Checking feasibility..." << endl;
		if (is_feasible())
		{
			inc = 0;
			feasible_sets.push_back(it);
		}
		else
			inc = ((it&~(it-1)) - 1);
		it = it + inc;
		//cout << "Clearing current set..." << endl << endl;
		clr_currset();
	}
}

void Algorithm3::decode_int(uint64_t x)
{
	uint64_t q = x / 2;
	uint64_t r = x % 2;

	if(r==0)
	{
		if(q==0)
		{
			//cout << endl << "ERROR: Impossible combination of q and r!" << endl;
			return;
		}
		else
		{
			index++;
			decode_int(q);
		}
	}
	else
	{
		current_set.push_back(network->get_link(index));	//add link to current set
		current_set.back()->get_sender()->inc_degree();		//update sender degree
		current_set.back()->get_recver()->inc_degree();		//update recver degree 
		if(q==0)
		{
			return;
		}
		else
		{
			index++;
			decode_int(q);
		}
	}
}

void Algorithm3::update_interference()
{
	if(current_set.size()<2)
		return;
	for (vector<Link*>::iterator i = current_set.begin(); i != current_set.end(); ++i)
	{
		for (vector<Link*>::iterator j = current_set.begin(); j != current_set.end(); ++j)
		{
			if(i!=j)
				(*i)->add_interf(calculate_interference((*j)->get_sender(), (*i)->get_recver()));
		}
	}
		
}

double Algorithm3::calculate_interference(Node* a, Node* b)
{
    double dist = a->distance(*b);
    if (dist > network->d0)
            return pow(10.0, ((network->tpower_dBm - network->l0_dB - 10 * network->alpha*log10(dist / network->d0)) / 10.0));
    else
            return pow(10.0, network->tpower_dBm - network->l0_dB / 10.0);
}

bool Algorithm3::is_feasible()
{
	if(current_set.size() < 2)
	{
		//cout << "Single link... OK!" << endl;
		return true;
	}
	
	#pragma omp sections
	{
		#pragma omp section 
		{ 	
			masks_test();
		}
		#pragma omp section
		{
			primary_test();
		}
		#pragma omp section
		{
			secondary_test();
		}
	}

	masks_test_end = false;
	prima_test_end = false;
	secon_test_end = false;

	if((masks_test_val == false)||(prima_test_val == false)||(secon_test_val == false))
	{
		masks_test_val = true;
		prima_test_val = true;
		secon_test_val = true;
		return false;
	}
	else
	{
		masks_test_val = true;
		prima_test_val = true;
		secon_test_val = true;
		return true;
	}
}

void Algorithm3::masks_test()
{
	//cout << "Testing masks patterns...";
	for(vector<uint64_t>::iterator i = masks.begin(); i != masks.end(); ++i)
	{
		if((!prima_test_end)||(!secon_test_end))
		{
			if((it & *i) == *i)
			{
				//cout << "Failed! (Found " << *i << " in " << it << ")" << endl;
				masks_test_end = true;
				masks_test_val = false;
				return;
			}
		}
		else
		{
			//cout << "Interference test has finished first" << endl;
		    masks_test_end = true;
   			masks_test_val = true;
			return;
		}

	}
	//cout << "OK!" << endl;
    masks_test_end = true;
    masks_test_val = true;	
	return;
}

void Algorithm3::primary_test()
{
	//cout << "Testing primary interference...";
	if (current_set.size() > n / 2)
	{
		//cout << "Failed!" << endl;
		prima_test_val = false;
		prima_test_end = true;
		return;
	}
	for (vector<Link*>::iterator i = current_set.begin(); i != current_set.end(); ++i)
	{
		if((masks_test_end==true)&&(masks_test_val==false))
		{
			//cout << "Masks test has finished first" << endl;
			prima_test_val = true;
			prima_test_end = true;
			return;
		}
		else
		{
			if(((*i)->get_sender()->get_degree() > 1)||((*i)->get_recver()->get_degree() > 1))
			{
				//cout << "Failed!" << endl;
				masks.push_back(it);				
				prima_test_val = false;
				prima_test_end = true;
		        return;
		    }
		}
	}
	//cout << "OK!" << endl;
	prima_test_val = true;
	prima_test_end = true;
	return;
}

void Algorithm3::secondary_test()
{
	//cout << "Testing secondary interference...";
	double sinr;
	for(vector<Link*>::iterator i = current_set.begin(); i != current_set.end(); ++i)
	{
		if(((masks_test_end==true)&&(masks_test_val==false))||
		   ((prima_test_end==true)&&(prima_test_val==false)))
		{
			//cout << "Masks test has finished first" << endl;
			secon_test_val = true;
			secon_test_end = true;
			return;
		}
		else
		{
			sinr = calculate_sinr(*i);
			if(sinr < network->beta_mW)
			{
				//cout << "Failed!" << endl;
				masks.push_back(it);
				secon_test_val = false;
				secon_test_end = true;
				return;
			}	
		}	
	}
	//cout << "OK!" << endl;
	secon_test_val = true;
	secon_test_end = true;
	return;
}

double Algorithm3::calculate_sinr(Link* l)
{
	//l->prt_interf();
	////cout << l->get_rpower() << "/(" << network->noise_mW << " + " << l->clc_interf() << ")" << endl;
    return l->get_rpower() / (network->noise_mW + l->clc_interf());
}

void Algorithm3::clr_currset()
{
	for (vector<Link*>::iterator i = current_set.begin(); i != current_set.end(); ++i)
	{
		(*i)->get_sender()->dec_degree(); 
		(*i)->get_recver()->dec_degree();
		(*i)->clr_interf();
	}
	current_set.clear();
}

void Algorithm3::print_currset()
{
	cout << "(";
	for (vector<Link*>::iterator i = current_set.begin(); i != current_set.end(); ++i)
		cout << " " << (*i)->get_id();
	cout << " )" << endl;
}

// public member functions

Algorithm3::Algorithm3(Network* g): n(g->get_nodes().size()), m(g->get_links().size()), network(g)
{
	cout << "Initializing Algorithm 3..." << endl;
	find_fsets();
	cout << "Feasible sets found." << endl;
}

vector<uint64_t> Algorithm3::get_fsets()
{
	return feasible_sets;
}

void Algorithm3::print_fsets()
{
	cout << "Printing fsets for Algorithm 3..." << endl;
	for (vector<uint64_t>::iterator i = feasible_sets.begin(); i != feasible_sets.end(); ++i)
	{
		cout << *i << " ";
		//index = 0;
		//decode_int(*i);
		//print_currset();
		//clr_currset();
	}
	cout << endl;
}
