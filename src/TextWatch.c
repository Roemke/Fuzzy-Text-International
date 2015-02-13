#include <pebble.h>

#include "num2words.h"

#define DEBUG 1 
//if debug to 1 appinfo.json.watchapp has to be used - not sure, but nearly :-)
#define MAX_WIDTH_PX 50  //kr to do 

//for function calls when updating lines see readmeKR.txt
//maybe I need to change the original flow because of the font
//problem 

#define NUM_LINES 4
#define LINE_LENGTH 7
#define BUFFER_SIZE (LINE_LENGTH + 2)
#define ROW_HEIGHT 37
#define ROW_HEIGHT_SMALL 27 //kr, todo
#define TOP_MARGIN 10

#define INVERT_KEY 0
#define TEXT_ALIGN_KEY 1
#define LANGUAGE_KEY 2

#define TEXT_ALIGN_CENTER 0
#define TEXT_ALIGN_LEFT 1
#define TEXT_ALIGN_RIGHT 2

// The time it takes for a layer to slide in or out.
#define ANIMATION_DURATION 400
// Delay between the layers animations, from top to bottom
#define ANIMATION_STAGGER_TIME 150
// Delay from the start of the current layer going out until the next layer slides in
#define ANIMATION_OUT_IN_DELAY 100

#define LINE_APPEND_MARGIN 0
// We can add a new word to a line if there are at least this many characters free after
#define LINE_APPEND_LIMIT (LINE_LENGTH - LINE_APPEND_MARGIN)

static AppSync sync;
static uint8_t sync_buffer[64];

static int text_align = TEXT_ALIGN_CENTER;
static bool invert = false;
static Language lang = EN_US;

static Window *window;

typedef struct {
	TextLayer *currentLayer;
	TextLayer *nextLayer;
	char lineStr1[BUFFER_SIZE];
	char lineStr2[BUFFER_SIZE];
	PropertyAnimation *animation1;
	PropertyAnimation *animation2;
} Line;

static Line lines[NUM_LINES];
static InverterLayer *inverter_layer;

//static time why, try to rename
static struct tm *actual_time; //renamed from t in the original source
#if DEBUG
static  struct tm debug_time = {.tm_sec=0, .tm_min=0, .tm_hour=0};
#endif
static int currentNLines;


// Animation handler
static void animationStoppedHandler(struct Animation *animation, bool finished, void *context)
{
	TextLayer *current = (TextLayer *)context;
	GRect rect = layer_get_frame((Layer *)current);
	rect.origin.x = 144;
	layer_set_frame((Layer *)current, rect);
}

// Animate line
static void makeAnimationsForLayer(Line *line, int delay)
{
	TextLayer *current = line->currentLayer;
	TextLayer *next = line->nextLayer;

	// Destroy old animations
	if (line->animation1 != NULL)
	{
		 property_animation_destroy(line->animation1);
	}
	if (line->animation2 != NULL)
	{
		 property_animation_destroy(line->animation2);
	}

	// Configure animation for current layer to move out
	GRect rect = layer_get_frame((Layer *)current);
	rect.origin.x =  -144;
	line->animation1 = property_animation_create_layer_frame((Layer *)current, NULL, &rect);
	animation_set_duration(&line->animation1->animation, ANIMATION_DURATION);
	animation_set_delay(&line->animation1->animation, delay);
	animation_set_curve(&line->animation1->animation, AnimationCurveEaseIn); // Accelerate

	// Configure animation for current layer to move in
	GRect rect2 = layer_get_frame((Layer *)next);
	rect2.origin.x = 0; //looks correct but string with width 142 does not fit into layer
	line->animation2 = property_animation_create_layer_frame((Layer *)next, NULL, &rect2);
	animation_set_duration(&line->animation2->animation, ANIMATION_DURATION);
	animation_set_delay(&line->animation2->animation, delay + ANIMATION_OUT_IN_DELAY);
	animation_set_curve(&line->animation2->animation, AnimationCurveEaseOut); // Deaccelerate

	// Set a handler to rearrange layers after animation is finished
	animation_set_handlers(&line->animation2->animation, (AnimationHandlers) {
		.stopped = (AnimationStoppedHandler)animationStoppedHandler
	}, current);

	// Start the animations
	animation_schedule(&line->animation1->animation);
	animation_schedule(&line->animation2->animation);	
}

static void updateLayerText(TextLayer* layer, char* text)
{
	const char* layerText = text_layer_get_text(layer);
	strcpy((char*)layerText, text);
	//(kr) hier haben wir die breite / koennen sie auslesen
	// To mark layer dirty
	text_layer_set_text(layer, layerText);
    //layer_mark_dirty(&layer->layer); //kr war auskommentiert - unnoetig?
}

// Update line
static void updateLineTo(Line *line, char *value, int delay)
{
	updateLayerText(line->nextLayer, value);
	makeAnimationsForLayer(line, delay);

	// Swap current/next layers
	TextLayer *tmp = line->nextLayer;
	line->nextLayer = line->currentLayer;
	line->currentLayer = tmp;
}

// Check to see if the current line needs to be updated
static bool needToUpdateLine(Line *line, char *nextValue)
{
	const char *currentStr = text_layer_get_text(line->currentLayer);

	if (strcmp(currentStr, nextValue) != 0) {
		return true;
	}
	return false;
}

static GTextAlignment lookup_text_alignment(int align_key)
{
	GTextAlignment alignment;
	switch (align_key)
	{
		case TEXT_ALIGN_LEFT:
			alignment = GTextAlignmentLeft;
			break;
		case TEXT_ALIGN_RIGHT:
			alignment = GTextAlignmentRight;
			break;
		default:
			alignment = GTextAlignmentCenter;
			break;
	}
	return alignment;
}

// 2015-02-12 (kr) configure layer 
static void configureLayer(TextLayer *textlayer,const char * fontKey )
{
	text_layer_set_font(textlayer, fonts_get_system_font(fontKey));
	text_layer_set_text_color(textlayer, GColorWhite);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, lookup_text_alignment(text_align));
	//APP_LOG(APP_LOG_LEVEL_INFO,"bold layer");
}

/*
// Configure bold line of text (kr) removed
static void configureBoldLayer(TextLayer *textlayer)
{
	text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
	text_layer_set_text_color(textlayer, GColorWhite);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, lookup_text_alignment(text_align));
	//APP_LOG(APP_LOG_LEVEL_INFO,"bold layer");
}

// Configure light line of text
static void configureLightLayer(TextLayer *textlayer)
{
	text_layer_set_font(textlayer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
	text_layer_set_text_color(textlayer, GColorWhite);
	text_layer_set_background_color(textlayer, GColorClear);
	text_layer_set_text_alignment(textlayer, lookup_text_alignment(text_align));
	//APP_LOG(APP_LOG_LEVEL_INFO,"light layer");
}
*/
// Configure the layers for the given text
//returns number of lines we need
static int configureLayersForText(char text[NUM_LINES][BUFFER_SIZE], char format[])
{
	int numLines = 0;
        //lines is array of size NUM_LINES (4) of type Line 
	// Set bold layer.
	int i;
	int flagWidth = 0;
	for (i = 0; i < NUM_LINES; i++) {
		if (strlen(text[i]) > 0) {
			if (format[i] == 'b')
			{
				configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_42_BOLD); //Bold for Hours (see in strings... marked with *
			}
			else
			{
				configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_42_LIGHT);
			}
			//2015-02-12 (kr) try to check size - sieht gut aus, aber es gibt zu wenig fonts
			//nein, hatte oben getestet, hier werde ich die breite noch nicht haben
			//stimmt, hab sie nicht - schade, evtl einen text_layer nur zum checken anlegen
			GSize size = text_layer_get_content_size(lines[i].nextLayer);
			APP_LOG(APP_LOG_LEVEL_INFO,"height is %i and width %i",size.h,size.w);
			if (size.w > MAX_WIDTH_PX) //need to fix the font and the line arrangement
				flagWidth = 1; //need to rearrange
			//-----------------

			
		}
		else
		{
			break;
		}
	}
	numLines = i;
	
	int rowHeight = ROW_HEIGHT;
	if (flagWidth)
	   rowHeight = ROW_HEIGHT_SMALL;
	// Calculate y position of top Line
	int ypos = (168 - numLines * rowHeight) / 2 - TOP_MARGIN;

	// Set y positions for the lines
	for (int i = 0; i < numLines; i++)
	{
		layer_set_frame((Layer *)lines[i].nextLayer, GRect(144, ypos, 144, 50)); //x y w h
		ypos += rowHeight;
	}

	return numLines;
}

static int time_to_lines(int hours, int minutes, int seconds, char lines[NUM_LINES][BUFFER_SIZE], char format[])
{
	int length = NUM_LINES * BUFFER_SIZE + 1;
	char timeStr[length];
	int delta = time_to_words(lang, hours, minutes, seconds, timeStr, length);
	//(kr) 2015-02-12: positve delta: real time is time + n Minutes
	// Empty all lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		lines[i][0] = '\0';
	}

	char *start = timeStr;
	char *end = strstr(start, " ");
	int l = 0;
	while (end != NULL && l < NUM_LINES) {
		// Check word for bold prefix
		if (*start == '*' && end - start > 1)
		{
			// Mark line bold and move start to the first character of the word
			format[l] = 'b';
			start++;
		}
		else
		{
			// Mark line normal
			format[l] = ' ';
		}

		// Can we add another word to the line?
		if (format[l] == ' ' && *(end + 1) != '*'    // are both lines formatted normal?
			&& end - start < LINE_APPEND_LIMIT - 1)  // is the first word is short enough?
		{
			// See if next word fits
			char *try = strstr(end + 1, " ");
			if (try != NULL && try - start <= LINE_APPEND_LIMIT)
			{
				end = try;
			}
		}

		// copy to line
		*end = '\0';
		strcpy(lines[l++], start);

		// Look for next word
		start = end + 1;
		end = strstr(start, " ");
	}
	return delta;
}

// Update screen based on new time
static void display_time(struct tm *t)
{
	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];
	char format[NUM_LINES];

        //(kr) delta is realTime - roundedTime (rounded: 5 minutes interval)
	int delta = time_to_lines(t->tm_hour, t->tm_min, t->tm_sec, textLine, format);
	
	//(kr) hier wird übergeben, aber Text nicht gesetzt
	int nextNLines = configureLayersForText(textLine, format);

	int delay = 0;
	for (int i = 0; i < NUM_LINES; i++) {
		if (nextNLines != currentNLines || needToUpdateLine(&lines[i], textLine[i])) {
			updateLineTo(&lines[i], textLine[i], delay);
			delay += ANIMATION_STAGGER_TIME;
		}
	}
	
	currentNLines = nextNLines; //currentNLines ist static
}

static void initLineForStart(Line* line)
{
	// Switch current and next layer
	TextLayer* tmp  = line->currentLayer;
	line->currentLayer = line->nextLayer;
	line->nextLayer = tmp;

	// Move current layer to screen;
	GRect rect = layer_get_frame((Layer *)line->currentLayer);
	rect.origin.x = 0;
	layer_set_frame((Layer *)line->currentLayer, rect);
}

// Update screen without animation first time we start the watchface
static void display_actual_time(struct tm *t)
{
	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];
	char format[NUM_LINES];

	time_to_lines(t->tm_hour, t->tm_min, t->tm_sec, textLine, format);

	// This configures the nextLayer for each line
	currentNLines = configureLayersForText(textLine, format);

	// Set the text and configure layers to the start position
	for (int i = 0; i < currentNLines; i++)
	{
		updateLayerText(lines[i].nextLayer, textLine[i]);
		// This call switches current- and nextLayer
		initLineForStart(&lines[i]);
	}	
}

// Time handler called every minute by the system
static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  #if DEBUG==0
	actual_time = tick_time;
	//APP_LOG(APP_LOG_LEVEL_DEBUG, "update time %i", t->tm_min);
	display_time(actual_time);	
	#endif
}

/**
 * Debug methods. For quickly debugging enable debug macro on top to transform the watchface into
 * a standard app and you will be able to change the time with the up and down buttons
 */
#if DEBUG

static void up_single_click_handler(ClickRecognizerRef recognizer, void * context){ // Window *window) {
	//(void)recognizer;
	//(void)window; //hmm, it does not work with actual_time (was t in original)
	if ( debug_time.tm_sec == 0 && debug_time.tm_min==0 && debug_time.tm_hour==0) //set time from actual_time
	{
		debug_time.tm_sec = actual_time->tm_sec ;
		debug_time.tm_min = actual_time->tm_min ;
		debug_time.tm_hour = actual_time->tm_hour ;
	}
	debug_time.tm_sec=0;
	debug_time.tm_min += 5;
	if (debug_time.tm_min >= 60) {
		debug_time.tm_min = 0;
		debug_time.tm_hour += 1;
		
		if (debug_time.tm_hour >= 24) {
			debug_time.tm_hour = 0;
		}
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG,"up click with minutes %i",debug_time.tm_min);
	display_time(&debug_time);
}


static void down_single_click_handler(ClickRecognizerRef recognizer, void * context) {//Window *window) {
	//(void)recognizer;
	//(void)window;
	if ( debug_time.tm_sec == 0 && debug_time.tm_min==0 && debug_time.tm_hour==0) //set time from actual_time
	{
		debug_time.tm_sec = actual_time->tm_sec ;
		debug_time.tm_min = actual_time->tm_min ;
		debug_time.tm_hour = actual_time->tm_hour ;
	}
	debug_time.tm_sec = 0;
	debug_time.tm_min -= 5;
	if (debug_time.tm_min < 0) {
		debug_time.tm_min = 55;
		debug_time.tm_hour -= 1;
		
		if (debug_time.tm_hour < 0) {
			debug_time.tm_hour = 23;
		}
	}
  APP_LOG(APP_LOG_LEVEL_DEBUG,"down click with minutes %i",debug_time.tm_min);
	display_time(&debug_time);
}

static void click_config_provider(void * config){ //ClickConfig **config, Window *window) {
  //(void)window; (kr) ClickConfig unknown, why (void)

  //config[BUTTON_ID_UP]->click.handler = (ClickHandler) up_single_click_handler;
  //config[BUTTON_ID_UP]->click.repeat_interval_ms = 100;
  //config[BUTTON_ID_DOWN]->click.handler = (ClickHandler) down_single_click_handler;
  //config[BUTTON_ID_DOWN]->click.repeat_interval_ms = 100;

  //(kr) ClickHandler seems to be necessary
  window_single_click_subscribe(BUTTON_ID_UP, (ClickHandler) up_single_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, (ClickHandler) down_single_click_handler);  
}

#endif

static void sync_error_callback(DictionaryResult dict_error, AppMessageResult app_message_error, void *context)
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d", app_message_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	GTextAlignment alignment;
	switch (key) {
		case TEXT_ALIGN_KEY:
			text_align = new_tuple->value->uint8;
			persist_write_int(TEXT_ALIGN_KEY, text_align);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Set text alignment: %i", text_align);

			alignment = lookup_text_alignment(text_align);
			for (int i = 0; i < NUM_LINES; i++)
			{
				text_layer_set_text_alignment(lines[i].currentLayer, alignment);
				text_layer_set_text_alignment(lines[i].nextLayer, alignment);
				layer_mark_dirty(text_layer_get_layer(lines[i].currentLayer));
				layer_mark_dirty(text_layer_get_layer(lines[i].nextLayer));
			}
			break;
		case INVERT_KEY:
			invert = new_tuple->value->uint8 == 1;
			persist_write_bool(INVERT_KEY, invert);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Set invert: %i", invert ? 1 : 0);

			layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert);
			layer_mark_dirty(inverter_layer_get_layer(inverter_layer));
			break;
		case LANGUAGE_KEY:
			lang = (Language) new_tuple->value->uint8;
			persist_write_int(LANGUAGE_KEY, lang);
			APP_LOG(APP_LOG_LEVEL_DEBUG, "Set language: %i", lang);
      //was soll das, ich denke sofortige Reaktion? ?
      
			if (actual_time)
			{
				display_time(actual_time);
			}
	}
}

static void init_line(Line* line)
{
	// Create layers with dummy position to the right of the screen
	line->currentLayer = text_layer_create(GRect(144, 0, 144, 50));//x y w h 
	line->nextLayer = text_layer_create(GRect(144, 0, 144, 50));
  //144 means at the right end of the frame?
  
	// Configure a style (kr) reduced functions
	//configureLightLayer(line->currentLayer);
	//configureLightLayer(line->nextLayer);
	configureLayer(line->currentLayer,FONT_KEY_BITHAM_42_LIGHT);	
	configureLayer(line->nextLayer,FONT_KEY_BITHAM_42_LIGHT);
	// Set the text buffers
	line->lineStr1[0] = '\0';
	line->lineStr2[0] = '\0';
	text_layer_set_text(line->currentLayer, line->lineStr1);
	text_layer_set_text(line->nextLayer, line->lineStr2);

	// Initially there are no animations
	line->animation1 = NULL;
	line->animation2 = NULL;
}

static void destroy_line(Line* line)
{
	// Free layers
	text_layer_destroy(line->currentLayer);
	text_layer_destroy(line->nextLayer);
}

static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
  APP_LOG(APP_LOG_LEVEL_INFO, "Window load:  x %i y %i w %i h %i", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
	//ok, get x 0 y 0 w 144 h 168
	// Init and load lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		init_line(&lines[i]);
		layer_add_child(window_layer, (Layer *)lines[i].currentLayer);
		layer_add_child(window_layer, (Layer *)lines[i].nextLayer);
	}

	inverter_layer = inverter_layer_create(bounds);
	layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert);
	layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));

	// Configure time on init
	time_t raw_time;
	time(&raw_time);
	actual_time = localtime(&raw_time); //where is memory handled
	display_actual_time(actual_time);

	Tuplet initial_values[] = {
		TupletInteger(TEXT_ALIGN_KEY, (uint8_t) text_align),
		TupletInteger(INVERT_KEY,     (uint8_t) invert ? 1 : 0),
		TupletInteger(LANGUAGE_KEY,   (uint8_t) lang)
	};

	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
			sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void window_unload(Window *window)
{
	app_sync_deinit(&sync);

	// Free layers
	inverter_layer_destroy(inverter_layer);
	for (int i = 0; i < NUM_LINES; i++)
	{
		destroy_line(&lines[i]);
	}
}

static void handle_init() {
	// Load settings from persistent storage
	if (persist_exists(TEXT_ALIGN_KEY))
	{
		text_align = persist_read_int(TEXT_ALIGN_KEY);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read text alignment from store: %i", text_align);
	}
	if (persist_exists(INVERT_KEY))
	{
		invert = persist_read_bool(INVERT_KEY);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read invert from store: %i", invert ? 1 : 0);
	}
	if (persist_exists(LANGUAGE_KEY))
	{
		lang = (Language) persist_read_int(LANGUAGE_KEY);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read language from store: %i", lang);
	}

	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});

	// Initialize message queue
	const int inbound_size = 64;
	const int outbound_size = 64;
	app_message_open(inbound_size, outbound_size);

	const bool animated = true;
	window_stack_push(window, animated);

	// Subscribe to minute ticks
	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

#if DEBUG
	// Button functionality
	window_set_click_config_provider(window, (ClickConfigProvider) click_config_provider);
#endif
}

static void handle_deinit()
{
	// Free window
	window_destroy(window);
}

int main(void)
{
	handle_init();
	app_event_loop();
	handle_deinit();
}

