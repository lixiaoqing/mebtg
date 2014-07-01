#include "datastruct.h"

bool cmp(const Cand *pl, const Cand *pr)
{
	return pl->score < pr->score;
}

void Candpq::push(Cand *cand_ptr)
{ 
	for (Cand *e_cand_ptr : data)
	{
		if (is_bound_same(cand_ptr,e_cand_ptr))
		{
			if (cand_ptr->score < e_cand_ptr->score)
			{
				return;
			}
			if (cand_ptr->score > e_cand_ptr->score)
			{
				*e_cand_ptr = *cand_ptr;
				make_heap(data.begin(),data.end(),cmp);
				return;
			}
		}
	}
	data.push_back(cand_ptr); 
	push_heap(data.begin(), data.end(),cmp); 
}

void Candpq::pop()
{
	pop_heap(data.begin(), data.end(),cmp);
	data.pop_back();
}

bool Candpq::is_bound_same(const Cand *a, const Cand *b)
{
	size_t len_a = a->tgt_wids.size();
	size_t len_b = b->tgt_wids.size();
	size_t bound_len_a = min(len_a, LM_ORDER-1);
	size_t bound_len_b = min(len_b, LM_ORDER-1);
	if (bound_len_a != bound_len_b)
		return false;
	if (bound_len_a < LM_ORDER && a->tgt_wids != b->tgt_wids)
		return false;
	for (size_t i=0;i<bound_len_a;i++)
	{
		if (a->tgt_wids.at(i) != b->tgt_wids.at(i) || a->tgt_wids.at(len_a-1-i) != b->tgt_wids.at(len_b-1-i))
			return false;
	}
	return true;
}

