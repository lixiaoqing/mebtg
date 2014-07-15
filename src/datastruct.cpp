#include "datastruct.h"

bool larger( const Cand *pl, const Cand *pr )
{
	return pl->score > pr->score;
}

/************************************************************************
 1. 函数功能: 将翻译候选加入列表中, 并进行假设重组
 2. 入口参数: 翻译候选的指针
 3. 出口参数: 如果候选被丢弃或者替换掉原来的候选,返回false;否则返回true
 4. 算法简介: a) 如果当前候选与优先级队列中的某个候选的目标端边界词相同,
              a.1) 如果当前候选的得分低, 则丢弃当前候选
              a.2) 如果当前候选的得分低, 则替换原候选
              a.3) 如果二者得分相同, 则将当前候选加入列表
              b) 如果当前候选与优先级队列中的所有候选的目标端边界词不同,
	         则将当前候选加入列表
 * **********************************************************************/
bool Candli::add(Cand *cand_ptr)
{ 
	vector<int> bound_wids = get_bound_wids(cand_ptr);
	auto it = bound_to_cand_map.find(bound_wids);
	if (it != bound_to_cand_map.end())
	{
		if (cand_ptr->score <= it->second->score)
		{
			return false;
		}
		else
		{
			*(it->second) = *cand_ptr;
			return false;
		}
	}
	data.push_back(cand_ptr);
	bound_to_cand_map.insert(make_pair(bound_wids,cand_ptr));
	return true;
}

vector<int> Candli::get_bound_wids(const Cand *cand_ptr)
{
	const vector<int> &wids = cand_ptr->tgt_wids;
	size_t BOUND_SIZE = LM_ORDER -1;
	if (wids.size() <= 2*BOUND_SIZE)
		return wids;
	vector<int> bound_wids(wids.begin(),wids.begin()+BOUND_SIZE);
	bound_wids.insert(bound_wids.end(),wids.end()-BOUND_SIZE,wids.end());
	return bound_wids;
}

