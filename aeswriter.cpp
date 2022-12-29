#define FINGERPRINT "[AES WRITER]"

#include "aeswriter.h"

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
	Window *window = new Window();
	return true;
}

Window::Window() : wxFrame( nullptr, wxID_ANY, "AES Writer", wxDefaultPosition, wxSize(900, 600) )
{
	wxMenuBar *menubar = new wxMenuBar();
	SetMenuBar(menubar);

	wxMenu *file = new wxMenu(wxID_FILE);
	wxMenu *edit = new wxMenu(wxID_EDIT);

	menubar->Append(file, "File");
	menubar->Append(edit, "Edit");

	file->Append(wxID_OPEN);
	file->Append(wxID_SAVE);
	file->Append(wxID_SAVEAS, "Save As...\tCtrl-Shift-S");

	edit->Append(wxID_UNDO);
	edit->Append(wxID_REDO);
	edit->Append(wxID_FIND);

	Bind(wxEVT_MENU, &Window::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &Window::OnSave, this, wxID_SAVE); 
	Bind(wxEVT_MENU, &Window::OnSaveAs, this, wxID_SAVEAS);
	Bind(wxEVT_MENU, &Window::OnUndo, this, wxID_UNDO);
	Bind(wxEVT_MENU, &Window::OnRedo, this, wxID_REDO);
	Bind(wxEVT_MENU, &Window::OnFind, this, wxID_FIND);

	text = new wxRichTextCtrl(this);

	Show();

	if (wxGetApp().argc < 2)
		this->filename = "";
	else if (OpenFile(wxGetApp().argv[1]) == -1)
		wxMessageBox("File Unopenable or Nonexistant");
}

Window::~Window()
{
	CloseFile();
}

void Window::OnOpen( wxCommandEvent &evt )
{
	CloseFile();

	wxFileDialog dialog(this);

	if (dialog.ShowModal() == wxID_OK)
		OpenFile(dialog.GetPath());
}

void Window::OnSave( wxCommandEvent &evt )
{
	if (filename == "")
	{
		OnSaveAs(evt);
		return;
	}

	SaveFile();
}

void Window::OnSaveAs( wxCommandEvent &evt )
{
	SaveFileAs();
}

void Window::OnUndo( wxCommandEvent &evt )
{
	text->Undo();
}

void Window::OnRedo( wxCommandEvent &evt )
{
	text->Redo();
}

void Window::OnFind( wxCommandEvent &evt )
{
	FindDialog *dialog = new FindDialog(this);
	dialog->Show();
}

int Window::OpenFile( wxString filename )
{
	// Get the path of the file from the user.
	char path[256];
	strncpy(path, filename.utf8_str(), 256);

	// Get the size of the file, and declare a buffer for it.
	struct stat meta;
	if (stat(path, &meta) == -1)
	{
		filename = "";
		return -1;
	}

	unsigned long size = (meta.st_size / 16 + (meta.st_size % 16 != 0)) * 16;
	std::byte *buffer = new std::byte[size];

	// Read the data in from the file.
	FILE *file = fopen(path, "rb");
	if (file == NULL)
	{
		delete[] buffer;
		file = nullptr;
		return -1;
	}
	fread(buffer, 1, meta.st_size, file);
	fclose(file);

	for (unsigned long i = meta.st_size; i < size; i++) buffer[i] = (std::byte)0;
	std::byte *store = new std::byte[size];
	memcpy(store, buffer, size);

	// Get the password from the user.
	wxString password = wxGetPasswordFromUser("Enter Password");
	while (password != "")
	{
		// Decrypt the data.
		edcrypt(0, (uint8_t*)buffer, size, password.utf8_str());

		wxString string = wxString((const char*)buffer, wxMBConvUTF8());
		if (string.StartsWith(FINGERPRINT))
		{
			text->SetValue(string.SubString(wxString(FINGERPRINT).Length(), string.Length()));
			this->filename = wxString(filename);
			this->password = wxString(password);
			break;
		}
		else
		{
			password = wxGetPasswordFromUser("Incorrect Password");
			memcpy(buffer, store, size);
		}
	}

	delete[] buffer;
	delete[] store;

	if (this->password == "")
	{
		this->filename = "";
		return -2;
	}

	text->DiscardEdits();
	return 0;
}

void Window::CloseFile()
{
	if (text->IsModified())
	{
		wxMessageDialog dialog( this, "The file is not saved.", "Unsaved", wxOK | wxCANCEL );
		dialog.SetOKCancelLabels( wxMessageDialog::ButtonLabel("Save"),
		                          wxMessageDialog::ButtonLabel("Don't Save") );

		if (dialog.ShowModal() == wxID_OK) SaveFile();
	}
}

void Window::SaveFile()
{
	if (filename == "")
	{
		SaveFileAs();
		return;
	}

	// Prepend the text with a header.
	wxString output = "[AES WRITER]" + text->GetValue();

	// Turn the data in the editor into bytes of multiple 16 bytes.
	unsigned long original_size = strlen(output.utf8_str());
	unsigned long size = (original_size / 16 + (original_size % 16 != 0)) * 16;

	std::byte *buffer = new std::byte[size];
	strcpy((char*)buffer, output.utf8_str());
	for (unsigned long i = original_size; i < size; i++) buffer[i] = (std::byte)0;

	// Encrypt the bytes.
	edcrypt(true, (uint8_t*)buffer, size, password.utf8_str());

	// Save the file.
	wxFile file(filename, wxFile::write);
	file.Write((char*)buffer, size);
	file.Close();

	delete[] buffer;
	text->DiscardEdits();
}

void Window::SaveFileAs()
{
	wxFileDialog dialog( this,
	                     wxFileSelectorPromptStr,
	                     wxEmptyString,
	                     wxEmptyString,
	                     wxFileSelectorDefaultWildcardStr,
	                     wxFD_SAVE );

	if (dialog.ShowModal() == wxID_OK)
	{
		wxString password1 = "a";
		wxString password2 = "b";
		wxString title = "Enter Password";

		while (password1 != password2)
		{
			password1 = wxGetPasswordFromUser(title);
			if (password1 == "") return; // User hit cancel.

			password2 = wxGetPasswordFromUser("Confirm Password");
			if (password2 == "") return; // User hit cancel.

			title = "Passwords didn't match.\nEnter password.";
		}

		password = password1;

		// Open a file in read/write mode or create it if it does not exist.
		if (!wxFile::Exists(dialog.GetFilename()))
		{
			wxFile tmp(dialog.GetFilename(), wxFile::write);
			// tmp gets closed when its destructor gets called here.
		}

		filename = dialog.GetFilename();
		SaveFile();
	}
}

wxRichTextCtrl *Window::GetTextEditor()
{
	return text;
}

FindDialog::FindDialog( Window *parent )
	: wxDialog(parent, FIND_DIALOG_ID, "Find & Replace", wxDefaultPosition, wxSize(400, 150) )
{
	this->parent = parent;

	findlabel = new wxStaticText( this,
	                              wxID_ANY,
	                              "Find",
	                              wxDefaultPosition,
	                              wxDefaultSize,
	                              wxALIGN_RIGHT);

	replacelabel = new wxStaticText( this,
	                                 wxID_ANY,
	                                 "Replace",
	                                 wxDefaultPosition,
	                                 wxDefaultSize,
	                                 wxALIGN_RIGHT);

	findctrl = new wxTextCtrl( this,
	                           FIND_CONTROL_ID,
	                           wxEmptyString,
	                           wxDefaultPosition,
	                           wxDefaultSize,
	                           wxTE_PROCESS_ENTER);

	replacectrl = new wxTextCtrl( this,
	                              REPLACE_CONTROL_ID,
	                              wxEmptyString,
	                              wxDefaultPosition,
	                              wxDefaultSize,
	                              wxTE_PROCESS_ENTER);

	findbtn = new wxButton(this, FIND_ID, "Find");
	replacebtn = new wxButton(this, REPLACE_ID, "Replace");
	replaceallbtn = new wxButton(this, REPLACEALL_ID, "Replace All");

	Bind(wxEVT_SIZE, &FindDialog::OnSize, this, FIND_DIALOG_ID);
	Bind(wxEVT_BUTTON, &FindDialog::OnFind, this, FIND_ID);
	Bind(wxEVT_BUTTON, &FindDialog::OnReplace, this, REPLACE_ID);
	Bind(wxEVT_BUTTON, &FindDialog::OnReplaceAll, this, REPLACEALL_ID);
	Bind(wxEVT_TEXT_ENTER, &FindDialog::OnFind, this, FIND_CONTROL_ID);
	Bind(wxEVT_TEXT_ENTER, &FindDialog::OnReplace, this, REPLACE_CONTROL_ID);
}

#include <iostream>
void FindDialog::OnSize( wxSizeEvent &evt )
{
	int width, height;
	GetClientSize(&width, &height);
	
	const double rowheight = height / 3.0;
	const double colwidth = width / 3.0;

	int textheight, textwidth;
	findlabel->GetTextExtent(findlabel->GetLabel(), &textwidth, &textheight);


	findlabel->SetSize( 0,
	                    round((rowheight/2.0) - (textheight/2.0)),
	                    90,
	                    round(rowheight) );

	replacelabel->SetSize( 0,
	                       round(rowheight + (rowheight/2.0) - (textheight/2.0)),
	                       90,
	                       round(rowheight) );

	findctrl->SetSize(100, 0, width-100, round(rowheight));
	replacectrl->SetSize(100, round(rowheight), width-100, round(rowheight));

	findbtn->SetSize(0, round(rowheight*2), round(colwidth), round(rowheight));
	replacebtn->SetSize(round(colwidth), round(rowheight*2), round(colwidth), round(rowheight));
	replaceallbtn->SetSize(round(colwidth*2), round(rowheight*2), round(colwidth), round(rowheight));
}

void FindDialog::OnFind( wxCommandEvent &evt )
{
	wxString find = findctrl->GetValue();
	if (find == "") return;

	wxString str = parent->GetTextEditor()->GetValue();
	size_t index = str.find(find, parent->GetTextEditor()->GetCaretPosition());

	if (index == -1)
	{
		index = str.find(find);
		if (index == -1) return;
	}

	parent->GetTextEditor()->SetSelection(index, index + find.Length());
}

void FindDialog::OnReplace( wxCommandEvent &evt )
{
	if (parent->GetTextEditor()->HasSelection())
	{
		long from, to;
		parent->GetTextEditor()->GetSelection(&from, &to);
		parent->GetTextEditor()->Replace(from, to, replacectrl->GetValue());
	}

	OnFind(evt);
}

void FindDialog::OnReplaceAll( wxCommandEvent &evt )
{
	if (findctrl->GetValue() == "") return;

	wxRichTextCtrl *textctrl = parent->GetTextEditor();
	wxString str = textctrl->GetValue();
	wxString find = findctrl->GetValue();
	wxString replace = replacectrl->GetValue();
	size_t length = find.Length();
	wxString batchname = "ReplaceAllBatch";

	textctrl->BeginBatchUndo(batchname);

	size_t index = 0;
	while ((ssize_t)(index = str.find(find, index)) != -1)
	{
		textctrl->Replace(index, index + length, replace);
		index++;
	}

	textctrl->EndBatchUndo();
}
