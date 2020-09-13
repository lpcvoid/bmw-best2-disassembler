//
// Created by lpcvoid on 2020-03-11.
//

#ifndef BESTVMTEST_BMWGUICONNECTIONDIALOG_H
#define BESTVMTEST_BMWGUICONNECTIONDIALOG_H


#include <wx/wx.h>
#include <wx/listctrl.h>
#include "../coms/bmwAdapterManager.hpp"
#include "bmwGUIBaseFrame.h"
#include "../core/bmwCarManager.h"

class bmwGUIConnectionDialog : public wxDialog{
private:
    bmwAdapterManager* _adapterman;
    bmwCarManager* _carman;
    bmwCarDescription* _selected_desc = nullptr;
    wxTimer _adapter_check_timer;
    void OnTimer(wxTimerEvent&);
private:
    wxStaticText* m_statictext_adapter;
    wxChoice* m_choice_adapter;
    wxStaticText* m_statictext_car;
    wxChoice* m_choice_car;
    wxStaticText* m_staticText3;
    wxListView* m_listctrl_device_info;
    wxButton* m_button_cancel;
    wxButton* m_button_connect;
public:
    bmwGUIConnectionDialog(wxWindow *parentwindow, bmwAdapterManager *adapterman, bmwCarManager* carman, const wxString &title);

protected:
    void OnShowHide(wxShowEvent &event);
    void OnClose(wxCloseEvent &event);
    void OnComboBoxClick(wxCommandEvent& event);
    void OnButtonClick(wxCommandEvent& event);


};



#endif //BESTVMTEST_BMWGUICONNECTIONDIALOG_H
