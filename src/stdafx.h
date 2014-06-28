#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <set>
#include <vector>
#include <map>

#include <algorithm>
#include <bitset>
#include <queue>
#include <functional>


#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>


using namespace std;

const int END_ID=0;
const int RULE_LEN_MAX=7;
const int PROB_NUM=4;





const int WORD_LEN_MAX=100;
const int SEN_LEN_MAX = 500;			  //���ӵ���󳤶�(������)
const int SEN_CHAR_MAX = 5000;         //���ӵ�����ַ���
const int PHRASE_LEN_MAX = 40;         //phrase max length
//const int FEAT_NUM=8;                   //�����ĸ���



//#define HUGE_VAL 1000

char* const g_pcPhraseSeparators= "|||";
            
char* const g_pcWordSeparators = " \t\r\n";			 //��֮��ķָ���
char* const g_Vocab_NULL = "NULL";					 //��
char* const g_Vocab_Unknown = "<unk>";				 //δ֪��
char* const g_Vocab_SentStart = "<s>";				  //���ӵĿ�ʼ��־
char* const     g_Vocab_SentEnd = "</s>";				  //���ӵĽ�����־
            
char* const g_Vocab_NE = "_NE";					  //����NE
char* const g_Vocab_Org = "_ORG";					//��֯������
char* const g_Vocab_Time = "_TIMEP" ;                //����ʱ��
char* const g_Vocab_Number= "_DIGITP" ;              //��������
char* const g_Vocab_Loc = "_LOCATION";				//����
char* const g_Vocab_Person = "_PERSON";				//����
char* const g_Vocab_Namex = "_NAMEX";				//�����
            
char* const g_NP_Separator="|||";						//�����ѡ��ķָ���
char* const g_Number_Start="<NUMBER>";					//�������﷭���ѡ���������ʼ��־
char* const g_Number_End = "</NUMBER>";					//�������﷭���ѡ������ֽ�����־
char* const g_Time_Start = "<TIME>";					//�������﷭���ѡ���ʱ����ʼ��־
char* const g_Time_End = "</TIME>";						//�������﷭���ѡ���ʱ�������־
char* const g_Loc_Start = "<LOC>";
char* const g_Loc_End = "</LOC>";
char* const g_Person_Start = "<PERSON>";
char* const g_Person_End = "</PERSON>";
char* const g_Org_Start = "<ORG>";
char* const g_Org_End = "</ORG>";
char* const g_Namex_Start = "<NAMEX>";					//
char* const g_Namex_End = "</NAMEX>";
char* const g_NE_Start = "<NE>";						//�������﷭���ѡ���NE��ʼ��־
char* const g_NE_End = "</NE>";							//�������﷭���ѡ���NE������־
char* const g_Normal_Start = "<NORMAL>";				//�������﷭���ѡ���һ�������ʼ��־
char* const g_Normal_End = "</NORMAL>";					//�������﷭���ѡ���һ����������־


//Confusion network ��־
char* const g_Epsilon_Word = "*DELETE*";
char* const g_Start_CN = "!SENT_START";
char* const g_END_CN = "!SENT_END";

//���ı�����
//const string g_Chinese_Punc_Set = "���������������������������ۣ�";
const string g_other_punc_set = ". , ; : \" ! ? ( ) < > [ ]";



const int g_DefaultNgramOrder = 3;      //Ĭ�ϵ�ngram order
const double LogP_PseudoZero = -99.0;			// non-inf value used for log 0
const double LogP_Zero = -HUGE_VAL;		    //log(0) 
const double LogP_Inf = HUGE_VAL;			// log(Inf) 
const double LogP_One = 0.0;			    // log(1) 
const int Vocab_None = (int)-1;  //��־�ʵ���Ĵ�


const int LINE_LEN_MAX = 256;                  //������ģ��ʱһ�е�����ַ���
const int MAX_NGRAM_ORDER = 9;               //����ģ�͵���� Order

typedef enum{XML, TXT, CN}FileFormate;
typedef enum{NOSEG, SEG, CHARACTER}SegFormate;
typedef enum{WORDS, SPACE}ParseWordType;
typedef enum{NULLWORD, SOURCEWORD}UnknownWord;
enum HEAPTYPE {MINHEAP, MAXHEAP};

