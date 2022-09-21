#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <poll.h>

#include "lvgl.h"
#include "wayland.h"


#define SCREEN_HOR_SIZE (1280)
#define SCREEN_VER_SIZE (1024)

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

static void lv_example_btn_1(void)
{
    lv_obj_t * label;

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_CENTER, 0, -40);

    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn2, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);

    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);
}

#define LVGL_TIMER_IN_MS 10
static void lvgl_timer_thread(void *args)
{
	while(1)
	{
		usleep(LVGL_TIMER_IN_MS*1000);
		lv_tick_inc(LVGL_TIMER_IN_MS);
	}
}

int main(int argc, char **argv)
{
	struct pollfd pfd;
	uint32_t time_till_next;
	int sleep;
	pthread_t lvgl_timer_inc_thread;

	printf("Bryton Rider 860 main application test-pal\n");

	printf("Init LVGL\n");
	lv_init();
	printf("Init LVGL Wayland\n");
	lv_wayland_init();

	/* Create a display */
	printf("Create wayland window\n");
	lv_disp_t * disp = lv_wayland_create_window(SCREEN_HOR_SIZE, SCREEN_VER_SIZE, "Bryton rider 860", NULL /*close_cb*/);

	if(disp == NULL)
	{
		printf("Error: create wayland window failed\n");
		return -1;
	}

	printf("Draw example button on screen\n");
	lv_example_btn_1();

	printf("Set window in fullscreen mode\n");
	lv_wayland_window_set_fullscreen(disp, true);

	printf("Start timer thread\n");
	if(pthread_create(&lvgl_timer_inc_thread, NULL, lvgl_timer_thread, NULL) != 0)
	{
		printf("Start lvgl_timer_thread failed\n");
		return -2;
	}

	printf("Run main loop\n");
	pfd.fd = lv_wayland_get_fd();
	pfd.events = POLLIN;

	while (1) {
		/* Handle any Wayland/LVGL timers/events */
		time_till_next = lv_wayland_timer_handler();

		/* Run until the last window closes */
		if (!lv_wayland_window_is_open(NULL)) {
			break;
		}

		/* Wait for something interesting to happen */
		if (time_till_next == LV_NO_TIMER_READY) {
			sleep = -1;
		} else if (time_till_next > INT_MAX) {
			sleep = INT_MAX;
		} else {
			sleep = time_till_next;
		}

		while ((poll(&pfd, 1, sleep) < 0) && (errno == EINTR));
	}

	return 0;
}
