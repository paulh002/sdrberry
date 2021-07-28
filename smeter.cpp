// 
// 
// 
#include "smeter.h"

CSmeter::CSmeter(lv_obj_t* parent, lv_align_t align ,lv_coord_t width, lv_coord_t height, lv_coord_t ofs_x, lv_coord_t ofs_y)
{
	
	m_max_value = 0;
	lv_draw_line_dsc_init(&m_line_dsc);
	m_line_dsc.color = LV_COLOR_WHITE;

	m_height = height;
	m_width = width;

	m_cbuf = (lv_color_t*)malloc(LV_CANVAS_BUF_SIZE_TRUE_COLOR(m_width, m_height));

	s_canvas = lv_canvas_create(parent, NULL);
	lv_canvas_set_buffer(s_canvas, m_cbuf, m_width, m_height, LV_IMG_CF_TRUE_COLOR);
	lv_obj_align(s_canvas, parent, align, ofs_x, ofs_y);
	lv_canvas_fill_bg(s_canvas, LV_COLOR_BLACK, LV_OPA_COVER);
	
	lv_point_t	point[2] = { { 0,26 }, { m_width - 34, 26 } };
	lv_canvas_draw_line(s_canvas, point, 2, &m_line_dsc);
	
	// lv_point_t	point[2] = { { 0, m_height - 2 }, { m_width - 19, m_height - 2 } };
	
	point[0].x = 0;
	point[0].y = m_height - 2;
	point[1].x = m_width - 34;//19;
	point[1].y = m_height - 2;


	lv_canvas_draw_line(s_canvas, point, 2, &m_line_dsc);
	
	lv_draw_label_dsc_init(&m_label_dsc);
	m_label_dsc.color = LV_COLOR_YELLOW;

	for (int i = 0; i < 12; i++)
	{
		char str[32];

		point[0].x = i * 20;
		point[0].y = 26;
		point[1].x = i * 20;
		point[1].y = 15;
		lv_canvas_draw_line(s_canvas, point, 2, &m_line_dsc);
		if (i < 9)
		{
			if (i == 0)
				strcpy(str, "S");
			else
				sprintf(str, "%d", i + 1);
			lv_canvas_draw_text(s_canvas, point[0].x, 2, 100, &m_label_dsc, str, LV_LABEL_ALIGN_LEFT);
		}
		else
		{
			m_label_dsc.color = LV_COLOR_RED;
			if (i < 11)
				sprintf(str, "%d", 20 * (i - 8));
			else
				sprintf(str, "%d db", 20 * (i - 8));
			lv_canvas_draw_text(s_canvas, point[0].x - 7, 2, 100, &m_label_dsc, str, LV_LABEL_ALIGN_LEFT);
		}
	}
	Draw(4096);
}

CSmeter::~CSmeter()
{
	free(m_cbuf);
}

void CSmeter::hide(bool hidden)
{
	lv_obj_set_hidden(s_canvas, hidden);
}

void CSmeter::Draw(uint16_t value)
{
	uint16_t max_value;

	// 4096 is max value of AD converter 12 bits
	max_value = (value * 22) / 4096;
	if (max_value == m_max_value)
		return;
	m_max_value = max_value;
	lv_draw_rect_dsc_init(&m_rect_dsc);
	m_rect_dsc.bg_color = LV_COLOR_GREEN;

	m_line_dsc.color = LV_COLOR_GREEN;
	for (uint16_t i = 0; i < 22; i++)  // max = 22
	{
	if (i < 16)
		m_rect_dsc.bg_color = LV_COLOR_GREEN;
	else
		m_rect_dsc.bg_color = LV_COLOR_RED;
	if ( i >= max_value)
		m_rect_dsc.bg_color = LV_COLOR_BLACK;
	lv_canvas_draw_rect(s_canvas, i * 10, 28, 8, m_height - 2 - 28, &m_rect_dsc);
	}

}
