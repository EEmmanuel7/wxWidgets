/////////////////////////////////////////////////////////////////////////////
// Name:        src/qt/toolbar.cpp
// Author:      Sean D'Epagnier
// Copyright:   (c) Sean D'Epagnier 2014
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#include "wx/toolbar.h"
#include "wx/qt/private/winevent.h"
#include "wx/qt/converter.h"

#if wxUSE_TOOLBAR

#include <QtWidgets/QToolButton>

class wxQtToolButton;
class wxToolBarTool : public wxToolBarToolBase
{
public:
    wxToolBarTool(wxToolBar *tbar, int id, const wxString& label, const wxBitmap& bitmap1,
                  const wxBitmap& bitmap2, wxItemKind kind, wxObject *clientData,
                  const wxString& shortHelpString, const wxString& longHelpString)
        : wxToolBarToolBase(tbar, id, label, bitmap1, bitmap2, kind,
                            clientData, shortHelpString, longHelpString)
    {
        m_qtToolButton = NULL;
    }

    wxToolBarTool(wxToolBar *tbar, wxControl *control, const wxString& label)
        : wxToolBarToolBase(tbar, control, label)
    {
        m_qtToolButton = NULL;
    }

    virtual void SetLabel( const wxString &label );
    virtual void SetDropdownMenu(wxMenu* menu);

    void SetIcon();
    void ClearToolTip();
    void SetToolTip();

    wxQtToolButton* m_qtToolButton;
};

class wxQtToolButton : public QToolButton, public wxQtSignalHandler< wxToolBarTool >
{

public:
    wxQtToolButton(wxToolBar *parent, wxToolBarTool *handler)
        : QToolButton(parent->GetHandle()),
          wxQtSignalHandler< wxToolBarTool >( handler ) {
        setContextMenuPolicy(Qt::PreventContextMenu);
    }

private:
    void mouseReleaseEvent( QMouseEvent *event );
    void mousePressEvent( QMouseEvent *event );
    void enterEvent( QEvent *event );
};

void wxQtToolButton::mouseReleaseEvent( QMouseEvent *event )
{
    QToolButton::mouseReleaseEvent(event);
    if(event->button() == Qt::LeftButton) {
        wxToolBarTool *handler = GetHandler();
        wxToolBarBase *toolbar = handler->GetToolBar();
        toolbar->OnLeftClick( handler->GetId(), isCheckable() ? 1 : 0 );
    }
}

void wxQtToolButton::mousePressEvent( QMouseEvent *event )
{
    QToolButton::mousePressEvent(event);
    if(event->button() == Qt::RightButton) {
        wxToolBarTool *handler = GetHandler();
        wxToolBarBase *toolbar = handler->GetToolBar();
        toolbar->OnRightClick( handler->GetId(), event->x(), event->y() );
    }
}

void wxQtToolButton::enterEvent( QEvent *WXUNUSED(event) )
{
    wxToolBarTool *handler = GetHandler();
    wxToolBarBase *toolbar = handler->GetToolBar();
//    toolbar->OnMouseEnter( handler->GetId() );
}

// is this needed?
//IMPLEMENT_DYNAMIC_CLASS(wxToolBar, wxControl)

void wxToolBarTool::SetLabel( const wxString &label )
{
    wxToolBarToolBase::SetLabel( label );

    if(m_qtToolButton) {
        m_qtToolButton->setText(wxQtConvertString( label ));
    }
}

void wxToolBarTool::SetDropdownMenu(wxMenu* menu)
{
    wxToolBarToolBase::SetDropdownMenu(menu);
    m_qtToolButton->setMenu(menu->GetHandle());
    menu->SetInvokingWindow(GetToolBar());
}

void wxToolBarTool::SetIcon()
{
    m_qtToolButton->setIcon( QIcon( *GetNormalBitmap().GetHandle() ));
}

void wxToolBarTool::ClearToolTip()
{
    m_qtToolButton->setToolTip("");
}

void wxToolBarTool::SetToolTip()
{
    m_qtToolButton->setToolTip(wxQtConvertString( GetShortHelp() ));
}

QToolBar *wxToolBar::QtToolBar() const
{
    return m_qtToolBar;
}

void wxToolBar::Init()
{
    m_qtToolBar = NULL;
}

wxToolBar::~wxToolBar()
{
    delete m_qtToolBar;
}

bool wxToolBar::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos,
                       const wxSize& size, long style, const wxString& name)
{
    m_qtToolBar = new QToolBar(wxQtConvertString( name ));

    // reset pointer so we don't try to delete the toolbar when we exit
    connect( m_qtToolBar, &QObject::destroyed, this, &wxToolBar::Init );

    SetWindowStyleFlag(style);

    QMainWindow *mainwindow = qobject_cast<QMainWindow*>(parent->GetHandle());
    if(!mainwindow)
        wxFAIL_MSG( wxT("wxToolBar::QtCreate() parent not a QMainWindow"));

    int area = 0;
    if(HasFlag(wxTB_LEFT))   area |= Qt::LeftToolBarArea;
    if(HasFlag(wxTB_RIGHT))  area |= Qt::RightToolBarArea;
    if(HasFlag(wxTB_TOP))    area |= Qt::TopToolBarArea;
    if(HasFlag(wxTB_BOTTOM)) area |= Qt::BottomToolBarArea;

    mainwindow->addToolBar((Qt::ToolBarArea)area, m_qtToolBar);

    return wxWindowBase::CreateBase( parent, id, pos, size, style, wxDefaultValidator, name );
}

wxToolBarToolBase *wxToolBar::FindToolForPosition(wxCoord WXUNUSED(x),
                                                  wxCoord WXUNUSED(y)) const
{
//    actionAt(x, y);
    wxFAIL_MSG( wxT("wxToolBar::FindToolForPosition() not implemented") );
    return NULL;
}

void wxToolBar::SetWindowStyleFlag( long style )
{
    wxToolBarBase::SetWindowStyleFlag(style);

    if ( !m_qtToolBar )
        return;

    m_qtToolBar->setOrientation( IsVertical() ? Qt::Vertical : Qt::Horizontal);

    Qt::ToolButtonStyle buttonStyle = GetButtonStyle();

    // bring the initial state of all the toolbar items in line with the
    for ( wxToolBarToolsList::const_iterator i = m_tools.begin();
          i != m_tools.end();         ++i )
    {
        wxToolBarTool* tool = static_cast<wxToolBarTool*>(*i);
        if(!tool->m_qtToolButton)
            continue;

        tool->m_qtToolButton->setToolButtonStyle(buttonStyle);
    }
}

bool wxToolBar::Realize()
{
    if ( !wxToolBarBase::Realize() )
        return false;

    // bring the initial state of all the toolbar items in line with the
    for ( wxToolBarToolsList::const_iterator i = m_tools.begin();
          i != m_tools.end();         ++i )
    {
        wxToolBarTool* tool = static_cast<wxToolBarTool*>(*i);
        if(!tool->m_qtToolButton)
            continue;

        tool->m_qtToolButton->setEnabled(tool->IsEnabled());
        tool->m_qtToolButton->setChecked(tool->IsToggled());

        if (HasFlag(wxTB_NO_TOOLTIPS))
            tool->ClearToolTip();
        else
            tool->SetToolTip();
    }

    return true;
}

QActionGroup* wxToolBar::GetActionGroup(size_t pos)
{
    QActionGroup *actionGroup = NULL;
    if(pos > 0)
        actionGroup = QtToolBar()->actions().at(pos-1)->actionGroup();
    if(actionGroup == NULL && (int)pos < QtToolBar()->actions().size() - 1)
        actionGroup = QtToolBar()->actions().at(pos+1)->actionGroup();
    if(actionGroup == NULL)
        actionGroup = new QActionGroup(QtToolBar());
    return actionGroup;
}

bool wxToolBar::DoInsertTool(size_t pos, wxToolBarToolBase *toolBase)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);
    QAction *before = NULL;
    if(pos >= 0 && pos < (size_t)QtToolBar()->actions().size())
        before = QtToolBar()->actions().at(pos);

    QAction *action;
    switch ( tool->GetStyle() )
    {
        case wxTOOL_STYLE_BUTTON:
            tool->m_qtToolButton = new wxQtToolButton(this, tool);
            tool->m_qtToolButton->setToolButtonStyle(GetButtonStyle());
            tool->SetLabel( tool->GetLabel() );

            if (!HasFlag(wxTB_NOICONS))
                tool->SetIcon();
            if (!HasFlag(wxTB_NO_TOOLTIPS))
                tool->SetToolTip();

            action = QtToolBar()->insertWidget(before, tool->m_qtToolButton);

            switch (tool->GetKind())
            {
            default:
                wxFAIL_MSG("unknown toolbar child type");
                // fall through
            case wxITEM_RADIO:
                GetActionGroup(pos)->addAction(action);
                // fall-through
            case wxITEM_CHECK:
                tool->m_qtToolButton->setCheckable(true);
            case wxITEM_DROPDOWN:
            case wxITEM_NORMAL:
                break;
            }
            break;

        case wxTOOL_STYLE_SEPARATOR:
            if(tool->IsStretchable()) {
                QWidget* spacer = new QWidget();
                spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                QtToolBar()->insertWidget(before, spacer);
            } else
                QtToolBar()->insertSeparator(before);
            break;

        case wxTOOL_STYLE_CONTROL:
            wxWindow* control = tool->GetControl();
            QtToolBar()->insertWidget(before, control->GetHandle());
            break;
    }

    InvalidateBestSize();

    return true;
}

bool wxToolBar::DoDeleteTool(size_t /* pos */, wxToolBarToolBase *toolBase)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);
    delete tool->m_qtToolButton;
    tool->m_qtToolButton = NULL;

    InvalidateBestSize();
    return false;
}

void wxToolBar::DoEnableTool(wxToolBarToolBase *toolBase, bool enable)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);
    tool->m_qtToolButton->setEnabled(enable);
}

void wxToolBar::DoToggleTool(wxToolBarToolBase *toolBase, bool toggle)
{
    wxToolBarTool* tool = static_cast<wxToolBarTool*>(toolBase);
    tool->m_qtToolButton->setChecked(toggle);
}

void wxToolBar::DoSetToggle(wxToolBarToolBase * WXUNUSED(tool),
                            bool WXUNUSED(toggle))
{
    // VZ: absolutely no idea about how to do it
    wxFAIL_MSG( wxT("not implemented") );
}

wxToolBarToolBase *wxToolBar::CreateTool(int id, const wxString& label, const wxBitmap& bmpNormal,
                                      const wxBitmap& bmpDisabled, wxItemKind kind, wxObject *clientData,
                                      const wxString& shortHelp, const wxString& longHelp)
{
    return new wxToolBarTool(this, id, label, bmpNormal, bmpDisabled, kind,
                             clientData, shortHelp, longHelp);
}

wxToolBarToolBase *wxToolBar::CreateTool(wxControl *control,
                                          const wxString& label)
{
    return new wxToolBarTool(this, control, label);
}

Qt::ToolButtonStyle wxToolBar::GetButtonStyle()
{
    if(!HasFlag(wxTB_NOICONS)) {
        if(HasFlag(wxTB_HORZ_LAYOUT))
            return Qt::ToolButtonTextBesideIcon;
        else if (HasFlag(wxTB_TEXT))
            return Qt::ToolButtonTextUnderIcon;
        else
            return Qt::ToolButtonIconOnly;
    }
    return Qt::ToolButtonTextOnly;
}

#endif // wxUSE_TOOLBAR
