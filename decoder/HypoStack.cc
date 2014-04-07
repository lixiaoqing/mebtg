/*
	HypoStack.cpp
	HypoStack类的实现
	*/

#include "HypoStack.h"
//
//
////////////////////////////////////////////////////////////////////////
// 1.主要功能：将假设放入到栈中，可以通过"RECOMBINE_HYPOTHESES"选择是否进行重组假设
// 2.入口参数：新的假设
// 3.出口参数：栈内假设的个数
// 4.生成日期：2006.04.10
// 5.算法简介：将假设的值存储在数据数组中，在排序数组中只存放假设在数据数组中的地址
//				进行重组假设时考虑三个条件：已翻译的源语言词是否一致；最后翻译的一个短语是否一致；
//											最后两个英文词是否一致。
//
//

bool s_Hypotheses::operator ==(const s_Hypotheses &right)const
{
//	if( (bitCovered  == right.bitCovered) 
//		&& (ulLastTwoEnWord[0] == right.ulLastTwoEnWord[0]) && ulLastTwoEnWord[1] == right.ulLastTwoEnWord[1]
//		&& (pstLastPhrase->ulLastWord == right.pstLastPhrase->ulLastWord) )
	if( (bitCovered  == right.bitCovered) 
		&& ( ngramLastWord == right.ngramLastWord )
		&& (pstLastPhrase->ulLastWord == right.pstLastPhrase->ulLastWord) )
		return true;
	else
		return false;

}

int 
HypoStack::push(s_Hypotheses &Hypotheses)
{
	//////////////////////////////////////////////////////////////////////////
	//如果“RECOMBINE_HYPOTHESES”为真，则进行重组假设
	if( RECOMBINE_HYPOTHESES )
	{
		//如果栈已满
		if( _ulSize == STACK_SIZE )
		{

			double m_dpro = Hypotheses.dPro;
			//如果新的hypotheses比栈中已有最差的hypotheses好，则考虑入栈，否则直接退出
			if( m_dpro > _pstHypotheseslist[_ulSize-1]->dPro )
			{
				bool m_bRecombine = true;                       //默认需要进行重组(recombine)的hypotheses
				int posInsert = _ulSize - 1;                    //默认按概率应该放置的位置；
				int posRecombine = -1;						    //如果需要重组，记录重组的位置
				int m_size = 0;
				for( m_size =_ulSize-2; m_size>=0; m_size-- )
				{
					if( m_dpro > _pstHypotheseslist[m_size]->dPro )
					{
						posInsert = m_size;                     //记录按照概率hypotheses应该放置的位置
					}
					//如果已经找到按概率值应该放置的位置，并且已经找到重组位置，则退出循环
					else if( posRecombine != -1 )
					{
						break;
					}
					if( Hypotheses == *_pstHypotheseslist[m_size] )
					{
						posRecombine = m_size;
					}			

				}
				//如果不需要进行重组，则按概率值插入
				if( posRecombine == -1 )
				{
					*_pstHypotheseslist[_ulSize-1] = Hypotheses;
					s_Hypotheses* m_pHypotheses = _pstHypotheseslist[_ulSize-1];
					for( int j=_ulSize-1; j>posInsert ; j--)
					{
						_pstHypotheseslist[j] = _pstHypotheseslist[j-1];
					}
					_pstHypotheseslist[posInsert] = m_pHypotheses;
				}
				//如果按概率插入的位置在重组之前， 则放入按概率的位置，将重组的替换掉
				//由于posInsert最小为0， 满足条件时posRecombine不为-1,即必然需要重组
				else if( posInsert <= posRecombine )
				{
					Hypotheses.addArc.push_back(*_pstHypotheseslist[posRecombine]);
					*_pstHypotheseslist[posRecombine] = Hypotheses;
					s_Hypotheses* m_pHypotheses = _pstHypotheseslist[posRecombine];

					for( int j=posRecombine; j>posInsert; j--)
					{
						_pstHypotheseslist[j] = _pstHypotheseslist[j-1];
					}
					_pstHypotheseslist[posInsert] = m_pHypotheses;
				}
				else
				{
					_pstHypotheseslist[posRecombine]->addArc.push_back( Hypotheses );
				}
			}
		}

		//栈未满
		else if( _ulSize > 0 && _ulSize < STACK_SIZE )
		{

			bool m_bRecombine = true;                //默认需要进行重组(recombine)的hypotheses
			double m_dpro = Hypotheses.dPro;
			int posInsert = _ulSize ;                    //默认按概率应该放置的位置；
			int posRecombine = -1;						    //如果需要重组，记录重组的位置
			int m_size=0;
			for( m_size = _ulSize-1; m_size>= 0; m_size--)
			{
				if( m_dpro > _pstHypotheseslist[m_size]->dPro )
				{
					posInsert = m_size;
				}
				//如果已经找到按概率值应该放置的位置，并且已经找到重组位置，则退出循环
				else if( posRecombine != -1 )
				{
					break;
				}
				if( Hypotheses == *_pstHypotheseslist[m_size] )
				{
					posRecombine = m_size;
				}	
			}
			//如果不需要进行重组，则按概率值插入
			if( posRecombine == -1 )
			{
				_pstHypothesesArray[_ulSize] = Hypotheses;
				for( int j=_ulSize; j>posInsert ; j--)
				{
					_pstHypotheseslist[j] = _pstHypotheseslist[j-1];
				}
				_pstHypotheseslist[posInsert] = &_pstHypothesesArray[_ulSize];
				_ulSize ++;
			}
			//如果按概率插入的位置在重组之前， 则放入按概率的位置，将重组的替换掉
			//由于posInsert最小为0， 满足条件时posRecombine不为-1,即必然需要重组
			else if( posInsert <= posRecombine )
			{
				Hypotheses.addArc.push_back( *_pstHypotheseslist[posRecombine] );
				*_pstHypotheseslist[posRecombine] = Hypotheses;
				s_Hypotheses* m_pHypotheses = _pstHypotheseslist[posRecombine];

				for( int j=posRecombine; j>posInsert; j--)
				{
					_pstHypotheseslist[j] = _pstHypotheseslist[j-1];
				}
				_pstHypotheseslist[posInsert] = m_pHypotheses;
			}
			else
			{
				_pstHypotheseslist[posRecombine]->addArc.push_back( Hypotheses );
			}

		}
		//栈为空
		else if( 0 == _ulSize )
		{
			_pstHypothesesArray[_ulSize] = Hypotheses;
			_pstHypotheseslist[_ulSize] = &_pstHypothesesArray[_ulSize];
			_ulSize++;
		}
	}	
	else	//如果“RECOMBINE_HYPOTHESES”为假，则不进行重组假设
	{
		//////////////////////////////////////////////////////////////////////////
		//如果栈已满
		if( _ulSize == STACK_SIZE )
		{
			double m_dpro = Hypotheses.dPro;
			if( m_dpro > _pstHypotheseslist[_ulSize-1]->dPro )                   //新插入的元素比栈中最差的元素代价低（概率高）
			{
				*(_pstHypotheseslist[_ulSize-1]) = Hypotheses;                   //将最差的元素替换为新插入的元素
				s_Hypotheses* m_pHypotheses = _pstHypotheseslist[ _ulSize-1 ];   //记录存储新插入元素的地址
				int m_size;

				//将新插入元素的地址按代价排序插入到存储地址数组中
				for( m_size =_ulSize-2; m_size>=0; m_size-- )
				{
					if( m_dpro > _pstHypotheseslist[m_size]->dPro )
					{
						_pstHypotheseslist[m_size+1] = _pstHypotheseslist[m_size];
					}
					else
					{
						_pstHypotheseslist[m_size+1] = m_pHypotheses;
						break;
					}
				}
				if( m_size< 0 )
				{
					_pstHypotheseslist[0] = m_pHypotheses;
				}
			}
		}

		//栈未满
		else if( _ulSize > 0 && _ulSize < STACK_SIZE )
		{
			_pstHypothesesArray[_ulSize] = Hypotheses;
			double m_dpro = Hypotheses.dPro;
			s_Hypotheses* m_pHypotheses = &_pstHypothesesArray[_ulSize];
			int m_size;
			for( m_size = _ulSize-1; m_size>= 0; m_size--)
			{
				if( m_dpro > _pstHypotheseslist[m_size]->dPro )
				{
					_pstHypotheseslist[m_size+1] = _pstHypotheseslist[m_size]; 
				}
				else
				{
					_pstHypotheseslist[m_size+1] = m_pHypotheses;
					break;
				}
			}
			if( m_size < 0)
			{
				_pstHypotheseslist[0] = m_pHypotheses;
			}
			_ulSize++;

		}
		//栈为空
		else if( 0 == _ulSize )
		{
			_pstHypothesesArray[_ulSize] = Hypotheses;
			_pstHypotheseslist[_ulSize] = &_pstHypothesesArray[_ulSize];
			_ulSize++;
		}
	}

	return _ulSize;	
}

/**********************************************************************************************/
//弹出栈内最好的元素
/**********************************************************************************************/
s_Hypotheses* 
HypoStack::pop()										
{
	if( 0 == _ulSize )
	{
		return NULL;
	}
	else if( _ulPopNum >= _ulSize )
	{
		_ulPopNum = 0;
		return NULL;
	}
	else
	{
		return _pstHypotheseslist[_ulPopNum++];
	}
}


void 
HypoStack::ShowHypotheses()
{
	for( int i=0; i<_ulSize; ++i )
	{
		cout<<"<number=\""<<i<<"\""<<endl;
		cout<<"<Hypothesis Father=("<<_pstHypotheseslist[i]->prestack<<","<<_pstHypotheseslist[i]->prenum<<")>"<<'\t';
		cout<<"<currentTranslation> ";
		for( int j=0; _pstHypotheseslist[i]->pstLastPhrase && j<_pstHypotheseslist[i]->pstLastPhrase->ulEnNum; ++j)
		{
			cout<<_pVocab->GetWord( _pstHypotheseslist[i]->pstLastPhrase->viEnPhrase[j] )<<" ";
		}
		cout<<"total pro: "<<_pstHypotheseslist[i]->dPro<<" Now Pro: "<<_pstHypotheseslist[i]->dNowPro; //<<" LM pro: "<<_pstHypotheseslist[i]->dLMPro;
		cout<<endl;
	}
}



