#include "db_flash.h"

S_Data_Map data_map;
S_Ram_Page ram_page;

void save_ram_page_to_flash(void)
{
	//save_flsh(ram_page.copy_page_number);
}
unsigned char load_flash_to_ram_page(unsigned char page_num)
{
	if(ram_page.is_this_data_effect&ram_page.is_this_data_change)
		{
			save_ram_page_to_flash();
		}
	//if(0 == read_flash(page_num))
		//{
			//return 2;
		//}
	ram_page.copy_page_number = page_num;
	ram_page.is_this_data_effect = 1;
}


unsigned short seek_data(P_S_Seek_Require req,P_S_Seek_Result res)
{
	unsigned short loopx;
	for(loopx = 0; loopx < data_map.save_num ;loopx++)
		{
			if(data_map.data[loopx].is_this_data_effect)	
				{
				
				}
			else
				{
					continue;
				}
		}
}

