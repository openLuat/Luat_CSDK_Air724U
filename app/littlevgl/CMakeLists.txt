# Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
# All rights reserved.
#
# This software is supplied "AS IS" without any warranties.
# RDA assumes no responsibility or liability for the use of the software,
# conveys no license or title under any patent, copyright, or mask work
# right to the product. RDA reserves the right to make changes in the
# software without notification.  RDA also make no representation or
# warranty that such application will be suitable for the specified use
# without further testing or modification.

#/*+\ADD\chenzhimin\2020.07.9\ */
target_include_directories(${target} PUBLIC ../../api/include)
target_include_directories(${target} PUBLIC ../components/newlib/include)
target_include_directories(${target} PUBLIC inc)
#/*-\ADD\chenzhimin\2020.07.9\ */

target_include_directories(${target} PUBLIC ${CMAKE_CURRENT_SRC_DIR} lvgl include
	lv_example/lv_apps/benchmark 
    lv_example/lv_apps/demo 
    lv_example/lv_apps/sysmon 
    lv_example/lv_apps/terminal 
    lv_example/lv_apps/tpcal
	lv_example/lv_tests 
    lv_example/lv_tests/lv_test_group 
    lv_example/lv_tests/lv_test_misc 
    lv_example/lv_tests/lv_test_obj 
    lv_example/lv_tests/lv_test_objx 
    lv_example/lv_tests/lv_test_stress 
    lv_example/lv_tests/lv_test_theme
	lv_example/lv_tests/lv_test_objx/lv_test_arc
	lv_example/lv_tests/lv_test_objx/lv_test_bar
	lv_example/lv_tests/lv_test_objx/lv_test_btn
	lv_example/lv_tests/lv_test_objx/lv_test_btnm
	lv_example/lv_tests/lv_test_objx/lv_test_canvas
	lv_example/lv_tests/lv_test_objx/lv_test_cb
	lv_example/lv_tests/lv_test_objx/lv_test_chart
	lv_example/lv_tests/lv_test_objx/lv_test_cont
	lv_example/lv_tests/lv_test_objx/lv_test_cpicker
	lv_example/lv_tests/lv_test_objx/lv_test_ddlist
	lv_example/lv_tests/lv_test_objx/lv_test_gauge
	lv_example/lv_tests/lv_test_objx/lv_test_img
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn
	lv_example/lv_tests/lv_test_objx/lv_test_kb
	lv_example/lv_tests/lv_test_objx/lv_test_label
	lv_example/lv_tests/lv_test_objx/lv_test_led
	lv_example/lv_tests/lv_test_objx/lv_test_line
	lv_example/lv_tests/lv_test_objx/lv_test_list
	lv_example/lv_tests/lv_test_objx/lv_test_lmeter
	lv_example/lv_tests/lv_test_objx/lv_test_mbox
	lv_example/lv_tests/lv_test_objx/lv_test_page
	lv_example/lv_tests/lv_test_objx/lv_test_preload
	lv_example/lv_tests/lv_test_objx/lv_test_roller
	lv_example/lv_tests/lv_test_objx/lv_test_slider
	lv_example/lv_tests/lv_test_objx/lv_test_sw
	lv_example/lv_tests/lv_test_objx/lv_test_ta
	lv_example/lv_tests/lv_test_objx/lv_test_table
	lv_example/lv_tests/lv_test_objx/lv_test_tabview
	lv_example/lv_tests/lv_test_objx/lv_test_tileview
	lv_example/lv_tests/lv_test_objx/lv_test_win)

target_sources(${target} PUBLIC
    lvgl/src/lv_core/lv_group.c
    lvgl/src/lv_core/lv_indev.c
    lvgl/src/lv_core/lv_disp.c
    lvgl/src/lv_core/lv_obj.c
    lvgl/src/lv_core/lv_refr.c
    lvgl/src/lv_core/lv_style.c
    lvgl/src/lv_core/lv_debug.c
    lvgl/src/lv_draw/lv_draw_basic.c
    lvgl/src/lv_draw/lv_draw.c
    lvgl/src/lv_draw/lv_draw_rect.c
    lvgl/src/lv_draw/lv_draw_label.c
    lvgl/src/lv_draw/lv_draw_line.c
    lvgl/src/lv_draw/lv_draw_img.c
    lvgl/src/lv_draw/lv_draw_arc.c
    lvgl/src/lv_draw/lv_draw_triangle.c
    lvgl/src/lv_draw/lv_img_decoder.c
    lvgl/src/lv_draw/lv_img_cache.c
    lvgl/src/lv_font/lv_font.c
    lvgl/src/lv_font/lv_font_fmt_txt.c
    lvgl/src/lv_font/lv_font_roboto_12.c
    lvgl/src/lv_font/lv_font_roboto_16.c
    lvgl/src/lv_font/lv_font_roboto_22.c
    lvgl/src/lv_font/lv_font_roboto_28.c
    lvgl/src/lv_font/lv_font_unscii_8.c
    lvgl/src/lv_hal/lv_hal_disp.c
    lvgl/src/lv_hal/lv_hal_indev.c
    lvgl/src/lv_hal/lv_hal_tick.c
    lvgl/src/lv_misc/lv_circ.c
    lvgl/src/lv_misc/lv_area.c
    lvgl/src/lv_misc/lv_task.c
    lvgl/src/lv_misc/lv_fs.c
    lvgl/src/lv_misc/lv_anim.c
    lvgl/src/lv_misc/lv_mem.c
    lvgl/src/lv_misc/lv_ll.c
    lvgl/src/lv_misc/lv_color.c
    lvgl/src/lv_misc/lv_txt.c
    lvgl/src/lv_misc/lv_math.c
    lvgl/src/lv_misc/lv_log.c
    lvgl/src/lv_misc/lv_gc.c
    lvgl/src/lv_misc/lv_utils.c
    lvgl/src/lv_misc/lv_async.c
    lvgl/src/lv_misc/lv_printf.c
    lvgl/src/lv_misc/lv_bidi.c
    lvgl/src/lv_objx/lv_arc.c
    lvgl/src/lv_objx/lv_bar.c
    lvgl/src/lv_objx/lv_cb.c
    lvgl/src/lv_objx/lv_cpicker.c
    lvgl/src/lv_objx/lv_ddlist.c
    lvgl/src/lv_objx/lv_kb.c
    lvgl/src/lv_objx/lv_line.c
    lvgl/src/lv_objx/lv_mbox.c
    lvgl/src/lv_objx/lv_preload.c
    lvgl/src/lv_objx/lv_roller.c
    lvgl/src/lv_objx/lv_table.c
    lvgl/src/lv_objx/lv_tabview.c
    lvgl/src/lv_objx/lv_tileview.c
    lvgl/src/lv_objx/lv_btn.c
    lvgl/src/lv_objx/lv_calendar.c
    lvgl/src/lv_objx/lv_chart.c
    lvgl/src/lv_objx/lv_canvas.c
    lvgl/src/lv_objx/lv_gauge.c
    lvgl/src/lv_objx/lv_label.c
    lvgl/src/lv_objx/lv_list.c
    lvgl/src/lv_objx/lv_slider.c
    lvgl/src/lv_objx/lv_ta.c
    lvgl/src/lv_objx/lv_spinbox.c
    lvgl/src/lv_objx/lv_btnm.c
    lvgl/src/lv_objx/lv_cont.c
    lvgl/src/lv_objx/lv_img.c
    lvgl/src/lv_objx/lv_imgbtn.c
    lvgl/src/lv_objx/lv_led.c
    lvgl/src/lv_objx/lv_lmeter.c
    lvgl/src/lv_objx/lv_page.c
    lvgl/src/lv_objx/lv_sw.c
    lvgl/src/lv_objx/lv_win.c
    lvgl/src/lv_themes/lv_theme_alien.c
    lvgl/src/lv_themes/lv_theme.c
    lvgl/src/lv_themes/lv_theme_default.c
    lvgl/src/lv_themes/lv_theme_night.c
    lvgl/src/lv_themes/lv_theme_templ.c
    lvgl/src/lv_themes/lv_theme_zen.c
    lvgl/src/lv_themes/lv_theme_material.c
    lvgl/src/lv_themes/lv_theme_nemo.c
    lvgl/src/lv_themes/lv_theme_mono.c

	lv_example/lv_apps/benchmark/benchmark.c
	lv_example/lv_apps/benchmark/benchmark_bg.c
	lv_example/lv_apps/demo/demo.c
	lv_example/lv_apps/demo/img_bubble_pattern.c
	lv_example/lv_apps/sysmon/sysmon.c
	lv_example/lv_apps/terminal/terminal.c
	lv_example/lv_apps/tpcal/tpcal.c
	lv_example/lv_tests/lv_test_group/lv_test_group.c
	lv_example/lv_tests/lv_test_misc/lv_test_task.c
	lv_example/lv_tests/lv_test_obj/lv_test_obj.c
	
	lv_example/lv_tests/lv_test_objx/lv_test_arc/lv_test_arc.c
	lv_example/lv_tests/lv_test_objx/lv_test_bar/lv_test_bar.c
	lv_example/lv_tests/lv_test_objx/lv_test_btn/lv_test_btn.c
	lv_example/lv_tests/lv_test_objx/lv_test_btnm/lv_test_btnm.c
	lv_example/lv_tests/lv_test_objx/lv_test_canvas/lv_test_canvas.c
	lv_example/lv_tests/lv_test_objx/lv_test_cb/lv_test_cb.c
	lv_example/lv_tests/lv_test_objx/lv_test_chart/lv_test_chart.c
	lv_example/lv_tests/lv_test_objx/lv_test_cont/lv_test_cont.c
	lv_example/lv_tests/lv_test_objx/lv_test_cpicker/lv_test_cpicker.c
	lv_example/lv_tests/lv_test_objx/lv_test_ddlist/lv_test_ddlist.c
	lv_example/lv_tests/lv_test_objx/lv_test_gauge/lv_test_gauge.c
	lv_example/lv_tests/lv_test_objx/lv_test_img/lv_test_img.c
	lv_example/lv_tests/lv_test_objx/lv_test_img/img_flower_icon.c
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn/lv_test_imgbtn.c
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn/imgbtn_img_1.c
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn/imgbtn_img_2.c
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn/imgbtn_img_3.c
	lv_example/lv_tests/lv_test_objx/lv_test_imgbtn/imgbtn_img_4.c
	lv_example/lv_tests/lv_test_objx/lv_test_kb/lv_test_kb.c
	lv_example/lv_tests/lv_test_objx/lv_test_label/lv_test_label.c
	lv_example/lv_tests/lv_test_objx/lv_test_led/lv_test_led.c
	lv_example/lv_tests/lv_test_objx/lv_test_line/lv_test_line.c
	lv_example/lv_tests/lv_test_objx/lv_test_list/lv_test_list.c
	lv_example/lv_tests/lv_test_objx/lv_test_lmeter/lv_test_lmeter.c
	lv_example/lv_tests/lv_test_objx/lv_test_mbox/lv_test_mbox.c
	lv_example/lv_tests/lv_test_objx/lv_test_page/lv_test_page.c
	lv_example/lv_tests/lv_test_objx/lv_test_preload/lv_test_preload.c
	lv_example/lv_tests/lv_test_objx/lv_test_roller/lv_test_roller.c
	lv_example/lv_tests/lv_test_objx/lv_test_slider/lv_test_slider.c
	lv_example/lv_tests/lv_test_objx/lv_test_sw/lv_test_sw.c
	lv_example/lv_tests/lv_test_objx/lv_test_ta/lv_test_ta.c
	lv_example/lv_tests/lv_test_objx/lv_test_table/lv_test_table.c
	lv_example/lv_tests/lv_test_objx/lv_test_tabview/lv_test_tabview.c
	lv_example/lv_tests/lv_test_objx/lv_test_tileview/lv_test_tileview.c
	lv_example/lv_tests/lv_test_objx/lv_test_win/lv_test_win.c
	
	lv_example/lv_tests/lv_test_stress/lv_test_stress.c
	lv_example/lv_tests/lv_test_theme/lv_test_theme_1.c
	lv_example/lv_tests/lv_test_theme/lv_test_theme_2.c
	lv_example/lv_tests/lv_test_theme/logo.c
)

target_sources(${target} PUBLIC
    lv_port/lv_gui_main.c
    lv_port/lv_gui_example1.c
    lv_port/lv_gui_example2.c
)
