#pragma once
#include "vfo.h"
#include "gui_vfo.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class gui_speech
{
  public:
	void init(lv_obj_t *o_tab, lv_coord_t w);
	void set_ratio_range();
	void set_ratio_range(int min, int max);
	void set_ratio_slider(int ratio);
	void set_group();
	void set_speech_mode(int m);

	lv_obj_t *get_ratio_slider_label() { return ratio_slider_label; }

	int getbuttons() { return ibuttons; }
	lv_obj_t *get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return button[i];
	}

	lv_obj_t *get_threshold_slider_label()
	{
		return threshold_slider_label;
	}

	lv_obj_t *get_atack_slider_label()
	{
		return atack_slider_label;
	}

	lv_obj_t *get_release_slider_label()
	{
		return release_slider_label;
	}

	lv_obj_t *get_bass_slider_label()
	{
		return bass_slider_label;
	}
	
	lv_obj_t *get_treble_slider_label()
	{
		return treble_slider_label;
	}

	void set_threshold(int s)
	{
		a_threshold = s;
	}

	void set_atack(float f)
	{
		a_atack = f;
	}

	void set_release(float f)
	{
		a_release = f;
	}

	void set_ratio(int s)
	{
		a_ratio = s;
	}

	void set_treble(int s)
	{
		a_treble = s;
	}

	void set_bass(int s)
	{
		a_bass = s;
	}
	
	int get_threshold()
	{
		return a_threshold; // - max_threshold;
	}

	float get_atack()
	{
		return a_atack;
	}

	float get_release()
	{
		return a_release;
	}

	float get_ratio()
	{
		return a_ratio;
	}

	int get_treble()
	{
		return a_treble;
	}

	int get_bass()
	{
		return a_bass;
	}
	
	void set_atack_slider(int t);
	void set_threshold_slider(int t);
	void set_release_slider(int t);
	void set_bass_slider(int t);
	void set_treble_slider(int t);
	
	int get_speech_mode()
	{
		return speech_mode;
	}

	//const int	max_threshold {200};

  private:
	lv_style_t style_btn;
	lv_obj_t *ratio_slider_label, *ratio_slider;
	lv_obj_t *threshold_slider_label, *threshold_slider;
	lv_obj_t *atack_slider_label, *atack_slider;
	lv_obj_t *release_slider_label, *release_slider;
	lv_obj_t *bass_slider_label, *bass_slider;
	lv_obj_t *treble_slider_label, *treble_slider;
	
	lv_obj_t *button[10];
	int ibuttons{0};
	const int number_of_buttons{5};
	const int top_y{10};
	int speech_mode{0};
	lv_group_t *m_button_group{nullptr};
	atomic<int> a_threshold;
	atomic<float> a_atack;
	atomic<float> a_release;
	atomic<int> a_ratio;
	atomic<int> a_treble;
	atomic<int> a_bass;
};

extern gui_speech gspeech;