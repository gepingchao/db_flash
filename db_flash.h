#ifndef __DB_FLASH__
#define __DB_FLASH__

#include "include.h"

/*******************************************
�������ݿ�
********************************************/
#define DB_START_PAGE	40
#define DB_CONFIG_PAGE_NUMBER	1
#define DB_DATA_PAGE_NUMBER	6
#define DB_DEFAULT_CELL_SIZE	20

//#define DB_CELL_SIZE	(sizeof())	
#ifndef DB_CELL_SIZE
#define DB_CELL_SIZE 	DB_DEFAULT_CELL_SIZE	
#endif

typedef enum
{
	db_type_null = 0,
	db_type_1,
	db_type_2,
	db_type_3,
	db_type_4,
	db_type_time,
	db_type_std_point,
	db_type_user_define_1,//��Ա���ֿ��Ը���ʵ�ʸ���
	db_type_user_define_2,
	db_type_user_define_3,
	db_type_posiation,
	db_type_user_define_5,
	db_type_user_define_6,
}E_Save_Data_Type;//��Ҫ����Ľṹ����Ҫ���������Ա


typedef struct
{
	E_Save_Data_Type data_type;
	unsigned char is_this_data_effect;
	unsigned char primary_key;//�����Ա���ֿ����Զ���ID��Ҫ�ô���������
////////////////////////////////���ϳ�Ա�Ǳ�����ڵ�

	unsigned char user_data_1[10];
	
	unsigned char user_data_3;
	unsigned short user_data_4;
	unsigned int user_data_5;
}S_DB_Demo,*P_S_DB_Demo;




typedef enum
{
	compare_equel = 0,
	compare_greater,
	compare_less,
	compare_not_equel,
}E_Compare_Reslut;

/**************************************

***************************************/
#define FLASH_START_ADDRESS	(0X08000000)
#define DB_TOTAL_SIZE		(DB_DATA_PAGE_NUMBER+DB_CONFIG_PAGE_NUMBER)
#define DB_CONFIG_PAGE		DB_START_PAGE
#define DB_DATA_START_PAGE	(DB_START_PAGE+DB_CONFIG_PAGE_NUMBER)
#define DB_DATA_PAGE(n)		(DB_DATA_START_PAGE + (n))
#define DB_DATA_END_PAGE	(DB_START_PAGE+DB_CONFIG_PAGE_NUMBER+DB_DATA_PAGE_NUMBER)
#define DB_PAGE_SIZE			FLASH_PAGE_SIZE
#define DB_CELL_NUM_PER_PAGE	(DB_PAGE_SIZE/DB_CELL_SIZE)


#define DB_CONFIG_PAGE_ADDRESS			(FLASH_START_ADDRESS + DB_CONFIG_PAGE*DB_PAGE_SIZE)
#define DB_DATA_START_PAGE_ADDRESS		(FLASH_START_ADDRESS + DB_DATA_START_PAGE*DB_PAGE_SIZE)
#define DB_DATA_PAGE_ADRESS(n)			(FLASH_START_ADDRESS + DB_DATA_PAGE(n)*DB_PAGE_SIZE)

#define offset_of(struct_type, name) 		(size_t)((char*)&((((struct_type) *)0)->name))




/*************************************
FLAGS ��־λ����
*************************************/
#define FLAG_THIS_DATA_EFFECT		((uint16_t)0x0001)  
#define FLAG_IS_THIS_DATA_CHANGED	((uint16_t)0x0002)  
#define FLAG_3	((uint16_t)0x0004)  
#define FLAG_4	((uint16_t)0x0008)  
#define FLAG_5	((uint16_t)0x0010)  
#define FLAG_6	((uint16_t)0x0020)  
#define FLAG_7	((uint16_t)0x0040)  
#define FLAG_8	((uint16_t)0x0080)  
#define FLAG_9	((uint16_t)0x0100)  
#define FLAG_10	((uint16_t)0x0200) 
#define FLAG_11	((uint16_t)0x0400)  
#define FLAG_12	((uint16_t)0x0800)  
#define FLAG_13	((uint16_t)0x1000)  
#define FLAG_14	((uint16_t)0x2000)  
#define FLAG_15	((uint16_t)0x4000)  
#define FLAG_16	((uint16_t)0x8000)  
#define FLAG_All	((uint16_t)0xFFFF)  

#define SET_FLAG_DISABLE(flags,flag_type)	((flags) &(~(flag_type)))
#define SET_FLAG_ENABLE(flags,flag_type)	((flags)|(flag_type))
#define CHECK_FLAG(flags,flag_type)			((flags)&(flag_type))


/******************************************
�ṹ��
******************************************/
typedef struct
{
	//unsigned char flags;
	unsigned char is_this_data_effect:1;
	unsigned char is_this_data_change:1;
	unsigned char is_this_data_deleted:1;
	//unsigned char index;
	//unsigned short offset;
}S_Data_Info,*P_S_Data_Info;
#define DATA_INFO_MAX_NUM	((FLASH_PAGE_SIZE - 200)/DB_DATA_PAGE_NUMBER)

typedef struct
{
	E_Save_Data_Type data_type[DB_DATA_PAGE_NUMBER];//ÿ��ҳ�������������
	unsigned short total_save_num[DB_DATA_PAGE_NUMBER];//ÿ��ҳ������ݱ������
	unsigned short save_num[DB_DATA_PAGE_NUMBER];//ÿ��ҳ��������ݸ���
	unsigned short delet_num[DB_DATA_PAGE_NUMBER];//ÿ��ҳɾ�������ݸ���
	unsigned short point[DB_DATA_PAGE_NUMBER];//ÿ��ҳ��ָ��
	unsigned char page_point[DB_DATA_PAGE_NUMBER];
	unsigned short cell_size[DB_DATA_PAGE_NUMBER];//ÿ������ҳ�Ĵ洢���С
	unsigned char this_data_effect;
	
	S_Data_Info data_info[DB_DATA_PAGE_NUMBER][DATA_INFO_MAX_NUM];
}S_Data_Map,*p_S_Data_Map;

typedef struct
{
	unsigned char is_this_data_effect:1;
	unsigned char is_this_data_change:1;
	unsigned char copy_page_number;
	unsigned char data[FLASH_PAGE_SIZE];
}S_Ram_Page,*P_S_Ram_Page;


typedef char(*compare_operat)(void* model,void* data,unsigned char compare_length);


typedef struct
{
	unsigned char page;
	unsigned short index;
}S_Posiation,*P_S_Posiation;
typedef struct
{
	unsigned char equal_count;//���ҵĵķ������������ݸ���
	unsigned char point;
	S_Posiation equal_id[20];//�������������ݵ�������ĵص� ҳ+ƫ����
	void* result[20];//�������������ݵ�flash�еĵ�ַ
}S_Seek_Result,*P_S_Seek_Result;

typedef struct
{
	E_Save_Data_Type data_type;//����/��ӵ���������
	unsigned short data_length;//save����ʱ��Ҫָ��Ҫ��������ݳ���
	unsigned char expect_seek_num;//�������ҵĸ���,�����ҵ��ĸ����ﵽ����ֵʱ����ֹͣ����

	E_Compare_Reslut compare_type;//�ȽϷ��� ���� С�� ����

	unsigned char effect_flags_length;//��Ч��־λ����
	unsigned char effect_flags_offset;//��Чλ����
	unsigned char effect_flags_value;//��Ч��־λ��ֵ

	unsigned char is_compare_member_is_string;//Ҫ�Ƚϵ����ַ�����
	unsigned char is_compare_member_signed;//Ҫ�Ƚϵ����з�������
	void* string_model;//�ַ���model
	long compare_value;//Ҫ�Ƚϵ�����
	
	unsigned char compare_member_length;//Ҫ�Ƚϵ���ֵ����
	unsigned short compare_member_offset;//Ҫ�Ƚϵ�����ƫ����
	unsigned short start_compare_offset;//��ʼ�Ƚϵĵ�ַƫ����

	////////////////////////////////////////////////////���ϳ�Ա�������������ֵ
	unsigned char primary_key_offset;//����ƫ�Ƶ�ַ
	unsigned short primary_key_value;//����ֵ
	compare_operat cmp_operat;
	
}S_Seek_Require,*P_S_Seek_Require;





/********************************************
ʹ�õ���ȫ�ֱ���
********************************************/
extern S_Data_Map data_map;
extern S_Ram_Page ram_page;




/********************************************
����
********************************************/

void set_mem(unsigned char* data,unsigned char value,unsigned short length);
void updata_data_map(void);

unsigned char set_cell_size(unsigned char page,unsigned short cell_size,unsigned char force);

unsigned char load_flash_to_ram_page(unsigned char page_num);
unsigned char save_ram_page_to_flash(void);




void reinit_data_map(void);//����
void init_data_map(void);//��ʼ��

unsigned char creat_database(E_Save_Data_Type data_type,unsigned short cell_size);//����
unsigned char seek_data(P_S_Seek_Require req,P_S_Seek_Result res);//��
unsigned char save_data(P_S_Seek_Require req,P_S_Seek_Result res,void* data);//��&��
unsigned char delete_data(P_S_Seek_Require req,P_S_Seek_Result res);//ɾ
unsigned char db_commit(void);//�ύ







void db_test(void);


#endif

