/*
	HypoStack.cpp
	HypoStack���ʵ��
	*/

#include "HypoStack.h"
//
//
////////////////////////////////////////////////////////////////////////
// 1.��Ҫ���ܣ���������뵽ջ�У�����ͨ��"RECOMBINE_HYPOTHESES"ѡ���Ƿ�����������
// 2.��ڲ������µļ���
// 3.���ڲ�����ջ�ڼ���ĸ���
// 4.�������ڣ�2006.04.10
// 5.�㷨��飺�������ֵ�洢�����������У�������������ֻ��ż��������������еĵ�ַ
//				�����������ʱ���������������ѷ����Դ���Դ��Ƿ�һ�£�������һ�������Ƿ�һ�£�
//											�������Ӣ�Ĵ��Ƿ�һ�¡�
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
	//�����RECOMBINE_HYPOTHESES��Ϊ�棬������������
	if( RECOMBINE_HYPOTHESES )
	{
		//���ջ����
		if( _ulSize == STACK_SIZE )
		{

			double m_dpro = Hypotheses.dPro;
			//����µ�hypotheses��ջ����������hypotheses�ã�������ջ������ֱ���˳�
			if( m_dpro > _pstHypotheseslist[_ulSize-1]->dPro )
			{
				bool m_bRecombine = true;                       //Ĭ����Ҫ��������(recombine)��hypotheses
				int posInsert = _ulSize - 1;                    //Ĭ�ϰ�����Ӧ�÷��õ�λ�ã�
				int posRecombine = -1;						    //�����Ҫ���飬��¼�����λ��
				int m_size = 0;
				for( m_size =_ulSize-2; m_size>=0; m_size-- )
				{
					if( m_dpro > _pstHypotheseslist[m_size]->dPro )
					{
						posInsert = m_size;                     //��¼���ո���hypothesesӦ�÷��õ�λ��
					}
					//����Ѿ��ҵ�������ֵӦ�÷��õ�λ�ã������Ѿ��ҵ�����λ�ã����˳�ѭ��
					else if( posRecombine != -1 )
					{
						break;
					}
					if( Hypotheses == *_pstHypotheseslist[m_size] )
					{
						posRecombine = m_size;
					}			

				}
				//�������Ҫ�������飬�򰴸���ֵ����
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
				//��������ʲ����λ��������֮ǰ�� ����밴���ʵ�λ�ã���������滻��
				//����posInsert��СΪ0�� ��������ʱposRecombine��Ϊ-1,����Ȼ��Ҫ����
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

		//ջδ��
		else if( _ulSize > 0 && _ulSize < STACK_SIZE )
		{

			bool m_bRecombine = true;                //Ĭ����Ҫ��������(recombine)��hypotheses
			double m_dpro = Hypotheses.dPro;
			int posInsert = _ulSize ;                    //Ĭ�ϰ�����Ӧ�÷��õ�λ�ã�
			int posRecombine = -1;						    //�����Ҫ���飬��¼�����λ��
			int m_size=0;
			for( m_size = _ulSize-1; m_size>= 0; m_size--)
			{
				if( m_dpro > _pstHypotheseslist[m_size]->dPro )
				{
					posInsert = m_size;
				}
				//����Ѿ��ҵ�������ֵӦ�÷��õ�λ�ã������Ѿ��ҵ�����λ�ã����˳�ѭ��
				else if( posRecombine != -1 )
				{
					break;
				}
				if( Hypotheses == *_pstHypotheseslist[m_size] )
				{
					posRecombine = m_size;
				}	
			}
			//�������Ҫ�������飬�򰴸���ֵ����
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
			//��������ʲ����λ��������֮ǰ�� ����밴���ʵ�λ�ã���������滻��
			//����posInsert��СΪ0�� ��������ʱposRecombine��Ϊ-1,����Ȼ��Ҫ����
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
		//ջΪ��
		else if( 0 == _ulSize )
		{
			_pstHypothesesArray[_ulSize] = Hypotheses;
			_pstHypotheseslist[_ulSize] = &_pstHypothesesArray[_ulSize];
			_ulSize++;
		}
	}	
	else	//�����RECOMBINE_HYPOTHESES��Ϊ�٣��򲻽����������
	{
		//////////////////////////////////////////////////////////////////////////
		//���ջ����
		if( _ulSize == STACK_SIZE )
		{
			double m_dpro = Hypotheses.dPro;
			if( m_dpro > _pstHypotheseslist[_ulSize-1]->dPro )                   //�²����Ԫ�ر�ջ������Ԫ�ش��۵ͣ����ʸߣ�
			{
				*(_pstHypotheseslist[_ulSize-1]) = Hypotheses;                   //������Ԫ���滻Ϊ�²����Ԫ��
				s_Hypotheses* m_pHypotheses = _pstHypotheseslist[ _ulSize-1 ];   //��¼�洢�²���Ԫ�صĵ�ַ
				int m_size;

				//���²���Ԫ�صĵ�ַ������������뵽�洢��ַ������
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

		//ջδ��
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
		//ջΪ��
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
//����ջ����õ�Ԫ��
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



