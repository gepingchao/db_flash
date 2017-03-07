STM32 M3 database frame
function:
seek
add
delete

功能说明:
seek ---> seek_data(&req, &res); req是查找条件结构体  res是查找结果结构体

req:
typedef struct
{
	E_Save_Data_Type data_type;//查找/添加的数据类型
	unsigned char expect_seek_num;//期望查找的个数,当查找到的个数达到期望值时立即停止查找

	E_Compare_Reslut compare_type;//比较方法 大于 小于 等于

	unsigned char effect_flags_length;//有效标志位长度
	unsigned char effect_flags_offset;//有效位长度
	unsigned char effect_flags_value;//有效标志位的值

	unsigned char is_compare_member_is_string;//要比较的是字符串吗
	unsigned char is_compare_member_signed;//要比较的是有符号数吗
	void* string_model;//字符串model
	long compare_value;//要比较的数据
	
	unsigned char compare_member_length;//要比较的数值长度
	unsigned short compare_member_offset;//要比较的数据偏移量
	unsigned short start_compare_offset;//开始比较的地址偏移量

	////////////////////////////////////////////////////以上成员必须是有意义的值
	
	unsigned char primary_key_offset;//主键偏移地址
	unsigned short primary_key_value;//主键值 主键默认长度2
	compare_operat cmp_operat;
	
}S_Seek_Require,*P_S_Seek_Require;


res:
typedef struct
{
	unsigned char page;//查找结果保存的页
	unsigned short index;//查找结果的偏移量
}S_Posiation,*P_S_Posiation;

typedef struct
{
	unsigned char equal_count;//查找的的符合条件的数据个数
	unsigned char point;
	S_Posiation equal_id[20];//符合条件的数据的所保存的地点 页+偏移量
	void* result[20];//符合条件的数据的flash中的地址
}S_Seek_Result,*P_S_Seek_Result;




save --->  save_data(&req,&demo);//add数据 req要保存的数据的要求信息(同查找中的req),要保存的数据地址
找出第一个符合条件的的数据,然后将数据demo插入,如果没有复合调价的数据则在最后面添加



