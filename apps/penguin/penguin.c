#include "penguin_icons.h"
#include <furi.h>
#include <furi_hal.h>

#include <gui/gui.h>
#include <input/input.h>

typedef struct {
    uint8_t x, y;
} ImagePosition;

typedef struct {
    float velx, vely;
} Velocity;

typedef struct {
    uint8_t width, height;
} Dimensions;

const Dimensions screen_dimensions = {.width = 128, .height = 64};

const Dimensions image_dimensions = {.width = 32, .height = 32};

static ImagePosition image_position = {.x = (screen_dimensions.width / 2) - (image_dimensions.width / 2), .y = (screen_dimensions.height / 2) - (image_dimensions.height / 2)};

static Velocity velocity = {.velx = 0, .vely = 0};

static void app_draw_callback(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);
    canvas_draw_icon(canvas, image_position.x % screen_dimensions.width, image_position.y % screen_dimensions.height, &I_penguin);
}

static void app_input_callback(InputEvent* input_event, void* ctx) {
    furi_assert(ctx);

    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

int32_t penguin_main(void* p) {
    UNUSED(p);
    FuriMessageQueue* event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));

    // Configure viewport
    ViewPort* view_port = view_port_alloc();
    view_port_draw_callback_set(view_port, app_draw_callback, view_port);
    view_port_input_callback_set(view_port, app_input_callback, event_queue);

    // Register viewport in GUI
    Gui* gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(gui, view_port, GuiLayerFullscreen);

	InputEvent event;

	bool running = true;
	while(running) {
		if(furi_message_queue_get(event_queue, &event, 100) == FuriStatusOk) {
			if((event.type == InputTypePress) || (event.type == InputTypeRepeat)) {
				switch(event.key) {
				case InputKeyLeft:
					velocity.velx = -10;
					break;
				case InputKeyRight:
					velocity.velx = 10;
					break;
				case InputKeyUp:
					velocity.vely = -5;
					break;
				case InputKeyDown:
					velocity.vely = 5;
					break;
				default:
					running = false;
					break;
				}
			}
		}
		// Velocity calculations
		if(velocity.velx > 0) {
			image_position.x += velocity.velx;
			velocity.velx -= 1;
		}
		if(velocity.velx < 0) {
			image_position.x += velocity.velx;
			velocity.velx += 1;
		}
		if(velocity.vely > 0) {
			image_position.y += velocity.vely;
			velocity.vely -= 1;
		}
		if(velocity.vely < 0) {
			image_position.y += velocity.vely;
			velocity.vely += 1;
		}
		
		//AABB
		if(image_position.y + image_dimensions.height > screen_dimensions.height) {
			image_position.y = screen_dimensions.height - image_dimensions.height;
		}

		if(image_position.x + image_dimensions.width > screen_dimensions.width) {
			image_position.x = screen_dimensions.width - image_dimensions.width;
		}
		
		view_port_update(view_port);
	}
	
	view_port_enabled_set(view_port, false);
    gui_remove_view_port(gui, view_port);
    view_port_free(view_port);
    furi_message_queue_free(event_queue);

    furi_record_close(RECORD_GUI);

    return 0;
}