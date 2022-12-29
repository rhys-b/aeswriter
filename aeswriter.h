#include <wx/app.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/strconv.h>

#include <sys/stat.h>
#include "aes.h"

class App : public wxApp
{
public:
	virtual bool OnInit();
};

class Window : public wxFrame
{
public:
	Window();
	~Window();

	wxRichTextCtrl *GetTextEditor();

private:
	wxRichTextCtrl *text;
	wxString filename;
	wxString password;

	void OnOpen( wxCommandEvent &evt );
	void OnSave( wxCommandEvent &evt );
	void OnSaveAs( wxCommandEvent &evt );
	void OnUndo( wxCommandEvent &evt );
	void OnRedo( wxCommandEvent &evt );
	void OnFind( wxCommandEvent &evt );

	// Attempts to open the file at the filename.
	// Returns 0 on success, -1 if the file could not be opened,
	// and -2 if the user couldn't get the password right.
	int OpenFile( wxString filename );

	// Closes the current file, including asking the user
	// if they want to save if the current file is not saved.
	void CloseFile();

	// Handles the actual saving of a file.
	void SaveFile();

	// Handles the save as functionality.
	void SaveFileAs();
};

enum
{
	FIND_DIALOG_ID, FIND_ID, REPLACE_ID, REPLACEALL_ID, FIND_CONTROL_ID, REPLACE_CONTROL_ID
};

class FindDialog : public wxDialog
{
public:
	FindDialog( Window *parent );	

private:
	wxStaticText *findlabel, *replacelabel;
	wxTextCtrl *findctrl, *replacectrl;
	wxButton *findbtn, *replacebtn, *replaceallbtn;
	Window *parent;

	void OnFind( wxCommandEvent &evt );
	void OnReplace( wxCommandEvent &evt );
	void OnReplaceAll( wxCommandEvent &evt );
	void OnSize( wxSizeEvent &evt );
};
