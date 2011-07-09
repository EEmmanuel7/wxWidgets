/////////////////////////////////////////////////////////////////////////////
// Name:        wx/osx/iphone/private/tablectrlimpl.h
// Purpose:     wxMoTableCtrl implementation classes that have to be exposed
// Author:      Linas Valiukas
// Modified by:
// Created:     2011-07-01
// RCS-ID:      $Id$
// Copyright:   (c) Linas Valiukas
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

#ifndef _WX_OSX_IPHONE_PRIVATE_TABLECTRL_H_
#define _WX_OSX_IPHONE_PRIVATE_TABLECTRL_H_

#include "wx/osx/private.h"
#include "wx/mobile/native/tablectrl.h"


#pragma mark Cocoa

@interface wxUITableView : UITableView
{
}
@end

@interface wxUITableViewController : UITableViewController <UITableViewDataSource, UITableViewDelegate>
{
@private
    wxMoTableCtrl *moTableCtrl;
}

@property (nonatomic) wxMoTableCtrl *moTableCtrl;

- (id)initWithTableView:(wxUITableView *)initTableView
          wxMoTableCtrl:(wxMoTableCtrl *)initMoTableCtrl;

@end

@interface wxUITableViewController (Private)

- (wxMoTableDataSource *)moDataSource;
- (UITableViewCell *)tableView:(UITableView *)tableView
    dummyCellForRowAtIndexPath:(NSIndexPath *)indexPath;

@end



class wxTableViewCtrlIPhoneImpl : public wxWidgetIPhoneImpl, public wxTableViewControllerImpl
{
public:
    wxTableViewCtrlIPhoneImpl( wxWindowMac* peer,
                              wxUITableViewController* tableViewController,
                              wxUITableView *tableView );
    
    virtual bool ReloadData();
        
private:
    wxUITableViewController *m_tableViewController;
    wxUITableView *m_tableView;
    wxMoTableCtrl *m_tableCtrl;
};

#endif // _WX_OSX_IPHONE_PRIVATE_TABLECTRL_H_