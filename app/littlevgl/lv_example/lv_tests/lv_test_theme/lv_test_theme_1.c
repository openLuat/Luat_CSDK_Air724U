/**
 * @file lv_test_theme_1.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_test_theme_1.h"
#include "iot_debug.h"

#if LV_USE_TESTS
/*********************
 *      DEFINES
 *********************/
LV_IMG_DECLARE(logo);

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void create_tab1(lv_obj_t * parent);
static void create_tab2(lv_obj_t * parent);
static void create_tab3(lv_obj_t * parent);
static void tv_switch(lv_task_t *task);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_group_t *g;
static lv_task_t *t;
static lv_obj_t *scr0;
static lv_obj_t *scr1;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

static void tv_switch(lv_task_t *task)
{
    lv_disp_load_scr(scr1);
}

/**
 * Create a test screen with a lot objects and apply the given theme on them
 * @param th pointer to a theme
 */
void start()
{
    scr0 = lv_cont_create(NULL, NULL);

    // lv_obj_t *con = lv_cont_create(scr0, NULL);
    lv_obj_t *tv0 = lv_tabview_create(scr0, NULL);

	lv_obj_set_size(tv0, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));

	lv_obj_t *tab0 = lv_tabview_add_tab(tv0, "LOGO");

	lv_tabview_set_btns_hidden(tv0, true);

#if 0
    static lv_style_t c_style;
    lv_style_copy(&c_style, &lv_style_transp_fit);
    c_style.body.padding.inner = 0;
    c_style.body.padding.left = 0;
    c_style.body.padding.right = 0;
    c_style.body.padding.top = 0;
    c_style.body.padding.bottom = 0;

    lv_obj_set_style(con, &c_style);
    lv_obj_set_click(con, false);

    lv_cont_set_fit(con, LV_FIT_TIGHT);
    lv_cont_set_layout(con, LV_LAYOUT_COL_M);

    lv_obj_t *img = lv_img_create(con, NULL);
    lv_img_set_src(img, &logo);
#endif

	create_tab2(tab0);

    scr1 = lv_cont_create(NULL, NULL);

    lv_obj_t * tv1 = lv_tabview_create(scr1, NULL);
    lv_group_add_obj(g, tv1);

    lv_obj_set_size(tv1, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
    lv_obj_t * tab1 = lv_tabview_add_tab(tv1, "AT");
    lv_group_add_obj(g, tab1);

    lv_obj_t * tab2 = lv_tabview_add_tab(tv1, "LUA");
    lv_group_add_obj(g, tab2);

    lv_obj_t * tab3 = lv_tabview_add_tab(tv1, "CSDK");
    lv_group_add_obj(g, tab3);

    lv_tabview_set_btns_hidden(tv1, true);

    create_tab1(tab1);
    create_tab2(tab2);
    create_tab3(tab3);

    lv_disp_load_scr(scr0);

    t = lv_task_create(tv_switch, 3000, LV_TASK_PRIO_MID, NULL);
    lv_task_once(t);

}

void lv_test_theme_1(lv_theme_t * th, lv_indev_t *kp_indev)
{
    g = lv_group_create();

    lv_indev_set_group(kp_indev, g);

    lv_theme_set_current(th);

    start();
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static void create_tab1(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY);

    lv_theme_t * th = lv_theme_get_current();

    static lv_style_t h_style;
    lv_style_copy(&h_style, &lv_style_transp);
    h_style.body.padding.inner = LV_DPI / 10;
    h_style.body.padding.left = LV_DPI / 4;
    h_style.body.padding.right = LV_DPI / 4;
    h_style.body.padding.top = LV_DPI / 10;
    h_style.body.padding.bottom = LV_DPI / 10;

    lv_obj_t * h = lv_cont_create(parent, NULL);
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);

    lv_obj_t * btn = lv_btn_create(h, NULL);
    lv_btn_set_fit(btn, LV_FIT_TIGHT);
    lv_btn_set_toggle(btn, true);
    lv_obj_t * btn_label = lv_label_create(btn, NULL);
    lv_label_set_text(btn_label, "Button");
    lv_group_add_obj(g, btn);

    btn = lv_btn_create(h, btn);
    lv_btn_toggle(btn);
    btn_label = lv_label_create(btn, NULL);
    lv_label_set_text(btn_label, "Toggled");
    lv_group_add_obj(g, btn);

    btn = lv_btn_create(h, btn);
    lv_btn_set_state(btn, LV_BTN_STATE_INA);
    btn_label = lv_label_create(btn, NULL);
    lv_label_set_text(btn_label, "Inactive");
    lv_group_add_obj(g, btn);

    lv_obj_t * label = lv_label_create(h, NULL);
    lv_label_set_text(label, "Primary");
    lv_obj_set_style(label, th->style.label.prim);
    // lv_group_add_obj(g, label);

    label = lv_label_create(h, NULL);
    lv_label_set_text(label, "Secondary");
    lv_obj_set_style(label, th->style.label.sec);
    // lv_group_add_obj(g, label);

    label = lv_label_create(h, NULL);
    lv_label_set_text(label, "Hint");
    lv_obj_set_style(label, th->style.label.hint);
    // lv_group_add_obj(g, label);

    static const char * btnm_str0[] = {LV_SYMBOL_OK, LV_SYMBOL_CLOSE, LV_SYMBOL_PLUS, ""};
    lv_obj_t * btnm0 = lv_btnm_create(h, NULL);
    lv_obj_set_size(btnm0, lv_disp_get_hor_res(NULL) / 2, lv_disp_get_ver_res(NULL) / 4);
    lv_btnm_set_map(btnm0, btnm_str0);
    lv_btnm_set_btn_ctrl_all(btnm0, LV_BTNM_CTRL_TGL_ENABLE);
    lv_btnm_set_one_toggle(btnm0, true);
    lv_group_add_obj(g, btnm0);

    static const char * btnm_str1[] = {"1", "2", "3", ""};
    lv_obj_t * btnm1 = lv_btnm_create(h, NULL);
    lv_obj_set_size(btnm1, lv_disp_get_hor_res(NULL) / 2, lv_disp_get_ver_res(NULL) / 4);
    lv_btnm_set_map(btnm1, btnm_str1);
    lv_btnm_set_btn_ctrl_all(btnm1, LV_BTNM_CTRL_TGL_ENABLE);
    lv_btnm_set_one_toggle(btnm1, true);
    lv_group_add_obj(g, btnm1);

    lv_obj_t * table = lv_table_create(h, NULL);
    lv_table_set_col_cnt(table, 3);
    lv_table_set_row_cnt(table, 4);
    lv_table_set_col_width(table, 0, LV_DPI / 3);
    lv_table_set_col_width(table, 1, LV_DPI / 2);
    lv_table_set_col_width(table, 2, LV_DPI / 2);
    lv_table_set_cell_merge_right(table, 0, 0, true);
    lv_table_set_cell_merge_right(table, 0, 1, true);

    lv_table_set_cell_value(table, 0, 0, "Table");
    lv_table_set_cell_align(table, 0, 0, LV_LABEL_ALIGN_CENTER);

    lv_table_set_cell_value(table, 1, 0, "1");
    lv_table_set_cell_value(table, 1, 1, "13");
    lv_table_set_cell_align(table, 1, 1, LV_LABEL_ALIGN_RIGHT);
    lv_table_set_cell_value(table, 1, 2, "ms");

    lv_table_set_cell_value(table, 2, 0, "2");
    lv_table_set_cell_value(table, 2, 1, "46");
    lv_table_set_cell_align(table, 2, 1, LV_LABEL_ALIGN_RIGHT);
    lv_table_set_cell_value(table, 2, 2, "ms");

    lv_table_set_cell_value(table, 3, 0, "3");
    lv_table_set_cell_value(table, 3, 1, "61");
    lv_table_set_cell_align(table, 3, 1, LV_LABEL_ALIGN_RIGHT);
    lv_table_set_cell_value(table, 3, 2, "ms");

    lv_obj_t * ddlist = lv_ddlist_create(h, NULL);
    lv_ddlist_set_fix_width(ddlist, lv_obj_get_width(ddlist) + LV_DPI / 2);   /*Make space for the arrow*/
    lv_ddlist_set_draw_arrow(ddlist, true);

    h = lv_cont_create(parent, h);

    lv_obj_t * list = lv_list_create(h, NULL);
    lv_group_add_obj(g, list);
    lv_obj_set_size(list, lv_disp_get_hor_res(NULL) / 2 + 20, lv_disp_get_ver_res(NULL) / 2);
    lv_obj_t * list_btn;
    list_btn = lv_list_add_btn(list, LV_SYMBOL_GPS,  "GPS");
    lv_btn_set_toggle(list_btn, true);

    lv_list_add_btn(list, LV_SYMBOL_WIFI, "WiFi");
    lv_list_add_btn(list, LV_SYMBOL_GPS, "GPS");
    lv_list_add_btn(list, LV_SYMBOL_AUDIO, "Audio");
    lv_list_add_btn(list, LV_SYMBOL_VIDEO, "Video");
    lv_list_add_btn(list, LV_SYMBOL_CALL, "Call");
    lv_list_add_btn(list, LV_SYMBOL_BELL, "Bell");
    lv_list_add_btn(list, LV_SYMBOL_FILE, "File");
    lv_list_add_btn(list, LV_SYMBOL_EDIT, "Edit");
    lv_list_add_btn(list, LV_SYMBOL_CUT,  "Cut");
    lv_list_add_btn(list, LV_SYMBOL_COPY, "Copy");

    lv_obj_t * roller = lv_roller_create(h, NULL);
    lv_group_add_obj(g, roller);
    lv_roller_set_options(roller, "Monday\nTuesday\nWednesday\nThursday\nFriday\nSaturday\nSunday", true);
    lv_roller_set_selected(roller, 1, false);
    lv_roller_set_visible_row_count(roller, 3);


}

static void create_tab2(lv_obj_t * parent)
{
#if 0
    lv_coord_t w = lv_page_get_scrl_width(parent);
    /*
    lv_obj_t * chart = lv_chart_create(parent, NULL);
    lv_chart_set_type(chart, LV_CHART_TYPE_AREA);
    lv_obj_set_size(chart, w / 3, lv_disp_get_ver_res(NULL) / 3);
    lv_obj_set_pos(chart, LV_DPI / 10, LV_DPI / 10);
    lv_chart_series_t * s1 = lv_chart_add_series(chart, LV_COLOR_RED);
    lv_chart_set_next(chart, s1, 30);
    lv_chart_set_next(chart, s1, 20);
    lv_chart_set_next(chart, s1, 10);
    lv_chart_set_next(chart, s1, 12);
    lv_chart_set_next(chart, s1, 20);
    lv_chart_set_next(chart, s1, 27);
    lv_chart_set_next(chart, s1, 35);
    lv_chart_set_next(chart, s1, 55);
    lv_chart_set_next(chart, s1, 70);
    lv_chart_set_next(chart, s1, 75);


    lv_obj_t * gauge = lv_gauge_create(parent, NULL);
    lv_gauge_set_value(gauge, 0, 40);
    lv_obj_set_size(gauge, w / 4, w / 4);
    lv_obj_align(gauge, chart, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);


    lv_obj_t * arc = lv_arc_create(parent, NULL);
    lv_obj_align(arc, gauge, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    */

    lv_obj_t * ta = lv_ta_create(parent, NULL);
    lv_obj_set_size(ta, w / 3, lv_disp_get_ver_res(NULL) / 4);
    lv_obj_align(ta, NULL, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_ta_set_cursor_type(ta, LV_CURSOR_BLOCK);

    lv_obj_t * kb = lv_kb_create(parent, NULL);
    lv_group_add_obj(g, kb);
    lv_obj_set_size(kb, w, lv_disp_get_ver_res(NULL) / 2);
    lv_obj_align(kb, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_kb_set_ta(kb, ta);

#if 0 // LV_USE_ANIMATION
    lv_obj_t * loader = lv_preload_create(parent, NULL);
    lv_obj_align(loader, NULL, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
#endif
#endif

    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY);
    
    static lv_style_t h_style;
    lv_style_copy(&h_style, &lv_style_transp);
    h_style.body.padding.inner = LV_DPI / 100;
    h_style.body.padding.left = LV_DPI / 4;
    h_style.body.padding.right = LV_DPI / 4;
    h_style.body.padding.top = LV_DPI / 50;
    h_style.body.padding.bottom = LV_DPI / 50;

    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &h_style);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &h_style);

    lv_page_set_scrl_fit(parent, LV_FIT_TIGHT);
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

    lv_obj_t * h = lv_cont_create(parent, NULL);
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);

    lv_obj_t *img = lv_img_create(h, NULL);
    lv_img_set_src(img, &logo);
	// lv_obj_set_size(img, 128, 100);
    // lv_obj_align(img, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    //lv_obj_t *label = lv_label_create(h, img);
    //lv_label_set_text(label, "img");
    //lv_obj_align(label, img, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
}


static void create_tab3(lv_obj_t * parent)
{
    lv_page_set_scrl_layout(parent, LV_LAYOUT_PRETTY);

    lv_theme_t * th = lv_theme_get_current();
    // lv_coord_t w = lv_page_get_scrl_width(parent);

    static lv_style_t h_style;
    lv_style_copy(&h_style, &lv_style_transp_fit);
    h_style.body.padding.inner = LV_DPI / 10;
    h_style.body.padding.left = LV_DPI / 4;
    h_style.body.padding.right = LV_DPI / 4;
    h_style.body.padding.top = LV_DPI / 10;
    h_style.body.padding.bottom = LV_DPI / 10;

    lv_page_set_style(parent, LV_PAGE_STYLE_BG, &h_style);
    lv_page_set_style(parent, LV_PAGE_STYLE_SCRL, &h_style);

    lv_page_set_scrl_fit(parent, LV_FIT_TIGHT);
    lv_page_set_sb_mode(parent, LV_SB_MODE_OFF);

	lv_obj_t * h = lv_cont_create(parent, NULL);
    lv_obj_set_style(h, &h_style);
    lv_obj_set_click(h, false);
    lv_cont_set_fit(h, LV_FIT_TIGHT);
    lv_cont_set_layout(h, LV_LAYOUT_COL_M);

    /*Create a Label in the Window*/
	static lv_style_t l_style;
	lv_style_copy(&l_style, &lv_style_plain_color);
    lv_obj_t *label0 = lv_label_create(h, NULL);
    lv_label_set_text(label0, "#ff0000 Luat#");
    // lv_obj_set_style(label0, &l_style);
    lv_label_set_recolor(label0, true);

    lv_obj_t *label1 = lv_label_create(h, label0);
    lv_label_set_text(label1, "#aabbccdd Make# #00ff00 life#\n#889977 simple# #11aaff again#!");
    lv_obj_set_style(label1, th->style.label.prim);
	lv_obj_align(label1, label1, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
	lv_label_set_recolor(label1, true);
    // lv_obj_set_size(label1, w, lv_disp_get_ver_res(NULL) / 2 + 40);

}

#endif /*LV_USE_TESTS*/
