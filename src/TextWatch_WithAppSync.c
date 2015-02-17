#include <pebble.h>

#include "num2words.h"

#define DEBUG 0 
//if debug to 1 appinfo.json.watchapp has to be used - not sure, but nearly :-)
#define MAX_WIDTH_PX 130  //kr  

//for function calls when updating lines see readmeKR.txt
//maybe I need to change the original flow because of the font
//problem 

#define NUM_LINES 4
#define LINE_LENGTH 7
#define BUFFER_SIZE (LINE_LENGTH + 2)
#define ROW_HEIGHT 37 //was 37
#define ROW_HEIGHT_SMALL 30 //kr

//define keys for persistence on pebble, need match in pebble-js-app
#define INVERT_KEY 0
#define TEXT_ALIGN_KEY 1
#define LANGUAGE_KEY 2
#define DELTA_KEY 3   
#define DONE_KEY 4  //show minutes done   
#define BATTERY_KEY 5 // battery bar 
#define WARNOWN_KEY 6 // warning / vibration 5 minutes before own lessen 
#define TIMETABLE_KEY 10 // does timetable exist / have to get them from phone
//#define DATA_KEY 12 // one dataset for timetable

//-----------------

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
static uint8_t sync_buffer[124]; //(kr) was 64

static int text_align = TEXT_ALIGN_CENTER;
static bool invert = false;
static bool delta = false; //kr options static, oh c kann bool dachte waere c++
	                        //delta is for showing exact - fuzzy
static bool done = false; //show minutes done and left
static bool battery = true; //show bar to indicate accu
static bool warnown = false;
static int timetable = 0; //how many entries in the TimeTable
 
static Language lang = EN_US;

static Window *window;

typedef struct {
	TextLayer *currentLayer;
	TextLayer *nextLayer;
	char lineStr1[BUFFER_SIZE];
	char lineStr2[BUFFER_SIZE];
	PropertyAnimation *animation1;
	PropertyAnimation *animation2;
	unsigned int currentFontSmall;
	unsigned int nextFontSmall;
} Line;

static Line lines[NUM_LINES];
static InverterLayer *inverter_layer;

static TextLayer * tempLayer ; // = text_layer_create(GRect(144, 0, 144, 50));
char tempLayerText[BUFFER_SIZE];

static TextLayer * deltaLayer; 
char deltaLayerText[]="(+0)"; // ( sign digit ) \0 without spaces 

static TextLayer * doneLayer; 
static char doneLayerText[]="           "; 

static Layer * batteryLayer;

static char tt_data[24]=""; 
//static time was *t ,  rename it
static struct tm *actual_time; //renamed from t in the original source
#if DEBUG
static  struct tm debug_time = {.tm_sec=0, .tm_min=0, .tm_hour=0};
#endif
static int currentNLines;


//error messages 
char *translate_error(AppMessageResult result) {
  switch (result) {
    case APP_MSG_OK: return "APP_MSG_OK";
    case APP_MSG_SEND_TIMEOUT: return "APP_MSG_SEND_TIMEOUT";
    case APP_MSG_SEND_REJECTED: return "APP_MSG_SEND_REJECTED";
    case APP_MSG_NOT_CONNECTED: return "APP_MSG_NOT_CONNECTED";
    case APP_MSG_APP_NOT_RUNNING: return "APP_MSG_APP_NOT_RUNNING";
    case APP_MSG_INVALID_ARGS: return "APP_MSG_INVALID_ARGS";
    case APP_MSG_BUSY: return "APP_MSG_BUSY";
    case APP_MSG_BUFFER_OVERFLOW: return "APP_MSG_BUFFER_OVERFLOW";
    case APP_MSG_ALREADY_RELEASED: return "APP_MSG_ALREADY_RELEASED";
    case APP_MSG_CALLBACK_ALREADY_REGISTERED: return "APP_MSG_CALLBACK_ALREADY_REGISTERED";
    case APP_MSG_CALLBACK_NOT_REGISTERED: return "APP_MSG_CALLBACK_NOT_REGISTERED";
    case APP_MSG_OUT_OF_MEMORY: return "APP_MSG_OUT_OF_MEMORY";
    case APP_MSG_CLOSED: return "APP_MSG_CLOSED";
    case APP_MSG_INTERNAL_ERROR: return "APP_MSG_INTERNAL_ERROR";
    default: return "UNKNOWN ERROR";
  }
}
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
	line->currentFontSmall = line->nextFontSmall;
}

// Check to see if the current line needs to be updated
static bool needToUpdateLine(Line *line, char *nextValue)
{
	const char *currentStr = text_layer_get_text(line->currentLayer);

	if (strcmp(currentStr, nextValue) != 0 ||
	    line->currentFontSmall != line->nextFontSmall) {
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

// Configure the layers for the given text
//returns number of lines we need
static int configureLayersForText(char text[NUM_LINES][BUFFER_SIZE], char format[])
{
	int numLines = 0;
        //lines is array of size NUM_LINES (4) of type Line 
	// Set bold layer.
	int i;
	int flagWidth = 0;
  //for layer check
  //TextLayer * tempLayer  = text_layer_create(GRect(144, 0, 144, 50)); besser in window_load evtl. performance


	for (i = 0; i < NUM_LINES; i++) {
		if (strlen(text[i]) > 0) {
			if (format[i] == 'b')
			{
				configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_42_BOLD); //Bold for Hours (see in strings... marked with *
			}
			else
			{
				configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_42_LIGHT);
				configureLayer(tempLayer,FONT_KEY_BITHAM_42_LIGHT);
			}
			lines[i].nextFontSmall = 0; //normal
			//2015-02-12 (kr) try to check size - sieht gut aus, aber es gibt zu wenig fonts
			//nein, hatte oben getestet, hier werde ich die breite noch nicht haben
			//stimmt, hab sie nicht - schade, evtl einen text_layer nur zum checken anlegen
			const char* layerText = text_layer_get_text(tempLayer);
			strcpy((char*)layerText, text[i]);//text_layer_set_tet not necessary
			//if this layer is marked diry is equal cause it's not visible 
			
      //--------------------
			GSize size = text_layer_get_content_size(tempLayer);
			//APP_LOG(APP_LOG_LEVEL_INFO,"height is %i and width %i",size.h,size.w);
			if (format[i] != 'b' && size.w > MAX_WIDTH_PX) //need to fix the font and the line arrangement
			{	 //bold lines could be longer (eg sieben in german)
			   flagWidth = 1; //need to rearrange	
			   break;
			}			
			//-----------------			
		}
		else
		{
			break;
		}	
	}
	int yPositions[]={10,47,84,121};
	if (flagWidth ) // problems with width of font	
	{ //one try with smaller font could be later extende to do while and font list 
		for (i = 0; i < NUM_LINES; i++) //looks ok
		{
			if (strlen(text[i]) > 0) {
				if (format[i] == 'b')
				{
					configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_42_BOLD); //Bold for Hours (see in strings... marked with *
				}
				else
				{
					configureLayer(lines[i].nextLayer,FONT_KEY_BITHAM_30_BLACK);
					lines[i].nextFontSmall = 1;
				}
			}
			else
			{
				break;
			}	
		}		
	}
	// Calculate y position of top Line
	numLines = i;
	if (numLines == 4 && (delta || done)) //obersten zwei Zeilen kleiner
	{
		yPositions[0] = 36;
		yPositions[1] = 60;
		configureLayer(lines[0].nextLayer,FONT_KEY_GOTHIC_28);
		configureLayer(lines[1].nextLayer,FONT_KEY_GOTHIC_28);
	}
	// Set y positions for the lines
	for (int i = 0; i < numLines; i++)
	{
    int ypos = yPositions[i + NUM_LINES - numLines];	  
		layer_set_frame((Layer *)lines[i].nextLayer, GRect(144, ypos, 144, 50)); //x y w h
	}

	return numLines;
}

static int time_to_lines(int hours, int minutes, int seconds, char lines[NUM_LINES][BUFFER_SIZE], char format[])
{
	int length = NUM_LINES * BUFFER_SIZE + 1;
	char timeStr[length];
	int deltaVal = time_to_words(lang, hours, minutes, seconds, timeStr, length);
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
	return deltaVal;
}

// Update screen based on new time
static void display_time(struct tm *t)
{
	// The current time text will be stored in the following strings
	char textLine[NUM_LINES][BUFFER_SIZE];
	char format[NUM_LINES];

	//(kr) delta is realTime - roundedTime (rounded: 5 minutes interval)
	int deltaVal = time_to_lines(t->tm_hour, t->tm_min, t->tm_sec, textLine, format);

	if (delta )//&& text_align != TEXT_ALIGN_CENTER)
	{
		if (deltaVal==0)
			snprintf(deltaLayerText,5," ");
		else if (deltaVal < 0)
			snprintf(deltaLayerText,5,"(%i)",deltaVal);
		else
			snprintf(deltaLayerText,5,"(+%i)",deltaVal);
		layer_set_hidden((Layer *) deltaLayer,false);
		//layer_set_frame((Layer *) deltaLayer,GRect(xDelta,yDelta,43,28));//x y w h	
	}
	else // frame ausblenden
	{
		//APP_LOG(APP_LOG_LEVEL_INFO,"delta aus");
		layer_set_hidden((Layer *) deltaLayer,true);
		//layer_mark_dirty((Layer *) deltaLayer);
	}	
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
  #if DEBUG==0 //switch off when using debug
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
	debug_time.tm_min += 1;
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
	debug_time.tm_min -= 1;
	if (debug_time.tm_min < 0) {
		debug_time.tm_min = 59;
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
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error: %d means %s", app_message_error, translate_error(app_message_error));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "App Message Sync Error Dict_Error: %d", dict_error);
}

static void sync_tuple_changed_callback(const uint32_t key, const Tuple* new_tuple, const Tuple* old_tuple, void* context) {
	GTextAlignment alignment;
	APP_LOG(APP_LOG_LEVEL_DEBUG, "key is : %u", (uint) key);
	switch (key) {
		case TEXT_ALIGN_KEY:
			text_align = new_tuple->value->uint8;
			persist_write_int(TEXT_ALIGN_KEY, text_align);
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set text alignment: %i", text_align);

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
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set invert: %i", invert ? 1 : 0);

			layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert);
			layer_mark_dirty(inverter_layer_get_layer(inverter_layer));
			break;
		case LANGUAGE_KEY:
			lang = (Language) new_tuple->value->uint8;
			persist_write_int(LANGUAGE_KEY, lang);
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set language: %i", lang);
			break;
			//extensions for v 1.3
		case DELTA_KEY:
			delta = new_tuple->value->uint8 == 1;
			persist_write_bool(DELTA_KEY, delta); //persistent
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set delta: %i", delta ? 1 : 0);
			break;
		case BATTERY_KEY:
			battery = new_tuple->value->uint8 == 1;
			persist_write_bool(BATTERY_KEY, battery); //persistent
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set battery: %i", battery ? 1 : 0);
			break;
		case WARNOWN_KEY:
			warnown = new_tuple->value->uint8 == 1;
			persist_write_bool(WARNOWN_KEY, warnown); //persistent
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set warnown: %i", warnown ? 1 : 0);
			break;
		case TIMETABLE_KEY:
			timetable = new_tuple->value->uint8;
			persist_write_bool(TIMETABLE_KEY, timetable); //persistent
			//APP_LOG(APP_LOG_LEVEL_DEBUG, "Set timetable: %i", timetable );
			break;
			
	}
	if (actual_time)
	{
		display_time(actual_time);
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
	line->currentFontSmall = 0;
	line->nextFontSmall = 0;
}

static void destroy_line(Line* line)
{
	// Free layers
	text_layer_destroy(line->currentLayer);
	text_layer_destroy(line->nextLayer);
}

static TextLayer * prepareTextLayer (Layer * window_layer, char *layerText)
{
  TextLayer * layer=text_layer_create(GRect(0,0,144,28));
	text_layer_set_text(layer,layerText); 
  configureLayer(layer,FONT_KEY_GOTHIC_28  );
  text_layer_set_text_alignment(layer,GTextAlignmentCenter);
	layer_set_hidden((Layer *) layer,false);
	layer_add_child(window_layer, (Layer *)layer);
	return layer;
}

//hmm, called very often, think caused by animation
//but doesnt matter 
static void batteryDisplayUpdate(Layer * layer, GContext * ctx)
{
	if (battery)
	{
		BatteryChargeState charge_state = battery_state_service_peek();
		int width = charge_state.charge_percent/100.0 * 144;
		graphics_context_set_fill_color(ctx, GColorWhite);
		graphics_fill_rect(ctx, GRect(0, 0, width, 2), 0, GCornerNone);
		//think refers to layer bounds 
		//APP_LOG(APP_LOG_LEVEL_INFO,"Done with width %i",width);		
	}
}
static void batteryDisplay()
{
	if (battery)
	{
		layer_set_hidden(batteryLayer,false);
		layer_mark_dirty(batteryLayer);    
	}
	else
		layer_set_hidden(batteryLayer,true);    
}

static void window_load(Window *window)
{
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_frame(window_layer);
  //APP_LOG(APP_LOG_LEVEL_INFO, "Window load:  x %i y %i w %i h %i", bounds.origin.x, bounds.origin.y, bounds.size.w, bounds.size.h);
	//ok, get x 0 y 0 w 144 h 168
	// Init and load lines
	for (int i = 0; i < NUM_LINES; i++)
	{
		init_line(&lines[i]);
		layer_add_child(window_layer, (Layer *)lines[i].currentLayer);
		layer_add_child(window_layer, (Layer *)lines[i].nextLayer);
	}
	tempLayer = text_layer_create(GRect(144, 0, 144, 50));//besser hier, performance?
	text_layer_set_text(tempLayer,tempLayerText); //sonst hat er keinen speicher
	layer_add_child(window_layer, (Layer *)tempLayer);

  deltaLayer = prepareTextLayer (window_layer, deltaLayerText);
  doneLayer = prepareTextLayer (window_layer, doneLayerText);  

  
  batteryLayer = layer_create(GRect(0,35,144,2));
  layer_set_update_proc(batteryLayer, batteryDisplayUpdate);
  layer_add_child(window_layer,batteryLayer);
  batteryDisplay();

	inverter_layer = inverter_layer_create(bounds);
	layer_set_hidden(inverter_layer_get_layer(inverter_layer), !invert);
	layer_add_child(window_layer, inverter_layer_get_layer(inverter_layer));

	// Configure time on init
	time_t raw_time;
	time(&raw_time);
	actual_time = localtime(&raw_time); //where is memory handled
	display_actual_time(actual_time);	
  APP_LOG(APP_LOG_LEVEL_INFO,"Buffer size is  %i",sizeof(sync_buffer));
	Tuplet initial_values[] = {
		TupletInteger(TEXT_ALIGN_KEY, (uint8_t) text_align),
		TupletInteger(INVERT_KEY,     (uint8_t) invert ? 1 : 0),
		TupletInteger(LANGUAGE_KEY,   (uint8_t) lang),
		TupletInteger(DELTA_KEY,		  (uint8_t) delta ? 1 : 0),
		TupletInteger(DONE_KEY,		  (uint8_t) done ? 1 : 0),
		TupletInteger(BATTERY_KEY,		(uint8_t) battery ? 1 : 0),
		TupletInteger(WARNOWN_KEY,		(uint8_t) warnown ? 1 : 0),
		TupletInteger(TIMETABLE_KEY,		(uint8_t) timetable)
	};
  
	app_sync_init(&sync, sync_buffer, sizeof(sync_buffer), initial_values, ARRAY_LENGTH(initial_values),
			sync_tuple_changed_callback, sync_error_callback, NULL);
}

static void window_unload(Window *window)
{
	app_sync_deinit(&sync);

	// Free layers
	inverter_layer_destroy(inverter_layer);
  text_layer_destroy(tempLayer);
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
	if (persist_exists(DELTA_KEY))
	{
		delta =  persist_read_bool(DELTA_KEY);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Read delta from store: %i", delta ? 1 : 0);
	}
	

	window = window_create();
	window_set_background_color(window, GColorBlack);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload
	});
  window_set_fullscreen(window, true); 
	
	// Initialize message queue
	const int inbound_size = sizeof(sync_buffer);
	const int outbound_size = sizeof(sync_buffer);//64;
  //buffer-size problem need some time to localize it 
  
	APP_LOG(APP_LOG_LEVEL_INFO,"max outbox %u max inbox %u garantie outbox %u garantie inbox %u",
	                    (uint) app_message_outbox_size_maximum(),
	                    (uint) app_message_inbox_size_maximum(),
	                    (uint) APP_MESSAGE_OUTBOX_SIZE_MINIMUM,
	                    (uint) APP_MESSAGE_INBOX_SIZE_MINIMUM
	                      );
	APP_LOG(APP_LOG_LEVEL_INFO,"if app_message_open uses below garantie, it's sure that it will work, we use %u",
	        (uint) outbound_size);                      
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

