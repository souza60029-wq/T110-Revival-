#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <lvgl/lvgl.h>
#include <lvgl/drivers/display/lv_linux_fbdev.h>
#include <lvgl/drivers/indev/lv_evdev.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define NBITS_FOR(n) (((n) + (sizeof(unsigned long) * 8U)) / (sizeof(unsigned long) * 8U))
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define COLOR_DESKTOP 0x142131
#define COLOR_PANEL   0x1E3044
#define COLOR_TITLE   0x243C55
#define COLOR_TASKBAR 0x101A27
#define COLOR_ACCENT  0x2D7DD2
#define COLOR_TEXT    0xF0F4F8
#define COLOR_MUTED   0xB6C5D4
#define COLOR_CLOSE   0xB64242

struct touch_device {
    char path[256];
    char name[128];
    int min_x;
    int min_y;
    int max_x;
    int max_y;
};

enum app_kind {
    APP_SYSTEM_INFO = 0,
    APP_SETTINGS = 1
};

struct app_window {
    enum app_kind kind;
    const char *title;
    lv_obj_t *root;
    lv_obj_t *task_button;
    lv_obj_t *content;
    bool created;
    bool open;
    bool minimized;
};

static lv_display_t *g_display;
static lv_obj_t *g_menu;
static lv_obj_t *g_taskbar;
static bool g_menu_open;
static int g_screen_w;
static int g_screen_h;
static char g_fb_summary[192];
static char g_touch_summary[192];
static struct app_window g_windows[] = {
    { .kind = APP_SYSTEM_INFO, .title = "System information" },
    { .kind = APP_SETTINGS, .title = "Settings" }
};

static bool bit_is_set(const unsigned long *bits, unsigned int bit)
{
    const unsigned int word_bits = (unsigned int)(sizeof(unsigned long) * 8U);
    return (bits[bit / word_bits] & (1UL << (bit % word_bits))) != 0;
}

static bool read_input_candidate(const char *path, struct touch_device *out, bool *preferred)
{
    int fd = open(path, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if(fd < 0) return false;

    unsigned long ev_bits[NBITS_FOR(EV_MAX + 1)] = {0};
    unsigned long abs_bits[NBITS_FOR(ABS_MAX + 1)] = {0};
    unsigned long key_bits[NBITS_FOR(KEY_MAX + 1)] = {0};
    char name[128] = {0};
    struct input_absinfo x_info;
    struct input_absinfo y_info;

    bool ok = ioctl(fd, EVIOCGNAME(sizeof(name)), name) >= 0 &&
              ioctl(fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits) >= 0 &&
              bit_is_set(ev_bits, EV_ABS) &&
              ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits) >= 0;

    if(!ok) {
        close(fd);
        return false;
    }

    const bool mt_xy = bit_is_set(abs_bits, ABS_MT_POSITION_X) &&
                       bit_is_set(abs_bits, ABS_MT_POSITION_Y);
    const bool abs_xy = bit_is_set(abs_bits, ABS_X) && bit_is_set(abs_bits, ABS_Y);
    if(!mt_xy && !abs_xy) {
        close(fd);
        return false;
    }

    bool touch_key = false;
    if(bit_is_set(ev_bits, EV_KEY) &&
       ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits) >= 0) {
        touch_key = bit_is_set(key_bits, BTN_TOUCH);
    }
    const bool tracking = mt_xy && bit_is_set(abs_bits, ABS_MT_TRACKING_ID);
    if(!touch_key && !tracking) {
        close(fd);
        return false;
    }

    const unsigned int x_code = mt_xy ? ABS_MT_POSITION_X : ABS_X;
    const unsigned int y_code = mt_xy ? ABS_MT_POSITION_Y : ABS_Y;
    if(ioctl(fd, EVIOCGABS(x_code), &x_info) < 0 ||
       ioctl(fd, EVIOCGABS(y_code), &y_info) < 0 ||
       x_info.maximum <= x_info.minimum || y_info.maximum <= y_info.minimum) {
        close(fd);
        return false;
    }

    const bool is_sec = strcasestr(name, "sec_touchscreen") != NULL;
    const bool is_bt532 = strcasestr(name, "bt532") != NULL;
    *preferred = is_sec || is_bt532;

    snprintf(out->path, sizeof(out->path), "%s", path);
    snprintf(out->name, sizeof(out->name), "%s", name[0] ? name : "unnamed touchscreen");
    out->min_x = x_info.minimum;
    out->min_y = y_info.minimum;
    out->max_x = x_info.maximum;
    out->max_y = y_info.maximum;
    close(fd);
    return true;
}

static bool find_touch_device(struct touch_device *result)
{
    glob_t matches;
    memset(&matches, 0, sizeof(matches));
    if(glob("/dev/input/event*", 0, NULL, &matches) != 0) {
        globfree(&matches);
        return false;
    }

    bool found = false;
    bool found_preferred = false;
    struct touch_device candidate;
    for(size_t i = 0; i < matches.gl_pathc; i++) {
        bool preferred = false;
        if(!read_input_candidate(matches.gl_pathv[i], &candidate, &preferred)) continue;
        if(!found || (preferred && !found_preferred)) {
            *result = candidate;
            found = true;
            found_preferred = preferred;
        }
        if(preferred) break;
    }
    globfree(&matches);
    return found;
}

static bool inspect_framebuffer(void)
{
    struct fb_fix_screeninfo fixed;
    struct fb_var_screeninfo variable;
    int fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);
    if(fd < 0) {
        fprintf(stderr, "goya-shell: cannot open /dev/fb0: %s\n", strerror(errno));
        return false;
    }
    if(ioctl(fd, FBIOGET_FSCREENINFO, &fixed) < 0 ||
       ioctl(fd, FBIOGET_VSCREENINFO, &variable) < 0) {
        fprintf(stderr, "goya-shell: framebuffer ioctl failed: %s\n", strerror(errno));
        close(fd);
        return false;
    }

    snprintf(g_fb_summary, sizeof(g_fb_summary),
             "%ux%u, %u bpp, stride %u, offset %u,%u; R%u@%u G%u@%u B%u@%u",
             variable.xres, variable.yres, variable.bits_per_pixel,
             fixed.line_length, variable.xoffset, variable.yoffset,
             variable.red.length, variable.red.offset,
             variable.green.length, variable.green.offset,
             variable.blue.length, variable.blue.offset);
    fprintf(stderr, "goya-shell: fb0 %s\n", g_fb_summary);
    close(fd);
    return true;
}

static lv_obj_t *make_label(lv_obj_t *parent, const char *text, const lv_font_t *font,
                            lv_color_t color, int width)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_WRAP);
    if(width > 0) lv_obj_set_width(label, width);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    return label;
}

static lv_obj_t *make_clickable(lv_obj_t *parent, const char *caption,
                                int width, int height, uint32_t color,
                                lv_event_cb_t callback, void *user_data)
{
    lv_obj_t *button = lv_obj_create(parent);
    lv_obj_set_size(button, width, height);
    lv_obj_set_style_bg_color(button, lv_color_hex(color), 0);
    lv_obj_set_style_bg_opa(button, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(button, 0, 0);
    lv_obj_set_style_radius(button, 8, 0);
    lv_obj_set_style_pad_all(button, 4, 0);
    lv_obj_add_flag(button, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(button, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_remove_flag(button, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(button, callback, LV_EVENT_CLICKED, user_data);

    lv_obj_t *label = make_label(button, caption, &lv_font_montserrat_14,
                                 lv_color_hex(COLOR_TEXT), width - 8);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
    return button;
}

static void hide_menu(void)
{
    if(g_menu) lv_obj_add_flag(g_menu, LV_OBJ_FLAG_HIDDEN);
    g_menu_open = false;
}

static void show_window(struct app_window *window);

static void on_open_info(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    show_window(&g_windows[APP_SYSTEM_INFO]);
    hide_menu();
}

static void on_open_settings(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    show_window(&g_windows[APP_SETTINGS]);
    hide_menu();
}

static void on_toggle_menu(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED || !g_menu) return;
    g_menu_open = !g_menu_open;
    if(g_menu_open) lv_obj_remove_flag(g_menu, LV_OBJ_FLAG_HIDDEN);
    else lv_obj_add_flag(g_menu, LV_OBJ_FLAG_HIDDEN);
}

static void on_focus_window(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_PRESSED) return;
    struct app_window *window = lv_event_get_user_data(event);
    if(window && window->root) lv_obj_move_to_index(window->root, -1);
}

static void on_task_button(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    struct app_window *window = lv_event_get_user_data(event);
    if(!window || !window->root) return;
    if(window->minimized) {
        window->minimized = false;
        lv_obj_remove_flag(window->root, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_to_index(window->root, -1);
    }
    else {
        window->minimized = true;
        lv_obj_add_flag(window->root, LV_OBJ_FLAG_HIDDEN);
    }
}

static void on_minimize_window(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    struct app_window *window = lv_event_get_user_data(event);
    if(!window || !window->root) return;
    window->minimized = true;
    lv_obj_add_flag(window->root, LV_OBJ_FLAG_HIDDEN);
}

static void on_close_window(lv_event_t *event)
{
    if(lv_event_get_code(event) != LV_EVENT_CLICKED) return;
    struct app_window *window = lv_event_get_user_data(event);
    if(!window || !window->root) return;
    window->open = false;
    window->minimized = false;
    lv_obj_add_flag(window->root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(window->task_button, LV_OBJ_FLAG_HIDDEN);
}

static void create_window(struct app_window *window)
{
    const int width = (g_screen_w > 780) ? 740 : g_screen_w - 24;
    const int height = (g_screen_h > 500) ? 410 : g_screen_h - 112;
    const int safe_width = width > 280 ? width : 280;
    const int safe_height = height > 220 ? height : 220;

    window->root = lv_obj_create(lv_screen_active());
    lv_obj_set_size(window->root, safe_width, safe_height);
    lv_obj_set_style_bg_color(window->root, lv_color_hex(COLOR_PANEL), 0);
    lv_obj_set_style_bg_opa(window->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(window->root, lv_color_hex(0x456079), 0);
    lv_obj_set_style_border_width(window->root, 1, 0);
    lv_obj_set_style_radius(window->root, 10, 0);
    lv_obj_set_style_pad_all(window->root, 0, 0);
    lv_obj_remove_flag(window->root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(window->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_align(window->root, LV_ALIGN_CENTER, 0, -4);
    lv_obj_add_event_cb(window->root, on_focus_window, LV_EVENT_PRESSED, window);

    lv_obj_t *titlebar = lv_obj_create(window->root);
    lv_obj_set_size(titlebar, safe_width, 48);
    lv_obj_align(titlebar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(titlebar, lv_color_hex(COLOR_TITLE), 0);
    lv_obj_set_style_bg_opa(titlebar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(titlebar, 0, 0);
    lv_obj_set_style_radius(titlebar, 8, 0);
    lv_obj_set_style_pad_all(titlebar, 6, 0);
    lv_obj_remove_flag(titlebar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(titlebar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(titlebar, on_focus_window, LV_EVENT_PRESSED, window);

    lv_obj_t *title = make_label(titlebar, window->title, &lv_font_montserrat_18,
                                 lv_color_hex(COLOR_TEXT), safe_width - 132);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 8, 0);

    lv_obj_t *minimize = make_clickable(titlebar, "_", 44, 36, 0x38526A,
                                        on_minimize_window, window);
    lv_obj_align(minimize, LV_ALIGN_RIGHT_MID, -52, 0);
    lv_obj_t *close = make_clickable(titlebar, "X", 44, 36, COLOR_CLOSE,
                                     on_close_window, window);
    lv_obj_align(close, LV_ALIGN_RIGHT_MID, -4, 0);

    window->content = make_label(window->root, "", &lv_font_montserrat_14,
                                 lv_color_hex(COLOR_TEXT), safe_width - 36);
    lv_obj_set_style_text_line_space(window->content, 6, 0);
    lv_obj_align(window->content, LV_ALIGN_TOP_LEFT, 16, 64);

    window->task_button = make_clickable(g_taskbar, window->title,
                                        168, 42, 0x2B4055,
                                        on_task_button, window);
    lv_obj_align(window->task_button, LV_ALIGN_LEFT_MID,
                 112 + (int)window->kind * 176, 0);
    lv_obj_remove_flag(window->task_button, LV_OBJ_FLAG_HIDDEN);
    window->created = true;
    lv_obj_add_flag(window->root, LV_OBJ_FLAG_HIDDEN);
}

static void update_window_content(struct app_window *window)
{
    char text[640];
    if(window->kind == APP_SYSTEM_INFO) {
        snprintf(text, sizeof(text),
                 "GoyaOS V2 desktop shell\n\n"
                 "Framebuffer: %s\n\n"
                 "Touch input: %s\n\n"
                 "USB Ethernet: %s\n"
                 "Input calibration follows the touchscreen's reported absolute range.",
                 g_fb_summary,
                 g_touch_summary[0] ? g_touch_summary : "not detected (display remains available)",
                 access("/sys/class/net/usb0", F_OK) == 0 ? "usb0 present" : "waiting for the USB host");
    }
    else {
        snprintf(text, sizeof(text),
                 "Touch-friendly desktop controls\n\n"
                 "CPU governor: ondemand\n"
                 "Swappiness: 60\n"
                 "Compressed swap: zRAM, kernel default size (25%% of RAM)\n\n"
                 "USB networking: device-mode gadget; SSH accepts public keys only.\n"
                 "Use the taskbar to minimize, restore, focus, or close this panel.");
    }
    lv_label_set_text(window->content, text);
}

static void show_window(struct app_window *window)
{
    if(!window->created) create_window(window);
    update_window_content(window);
    window->open = true;
    window->minimized = false;
    lv_obj_remove_flag(window->root, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(window->task_button, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_to_index(window->root, -1);
}

static void build_desktop(void)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(COLOR_DESKTOP), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *topbar = lv_obj_create(screen);
    lv_obj_set_size(topbar, g_screen_w, 46);
    lv_obj_align(topbar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(topbar, lv_color_hex(COLOR_TITLE), 0);
    lv_obj_set_style_bg_opa(topbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topbar, 0, 0);
    lv_obj_set_style_radius(topbar, 0, 0);
    lv_obj_set_style_pad_all(topbar, 6, 0);
    lv_obj_remove_flag(topbar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *brand = make_label(topbar, "GOYA OS  V2", &lv_font_montserrat_18,
                                 lv_color_hex(COLOR_TEXT), 220);
    lv_obj_align(brand, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *device = make_label(topbar, "SM-T110  |  framebuffer + touchscreen",
                                  &lv_font_montserrat_14,
                                  lv_color_hex(COLOR_MUTED), g_screen_w - 260);
    lv_obj_align(device, LV_ALIGN_RIGHT_MID, -12, 0);

    g_taskbar = lv_obj_create(screen);
    lv_obj_set_size(g_taskbar, g_screen_w, 58);
    lv_obj_align(g_taskbar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(g_taskbar, lv_color_hex(COLOR_TASKBAR), 0);
    lv_obj_set_style_bg_opa(g_taskbar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_taskbar, 0, 0);
    lv_obj_set_style_radius(g_taskbar, 0, 0);
    lv_obj_set_style_pad_all(g_taskbar, 6, 0);
    lv_obj_remove_flag(g_taskbar, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *start = make_clickable(g_taskbar, "Menu", 100, 44,
                                     COLOR_ACCENT, on_toggle_menu, NULL);
    lv_obj_align(start, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_t *status = make_label(g_taskbar, "Touch desktop  |  T110",
                                  &lv_font_montserrat_14,
                                  lv_color_hex(COLOR_MUTED), 220);
    lv_obj_align(status, LV_ALIGN_RIGHT_MID, -10, 0);

    g_menu = lv_obj_create(screen);
    lv_obj_set_size(g_menu, 238, 166);
    lv_obj_align(g_menu, LV_ALIGN_BOTTOM_LEFT, 8, -64);
    lv_obj_set_style_bg_color(g_menu, lv_color_hex(0x20334A), 0);
    lv_obj_set_style_bg_opa(g_menu, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(g_menu, lv_color_hex(0x456079), 0);
    lv_obj_set_style_border_width(g_menu, 1, 0);
    lv_obj_set_style_radius(g_menu, 10, 0);
    lv_obj_set_style_pad_all(g_menu, 10, 0);
    lv_obj_remove_flag(g_menu, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *menu_title = make_label(g_menu, "Applications", &lv_font_montserrat_18,
                                      lv_color_hex(COLOR_TEXT), 210);
    lv_obj_align(menu_title, LV_ALIGN_TOP_LEFT, 6, 4);
    lv_obj_t *info = make_clickable(g_menu, "System information", 214, 44,
                                    0x2B4055, on_open_info, NULL);
    lv_obj_align(info, LV_ALIGN_TOP_LEFT, 2, 42);
    lv_obj_t *settings = make_clickable(g_menu, "Settings", 214, 44,
                                        0x2B4055, on_open_settings, NULL);
    lv_obj_align(settings, LV_ALIGN_TOP_LEFT, 2, 94);
    lv_obj_add_flag(g_menu, LV_OBJ_FLAG_HIDDEN);
}

int main(void)
{
    struct touch_device touch;
    memset(&touch, 0, sizeof(touch));

    if(!inspect_framebuffer()) return EXIT_FAILURE;
    const bool have_touch = find_touch_device(&touch);
    if(have_touch) {
        snprintf(g_touch_summary, sizeof(g_touch_summary), "%s (%s), X %d..%d, Y %d..%d",
                 touch.name, touch.path, touch.min_x, touch.max_x, touch.min_y, touch.max_y);
        fprintf(stderr, "goya-shell: touchscreen %s\n", g_touch_summary);
    }
    else {
        snprintf(g_touch_summary, sizeof(g_touch_summary), "no EVDEV touchscreen with absolute coordinates found");
        fprintf(stderr, "goya-shell: %s\n", g_touch_summary);
    }

    lv_init();
    g_display = lv_linux_fbdev_create();
    if(g_display == NULL || lv_linux_fbdev_set_file(g_display, "/dev/fb0") != LV_RESULT_OK) {
        fprintf(stderr, "goya-shell: LVGL could not initialize fb0\n");
        return EXIT_FAILURE;
    }
    g_screen_w = lv_display_get_horizontal_resolution(g_display);
    g_screen_h = lv_display_get_vertical_resolution(g_display);
    if(g_screen_w < 280 || g_screen_h < 220) {
        fprintf(stderr, "goya-shell: unsupported display size %dx%d\n", g_screen_w, g_screen_h);
        return EXIT_FAILURE;
    }

    if(have_touch) {
        lv_indev_t *input = lv_evdev_create(LV_INDEV_TYPE_POINTER, touch.path);
        if(input != NULL) {
            lv_indev_set_display(input, g_display);
            lv_evdev_set_calibration(input, touch.min_x, touch.min_y, touch.max_x, touch.max_y);
        }
        else {
            fprintf(stderr, "goya-shell: LVGL could not open %s\n", touch.path);
        }
    }

    fprintf(stderr, "goya-shell: starting desktop at %dx%d\n", g_screen_w, g_screen_h);
    build_desktop();
    for(;;) {
        uint32_t delay_ms = lv_timer_handler();
        if(delay_ms < 5) delay_ms = 5;
        if(delay_ms > 50) delay_ms = 50;
        struct timespec delay = { .tv_sec = delay_ms / 1000,
                                  .tv_nsec = (long)(delay_ms % 1000) * 1000000L };
        nanosleep(&delay, NULL);
    }
    return EXIT_SUCCESS;
}
