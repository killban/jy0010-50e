#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"


#define MY_UUID { 0x6A, 0x9D, 0x93, 0x6D, 0xE6, 0xB1, 0x4C, 0xE6, 0x80, 0x67, 0x98, 0x6A, 0x21, 0xCD, 0xAF, 0x9C }
PBL_APP_INFO(MY_UUID,
             "JY0010-50E", "killban",
             0, 4, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

#define DISPLAY_SECONDS true
#define HOUR_VIBRATION true
#define HOUR_VIBRATION_START 8
#define HOUR_VIBRATION_END 20

Window window;


BmpContainer background_image_container;

Layer minute_display_layer;
Layer hour_display_layer;
Layer center_display_layer;
#if DISPLAY_SECONDS
Layer second_display_layer;
#endif

TextLayer local_layer;
GFont local_font;
static char local_text[] = "LON";

TextLayer date_layer;
GFont date_font;
static char date_text[] = "Sat 13";

const GPathInfo MINUTE_HAND_PATH_POINTS = {
  8,
  (GPoint []) {
    {0, 9},
    {2, 6},
    {4, 0},
    {2, -50},
    {0, -61},
    {-2, -50},
    {-4, 0},
    {-2, 6},
  }
};

const GPathInfo MINUTE_HAND_OUTLINE_PATH_POINTS = {
  8,
  (GPoint []) {
    {0, 10},
    {3, 7}, 
    {5, 0},
    {3, -51},
    {0, -62},
    {-3, -51},
    {-5, 0},
    {-3, 7},
  }
};

const GPathInfo HOUR_HAND_PATH_POINTS = {
  8,
  (GPoint []) {
    {0, 10},
    {3, 7},
    {5, 0},
    {3, -27},
    {0, -37},
    {-3, -27},
    {-5, 0},
    {-3, 7},
  }
};

const GPathInfo HOUR_HAND_OUTLINE_PATH_POINTS = {
  4,
  (GPoint []) {
    {0, 10},
    {3, 7},
    {5, 0},
    {3, -27},
    {0, -37},
    {-3, -27},
    {-5, 0},
    {-3, 7},
  }
};

const GPathInfo AMPM_HAND_PATH_POINTS = {
  4,
  (GPoint []) {
    {-1, 0},
    {1, 0},
    {1, -10},
    {-1,  -10},
  }
};

const GPathInfo AMPM_HAND_OUTLINE_PATH_POINTS = {
  4,
  (GPoint []) {
    {-2, 11},
    {2, 11},
    {2, -11},
    {-2,  -11},
  }
};

GPath ampm_hand_path;
GPath ampm_hand_outline_path;
GPath hour_hand_path;
GPath hour_hand_outline_path;
GPath minute_hand_path;
GPath minute_hand_outline_path;

#if DISPLAY_SECONDS
void second_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;
  get_time(&t);

  int32_t second_angle = t.tm_sec * (0xffff/60);
  int32_t counter_second_angle = t.tm_sec * (0xffff/60);
  if(t.tm_sec<30)
  {
     counter_second_angle += 0xffff/2;
  }
  else
  {
     counter_second_angle -= 0xffff/2;
  }
  int second_hand_length = 70;
  int counter_second_hand_length = 15;

  graphics_context_set_fill_color(ctx, GColorWhite);

  GPoint center = grect_center_point(&me->frame);
  GPoint counter_second = GPoint(center.x + counter_second_hand_length * sin_lookup(counter_second_angle)/0xffff,
				center.y + (-counter_second_hand_length) * cos_lookup(counter_second_angle)/0xffff);
  GPoint second = GPoint(center.x + second_hand_length * sin_lookup(second_angle)/0xffff,
				center.y + (-second_hand_length) * cos_lookup(second_angle)/0xffff);

  graphics_draw_line(ctx, counter_second, second);
}
#endif

void center_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  GPoint center = grect_center_point(&me->frame);
  //graphics_context_set_fill_color(ctx, GColorBlack);
  //graphics_fill_circle(ctx, center, 5);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_circle(ctx, center, 4);
}

void minute_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = t.tm_min * 6 + t.tm_sec / 15;
  gpath_rotate_to(&minute_hand_path, (TRIG_MAX_ANGLE / 360) * angle);
  gpath_rotate_to(&minute_hand_outline_path, (TRIG_MAX_ANGLE / 360) * angle);
  
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, &minute_hand_outline_path);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, &minute_hand_path);
}

void hour_display_layer_update_callback(Layer *me, GContext* ctx) {
  (void)me;

  PblTm t;

  get_time(&t);

  unsigned int angle = t.tm_hour * 30 + t.tm_min / 2;
  gpath_rotate_to(&hour_hand_path, (TRIG_MAX_ANGLE / 360) * angle);
  gpath_rotate_to(&hour_hand_outline_path, (TRIG_MAX_ANGLE / 360) * angle);

  graphics_context_set_fill_color(ctx, GColorBlack);
  gpath_draw_filled(ctx, &hour_hand_outline_path);
  graphics_context_set_fill_color(ctx, GColorWhite);
  gpath_draw_filled(ctx, &hour_hand_path);
}

void draw_date(){
  PblTm t;
  get_time(&t);
  
  string_format_time(date_text, sizeof(date_text), "%d", &t);
  text_layer_set_text(&date_layer, date_text);
}

void draw_local(){

  text_layer_set_text(&local_layer, local_text);

}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Citizen Watch");
  window_stack_push(&window, true /* Animated */);

  resource_init_current_app(&APP_RESOURCES);


  bmp_init_container(RESOURCE_ID_IMAGE_BACKGROUND, &background_image_container);

  layer_add_child(&window.layer, &background_image_container.layer.layer);

  date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPENSANS_REGULAR_14));
  text_layer_init(&date_layer, GRect(55, 110, 34, 20));
  text_layer_set_text_alignment(&date_layer, GTextAlignmentCenter);
  text_layer_set_text_color(&date_layer, GColorWhite);
  text_layer_set_background_color(&date_layer, GColorClear);
  text_layer_set_font(&date_layer, date_font);
  layer_add_child(&window.layer, &date_layer.layer);
  draw_date();

  local_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_OPENSANS_REGULAR_14));
  text_layer_init(&local_layer, GRect(21, 83, 28, 13));
  text_layer_set_text_alignment(&local_layer, GTextAlignmentCenter);
  text_layer_set_text_color(&local_layer, GColorBlack);
  text_layer_set_background_color(&local_layer, GColorClear);
  text_layer_set_font(&local_layer, local_font);
  layer_add_child(&window.layer, &local_layer.layer);
  draw_local();

  layer_init(&hour_display_layer, window.layer.frame);
  hour_display_layer.update_proc = &hour_display_layer_update_callback;
  layer_add_child(&window.layer, &hour_display_layer);

  gpath_init(&hour_hand_outline_path, &HOUR_HAND_OUTLINE_PATH_POINTS);
  gpath_move_to(&hour_hand_outline_path, grect_center_point(&hour_display_layer.frame));
  gpath_init(&hour_hand_path, &HOUR_HAND_PATH_POINTS);
  gpath_move_to(&hour_hand_path, grect_center_point(&hour_display_layer.frame));

  layer_init(&minute_display_layer, window.layer.frame);
  minute_display_layer.update_proc = &minute_display_layer_update_callback;
  layer_add_child(&window.layer, &minute_display_layer);

  gpath_init(&minute_hand_outline_path, &MINUTE_HAND_OUTLINE_PATH_POINTS);
  gpath_move_to(&minute_hand_outline_path, grect_center_point(&minute_display_layer.frame));
  gpath_init(&minute_hand_path, &MINUTE_HAND_PATH_POINTS);
  gpath_move_to(&minute_hand_path, grect_center_point(&minute_display_layer.frame));

  layer_init(&center_display_layer, window.layer.frame);
  center_display_layer.update_proc = &center_display_layer_update_callback;
  layer_add_child(&window.layer, &center_display_layer);

#if DISPLAY_SECONDS
  layer_init(&second_display_layer, window.layer.frame);
  second_display_layer.update_proc = &second_display_layer_update_callback;
  layer_add_child(&window.layer, &second_display_layer);
#endif
}


void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&background_image_container);

  fonts_unload_custom_font(date_font);

  fonts_unload_custom_font(local_font);
}



void handle_tick(AppContextRef ctx, PebbleTickEvent *t){
  (void)t;
  (void)ctx;
/*
#if DISPLAY_SECONDS
  layer_mark_dirty(&second_display_layer);
#endif

  if(t->tick_time->tm_sec%10==0)
  {
     if(t->tick_time->tm_min%2==0)
     {
        layer_mark_dirty(&hour_display_layer);
     }
     layer_mark_dirty(&minute_display_layer);
  }

  if(t->tick_time->tm_min==0&&t->tick_time->tm_hour==0)
  {
     draw_date();
     draw_local();
  }
*/


  if(t->tick_time->tm_sec%15==0)
  {
     layer_mark_dirty(&minute_display_layer);
     
     if(t->tick_time->tm_sec==0)
     {
        if(t->tick_time->tm_min%2==0)
        {
           layer_mark_dirty(&hour_display_layer);
           if(t->tick_time->tm_min==0&&t->tick_time->tm_hour==0)
           {
              draw_date();
              draw_local();
           }
#if HOUR_VIBRATION
           if(t->tick_time->tm_min==0 &&
                 t->tick_time->tm_hour>=HOUR_VIBRATION_START &&
                    t->tick_time->tm_hour<=HOUR_VIBRATION_END)
           {
              vibes_double_pulse();
           }
#endif
        }
     }
  }

#if DISPLAY_SECONDS
  layer_mark_dirty(&second_display_layer);
#endif
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
			.tick_handler = &handle_tick,
#if DISPLAY_SECONDS
			.tick_units = SECOND_UNIT
#else
			.tick_units = MINUTE_UNIT
#endif
		}
  };
  app_event_loop(params, &handlers);
}
