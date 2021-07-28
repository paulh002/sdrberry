// smeter.h

#ifndef _smeter_h
#define _smeter_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class CSmeter
{
public:
	CSmeter(lv_obj_t* parent, lv_align_t align, lv_coord_t width, lv_coord_t height, lv_coord_t ofs_x, lv_coord_t ofs_y);
	~CSmeter();
	void Draw(uint16_t value);
	void hide(bool hidden);

private:
	uint16_t	m_max_value;
	lv_obj_t	*s_canvas;
	lv_coord_t  m_height, m_width;
	lv_color_t* m_cbuf;
	lv_draw_line_dsc_t	m_line_dsc;
	lv_draw_label_dsc_t m_label_dsc;
	lv_draw_rect_dsc_t  m_rect_dsc;
};

#endif

