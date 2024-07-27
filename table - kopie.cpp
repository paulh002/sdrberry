#include<stdlib.h>
#include "table.h"

void lv_table_remove_row_cnt(lv_obj_t *obj, uint16_t start_row, uint16_t rows)
{
	LV_ASSERT_OBJ(obj, MY_CLASS);

	lv_table_t *table = (lv_table_t *)obj;

	if (rows == 0 || (table->row_cnt - start_row) < rows)
		return;

	uint16_t old_row_cnt = table->row_cnt;
	table->row_cnt = table->row_cnt - rows;
	lv_coord_t *old_row_h = table->row_h;
	
	table->row_h = (lv_coord_t *)lv_mem_alloc(table->row_cnt * sizeof(table->row_h[0]));
	lv_memset_00(table->row_h,table->row_cnt * sizeof(table->row_h[0]));
	LV_ASSERT_MALLOC(table->row_h);
	if (table->row_h == NULL)
	{
		LV_LOG_WARN("lv_mem_realloc failed");
		return;
	}
	lv_memcpy((void *)&table->row_h, (void *)old_row_h, start_row * sizeof(table->row_h[0]));
	lv_memcpy((void *)&table->row_h[start_row], (void *)&old_row_h[rows + start_row], (table->row_cnt - start_row) * sizeof(table->row_h[0]));
	free(old_row_h);	/*Free the unused cells*/
	if (rows)
	{
		uint32_t start_cell_cnt = table->col_cnt * start_row;
		uint32_t new_cell_cnt = table->col_cnt * (rows + start_row);
		for (uint32_t i = start_cell_cnt; i < new_cell_cnt; i++)
		{
#if LV_USE_USER_DATA
			if (table->cell_data[i]->user_data)
			{
				lv_mem_free(table->cell_data[i]->user_data);
				table->cell_data[i]->user_data = NULL;
			}
#endif
			lv_mem_free(table->cell_data[i]);
		}
	}

	lv_table_cell_t **old_cell_data = table->cell_data;

	table->cell_data = (lv_table_cell_t **)lv_mem_alloc(table->row_cnt * table->col_cnt * sizeof(lv_table_cell_t *));
	lv_memset_00(table->cell_data, table->row_cnt * table->col_cnt * sizeof(lv_table_cell_t *));
	LV_ASSERT_MALLOC(table->cell_data);
	if (table->cell_data == NULL)
	{
		LV_LOG_WARN("lv_mem_realloc failed");
		return;
	}
	lv_memcpy((void *)table->cell_data, old_cell_data, start_row * table->col_cnt * sizeof(lv_table_cell_t *));
	lv_memcpy((void *)&table->cell_data[start_row], &old_cell_data[(rows + start_row) * table->col_cnt], (table->row_cnt - start_row) * table->col_cnt * sizeof(lv_table_cell_t *));
	lv_mem_free(old_cell_data);
	lv_obj_refresh_self_size(obj);
	lv_obj_invalidate(obj);
}