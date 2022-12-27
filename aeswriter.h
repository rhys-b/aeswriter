#include <wx/app.h>
#include <wx/frame.h>
#include <wx/menu.h>
#include <wx/richtext/richtextctrl.h>
#include <wx/filedlg.h>
#include <wx/file.h>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>

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

private:
	wxRichTextCtrl *text;
	wxFile *file;
	wxString password;

	void OnOpen( wxCommandEvent &evt );
	void OnSave( wxCommandEvent &evt );
	void OnSaveAs( wxCommandEvent &evt );
	void OnUndo( wxCommandEvent &evt );
	void OnRedo( wxCommandEvent &evt );

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
