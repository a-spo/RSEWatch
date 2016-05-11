#include <pebble.h>
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_UNITS 2
#define KEY_DATE 3
#define SETTINGS_KEY 6

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static GFont s_time_font;
static GFont s_weather_font;
static Layer *s_background_layer;
static BitmapLayer *s_backsprite_layer;
static BitmapLayer *s_frontsprite_layer;
static BitmapLayer *s_slp_layer;
static BitmapLayer *s_prlyz_layer;
static GBitmap *s_background_bitmap;
static GBitmap *s_backsprite_bitmap;
static GBitmap *s_frontsprite_bitmap;
static GBitmap *s_slp_bitmap;
static GBitmap *s_prlyz_bitmap;
static TextLayer *s_weather_layer;
static TextLayer *s_frontname_layer;
static TextLayer *s_oppname_layer;
static TextLayer *s_opp_level_layer;
static int s_battery_level;
static Layer *s_battery_layer;
static Layer *s_time_left_layer;
static char conditions_buffer[32];
char batt_lev[4];
char opplevel[3];
bool old_units = true;

typedef struct persist {
  int hour;
    int min;
    int bg;
    int oppnumber;
    int yournumber;
    int opplevel;
    bool units_use_f;
    bool date_format_us;
} __attribute__((__packed__)) persist;

persist settings = {
  .hour = 0,
  .min = 0,
  .bg = 1,
  .oppnumber = 0,
  .yournumber = 0,
  .opplevel = 10,
  .units_use_f = true,
  .date_format_us = true,
};
bool already_read = false;
static uint32_t background_list[5] = {RESOURCE_ID_BATTLE_SCENE_CLEAR, RESOURCE_ID_BATTLE_SCENE_64, RESOURCE_ID_BATTLE_SCENE_CLOUDS, RESOURCE_ID_BATTLE_SCENE_RAIN, RESOURCE_ID_BATTLE_SCENE_SNOW};
static uint32_t background_round_list[5] = {RESOURCE_ID_BACKGROUND_CLEAR_ROUND, RESOURCE_ID_BACKGROUND_64_ROUND, RESOURCE_ID_BACKGROUND_CLOUDS_ROUND, RESOURCE_ID_BACKGROUND_RAIN_ROUND, RESOURCE_ID_BACKGROUND_SNOW_ROUND};
static uint32_t pokemon_back_list[34] = {RESOURCE_ID_POKEMONBACK_1, RESOURCE_ID_POKEMONBACK_2, RESOURCE_ID_POKEMONBACK_3, RESOURCE_ID_POKEMONBACK_4, RESOURCE_ID_POKEMONBACK_5, RESOURCE_ID_POKEMONBACK_6, RESOURCE_ID_POKEMONBACK_7,
                                    RESOURCE_ID_POKEMONBACK_8, RESOURCE_ID_POKEMONBACK_9, RESOURCE_ID_POKEMONBACK_10, RESOURCE_ID_POKEMONBACK_11, RESOURCE_ID_POKEMONBACK_12, RESOURCE_ID_POKEMONBACK_13,
                                    RESOURCE_ID_POKEMONBACK_14, RESOURCE_ID_POKEMONBACK_15, RESOURCE_ID_POKEMONBACK_16, RESOURCE_ID_POKEMONBACK_17, RESOURCE_ID_POKEMONBACK_18, RESOURCE_ID_POKEMONBACK_19,
                                    RESOURCE_ID_POKEMONBACK_20, RESOURCE_ID_POKEMONBACK_21, RESOURCE_ID_POKEMONBACK_22, RESOURCE_ID_POKEMONBACK_23, RESOURCE_ID_POKEMONBACK_24, RESOURCE_ID_POKEMONBACK_25,
                                    RESOURCE_ID_POKEMONBACK_26, RESOURCE_ID_POKEMONBACK_27, RESOURCE_ID_POKEMONBACK_28, RESOURCE_ID_POKEMONBACK_29, RESOURCE_ID_POKEMONBACK_30, RESOURCE_ID_POKEMONBACK_31,
                                    RESOURCE_ID_POKEMONBACK_32, RESOURCE_ID_POKEMONBACK_33, RESOURCE_ID_POKEMONBACK_34};
static uint32_t pokemon_front_list[34] = {RESOURCE_ID_POKEMONFRONT_1, RESOURCE_ID_POKEMONFRONT_2, RESOURCE_ID_POKEMONFRONT_3, RESOURCE_ID_POKEMONFRONT_4, RESOURCE_ID_POKEMONFRONT_5, RESOURCE_ID_POKEMONFRONT_6, RESOURCE_ID_POKEMONFRONT_7,
                                    RESOURCE_ID_POKEMONFRONT_8, RESOURCE_ID_POKEMONFRONT_9, RESOURCE_ID_POKEMONFRONT_10, RESOURCE_ID_POKEMONFRONT_11, RESOURCE_ID_POKEMONFRONT_12, RESOURCE_ID_POKEMONFRONT_13,
                                    RESOURCE_ID_POKEMONFRONT_14, RESOURCE_ID_POKEMONFRONT_15, RESOURCE_ID_POKEMONFRONT_16, RESOURCE_ID_POKEMONFRONT_17, RESOURCE_ID_POKEMONFRONT_18, RESOURCE_ID_POKEMONFRONT_19,
                                    RESOURCE_ID_POKEMONFRONT_20, RESOURCE_ID_POKEMONFRONT_21, RESOURCE_ID_POKEMONFRONT_22, RESOURCE_ID_POKEMONFRONT_23, RESOURCE_ID_POKEMONFRONT_24, RESOURCE_ID_POKEMONFRONT_25,
                                    RESOURCE_ID_POKEMONFRONT_26, RESOURCE_ID_POKEMONFRONT_27, RESOURCE_ID_POKEMONFRONT_28, RESOURCE_ID_POKEMONFRONT_29, RESOURCE_ID_POKEMONFRONT_30, RESOURCE_ID_POKEMONFRONT_31,
                                    RESOURCE_ID_POKEMONFRONT_32, RESOURCE_ID_POKEMONFRONT_33, RESOURCE_ID_POKEMONFRONT_34};
static int pokemon_level[34] = {7, 22, 1, 1, 1, 1, 1, 1, 1, 25, 1, 32, 1, 1, 1, 75, 45, 15, 1, 15, 25, 50, 50, 70, 70, 1, 20, 1, 32, 42, 1, 1, 45, 70};
const char *pokemon_names[] = {"KAKUNA", "ARBOK", "RAICHU", "VULPIX", "ZUBAT", "PARAS", "MEOWTH", "ABRA", "PONYTA", "GENGAR", "ONIX", "SEADRA", "TAUROS", "LAPRAS",
                            "EEVEE", "MEWTWO", "MEW", "FURRET", "YANMA", "ESPEON", "SCIZOR", "RAIKOU", "ENTEI", "LUGIA", "HO-OH", "MUDKIP", "KIRLIA", "ARON",
                            "LAIRON", "AGGRON", "ABSOL", "BELDUM", "LATIAS", "KYOGRE"};

static void update_time() {
  // Get a tm structure for time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_date() {
  // Get a tm structure for time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[12];
  // Change date format depending on configuration
  if (settings.date_format_us){
    strftime(s_buffer, sizeof(s_buffer), "%m  %d", tick_time);
  }
  else{
    strftime(s_buffer, sizeof(s_buffer), "%d  %m", tick_time);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_date_layer, s_buffer);
}

// For converting temperature upon configuration
static void update_opp_level(){
  if (old_units != settings.units_use_f){
    if (settings.units_use_f){
      //convert from c to f
      settings.opplevel = (int)(settings.opplevel*(9.0/5.0)+32);
    }
    else{
      //convert f to c
      settings.opplevel = (int)((settings.opplevel-32)*(5.0/9.0));
    }
    static char temperature_buffer[8];
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", settings.opplevel);
    strcpy(opplevel, temperature_buffer);
    text_layer_set_text(s_weather_layer, opplevel);
  }
}

static void get_background_image(char conditions_buffer[]){
  //Destroy background image to free memory
  gbitmap_destroy(s_background_bitmap);
  //get different background depending on weather
  int bg_number;
  if(strstr(conditions_buffer, "Rain") != NULL || strstr(conditions_buffer, "Drizzle") != NULL) {
    bg_number = 3;
  }
  else if(strstr(conditions_buffer, "Snow") != NULL){
    bg_number = 4;
  }
  else if(strstr(conditions_buffer, "Clear") != NULL){
    bg_number = 1;
  }else if(strstr(conditions_buffer, "Cloud") != NULL || strstr(conditions_buffer, "Overcast") != NULL){
    bg_number = 2;
  }
  else{
    // grab old one
    if(persist_exists(SETTINGS_KEY)){
      bg_number = settings.bg;
    }
    else{
      bg_number = 0;
    }
  }
  s_background_bitmap = gbitmap_create_with_resource(PBL_IF_ROUND_ELSE(background_round_list[bg_number],background_list[bg_number]));
  settings.bg = bg_number;
}

static void background_update_proc(Layer *layer, GContext *ctx) {
  //get new bitmap and store to memory, then set as background
  get_background_image(conditions_buffer);
  //graphics_draw_bitmap_in_rect(ctx, s_background_bitmap, GRect(0,0,144,168));
  graphics_draw_bitmap_in_rect(ctx, s_background_bitmap, layer_get_bounds(layer));
  
  //get time and draw exp bar
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int s_hour = tick_time->tm_hour;
  int width = (int)(float)((float)s_hour*2.0F);
  
  graphics_context_set_fill_color(ctx, GColorElectricBlue);
  graphics_fill_rect(ctx, GRect(PBL_IF_ROUND_ELSE(114, 88), PBL_IF_ROUND_ELSE(130,118), width, 2), 0, GCornerNone);
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 54.0F);
  int temp = s_battery_level;
  snprintf(batt_lev, sizeof(batt_lev), "%d", temp);
  text_layer_set_text(s_opp_level_layer, batt_lev);

  // Draw the background of HP bar
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  GColor top_color = GColorDarkGray;
  GColor btm_color = GColorDarkGray;

  // Draw the bar
  //Change color based on health
  if(width>=27){
    top_color = GColorMayGreen;
    btm_color = GColorMediumAquamarine;
  }
  else if(width<27 && width>11){
    top_color = GColorChromeYellow;
    btm_color = GColorYellow;
  }
  else{
    top_color = GColorDarkCandyAppleRed;
    btm_color = GColorRed;
  }
  graphics_context_set_stroke_color(ctx, top_color);
  graphics_context_set_stroke_width(ctx, 1);
  GPoint top_pos1 = bounds.origin;
  GPoint top_pos2 = top_pos1;
  top_pos2.x += width;
  graphics_draw_line(ctx, top_pos1, top_pos2);
  graphics_context_set_stroke_color(ctx, btm_color);
  graphics_context_set_stroke_width(ctx, 1);
  GPoint btm_pos1 = bounds.origin;
  btm_pos1.y += 1;
  GPoint btm_pos2 = btm_pos1;
  btm_pos2.x += width;
  graphics_draw_line(ctx, btm_pos1, btm_pos2); 
  
  //add or remove par status condition based on charging or not
  layer_set_hidden(bitmap_layer_get_layer(s_prlyz_layer), !battery_state_service_peek().is_charging);
  
}

static void time_left_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int s_hour = tick_time->tm_hour;
  int width = 48-(int)(float)((float)s_hour*2.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorDarkGray);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  
  GColor top_color = GColorDarkGray;
  GColor btm_color = GColorDarkGray;

  // Draw the bar
  if(width>=24){
    top_color = GColorMayGreen;
    btm_color = GColorMediumAquamarine;
  }
  else if(width<24 && width>9){
    top_color = GColorChromeYellow;
    btm_color = GColorYellow;
  }
  else{
    top_color = GColorDarkCandyAppleRed;
    btm_color = GColorRed;
  }
  graphics_context_set_stroke_color(ctx, top_color);
  graphics_context_set_stroke_width(ctx, 1);
  GPoint top_pos1 = bounds.origin;
  GPoint top_pos2 = top_pos1;
  top_pos2.x += width;
  graphics_draw_line(ctx, top_pos1, top_pos2);
  graphics_context_set_stroke_color(ctx, btm_color);
  graphics_context_set_stroke_width(ctx, 1);
  GPoint btm_pos1 = bounds.origin;
  btm_pos1.y += 1;
  GPoint btm_pos2 = btm_pos1;
  btm_pos2.x += width;
  graphics_draw_line(ctx, btm_pos1, btm_pos2); 
  
}

static void get_new_sprite(int level, bool front_back){
  //get random number from 0 to 33
  //pokemon's level must be less than or equal to supplied level
  int len = sizeof(pokemon_back_list) / sizeof(pokemon_back_list[0]);
  if(level<1){
    level = 1;
  }
  int random_num = 1;
  do{
    random_num = rand() % len;
  }while(pokemon_level[random_num]>level);
  
  //destroy current sprite and set new one to memory
  //Set new name to layer
  if(front_back){
    gbitmap_destroy(s_backsprite_bitmap);
    s_backsprite_bitmap = gbitmap_create_with_resource(pokemon_back_list[random_num]);
    text_layer_set_text(s_frontname_layer, pokemon_names[random_num]);
    settings.yournumber = random_num;
  }
  else{
    gbitmap_destroy(s_frontsprite_bitmap);
    s_frontsprite_bitmap = gbitmap_create_with_resource(pokemon_front_list[random_num]);
    text_layer_set_text(s_oppname_layer, pokemon_names[random_num]);
    settings.oppnumber = random_num;
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char weather_layer_buffer[32];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);
  Tuple *units_tuple = dict_find(iterator, KEY_UNITS);
  Tuple *date_tuple = dict_find(iterator, KEY_DATE);

  // Determine if receiving weather or configuration
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    
    //get new background
    layer_mark_dirty(s_background_layer);
  
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
    // save weather_layer_buffer to persistent storage
    settings.opplevel = atoi(weather_layer_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  
    //set your sprite 
    get_new_sprite(s_battery_level, true);
    bitmap_layer_set_bitmap(s_backsprite_layer, s_backsprite_bitmap);
  
    //set opponent sprite
    int temp = (int)temp_tuple->value->int32;
    if (!settings.units_use_f){
      temp = (int)(temp*(9.0/5.0)+32);
    }
    get_new_sprite(temp, false);
    bitmap_layer_set_bitmap(s_frontsprite_layer, s_frontsprite_bitmap);
  }
  else if(units_tuple && date_tuple){
    // Set old units to compare if need to change temp
    old_units = settings.units_use_f;
    if (units_tuple && units_tuple->value->int8 > 0){
      settings.units_use_f = false;
    }
    else{
      settings.units_use_f = true;
    }
    if (date_tuple && date_tuple->value->int8 > 0){
      settings.date_format_us = false;
    }
    else{
      settings.date_format_us = true;
    }
    //APP_LOG(APP_LOG_LEVEL_INFO, "Units use f?: %d, Date use mm/dd?: %d", settings.units_use_f, settings.date_format_us);
    update_date();
    update_opp_level();
  }  
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void handle_tap(AccelAxisType axis, int32_t direction){
  get_new_sprite(s_battery_level, true);
  bitmap_layer_set_bitmap(s_backsprite_layer, s_backsprite_bitmap);
  
  //set opponent sprite
  get_new_sprite(settings.opplevel, false);
  bitmap_layer_set_bitmap(s_frontsprite_layer, s_frontsprite_bitmap);
}

static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;
  
  // Update meter
  layer_mark_dirty(s_battery_layer);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_slp_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void main_window_load(Window *window) {
  // Load old data from persist
  already_read = false;
  bool read_from_persist = false;
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
    read_from_persist = true;
  }
  
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Create background, set update process
  s_background_layer = layer_create(bounds);
  layer_set_update_proc(s_background_layer, background_update_proc);
  layer_add_child(window_layer, s_background_layer);
  
  // Add back sprite
  int load_yournumber = rand() % sizeof(pokemon_back_list);
  if(read_from_persist){
    load_yournumber = settings.yournumber;
  }
  else{
    settings.yournumber = load_yournumber;
  }
  s_backsprite_bitmap = gbitmap_create_with_resource(pokemon_back_list[load_yournumber]);
  s_backsprite_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(8,-8), PBL_IF_ROUND_ELSE(68,56), 64, 64));
  bitmap_layer_set_background_color(s_backsprite_layer, GColorClear);
  bitmap_layer_set_bitmap(s_backsprite_layer, s_backsprite_bitmap);
  bitmap_layer_set_compositing_mode(s_backsprite_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_backsprite_layer));
  
  
  // Add front sprite
  int load_oppnumber = rand() % sizeof(pokemon_back_list);
  if(read_from_persist){
    load_oppnumber = settings.oppnumber;
  }
  else{
    settings.oppnumber = load_oppnumber;
  }
  s_frontsprite_bitmap = gbitmap_create_with_resource(pokemon_front_list[load_oppnumber]);
  s_frontsprite_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(103,80), PBL_IF_ROUND_ELSE(25,4), 64, 64));
  bitmap_layer_set_background_color(s_frontsprite_layer, GColorClear);
  bitmap_layer_set_bitmap(s_frontsprite_layer, s_frontsprite_bitmap);
  bitmap_layer_set_compositing_mode(s_frontsprite_layer, GCompOpSet);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_frontsprite_layer));
  
  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(107,81), PBL_IF_ROUND_ELSE(110,98), 55, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  battery_callback(battery_state_service_peek());
  
  // Create time left layer
  s_time_left_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(54,26), PBL_IF_ROUND_ELSE(44,22), 48, 2));
  layer_set_update_proc(s_time_left_layer, time_left_update_proc);
  layer_add_child(window_get_root_layer(window), s_time_left_layer);
  
  // Create the Bluetooth icon GBitmap
  s_slp_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SLP_ICON);
  s_slp_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(88,65), PBL_IF_ROUND_ELSE(117,105), 20, 8));
  bitmap_layer_set_background_color(s_slp_layer, GColorClear);
  bitmap_layer_set_bitmap(s_slp_layer, s_slp_bitmap);
  bitmap_layer_set_compositing_mode(s_slp_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_slp_layer));    
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
  // Create charging icon GBitmap
  s_prlyz_bitmap = gbitmap_create_with_resource(RESOURCE_ID_PRLYZ_ICON);
  s_prlyz_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(109,86), PBL_IF_ROUND_ELSE(117,105), 20, 8));
  bitmap_layer_set_background_color(s_prlyz_layer, GColorClear);
  bitmap_layer_set_bitmap(s_prlyz_layer, s_prlyz_bitmap);
  bitmap_layer_set_compositing_mode(s_prlyz_layer, GCompOpSet);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_prlyz_layer));

  // Create the time Layer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(135, 124), bounds.size.w, 50));
  
  // Create the date Layer with specific bounds
  s_date_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(57,49), PBL_IF_ROUND_ELSE(114, 102), bounds.size.w, 50));
  
  // Create temperature Layer--changed to opponent level
  s_weather_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(88,60), PBL_IF_ROUND_ELSE(28, 5), bounds.size.w, 25));
  
  // Create frontname Layer
  s_frontname_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(89,63), PBL_IF_ROUND_ELSE(93, 81), bounds.size.w, 25));
  
  // Create oppname Layer
  s_oppname_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(35,7), PBL_IF_ROUND_ELSE(28, 5), bounds.size.w, 25));
  
  // Create opp level Layer--changed to your level, start point was 120 for left adjusted, -7 for right
  s_opp_level_layer = text_layer_create(
      GRect(PBL_IF_ROUND_ELSE(146,120), PBL_IF_ROUND_ELSE(93, 81), bounds.size.w, 25));
  
  // Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKEMON_35));
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_POKEMON_12));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorDarkGray);
  text_layer_set_font(s_date_layer, s_weather_font);
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
  
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  char load_opplevel[4] = "--";
  if (read_from_persist){
    int temp = settings.opplevel;
    snprintf(load_opplevel, sizeof(load_opplevel), "%d", temp);
  }
  else{
    settings.opplevel = 10;
  }
  strcpy(opplevel, load_opplevel);
  text_layer_set_text(s_weather_layer, opplevel);
  text_layer_set_font(s_weather_layer, s_weather_font);
  
  text_layer_set_background_color(s_frontname_layer, GColorClear);
  text_layer_set_text_color(s_frontname_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_frontname_layer, GTextAlignmentLeft);
  text_layer_set_text(s_frontname_layer, pokemon_names[load_yournumber]);
  text_layer_set_font(s_frontname_layer, s_weather_font);
  
  text_layer_set_background_color(s_oppname_layer, GColorClear);
  text_layer_set_text_color(s_oppname_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_oppname_layer, GTextAlignmentLeft);
  text_layer_set_text(s_oppname_layer, pokemon_names[load_oppnumber]);
  text_layer_set_font(s_oppname_layer, s_weather_font);
  
  // Actually your battery info
  text_layer_set_background_color(s_opp_level_layer, GColorClear);
  text_layer_set_text_color(s_opp_level_layer, GColorDarkGray);
  text_layer_set_text_alignment(s_opp_level_layer, GTextAlignmentLeft);
  text_layer_set_text(s_opp_level_layer, "--");
  text_layer_set_font(s_opp_level_layer, s_weather_font);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_frontname_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_oppname_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_opp_level_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
  // save time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  int hour = tick_time->tm_hour;
  int min = tick_time->tm_min;
  settings.hour = hour;
  settings.min = min;
  persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(s_frontname_layer);
  text_layer_destroy(s_oppname_layer);
  text_layer_destroy(s_opp_level_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_weather_font);
  
  //destroy everything else
  gbitmap_destroy(s_frontsprite_bitmap);
  bitmap_layer_destroy(s_frontsprite_layer);
  gbitmap_destroy(s_backsprite_bitmap);
  bitmap_layer_destroy(s_backsprite_layer);
  gbitmap_destroy(s_background_bitmap);
  layer_destroy(s_background_layer);
  gbitmap_destroy(s_slp_bitmap);
  bitmap_layer_destroy(s_slp_layer);
  gbitmap_destroy(s_prlyz_bitmap);
  bitmap_layer_destroy(s_prlyz_layer);
  layer_destroy(s_battery_layer);
  layer_destroy(s_time_left_layer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_date();
  
  if(tick_time->tm_min == 0) {
    // Update meter
    layer_mark_dirty(s_time_left_layer);
  }
  
  // Get weather update every 30 minutes
  bool fetch_weather = false;
  if (!already_read) {
    already_read = true;
    
    if(persist_exists(SETTINGS_KEY)){
      int old_hour = settings.hour;
      int old_min = settings.min;
      int curr_hour = tick_time->tm_hour;
      int curr_min = tick_time->tm_min;
      APP_LOG(APP_LOG_LEVEL_INFO, "Current hours is %d and current min is %d", curr_hour, curr_min);
      APP_LOG(APP_LOG_LEVEL_INFO, "Old hours is %d and old min is %d", old_hour, old_min);
      APP_LOG(APP_LOG_LEVEL_INFO, "Hour diff is %d and min diff is %d", curr_hour-old_hour, curr_min-old_min);
      if (curr_hour-old_hour != 0){
        fetch_weather = true;
      }
      else{
        if(curr_min-old_min>=30){
          fetch_weather = true;
        }
        else if (curr_min > 30 && old_min < 30){
          fetch_weather = true;
        }
        else if (curr_min <30 && old_min >30){
          fetch_weather = true;
        }
        else{
          fetch_weather = false;
        }
      }
    }
    else{
      fetch_weather = true;
    }
  }
  if(tick_time->tm_min % 30 == 0 || fetch_weather) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  window_set_background_color(s_main_window, GColorBlack);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  // Make sure the time is displayed from the start
  update_time();
  update_date();
  layer_mark_dirty(s_time_left_layer);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
  // Register for wrist shake/tap
  accel_tap_service_subscribe(handle_tap);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}