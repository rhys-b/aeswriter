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

	Bind(wxEVT_MENU, &Window::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_MENU, &Window::OnSave, this, wxID_SAVE); 
	Bind(wxEVT_MENU, &Window::OnSaveAs, this, wxID_SAVEAS);
	Bind(wxEVT_MENU, &Window::OnUndo, this, wxID_UNDO);
	Bind(wxEVT_MENU, &Window::OnRedo, this, wxID_REDO);

	text = new wxRichTextCtrl(this);

	Show();

	if (wxGetApp().argc < 2)
		this->file = nullptr;
	else if (OpenFile(wxGetApp().argv[1]) == -1)
		wxMessageBox("File Unopenable or Nonexistant");
}

Window::~Window()
{
	// wxFile destructor will close the file.
	delete file;
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
	if (file == nullptr)
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

int Window::OpenFile( wxString filename )
{
	// Get the path of the file from the user.
	char path[256];
	strncpy(path, filename.mb_str(), 256);

	// Get the size of the file, and declare a buffer for it.
	struct stat meta;
	if (stat(path, &meta) == -1)
	{
		file = nullptr;
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
		edcrypt(0, (uint8_t*)buffer, size, password.mb_str());

		wxString string = wxString((const char*)buffer);
		if (string.StartsWith(FINGERPRINT))
		{
			text->SetValue(string.SubString(wxString(FINGERPRINT).Length(), string.Length()));
			this->file = new wxFile(path, wxFile::read_write);
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
		this->file = nullptr;
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
	if (file == nullptr)
	{
		SaveFileAs();
		return;
	}

	// Prepend the text with a header.
	wxString output = "[AES WRITER]" + text->GetValue();

	// Turn the data in the editor into bytes of multiple 16 bytes.
	unsigned long original_size = strlen(output.mb_str());
	unsigned long size = (original_size / 16 + (original_size % 16 != 0)) * 16;

	std::byte *buffer = new std::byte[size];
	strcpy((char*)buffer, output.mb_str());
	for (unsigned long i = original_size; i < size; i++) buffer[i] = (std::byte)0;

	// Encrypt the bytes.
	edcrypt(true, (uint8_t*)buffer, size, password.mb_str());

	// Save the file.
	file->Write((char*)buffer, size);

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

		file = new wxFile(dialog.GetFilename(), wxFile::read_write);
		SaveFile();
	}
}
