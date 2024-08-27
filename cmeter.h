#pragma once
#include "lvgl_.h"

template <typename Class, void (Class::*MemberFuncMeterEvent)(const lv_obj_class_t *, lv_event_t *)>
struct EventHandlerMeter
{
	static void staticHandler(const lv_obj_class_t *c, lv_event_t *e)
	{
		(static_cast<Class *>(e->user_data)->*MemberFuncMeterEvent)(c,e);
	}
};

class lv_meter_scale_c
{
  public:
	lv_color_t tick_color;
	uint16_t tick_cnt;
	uint16_t tick_length;
	uint16_t tick_width;

	lv_color_t tick_major_color;
	uint16_t tick_major_nth;
	uint16_t tick_major_length;
	uint16_t tick_major_width;

	int16_t label_gap;

	int32_t min;
	int32_t max;
	int16_t r_mod;
	uint16_t angle_range;
	int16_t rotation;
	int16_t c_mod;
} ;

enum
{
	LV_CMETER_INDICATOR_TYPE_NEEDLE_IMG,
	LV_CMETER_INDICATOR_TYPE_NEEDLE_LINE,
	LV_CMETER_INDICATOR_TYPE_SCALE_LINES,
	LV_CMETER_INDICATOR_TYPE_ARC,
};
typedef uint8_t lv_meter_indicator_cype_c;

class lv_meter_indicator_c
{
  public:
	lv_meter_scale_c *scale;
	lv_meter_indicator_cype_c type;
	lv_opa_t opa;
	int32_t start_value;
	int32_t end_value;
	union
	{
		struct
		{
			const void *src;
			lv_point_t pivot;
		} needle_img;
		struct
		{
			uint16_t width;
			int16_t r_mod;
			lv_color_t color;
		} needle_line;
		struct
		{
			uint16_t width;
			const void *src;
			lv_color_t color;
			int16_t r_mod;
			int16_t c_mod;
		} arc;
		struct
		{
			int16_t width_mod;
			lv_color_t color_start;
			lv_color_t color_end;
			uint8_t local_grad : 1;
		} scale_lines;
	} type_data;
} ;

/*Data of line meter*/
class lv_meter_c
{
  public:
	lv_obj_t obj;
	lv_ll_t scale_ll;
	lv_ll_t indicator_ll;
};

typedef enum
{
	LV_CMETER_DRAW_PART_ARC,			/**< The arc indicator*/
	LV_CMETER_DRAW_PART_NEEDLE_LINE, /**< The needle lines*/
	LV_CMETER_DRAW_PART_NEEDLE_IMG,  /**< The needle images*/
	LV_CMETER_DRAW_PART_TICK,		/**< The tick lines and labels*/
} lv_meter_draw_part_type_c;

class cmeter
{
  private:
	lv_obj_t *parent;
	lv_obj_t *meterObject;
	lv_style_t meter_style;
	lv_obj_t *lv_cmeter_create(lv_obj_t *parent);

  public:
	static void lv_cmeter_constructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
	static void lv_cmeter_destructor(const lv_obj_class_t *class_p, lv_obj_t *obj);
	static void lv_cmeter_event(const lv_obj_class_t *class_p, lv_event_t *e);
	static void draw_arcs(lv_obj_t *obj, lv_draw_ctx_t *draw_ctx, const lv_area_t *scale_area);
	static void draw_ticks_and_labels(lv_obj_t *obj, lv_draw_ctx_t *draw_ctx, const lv_area_t *scale_area);
	static void draw_needles(lv_obj_t *obj, lv_draw_ctx_t *draw_ctx, const lv_area_t *scale_area);
	static void inv_arc(lv_obj_t *obj, lv_meter_indicator_c *indic, int32_t old_value, int32_t new_value);
	static void inv_line(lv_obj_t *obj, lv_meter_indicator_c *indic, int32_t value);

	cmeter(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	~cmeter();

	/*=====================
 * Add scale
 *====================*/

	/**
 * Add a new scale to the meter.
 * @param obj   pointer to a meter object
 * @param r_mod the radius modifier (added to the scale's radius) to get the lines length
 * @param c_mod         the center modifier (added to the scale's y size) to get the center of the arc outside of the object box
 * @return      the new scale
 * @note        Indicators can be attached to scales.
 */

	lv_meter_scale_c *lv_meter_add_scale(int16_t r_mod, int16_t c_mod);

	/**
 * Set the properties of the ticks of a scale
 * @param obj       pointer to a meter object
 * @param scale     pointer to scale (added to `meter`)
 * @param cnt       number of tick lines
 * @param width     width of tick lines
 * @param len       length of tick lines
 * @param color     color of tick lines
 */
	void lv_meter_set_scale_ticks(lv_meter_scale_c *scale, uint16_t cnt, uint16_t width, uint16_t len,
								  lv_color_t color);

	/**
 * Make some "normal" ticks major ticks and set their attributes.
 * Texts with the current value are also added to the major ticks.
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param nth           make every Nth normal tick major tick. (start from the first on the left)
 * @param width         width of the major ticks
 * @param len           length of the major ticks
 * @param color         color of the major ticks
 * @param label_gap     gap between the major ticks and the labels
 */
	void lv_meter_set_scale_major_ticks(lv_meter_scale_c *scale, uint16_t nth, uint16_t width,
										uint16_t len, lv_color_t color, int16_t label_gap);

	/**
 * Set the value and angular range of a scale.
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param min           the minimum value
 * @param max           the maximal value
 * @param angle_range   the angular range of the scale
 * @param rotation      the angular offset from the 3 o'clock position (clock-wise)
 */
	void lv_meter_set_scale_range(lv_meter_scale_c *scale, int32_t min, int32_t max, uint32_t angle_range,
								  uint32_t rotation);

	/*=====================
 * Add indicator
 *====================*/

	/**
 * Add a needle line indicator the scale
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param width         width of the line
 * @param color         color of the line
 * @param r_mod         the radius modifier (added to the scale's radius) to get the lines length
 * @return              the new indicator
 */
	lv_meter_indicator_c *lv_meter_add_needle_line(lv_meter_scale_c *scale, uint16_t width,
												   lv_color_t color, int16_t r_mod);

	/**
 * Add a needle image indicator the scale
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param src           the image source of the indicator. path or pointer to ::lv_img_dsc_t
 * @param pivot_x       the X pivot point of the needle
 * @param pivot_y       the Y pivot point of the needle
 * @return              the new indicator
 * @note                the needle image should point to the right, like -O----->
 */
	lv_meter_indicator_c *lv_meter_add_needle_img(lv_meter_scale_c *scale, const void *src,
												  lv_coord_t pivot_x, lv_coord_t pivot_y);

	/**
 * Add an arc indicator the scale
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param width         width of the arc
 * @param color         color of the arc
 * @param r_mod         the radius modifier (added to the scale's radius) to get the outer radius of the arc
 * @param c_mod         the center modifier (added to the scale's y size) to get the center of the arc outside of the object box
 * @return              the new indicator
 */
	lv_meter_indicator_c *lv_meter_add_arc(lv_meter_scale_c *scale, uint16_t width, lv_color_t color,
										   int16_t r_mod, int16_t c_mod);

	/**
 * Add a scale line indicator the scale. It will modify the ticks.
 * @param obj           pointer to a meter object
 * @param scale         pointer to scale (added to `meter`)
 * @param color_start   the start color
 * @param color_end     the end color
 * @param local         tell how to map start and end color. true: the indicator's start and end_value; false: the scale's min max value
 * @param width_mod     add this the affected tick's width
 * @return              the new indicator
 */
	lv_meter_indicator_c *lv_meter_add_scale_lines(lv_meter_scale_c *scale, lv_color_t color_start,
												   lv_color_t color_end, bool local, int16_t width_mod);

/*=====================
 * Set indicator value
 *====================*/

	/**
 * Set the value of the indicator. It will set start and and value to the same value
 * @param obj           pointer to a meter object
 * @param indic         pointer to an indicator
 * @param value         the new value
 */
	void lv_meter_set_indicator_value(lv_meter_indicator_c *indic, int32_t value);

	/**
 * Set the start value of the indicator.
 * @param obj           pointer to a meter object
 * @param indic         pointer to an indicator
 * @param value         the new value
 */
	void lv_meter_set_indicator_start_value(lv_meter_indicator_c *indic, int32_t value);

	/**
 * Set the start value of the indicator.
 * @param obj           pointer to a meter object
 * @param indic         pointer to an indicator
 * @param value         the new value
 */
	void lv_meter_set_indicator_end_value(lv_meter_indicator_c *indic, int32_t value);

	struct _lv_event_dsc_t *lv_obj_add_event_cb(lv_event_cb_t event_cb, lv_event_code_t filter, void *user_data);
};

