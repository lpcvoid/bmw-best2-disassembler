 //
// Created by lpcvoid on 2020-03-11.
//

#include "bmwGUIConnectionDialog.h"
#include "../common/bmwLogger.h"
#include "../coms/bmwAdapterBMWFAST.hpp"
#include "../coms/bmwTransportSerial.h"
#include "../coms/bmwTransportEnet.h"

#define EVENT_CB_ADAPTER wxID_HIGHEST+1
#define EVENT_BTN_CANCEL wxID_HIGHEST+2
#define EVENT_BTN_CONNECT wxID_HIGHEST+3
#define EVENT_CB_CAR wxID_HIGHEST+4

bmwGUIConnectionDialog::bmwGUIConnectionDialog(wxWindow *parentwindow, bmwAdapterManager *adapterman, bmwCarManager* carman, const wxString &title) :
        wxDialog(parentwindow, wxID_ANY, title, wxDefaultPosition, wxSize(300,450)),
_adapterman(adapterman),
_carman(carman)
{


    _adapter_check_timer.Bind(wxEVT_TIMER, &bmwGUIConnectionDialog::OnTimer, this);
    _adapter_check_timer.Start(250);

    Bind(wxEVT_SHOW, &bmwGUIConnectionDialog::OnShowHide, this);
    Bind(wxEVT_CLOSE_WINDOW, &bmwGUIConnectionDialog::OnClose, this);

    this->SetSizeHints( wxDefaultSize, wxDefaultSize );

    wxBoxSizer* s_main_v;
    s_main_v = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* s_adapter_v;
    s_adapter_v = new wxBoxSizer( wxHORIZONTAL);

    m_statictext_adapter = new wxStaticText( this, wxID_ANY, wxT("Chose adapter"), wxDefaultPosition, wxDefaultSize, 0 );
    s_adapter_v->Add( m_statictext_adapter, 0, wxALL, 5 );

    wxArrayString m_choice_adapterChoices;
    m_choice_adapter = new wxChoice( this, EVENT_CB_ADAPTER, wxDefaultPosition, wxDefaultSize, m_choice_adapterChoices, 0 );
    m_choice_adapter->SetSelection( 0 );
    m_choice_adapter->Bind(wxEVT_CHOICE, &bmwGUIConnectionDialog::OnComboBoxClick, this);


    s_adapter_v->Add( m_choice_adapter, 1, wxALL | wxEXPAND, 5 );


    s_main_v->Add( s_adapter_v, 0, wxEXPAND, 5 );

    wxBoxSizer* s_car_v;
    s_car_v = new wxBoxSizer( wxHORIZONTAL );

    m_statictext_car = new wxStaticText( this, wxID_ANY, wxT("Chose car type"), wxDefaultPosition, wxSize( 100,-1 ), 0 );
    m_statictext_car->Wrap( -1 );
    s_car_v->Add( m_statictext_car, 0, wxALL | wxEXPAND, 5 );

    wxArrayString m_choice_carChoices;
    m_choice_car = new wxChoice( this, EVENT_CB_CAR, wxDefaultPosition, wxDefaultSize, m_choice_carChoices, 0 );
    m_choice_car->SetSelection( 0 );
    m_choice_car->Bind(wxEVT_CHOICE, &bmwGUIConnectionDialog::OnComboBoxClick, this);
    s_car_v->Add( m_choice_car, 1, wxALL | wxEXPAND, 5 );


    s_main_v->Add( s_car_v, 0, wxEXPAND, 5 );

    wxBoxSizer* s_info_v;
    s_info_v = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer* bSizer7;
    bSizer7 = new wxBoxSizer( wxVERTICAL );

    m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Adapter properties"), wxDefaultPosition, wxDefaultSize, 0 );
    m_staticText3->Wrap( -1 );
    bSizer7->Add( m_staticText3, 0, wxALL, 5 );

    m_listctrl_device_info = new wxListView( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_AUTOARRANGE );

    m_listctrl_device_info->AppendColumn("Property");
    m_listctrl_device_info->AppendColumn("Value");

    m_listctrl_device_info->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
    m_listctrl_device_info->SetColumnWidth(1, wxLIST_AUTOSIZE_USEHEADER);



    bSizer7->Add( m_listctrl_device_info, 1, wxALL|wxEXPAND, 5 );


    s_info_v->Add( bSizer7, 1, wxEXPAND, 5 );


    s_main_v->Add( s_info_v, 1, wxEXPAND, 5 );

    wxBoxSizer* s_buttons_v;
    s_buttons_v = new wxBoxSizer( wxHORIZONTAL );

    m_button_cancel = new wxButton( this, EVENT_BTN_CANCEL, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
    m_button_cancel->Bind(wxEVT_BUTTON, &bmwGUIConnectionDialog::OnButtonClick, this);

    s_buttons_v->Add( m_button_cancel, 0, wxALL, 5 );



    m_button_connect = new wxButton( this, EVENT_BTN_CONNECT, wxT("Connect"), wxDefaultPosition, wxDefaultSize, 0 );
    m_button_connect->Bind(wxEVT_BUTTON, &bmwGUIConnectionDialog::OnButtonClick, this);

    s_buttons_v->Add( m_button_connect, 0, wxALL, 5 );


    s_main_v->Add( s_buttons_v, 0, wxALIGN_RIGHT|wxRIGHT, 5 );


    this->SetSizer( s_main_v );
    this->Layout();

    this->Centre( wxBOTH );
}



void bmwGUIConnectionDialog::OnComboBoxClick(wxCommandEvent &event) {


    switch (event.GetId()){
        case EVENT_CB_ADAPTER:{
            bmwAdapterBase* a = (bmwAdapterBase*) event.GetClientData();

            m_listctrl_device_info->DeleteAllItems();

            if (_adapterman->select_adapter(a)){
                if (dynamic_cast<bmwTransportSerial*>(a->GetTransport()) != nullptr){
                    //this is a kline adapter
                    auto serialtransport = dynamic_cast<bmwTransportSerial*>(a->GetTransport());


                    m_listctrl_device_info->InsertItem(0, "Adapter Type");
                    m_listctrl_device_info->SetItem(0, 1, "USB");

                    m_listctrl_device_info->InsertItem(1, "Adapter Name");
                    m_listctrl_device_info->SetItem(1, 1, a->GetName());

                    m_listctrl_device_info->InsertItem(2, "Adapter Serial Number");
                    m_listctrl_device_info->SetItem(2, 1, a->GetIdent());

                    m_listctrl_device_info->InsertItem(3, "Adapter Manufacturer");
                    m_listctrl_device_info->SetItem(3, 1, serialtransport->GetManufacturer());

                    m_listctrl_device_info->InsertItem(4, "Adapter Port");
                    m_listctrl_device_info->SetItem(4, 1, a->GetName());

                    m_listctrl_device_info->InsertItem(5, "Adapter Vendor Id");
                    m_listctrl_device_info->SetItem(5, 1, std::to_string(serialtransport->GetVid()));

                    m_listctrl_device_info->InsertItem(6, "Adapter Product Id");
                    m_listctrl_device_info->SetItem(6, 1, std::to_string(serialtransport->GetPid()));

                    m_listctrl_device_info->InsertItem(7, "Baudrate");
                    m_listctrl_device_info->SetItem(7, 1, std::to_string(serialtransport->GetBaudrate()));
                }

                if (dynamic_cast<bmwTransportEnet*>(a->GetTransport()) != nullptr){
                    //this is a kline adapter
                    auto enettransport = dynamic_cast<bmwTransportEnet*>(a->GetTransport());


                    m_listctrl_device_info->InsertItem(0, "Adapter Type");
                    m_listctrl_device_info->SetItem(0, 1, "Ethernet");

                    m_listctrl_device_info->InsertItem(1, "Car VIN");
                    m_listctrl_device_info->SetItem(1, 1, a->GetName());

                    m_listctrl_device_info->InsertItem(2, "Car MAC Adress");
                    m_listctrl_device_info->SetItem(2, 1, a->GetIdent());

                    m_listctrl_device_info->InsertItem(3, "Car IP");
                    m_listctrl_device_info->SetItem(3, 1, enettransport->GetIp());

                    m_listctrl_device_info->InsertItem(4, "Car Port");
                    m_listctrl_device_info->SetItem(4, 1, std::to_string(enettransport->GetPort()));
                }

                m_listctrl_device_info->Arrange();
            } else {
                //adapter cannot be found anymore, isConnected() retruns false
                wxMessageBox("Failed to connect to this adapter. It was disconnected.");
                Show(false);
            }
        } break;
        case EVENT_CB_CAR:{
            _selected_desc = (bmwCarDescription*) event.GetClientData();
        } break;
    }

}

void bmwGUIConnectionDialog::OnTimer(wxTimerEvent &) {

}

void bmwGUIConnectionDialog::OnShowHide(wxShowEvent &event) {
    if (event.IsShown()){
        m_choice_adapter->Clear();
        auto adapters = _adapterman->get_adapter_list();
        int32 selection = -1;
        int32 c = 0;
        for (bmwAdapterBase* a : adapters){
            if (a == _adapterman->get_current_adapter())
                selection = c;
            m_choice_adapter->Append(a->GetName(), a);
            c++;
        }
        m_choice_adapter->Select(selection);

        //cars
        if (_carman){
            std::vector<bmwCarDescription*> desc = _carman->GetKnownCars();
            if (desc.size()){
                m_choice_car->Clear();
                for (auto d : desc){
                    m_choice_car->Append(d->friendly_name,d);
                }
            }
        }

    } else {
        //hidden

    }

}

void bmwGUIConnectionDialog::OnButtonClick(wxCommandEvent &event) {

    switch(event.GetId())
    {
        case EVENT_BTN_CANCEL:{
            Show(false);
        }
        break;

        case EVENT_BTN_CONNECT:{
            if (_adapterman->get_current_adapter() != nullptr){
                BMWLOG().log("Starting connection to " + _adapterman->get_current_adapter()->GetName());
                Show(false);
                if ((_carman) && (_selected_desc)){
                    bmwCarBase* car = _carman->LoadCar(_selected_desc);
                    if (car){
                        _carman->SetCurrentCar(car);
                        car->SetAdapter(_adapterman->get_current_adapter());
                    }
                }
            }

        }
        break;
    }

}

void bmwGUIConnectionDialog::OnClose(wxCloseEvent &event) {

}
