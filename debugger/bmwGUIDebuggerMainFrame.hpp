//
// Created by lpcvoid on 2020-02-24.
//

#ifndef BESTVMTEST_BMWGUIDEBUGGERMAINFRAME_HPP
#define BESTVMTEST_BMWGUIDEBUGGERMAINFRAME_HPP

#include <wx/wx.h>
#include "../../coms/bmwAdapterManager.hpp"
#include "../bmwGUIConnectionDialog.h"

#define BMW_GUI_ID_VM (wxID_HIGHEST + 1)

class bmwGUIDebuggerMainFrame : public wxFrame {
private:
    const int CPU_HOR_BASE = 30;
    const int CPU_VER_BASE = 5;
    const int REGS_HOR_BASE = 10;
    const int REGS_HOR_DIST = 100;
    const int REGS_VER_BASE = 30;
    const int REGS_VER_DIST = 15;
    const int REGS_STR_HOR_BASE = ( REGS_HOR_BASE + REGS_HOR_DIST * 3 ) + 20;
    const int REGS_FLT_HOR_BASE = REGS_STR_HOR_BASE;
    const int STACK_HOR_BASE = REGS_FLT_HOR_BASE + 130;
private:
    //cpu
    // how many lines we can currently fit into the cpu window vertically
    int32 _cpu_window_cur_max_line_cnt;
    // scroll distance from top in mouse wheel units
    int32 _cpu_window_scroll_distance_lines;
    // first addr <> opcodes div line horz dist
    int32 _cpu_window_vertline_addr_opcode_hor_dist;
    // second opcodes <> memnonics div line horz dist
    int32 _cpu_window_vertline_opcode_menonics_hor_dist;
    //colors
    wxColour _color_cpu_bg;
    wxColour _color_cpu_entry;
    wxColour _color_cpu_bp;
    wxColour _color_cpu_current;
    wxColour _color_cpu_missing_impl;
    wxColour _color_cpu_conditional;
    wxColour _color_cpu_trace;
    bmwBESTVMDissasembly* _current_disam;

    wxFont _font_reg_types;
    wxFont _font_reg_normal;
private:
    wxPanel* _panel_cpu;
    wxPanel* _panel_regs;
    bmwGUIConnectionDialog* _con_dlg;
    //bnmw
    bmwBESTVMDebuggableVirtualMachine* _vm;
    bmwBESTVMExecutionResult _vm_result;
    bmwBESTObject* _bo;
    bmwAdapterManager* _adapterman;

    wxTimer _timer;

    bool SetJob(bmwBESTJob* job);
    void redraw();
private:
    wxMenuBar* _mainmenu;
    wxMenu* _menu_debugger;
    wxMenu* _menu_jobs;
    wxMenu* _menu_adapters;
    wxMenu* _debugger;
    void event_key(wxKeyEvent &evt);
    void event_menu_open(wxMenuEvent &evt);
    void event_menu_item(wxCommandEvent &evt);
    void event_heartbeat(wxTimerEvent&);
    void event_paint(wxPaintEvent & evt);
    void mouse_wheel(wxMouseEvent& event);
    void mouse_click(wxMouseEvent& event);

    void ScrollToLine(uint32 line_number);

public:
    //init
    bmwGUIDebuggerMainFrame();

};




#endif //BESTVMTEST_BMWGUIDEBUGGERMAINFRAME_HPP
