/*
SearchSpace.cpp
*/


#include "SearchSpaceStock.h"

//realization of the class HypoHeap
void HypoHeap::Init(HEAPTYPE heapType, int heapSize)
{
	_heapType = heapType;
	_heapSize = heapSize;	
	_count = 0;
	_pHypoList.resize(_heapSize, 0);
}

HypoHeap::~HypoHeap()
{
	_pHypoList.clear();
	_heapSize = 0;
	_count = 0;
}

void HypoHeap::Clear()
{
	_pHypoList.resize(_heapSize, 0);
	_count = 0;
}

//just add the capacity of the heap
void HypoHeap::Resize(int newHeapSize)
{
	if( newHeapSize > _heapSize )
	{
		_heapSize = newHeapSize;
		_pHypoList.resize(_heapSize);
	}
}

bool HypoHeap::Empty()
{
	return 0 == _count;
}

bool HypoHeap::Full()
{
	return _count == _heapSize;
}

int HypoHeap::Count()
{
	return _count;
}

bool HypoHeap::Compare(s_SearchSpace* oneHypo, s_SearchSpace* anotherHypo)
{
	if( MINHEAP == _heapType )
		return oneHypo->dPro < anotherHypo->dPro;
	else
		return oneHypo->dPro > anotherHypo->dPro;
}

void HypoHeap::Push(s_SearchSpace* hypo)
{
	if( _count == _heapSize ) //the heap is full
	{
		return;
	}

	_pHypoList[_count] = hypo;
	Up(_count);
	_count++;
}

s_SearchSpace* HypoHeap::Pop()
{
	if( 0 == _count )
		return NULL;

	s_SearchSpace* tmpHypo = _pHypoList[0];
	_count--;
	_pHypoList[0] = _pHypoList[_count];
	_pHypoList[_count] = NULL;
	Down(0);

	return tmpHypo;
}

/*********************************************************************
 Update function is a little hard to understand
 take the minheap for example, it is used to store the N largest hypos
 and the top one is the minimum of the N largest hypos, thus for an input
 hypo, its value should be larger than the minimum one or the one has the
 same translation with the input hypo
**********************************************************************/
void HypoHeap::Update(s_SearchSpace* hypo)
{
	if( _count == _heapSize && !Compare(_pHypoList[0], hypo) )
		return;

	//update the hypothesis
	for( int i = 0; i < _count; i++ )
	{
		if( hypo->m_en_str != _pHypoList[i]->m_en_str )
			continue;
		if( Compare(_pHypoList[i], hypo) )
		{
			_pHypoList[i] = hypo;
			Down(i);
		}
		return;
	}

	if( _count < _heapSize )
	{
		Push(hypo);
		return;
	}

	//hypo is better than _pHypoList[0]
	_pHypoList[0] = hypo;
	Down(0);
}

void HypoHeap::Up(int hypoPos)
{
	int childPos = hypoPos;
	int parentPos = (childPos-1)/2;
	while( parentPos >= 0 )
	{
		if( Compare(_pHypoList[childPos], _pHypoList[parentPos]) )
		{
			s_SearchSpace* tmpHypo = _pHypoList[childPos];
			_pHypoList[childPos] = _pHypoList[parentPos];
			_pHypoList[parentPos] = tmpHypo;
			childPos = parentPos;
			parentPos = (parentPos-1)/2;
		}
		else
			return;
	}
}

void HypoHeap::Down(int hypoPos)
{
	int parentPos = hypoPos;
	int leftChildPos = parentPos*2+1, rightChildPos = parentPos*2+2;
	while( leftChildPos < _count )
	{
		int keyPos = rightChildPos>=_count || Compare(_pHypoList[leftChildPos], _pHypoList[rightChildPos])?leftChildPos:rightChildPos;
		if( !Compare(_pHypoList[keyPos], _pHypoList[parentPos]) )
			break;
		s_SearchSpace* tmpHypo = _pHypoList[parentPos];
		_pHypoList[parentPos] = _pHypoList[keyPos];
		_pHypoList[keyPos] = tmpHypo;
		parentPos = keyPos;
		leftChildPos = parentPos*2+1;
		rightChildPos = parentPos*2+2;
	}
}


/*
////////////////////////////////////////////////////////////////////////
// 1.主要功能：将新假设入栈
// 2.入口参数：新假设
// 3.出口参数：栈内元素的个数
// 4.生成日期：2006.04.10
// 5.算法简介：考虑栈空、未满、已满三种情况，分别处理；
//				假设存在数据数组中，_stSearchSpaceList中只是按照概率的大小存储假设的地址
//
int 
SearchSpaceStock::push(s_SearchSpace &searchspace)
{
	//如果没有锁定，进行入栈操作
	if( !_bClose )
	{
		//栈已满
		if( _ulSize == TRAN_TABLE_SIZE )
		{
			double m_dpro = searchspace.dPro;
			if( m_dpro > _stSearchSpaceList[_ulSize-1]->dPro )
			{
				//
				//如果新的search space 的cost 值比当前栈中的最差值好，则插入
				//
				*_stSearchSpaceList[_ulSize-1] = searchspace;
				s_SearchSpace* m_pSearchSpace =_stSearchSpaceList[_ulSize-1];			
				int m_size;
				//查找应该插入的位置
				for(  m_size = _ulSize -2; m_size>=0; m_size-- )
				{
					if( m_dpro > _stSearchSpaceList[m_size]->dPro )				//如果新的search space 的概率值比当前值大，则插入
					{
						_stSearchSpaceList[m_size+1] = _stSearchSpaceList[m_size];
					}
					else
					{
						_stSearchSpaceList[m_size+1] = m_pSearchSpace;
						break;
					}
				}
				if(m_size < 0 )
				{
					_stSearchSpaceList[0] = m_pSearchSpace;
				}			
			}	

		}
		//栈未满
		else if( _ulSize < TRAN_TABLE_SIZE && _ulSize > 0)
		{
			_stSearchSpaceArray[ _ulSize ] = searchspace;
			double m_dpro = searchspace.dPro; 
			s_SearchSpace* m_pSearchSpace = &_stSearchSpaceArray[_ulSize];
			int m_size;
			//查找应该插入的位置
			for( m_size = _ulSize -1; m_size>=0; m_size -- )
			{
				if( m_dpro > _stSearchSpaceList[m_size]->dPro )                   //如果新的search space 的概率值比当前值大，则插入
				{
					_stSearchSpaceList[m_size+1] = _stSearchSpaceList[m_size];

				}
				else
				{
					_stSearchSpaceList[m_size+1] = m_pSearchSpace;
					break;
				}
			}
			if( m_size < 0 )
			{
				_stSearchSpaceList[0] = m_pSearchSpace;
			}

			_ulSize++;
		}
		//栈空
		else if( _ulSize == 0 )
		{
			_stSearchSpaceArray[_ulSize] = searchspace;
			_stSearchSpaceList[_ulSize] = &_stSearchSpaceArray[_ulSize ];
			_ulSize++;
		}	
	}
	

	return _ulSize;
}

//判断两个数组的前后两个部分是否相同,由张家俊08年12月25日添加
int HasSamePart(vector<int> a, vector<int> b, int len)
{
    int c_len;
    if((int)a.size()<len || (int)b.size()<len)
	if(a.size() != b.size())
	    return 0;
	else
	   c_len=(int)a.size();
    else
	c_len=len;

    for(int i=0;i<c_len;i++)
	if(a[i] != b[i])
	    return 0;
    for(int i=(int)a.size()-c_len,j=(int)b.size()-c_len;i<(int)a.size() && j<(int)b.size();i++,j++)
	if(a[i] != b[j])
	    return 0;

    return 1;
}

//以下的函数实现由张家俊08年12月25日添加
//将句子的翻译过程看作CKY的分析过程，以下实现分析过程中的加边操作
int 
SearchSpaceStock::AddEdge(s_SearchSpace &searchspace)
{
    //threshold pruning
    double alpha = 0.2;
    if(_ulSize > 0 && searchspace.dPro < log10(alpha) +  _stSearchSpaceList[_ulSize-1]->dPro )
	return 0;

    //recombination pruning
    for(int i=_ulSize-1;i>=0;i--)
    {
	double c_trans_prob=searchspace.dPro;
	if(HasSamePart(searchspace.viEnPhrase,_stSearchSpaceList[i]->viEnPhrase, m_lm_order-1))
	{
	    if(c_trans_prob > _stSearchSpaceList[i]->dPro)
	    {
		s_SearchSpace c_tmp_searchspace = *_stSearchSpaceList[i];
		searchspace.m_recombine_array = c_tmp_searchspace.m_recombine_array;
		c_tmp_searchspace.m_recombine_array.clear();
		searchspace.m_recombine_array.push_back(c_tmp_searchspace);
		//for(int k=0; k<c_tmp_searchspace.m_recombine_array.size(); k++)
		    //searchspace.m_recombine_array.push_back(c_tmp_searchspace.m_recombine_array[k]);
		*_stSearchSpaceList[i] = searchspace;
		s_SearchSpace* c_pSearchSpace = _stSearchSpaceList[i];
		int c_tmp_pos;
		for( c_tmp_pos = i-1; c_tmp_pos>=0; c_tmp_pos-- )
		{
		    if( c_trans_prob > _stSearchSpaceList[c_tmp_pos]->dPro )
		    {
			_stSearchSpaceList[c_tmp_pos+1] = _stSearchSpaceList[c_tmp_pos];
		    }
		    else
		    {
			_stSearchSpaceList[c_tmp_pos+1] = c_pSearchSpace;
			break;
		    }
		}
		if(c_tmp_pos<0)
		{
		    _stSearchSpaceList[0] = c_pSearchSpace;
		}
	    }
	    else
	    {
		(_stSearchSpaceList[i]->m_recombine_array).push_back(searchspace);
	    }

	    return 0;
	}
    }
    //Historgram pruning
    //下面就看是否超出搜索栈的大小,张家俊08年12月25日添加
    //栈已满
    if( _ulSize == TRAN_TABLE_SIZE )
    {
	double m_dpro = searchspace.dPro;
	if( m_dpro > _stSearchSpaceList[_ulSize-1]->dPro )
	{
	    //
	    //如果新的search space 的cost 值比当前栈中的最差值好，则插入
	    //
	    *_stSearchSpaceList[_ulSize-1] = searchspace;
	    s_SearchSpace* m_pSearchSpace =_stSearchSpaceList[_ulSize-1];			
	    int m_size;
	    //查找应该插入的位置
	    for(  m_size = _ulSize -2; m_size>=0; m_size-- )
	    {
		if( m_dpro > _stSearchSpaceList[m_size]->dPro )				//如果新的search space 的概率值比当前值大，则插入
		{
		    _stSearchSpaceList[m_size+1] = _stSearchSpaceList[m_size];
		}
		else
		{
		    _stSearchSpaceList[m_size+1] = m_pSearchSpace;
		    break;
		}
	    }
	    if(m_size < 0 )
	    {
		_stSearchSpaceList[0] = m_pSearchSpace;
	    }			
	}	

    }
    //栈未满
    else if( _ulSize < TRAN_TABLE_SIZE && _ulSize > 0)
    {
	_stSearchSpaceArray[ _ulSize ] = searchspace;
	double m_dpro = searchspace.dPro; 
	s_SearchSpace* m_pSearchSpace = &_stSearchSpaceArray[_ulSize];
	int m_size;
	//查找应该插入的位置
	for( m_size = _ulSize -1; m_size>=0; m_size -- )
	{
	    if( m_dpro > _stSearchSpaceList[m_size]->dPro )                   //如果新的search space 的概率值比当前值大，则插入
	    {
		_stSearchSpaceList[m_size+1] = _stSearchSpaceList[m_size];

	    }
	    else
	    {
		_stSearchSpaceList[m_size+1] = m_pSearchSpace;
		break;
	    }
	}
	if( m_size < 0 )
	{
	    _stSearchSpaceList[0] = m_pSearchSpace;
	}

	_ulSize++;
    }
    //栈空
    else if( _ulSize == 0 )
    {
	_stSearchSpaceArray[_ulSize] = searchspace;
	_stSearchSpaceList[_ulSize] = &_stSearchSpaceArray[_ulSize ];
	_ulSize++;
    }	


    return _ulSize;
}
*/

