#include "include.h"
#include "db_flash.h"
S_Data_Map data_map;
S_Ram_Page ram_page;

void set_mem(unsigned char* data,unsigned char value,unsigned short length)//����һ���ڴ�ֵΪvalue
{
	unsigned short tmp_length = length;
	unsigned char* tmp_data = data;
	while(tmp_length --)
		{
			*(tmp_data ++) = value;
		}
}

void updata_data_map(void)//��data_mapд��flash
{
	write_flash(DB_CONFIG_PAGE_ADDRESS,(char*)&data_map,sizeof(data_map));
}

void init_data_map(void)//��ʼ��data_map ,data_map���ڲ������ݵ�
{
	read_flash(DB_CONFIG_PAGE_ADDRESS,(char*)&data_map,sizeof(data_map));
	if(data_map.this_data_effect != EFFECT)
		{
			set_mem((unsigned char*)&data_map,0,sizeof(data_map));
			data_map.this_data_effect = EFFECT;
			updata_data_map();
		}
}
	



unsigned char set_cell_size(unsigned char page,unsigned short cell_size,unsigned char force)//����ҳ��ÿ�����ݿ��С
{
	if(page > DB_DATA_PAGE_NUMBER)
		{
			return 2;
		}
	if((data_map.cell_size[page]) && (0 == force))//����ǿ�Ƹ���
		{
			return 3;
		}
	data_map.cell_size[page] = cell_size;
	return 1;
}

unsigned char is_page_full(unsigned char page)//���ָ����ҳ�Ƿ����
{
	if(0 == data_map.cell_size[page])
		{
			return 2;//δ��ʼ��
		}
	if(data_map.save_num[page] < (DB_PAGE_SIZE / data_map.cell_size[page]))
		{
			return 1;//ҳδ��
		}
	return 0;//ҳ��
}

unsigned char creat_database(E_Save_Data_Type data_type,unsigned short cell_size)//����һ�����ݱ�
{
	unsigned char loopx;
	for(loopx =0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if((data_map.data_type[loopx] == data_type)&&(1 == is_page_full(loopx)))
				{
					return 0;//�������͵ı��Ѿ�������,���Ҳ���
				}
		}
	for(loopx =0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if(data_map.data_type[loopx] == db_type_null)
				{
					data_map.data_type[loopx] = data_type;
					data_map.cell_size[loopx] =cell_size;
					data_map.page_point[0] = loopx;
					return 1;//������һ���µı�
				}
		}
	return 2;//���������������ݱ�������������
}

unsigned short cell_size_in_page(unsigned char page)//�ҳ�ָ��ҳ�����ݿ��С
{
	return data_map.cell_size[page];
}

unsigned char dummy_address_write(unsigned int address,char* data,unsigned short len)//�����ַд����д��ram��
{
	if((DB_DATA_PAGE_ADRESS(ram_page.copy_page_number+1) - address) > DB_PAGE_SIZE)
		{
			return 0;//Ҫд�ĵ�ַ����ramҳ��Χ��
		}
	if((DB_DATA_PAGE_ADRESS(ram_page.copy_page_number+1) - address) < len)
		{
			return 2;//Ҫд�����ݽ������
		}
	unsigned short tmp_len = len;
	unsigned int offset = address - DB_DATA_PAGE_ADRESS(ram_page.copy_page_number);
	char* tmp_data = data;
	unsigned char* write_address = (ram_page.data + offset);
	while(tmp_len)
		{
			*(write_address ++) = *(tmp_data ++);
			tmp_len --;
		}
	ram_page.is_this_data_change = 1;
	return 1;
}

unsigned char dummy_address_read(unsigned address,char* data,unsigned short len)
{
  return 1;
}


unsigned char save_ram_page_to_flash(void)//��ram������������д��flash
{
	unsigned char reslut;
	reslut = write_flash(DB_DATA_PAGE_ADRESS(ram_page.copy_page_number),(char*)(ram_page.data),DB_PAGE_SIZE);
	if(reslut)
		{
			ram_page.is_this_data_change = 0;
		}
	return reslut;
}
unsigned char read_flash_page_to_ram(void)//��flash�洢���������뵽ram��
{
	unsigned char reslut;
	reslut = read_flash(DB_DATA_PAGE_ADRESS(ram_page.copy_page_number),(char*)(ram_page.data),DB_PAGE_SIZE);
	if(reslut)
		{
			ram_page.is_this_data_change = 0;
		}
	return reslut;
}


	
unsigned char load_flash_to_ram_page(unsigned char page_num)//��ָ��ҳ�����ݴ洢��ram��
{
	if((ram_page.is_this_data_effect)&&(ram_page.is_this_data_change)&&(ram_page.copy_page_number != page_num))
		{
			save_ram_page_to_flash();
			read_flash_page_to_ram();
		}
	if(ram_page.copy_page_number == page_num)
		{
			return 2;
		}
	read_flash_page_to_ram();
	ram_page.copy_page_number = page_num;
	ram_page.is_this_data_effect = 1;
	return 1;
}

unsigned char find_data_type(E_Save_Data_Type data_type)//�ҳ�ָ���������͵ı���ҳ
{
	unsigned char loopx;
	unsigned char count = 0;
	for(loopx = 0 ; loopx < DB_DATA_PAGE_NUMBER ; loopx ++)
		{
			if(data_map.data_type[loopx] == data_type)
				{
					data_map.page_point[count] = data_type;
					count ++;
				}
		}
	return count;
}

unsigned char is_data_effect(unsigned char page,unsigned index)//�ж�ҳ��ָ�����ݿ��Ƿ���Ч
{
	if(data_map.data_info[page][index].is_this_data_effect)
		{
			return 1;
		}
	return 0;
}
unsigned char is_data_delet(unsigned char page,unsigned index)//�ж�ҳ��ָ�����ݿ��Ƿ�ɾ����
{
	if(data_map.data_info[page][index].is_this_data_deleted)
		{
			return 1;
		}
	return 0;
}

unsigned short data_save_num_in_page(unsigned char page)//ָ��ҳ�����ݿ�ı�������ʵ���������ܻ�С�ڷ���ֵ,���ֵ���Ǳ�����������������ֵ
{
	return data_map.save_num[page];
}

unsigned short data_delet_num_in_page(unsigned char page)//ָ��ҳ�����ݿ��Ѿ�ɾ���ĸ���
{
	return data_map.delet_num[page];
}

void get_value(void* address,unsigned char data_length,long* value)//��ȡָ����ַ�ϵ�ֵ���з�������ֵ
{
	
		switch(data_length)
		{
			case 1:
				*value = *(char*)address;
				break;

			case 2:
				*value = *(short*)address;
				break;

			case 4:
				*value = *(int*)address;
				break;

			case 8:
				*value = *(long*)address;
				break;
		}
}

void get_unsigned_value(void* address,unsigned char data_length,long* value)//��ȡָ����ַ�ϵ�ֵ���޷�������ֵ
{
	
		switch(data_length)
		{
			case 1:
				*value = *(unsigned char*)address;
				break;

			case 2:
				*value = *(unsigned short*)address;
				break;

			case 4:
				*value = *(unsigned int*)address;
				break;

			case 8:
				*value = *(long*)address;
				break;
		}
}

E_Compare_Reslut math_compare (long model_value,void* compare_address,unsigned char data_length,unsigned char is_data_signed)//�Ƚ�ָ����ַ�ϵ�����model�Ĵ�С��ϵ
{
	long compare_value = 0;
	if(1 == is_data_signed)
		{
			get_value(compare_address,data_length,&compare_value);
		}
	else
		{
			get_unsigned_value(compare_address,data_length,&compare_value);
		}
	if(model_value < compare_value)
		{
			return compare_greater;
		}
	if(model_value == compare_value)
		{
			return compare_equel;
		}
	if(model_value > compare_value)
		{
			return compare_less;
		}
	return compare_not_equel;
}


E_Compare_Reslut string_compare(unsigned char* model,unsigned char* compare_str,unsigned char compare_str_length)//���ֽڱȽ�ָ����ַ��model�Ĺ�ϵ
{
	unsigned char loopx = 0;
	loopx = compare_str_length;
	unsigned char* tmp_model = model;
	unsigned char* tmp_compare_str = compare_str;
	while(loopx)
		{
			if(*tmp_model == *tmp_compare_str)
				{
					tmp_model ++;
					tmp_compare_str ++;
				}
			else
				{
					return compare_not_equel;
				}
			loopx --;
		}
	return compare_equel;
}

unsigned char compare_data(void* compare,P_S_Seek_Require req,P_S_Seek_Result res)//�Ƚ�
{
	long effect_value = 0;
	long compare_value = req->compare_value;
	get_unsigned_value((void*)((int)compare+(req->effect_flags_offset)),(req->effect_flags_length),&effect_value);
	if((req->effect_flags_value) != effect_value)
		{
			return 0;
		}

	if(1 == req->is_compare_member_is_string)
		{
			if((req->compare_type) == string_compare((unsigned char*)(req->string_model),(unsigned char*)((int)compare+(req->compare_member_offset)),(req->compare_member_length)))
				{
					if(res != NULL)
						{
							res->equal_count ++;
							res->result[res->point] = compare;
							res->point ++;
						}
					return 1;
				}
			else
				{
					return 0;
				}
		}	

	if((req->compare_type) == math_compare(compare_value,(void*)((int)compare+(req->compare_member_offset)),(req->compare_member_length),(req->is_compare_member_signed)))
		{
			if(res != NULL)
				{
					res->equal_count ++;
					res->result[res->point] = compare;
					res->point ++;
				}
			return 1;
		}
	else
		{
			return 0;
		}
	
}

unsigned char find_data_in_page(unsigned char page,P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned char seek_count;
	unsigned short loopx;
	unsigned short tmp_save_num;
	void* compare_ddress;
	seek_count = req->expect_seek_num;
	while(seek_count)
		{
			tmp_save_num = data_save_num_in_page(page);
			for(loopx = 0 ; loopx < tmp_save_num ; loopx ++)
				{
					if(is_data_effect(page,loopx))
						{
							compare_ddress = (void*)(DB_DATA_PAGE_ADRESS(page) + (req->compare_member_offset + loopx)*(data_map.cell_size[page]));
							if(1 ==compare_data(compare_ddress,req,res))
								{
									seek_count --;
									if(0 == seek_count)
										{
											return 2;//�ҵ��ķ������������ݴﵽ��������
										}
								}
             
						}
					else
						{
							continue;
						}
				}
			return 0;//δ�ҵ���������������
		}
}


unsigned char seek_data(P_S_Seek_Require req,P_S_Seek_Result res)//seek
{
	unsigned char loopx;
	loopx = find_data_type(req->data_type);
	while(loopx)
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,res))
				{
					break;
				}
			loopx --;
		}
	return 1;
}

void insert_data_to_page(unsigned char page)
{	
	load_flash_to_ram_page(page);
}

unsigned char save_data(P_S_Seek_Require req,void* data)
{
	unsigned char page_num,loopx;
	S_Seek_Result res;
	req->expect_seek_num =1;
	page_num = find_data_type(req->data_type);
	loopx = page_num;
	while(loopx)
		{
			if(2 == find_data_in_page(data_map.page_point[loopx],req,&res))
				{
					load_flash_to_ram_page(loopx);//�����ҵĵ���������ҳ�����ڴ�
					dummy_address_write((unsigned int)(res.result[0]),data,data_map.cell_size[loopx]);
				}
			loopx --;
		}
	for(loopx = 0 ; loopx < page_num ; loopx++)
		{
			
		}
	return 1;
}












/*

���Ժ���
*/



void db_test(void)
{
	S_DB_Demo demo;
	demo.data_type = db_type_user_define_1;
	demo.is_this_data_effect = EFFECT;
	demo.primary_key = 1;
	demo.user_data_3 = 0x88;
	init_data_map();
	creat_database(demo.data_type,sizeof(demo));

	S_Seek_Require req;
	req.data_type = demo.data_type;
	req.effect_flags_length = 1;
	req.compare_member_offset = offsetof(S_DB_Demo,is_this_data_effect);
	req.compare_value = EFFECT;
	req.compare_member_offset = offsetof(S_DB_Demo,primary_key);
	req.compare_type = compare_equel;
	save_data(&req,&demo);
}


























