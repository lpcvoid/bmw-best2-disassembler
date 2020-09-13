//
// Created by lpcvoid on 2020-02-24.
//

#include "bmwGUIDebuggerMainFrame.hpp"
#include "../../common/bmwLogger.h"
#include "../../common/bmwStringHelper.hpp"
#include "../../best/bmwBESTVMJobConstants.h"
#include "../../common/bmwTextFile.hpp"

//generate an ID that we can use to determine function and parameter.
//wx only offers one id field, and it's even constrained
//(itemid >= 0 && itemid < 0x7fff) || (itemid >= wxID_AUTO_LOWEST && itemid <= wxID_AUTO_HIGHEST)
//so we construct valid ID which we can use to dermine which item was clicked

// ((((0x3eff | (uint16)function) << 8)) | (0x1ff | (uint16)paramid))

//5 bits for the function id
//9 bits for the paramid
#define GENERATEWXID(function, paramid) ((((0x3eff & (uint16)function) << 9)) | (0x1ff & (uint16)paramid))

//get the function id from a wxid
#define GETWXFUNCTION(id) (0x3eff & (uint16)(id >> 9))

//get the param id from a wxid
#define GETWXPARAMID(id) (0x1ff & (uint16)(id))


#define BMW_EVENT_ID_JOB  1
#define BMW_EVENT_ID_ADAPTER  2
#define BMW_EVENT_ID_DEBUGGER  3


bmwGUIDebuggerMainFrame::bmwGUIDebuggerMainFrame() : wxFrame((wxFrame *) NULL, -1, wxT("BEST Debugger"),
                                                             wxPoint(50, 50), wxSize(1350, 650)) {

    _color_cpu_bg.Set(wxT("#0c0c0c"));
    _color_cpu_bp.Set(wxT("#ff0000"));
    _color_cpu_entry.Set(wxT("#000000"));
    _color_cpu_current.Set(wxT("#6495ed")); //cornflowerblue
    _color_cpu_missing_impl.Set(wxT("#008080")); //teal
    _color_cpu_conditional.Set(wxT("#008000"));
    _color_cpu_trace.Set(wxT("#800080"));

    _cpu_window_scroll_distance_lines = 0;
    _cpu_window_cur_max_line_cnt = 80;
    _cpu_window_vertline_addr_opcode_hor_dist = 130;
    _cpu_window_vertline_opcode_menonics_hor_dist = 400;

    _font_reg_types.SetWeight(wxFONTWEIGHT_BOLD);
    _font_reg_normal.SetWeight(wxFONTWEIGHT_NORMAL);


    CreateStatusBar(2);

    SetStatusText("Test2", 1);


    _timer.Bind(wxEVT_TIMER, &bmwGUIDebuggerMainFrame::event_heartbeat, this);
    _timer.Start(100);

    Bind(wxEVT_KEY_DOWN, &bmwGUIDebuggerMainFrame::event_key, this);
    Bind(wxEVT_MENU_OPEN, &bmwGUIDebuggerMainFrame::event_menu_open, this);
    Bind(wxEVT_MENU, &bmwGUIDebuggerMainFrame::event_menu_item, this);
    Bind(wxEVT_MOUSEWHEEL, &bmwGUIDebuggerMainFrame::mouse_wheel, this);
    Bind(wxEVT_PAINT, &bmwGUIDebuggerMainFrame::event_paint, this);
    Bind(wxEVT_LEFT_DOWN, &bmwGUIDebuggerMainFrame::mouse_click, this);
    Bind(wxEVT_RIGHT_DOWN, &bmwGUIDebuggerMainFrame::mouse_click, this);


    BMWLOG().log("Starting vm test...");

    _bo = ReadBESTObject("/home/lpcvoid/dev/bmw/prg/CAS.prg");
   // _bo = reader.ReadBESTObject("/home/lpcvoid/dev/bmw/daten/E89/ecu/IHKA87.prg", 0);

    _vm = new bmwBESTVMDebuggableVirtualMachine();

    _vm->SetCbLoadObject( [this](std::string ecuname) {
        std::optional<bmwBESTObject*> res;
        return ReadBESTObject("/home/lpcvoid/dev/bmw/daten/combinedecu/" + ecuname + ".prg");
    });


    //_vm->SetCallbacks([this]( std::vector<uint8>* sendbuf){_stream_proc.SendMessageRaw(&sendbuf);} , [this]( std::vector<uint8> sendbuf){_stream_proc.SendMessageRaw(&sendbuf);}, nullptr, nullptr);


    _vm->SetBESTObject(_bo);
    SetStatusText(_bo->filename, 0);
    if (SetJob(_bo->jobs[BMW_BEST_JOB_NAME_INITIALISIERUNG])) {

        /*
        bmwBESTVMTrace* trace = new bmwBESTVMTrace();
        if (!trace->LoadTraceFile("/home/lpcvoid/dev/code/cpp/bmwizard-cpp/bmwizard-cpp/src/bestvmtest/cmake-build-debug/_traces/E60.txt.rebased")){
            BMWLOG().error("Trace not loaded...");
        }

         */


        //_vm->SetTrace(trace);

        //_vm->DissasembleOnly();
        //_vm->Execute();
        //_vm->ExecuteJob();
        //vm.Execute();
        //auto res = vm.GetResults();
/*
        for (bmwBESTVMResult* result : res){
            std::cout << result->GetResultIdentifier() << " : " << result->DataToString() << std::endl;
        }
        */

    } else {
        BMWLOG().error("Could not load job");
    }


    _adapterman = new bmwAdapterManager();

    _con_dlg = new bmwGUIConnectionDialog(this, _adapterman, nullptr, "Adapter Manager");


    _panel_cpu = new wxPanel(this);
    //bind hotkeys
    _panel_cpu->Bind(wxEVT_CHAR_HOOK, &bmwGUIDebuggerMainFrame::event_key, this);
    _panel_regs = new wxPanel(this);
    _panel_regs->Bind(wxEVT_CHAR_HOOK, &bmwGUIDebuggerMainFrame::event_key, this);




    // dp_log = new DrawPlaneDebugger((wxFrame*)frame);
    //dp_log->SetSize(0,300);

    wxBoxSizer *sizer_h = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *sizer_v = new wxBoxSizer(wxVERTICAL);

    sizer_h->Add(_panel_cpu, 1, wxEXPAND);
    sizer_h->Add(_panel_regs, 1, wxEXPAND);

    //sizer_v->Add(sizer_h,1,wxEXPAND);
    //sizer_v->Add(dp_log,1,wxEXPAND);


    /*
    drawPane->Bind(wxEVT_LEFT_DOWN, &DrawPlaneDebugger::mouse_down, drawPane);
    drawPane->Bind(wxEVT_KEY_DOWN, &DrawPlaneDebugger::keyPressed, drawPane);
    drawPane->Bind(wxEVT_TIMER, &DrawPlaneDebugger::event_timer, drawPane);

     */


    //frame->AddChild(_dp_cpu);
    this->SetSizer(sizer_h);
    this->SetAutoLayout(true);


    _mainmenu = new wxMenuBar();
    // File Menu
    _menu_debugger = new wxMenu();
    _menu_debugger->Append(wxID_EXIT, _T("&Quit"));

    _mainmenu->Append(_menu_debugger, "&File");
    // About menu
    _menu_jobs = new wxMenu();
    _mainmenu->Append(_menu_jobs, "&Jobs");

    _menu_adapters = new wxMenu();
    _mainmenu->Append(_menu_adapters, "&Adapters");

    _debugger = new wxMenu();
    _mainmenu->Append(_debugger, "&Debugger");
    _debugger->Append(new wxMenuItem(_debugger, GENERATEWXID(BMW_EVENT_ID_DEBUGGER, 0), "Toggle Trace"));
    _debugger->Append(new wxMenuItem(_debugger, GENERATEWXID(BMW_EVENT_ID_DEBUGGER, 1), "Clear Regs/Stack"));

    SetMenuBar(_mainmenu);

}


void bmwGUIDebuggerMainFrame::mouse_wheel(wxMouseEvent &event) {

    _cpu_window_scroll_distance_lines -= event.GetWheelRotation() / event.GetWheelDelta();
    if (_cpu_window_scroll_distance_lines < 0)
        _cpu_window_scroll_distance_lines = 0;
    this->Refresh();

}

void bmwGUIDebuggerMainFrame::mouse_click(wxMouseEvent &event) {
    if (event.GetButton() == wxMOUSE_BTN_LEFT) {
        wxPoint pt = event.GetPosition();
        if (pt.x < CPU_HOR_BASE) {
            uint32 calcy = pt.y - CPU_VER_BASE;
            calcy = calcy / 20;
            if ((_cpu_window_scroll_distance_lines + calcy) < _current_disam->size() - 1) {
                bmwBESTVMDissasemblyToken *selected_disasm = _current_disam->at(
                        _cpu_window_scroll_distance_lines + calcy);
                bmwBESTVMBreakpoint *bp = _vm->GetBreakpoint(selected_disasm->token_address);
                if ((bp != nullptr))
                    _vm->RemoveBreakpoint(bp->break_address);
                else
                    _vm->AddBreakpoint(selected_disasm->token_address);
                this->Refresh();
            }
        }
    }

    // if (event.GetButton() == wxMOUSE_BTN_RIGHT) {
    //wxMenu mnu;
    // mnu.Append(7000, 	"Do something");
    //mnu.Bind(wxEVT_COMMAND_MENU_SELECTED, &bmwGUIDebuggerMainFrame::on_popup_click, this);
    //PopupMenu(&mnu);
    // }
}


void bmwGUIDebuggerMainFrame::ScrollToLine(uint32 line_number) {
    _cpu_window_scroll_distance_lines = line_number - (_cpu_window_cur_max_line_cnt / 2); //center line
    if (_cpu_window_scroll_distance_lines < 0)
        _cpu_window_scroll_distance_lines = 0;
    redraw();
}

void bmwGUIDebuggerMainFrame::event_menu_open(wxMenuEvent &evt) {

    if (evt.GetMenu() == _menu_jobs) {
        //do we already have jobs set?
        if (evt.GetMenu()->GetMenuItemCount() == 0) {
            uint32 c = 0;
            for (std::pair<std::string, bmwBESTJob *> element : _bo->jobs) {
                _menu_jobs->Append(new wxMenuItem(_menu_jobs, GENERATEWXID(BMW_EVENT_ID_JOB, c++), element.first));
            }

        }

    }

    if (evt.GetMenu() == _menu_adapters) {


        _con_dlg->Show();



        /*
        //do we already have jobs set?
        if (evt.GetMenu()->GetMenuItemCount() == 0){
            std::vector<bmwAdapterBase*> adapters = _adapterman->get_adapter_list();
            uint32 c = 0;
            for (bmwAdapterBase* element : adapters){
                _menu_adapters->Append(new wxMenuItem(_menu_adapters, GENERATEWXID(BMW_EVENT_ID_ADAPTER, c++),element->GetName()));
            }

        }
         */

    }

    if (evt.GetMenu() == _debugger) {

    }

}

void bmwGUIDebuggerMainFrame::event_menu_item(wxCommandEvent &evt) {

    wxMenuItem *p_menuitem = _mainmenu->FindItem(evt.GetId());

    std::string label_name = p_menuitem->GetItemLabelText().ToStdString();

    switch (GETWXFUNCTION(evt.GetId())) {
        case BMW_EVENT_ID_JOB: {
            BMWLOG().log("Loading job : " + std::string(label_name.c_str()));
            SetJob(_bo->jobs[label_name]);


            break;
        }

        case BMW_EVENT_ID_ADAPTER: {
            bmwAdapterBase *sa = _adapterman->select_adapter(label_name);
            if (sa) {
                if (sa->GetTransport()->Connect()) {
                    BMWLOG().log("Connected to adapter " + sa->ToString());
                    _vm->SetAdapter(sa);
                } else
                    BMWLOG().error("Failed to connect to adapter!");
            }
            break;
        }

        case BMW_EVENT_ID_DEBUGGER: {
            switch (GETWXPARAMID(evt.GetId())) {
                case 0: {
                    //toggle trace
                    if (_vm->GetTrace()) {
                        delete _vm->GetTrace();
                        _vm->SetTrace(nullptr);
                        SetStatusText(_vm->GetCurrentJob()->name + ", untraced", 1);
                    } else {
                        //load trace
                        bmwBESTVMTrace *trace = new bmwBESTVMTrace();
                        if (!trace->LoadTraceFile(
                                "/home/lpcvoid/dev/code/cpp/bmwizard-cpp/bmwizard-cpp/include/bestvm_traces/" +
                                bmwStringHelper::uppercase(_bo->filename) + "_" + bmwStringHelper::uppercase(_vm->GetCurrentJob()->name) + ".txt.rebased")) {
                            BMWLOG().error("Trace not loaded...");
                            SetStatusText(_vm->GetCurrentJob()->name + ", trace load error", 1);
                        } else {
                            trace->ResetTrace();
                            SetStatusText(_vm->GetCurrentJob()->name + ", trace loaded!", 1);
                            _vm->SetTrace(trace);
                        }
                    }

                    this->Refresh();
                }
                    break;

                case 1: {
                    _vm->ClearAllRegistersAndStack();
                    BMWLOG().debug("Cleared VM registers and stack");
                }
                default:
                    break;
            }
        }

        default: {
            BMWLOG().error("Unhandled main menu item clicked!");
            break;
        }
    }


}

void bmwGUIDebuggerMainFrame::event_key(wxKeyEvent &evt) {

    switch (evt.GetKeyCode()) {
        case 'R': {
            _vm->ResetVM();
            this->Refresh();
        } break;


        case wxKeyCode::WXK_F8: {

            _vm->ExecuteSingleInstruction();
            ScrollToLine(_vm->GetLineOfAddress(_vm->GetEIP()));
            this->Refresh();
        }break;


        case wxKeyCode::WXK_F9: {
            //_vm->AddParameter("0x9C6C");
            _vm->SetAdapter(_adapterman->get_current_adapter());
            _vm_result = _vm->ExecuteJob(bmwBESTVMRunMode_normal);
            switch (_vm_result.first) {
                case bmwBESTVM_Result_Success:
                    BMWLOG().log("VM execution complete!");
                    this->Refresh();
                    break;
                case bmwBESTVM_breakpoint_hit:
                    BMWLOG().log("VM hit a breakpoint!");
                    ScrollToLine(_vm->GetLineOfAddress(_vm->GetEIP()));
                    this->Refresh();
                    break;
                case bmwBESTVM_trace_incorrect:
                    BMWLOG().log("VM trace was incorrect...");
                    ScrollToLine(_vm->GetLineOfAddress(_vm->GetEIP()));
                    this->Refresh();
                    break;
                case bmwBESTVM_Result_Error:
                    BMWLOG().log("VM bmwBESTVM_Result_Error...");
                    break;
                case bmwBESTVM_Result_NotFound:
                    BMWLOG().log("VM bmwBESTVM_Result_NotFound...");
                    break;
                case bmwBESTVM_missing_opcode_impl:
                    BMWLOG().log("VM bmwBESTVM_missing_opcode_impl...");
                    break;
                case bmwBESTVM_EOF:
                    BMWLOG().log("VM bmwBESTVM_EOF...");
                    break;
            }
        }
            break;

        case 'G': {
            //ask user for address to go it
            wxString valueTyped;
            wxTextEntryDialog myDialog(this, _("GOTO"), _("Enter address (XXXXXXXX hex)"), _("00000000"));
            if (myDialog.ShowModal() == wxID_OK) {
                valueTyped = myDialog.GetValue();
                uint32 address = bmwStringHelper::hex_to_int<uint32>(valueTyped.ToStdString());
                ScrollToLine(_vm->GetLineOfAddress(address));

            }
        }
            break;

        case 'I': {
            bmwBESTVMTrace *trace = new bmwBESTVMTrace();
            if (!trace->LoadTraceFile(
                    "/home/lpcvoid/dev/code/cpp/bmwizard-cpp/bmwizard-cpp/src/bestvmtest/cmake-build-debug/_traces/KOMB60_IDENT.txt.rebased")) {
                BMWLOG().error("Trace not loaded...");
            }
        }
            break;

    }

}


bool bmwGUIDebuggerMainFrame::SetJob(bmwBESTJob *job) {

    _vm->SetJob(job);
    _vm_result = _vm->ExecuteJob(bmwBESTVMRunMode_disassembly);
    return _vm_result.first == bmwBESTVM_Result_Success;
}


void bmwGUIDebuggerMainFrame::event_heartbeat(wxTimerEvent &) {


}

void bmwGUIDebuggerMainFrame::redraw() {

}

void bmwGUIDebuggerMainFrame::event_paint(wxPaintEvent &evt) {
    wxPaintDC dc_panel(this);

    uint32 x_base_registers = (this->GetRect().GetWidth() / 3) * 2; //last third is for registers

    dc_panel.Clear();
    dc_panel.SetFont(_font_reg_types);
    dc_panel.DrawText("Int32", x_base_registers + REGS_HOR_BASE, 5);
    dc_panel.DrawText("Int16", x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 1), 5);
    dc_panel.DrawText("Int8", x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 2), 5);
    dc_panel.DrawText("Strings", x_base_registers + REGS_STR_HOR_BASE, 5);
    dc_panel.DrawText("F32", x_base_registers + REGS_FLT_HOR_BASE, REGS_VER_BASE + (REGS_VER_DIST * 20));
    dc_panel.DrawText("Stack", x_base_registers + STACK_HOR_BASE, REGS_VER_BASE + (REGS_VER_DIST * 20));
    dc_panel.DrawText("Results", x_base_registers + REGS_FLT_HOR_BASE, REGS_VER_BASE + (REGS_VER_DIST * 32));
    dc_panel.SetFont(_font_reg_normal);

    for (int i = 0; i < 8; i++) {
        // first row
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegL, i)->ToString(), x_base_registers + REGS_HOR_BASE,
                          REGS_VER_BASE + (REGS_VER_DIST * (i * 4)));
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegI, i * 2)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 1),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4))));
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegAB, i * 4)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 2),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4))));

        // second row
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegI, i * 2 + 1)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 1),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4) + 1)));
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegAB, i * 4 + 1)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 2),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4) + 1)));

        // third row
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegAB, i * 4 + 2)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 2),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4) + 2)));

        // forth row
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegAB, i * 4 + 3)->ToString(),
                          x_base_registers + REGS_HOR_BASE + (REGS_HOR_DIST * 2),
                          REGS_VER_BASE + (REGS_VER_DIST * ((i * 4) + 3)));
    }


    for (int i = 0; i < 16; i++) {
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegS, i)->ToString(),
                          x_base_registers + REGS_STR_HOR_BASE, REGS_VER_BASE + (REGS_VER_DIST * i));
    }


    for (int i = 0; i < 8; i++) {
        dc_panel.DrawText(_vm->GetRegister(bmwBESTRegisterType_RegF, i)->ToString(),
                          x_base_registers + REGS_FLT_HOR_BASE, REGS_VER_BASE + (REGS_VER_DIST * (i + 21)));
    }

    //results
    std::vector<bmwBESTVMJobResultGroup> results = _vm->GetResults();

    if (!results.empty()) {
        bmwTextFile textfile;

        int32 c = 0;
        for (int32 i = 0; i < results.size(); i++) {
            dc_panel.SetFont(_font_reg_types);
            dc_panel.DrawText("Result set " + std::to_string(i), x_base_registers + REGS_FLT_HOR_BASE,
                              REGS_VER_BASE + (REGS_VER_DIST * (c++ + 33)));
            textfile.AddLine("Result set " + std::to_string(i));
            dc_panel.SetFont(_font_reg_normal);


            for (auto &r : results[i]) {
                dc_panel.DrawText(r.first + " : " + r.second, x_base_registers + REGS_FLT_HOR_BASE,
                                  REGS_VER_BASE + (REGS_VER_DIST * (c++ + 33)));
                textfile.AddLine(r.first + " : " + r.second);
            }


        }

        textfile.SaveFile("results.txt");
    }

    auto stack = _vm->GetStack();
    for (int i = 0; i < stack.size(); i++)
        dc_panel.DrawText("0x" + bmwStringHelper::int_to_hex<uint8>(stack[i]), x_base_registers + STACK_HOR_BASE,
                          REGS_VER_BASE + (REGS_VER_DIST * (i + 21)));

    // PC
    dc_panel.DrawText("EIP : " + bmwStringHelper::int_to_hex<uint32>(_vm->GetEIP()), x_base_registers + REGS_HOR_BASE,
                      REGS_VER_BASE + (REGS_VER_DIST * 32));
    // flags
    auto bvmflags = _vm->GetFlags();
    dc_panel.DrawText("Zero : " + std::to_string(bvmflags.f_zero), x_base_registers + REGS_HOR_BASE,
                      REGS_VER_BASE + (REGS_VER_DIST * 34));
    dc_panel.DrawText("Carry : " + std::to_string(bvmflags.f_carry), x_base_registers + REGS_HOR_BASE,
                      REGS_VER_BASE + (REGS_VER_DIST * 35));
    dc_panel.DrawText("Sign : " + std::to_string(bvmflags.f_sign), x_base_registers + REGS_HOR_BASE,
                      REGS_VER_BASE + (REGS_VER_DIST * 36));
    dc_panel.DrawText("Overflow : " + std::to_string(bvmflags.f_overflow), x_base_registers + REGS_HOR_BASE,
                      REGS_VER_BASE + (REGS_VER_DIST * 37));
    //dc.DrawText( REGS_HOR_BASE, REGS_VER_BASE + ( REGS_VER_DIST * 39 ), "Elapsed : " + elapsted_time );
    //auto bvmstats = _vm->GetStatistics( );
    //dc.DrawText(  "Num ops/exec ops : " + std::to_string( bvmstats.instruction_count ) + "/" + std::to_string( bvmstats.instructions_executed ), REGS_HOR_BASE, REGS_VER_BASE + ( REGS_VER_DIST * 40 ) );
    //dc.DrawText( REGS_HOR_BASE, REGS_VER_BASE + ( REGS_VER_DIST * 41 ), "Status : " + status_string );


    //cpu

    bmwBESTVMBreakpoint *bp;
    dc_panel.SetPen(wxPen(_color_cpu_bg, 1));
    unsigned int jumpto_address = 0;
    _current_disam = _vm->GetDissasembly();
    bmwBESTVMTrace *trace = _vm->GetTrace();
    for (bmwBESTVMDissasemblyToken *token : *_current_disam) {
        if (token->token_address == _vm->GetEIP())
            jumpto_address = token->token_reference_address;
    }
    for (int i = _cpu_window_scroll_distance_lines;
         i < _cpu_window_scroll_distance_lines + _cpu_window_cur_max_line_cnt; i++) {
        if (_current_disam->size() <= i)
            continue;


        bp = _vm->GetBreakpoint(_current_disam->at(i)->token_address);
        if (bp != nullptr) {
            // imgCPU.Canvas.Rectangle(5,CPU_VER_BASE + (i - cpu_window_scroll_distance_lines) * 20, 25, (CPU_VER_BASE + (i - cpu_window_scroll_distance_lines) * 20) + 5);
            dc_panel.SetTextForeground(_color_cpu_bp);
            dc_panel.DrawText("X", 5, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);

        } else {
            dc_panel.SetPen(wxPen(_color_cpu_entry, 1));
        }
        if (_current_disam->at(i)->token_address == _vm->GetEIP()) {
            // jumpto_address := _current_disam->at(i).token_reference_address;
            dc_panel.SetTextForeground(_color_cpu_current);
            dc_panel.DrawText("-> " + bmwStringHelper::int_to_hex<uint32>(_current_disam->at(i)->token_address),
                              CPU_HOR_BASE, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        } else if ((_current_disam->at(i)->token_address == jumpto_address) && (jumpto_address != 0)) {
            // another operation which is the jump target of this one, mark it somehow
            //imgCPU.Canvas.Brush.Color = clHighlight;
            dc_panel.DrawText(">> " + bmwStringHelper::int_to_hex<uint32>(_current_disam->at(i)->token_address),
                              CPU_HOR_BASE, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        } else if ((_current_disam->at(i)->token_flags & BMW_BESTVM_DISASM_FLAGS_MISSING_OPCODE_IMPL)) {
            // missing opcode

            dc_panel.SetTextForeground(_color_cpu_missing_impl);
            dc_panel.DrawText("xx " + bmwStringHelper::int_to_hex<uint32>(_current_disam->at(i)->token_address),
                              CPU_HOR_BASE, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);


        } else if ((_current_disam->at(i)->token_flags & BMW_BESTVM_DISASM_FLAGS_CONDITIONAL)) {
            // conditional
            //imgCPU.Canvas.Pen.Color = clGreen;
            dc_panel.SetTextForeground(_color_cpu_conditional);
            dc_panel.DrawText(bmwStringHelper::int_to_hex<uint32>(_current_disam->at(i)->token_address), CPU_HOR_BASE,
                              CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        } else {
            // other, normal line
            //dc_cpu.SetPen(wxPen(_color_cpu_missing_impl, 1));
            dc_panel.SetTextForeground(_color_cpu_entry);
            dc_panel.DrawText(bmwStringHelper::int_to_hex<uint32>(_current_disam->at(i)->token_address), CPU_HOR_BASE,
                              CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        }

        if (_current_disam->at(i)->token_bytecode.size() > 9) {
            dc_panel.DrawText(
                    bmwStringHelper::byte_array_to_string(_current_disam->at(i)->token_bytecode, 9, true, ' ') + "...",
                    CPU_HOR_BASE + 150, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        } else {
            dc_panel.DrawText(bmwStringHelper::byte_array_to_string(_current_disam->at(i)->token_bytecode,
                                                                    _current_disam->at(i)->token_bytecode.size(), true,
                                                                    ' '), CPU_HOR_BASE + 150,
                              CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);
        }

        dc_panel.DrawText(_current_disam->at(i)->token_text, CPU_HOR_BASE + 420,
                          CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);

        dc_panel.DrawText(std::to_string(i), 5, CPU_VER_BASE + (i - _cpu_window_scroll_distance_lines) * 20);

        //draw indexes

        // behind, draw executions using trace


        if (trace) {
            if (trace->ContainsTraceEntry(_current_disam->at(i)->token_address)) {
                dc_panel.SetTextForeground(_color_cpu_trace);
                dc_panel.DrawText("*", CPU_HOR_BASE + 90,
                                  CPU_VER_BASE + 3 + (i - _cpu_window_scroll_distance_lines) * 20);
            }
        }
    }
    dc_panel.SetTextForeground(_color_cpu_entry);
    dc_panel.DrawLine(CPU_HOR_BASE + _cpu_window_vertline_addr_opcode_hor_dist, this->GetRect().GetHeight(),
                      CPU_HOR_BASE + _cpu_window_vertline_addr_opcode_hor_dist, 0);
    dc_panel.DrawLine(CPU_HOR_BASE + _cpu_window_vertline_opcode_menonics_hor_dist, this->GetRect().GetHeight(),
                      CPU_HOR_BASE + _cpu_window_vertline_opcode_menonics_hor_dist, 0);
}

/*

void bmwGUIDebuggerMainFrame::event_paint_cpu(wxPaintEvent &evt) {
    wxPaintDC dc_cpu(_panel_cpu);
    //cpu

}
*/
