/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "localizationeditordialog.h"
#include "ui_localizationeditordialog.h"
#include "Helpers/LocalizationSystemHelper.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/FileSystem.h"
#include "Helpers/ResourcesManageHelper.h"

#include "EditorFontManager.h"

#include "regexpinputdialog.h"

#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include "Utils/QtDavaConvertion.h"

using namespace DAVA;

//TODO: move to utils
// Helper methods to convert between QString and WideString.
// Don't use QString::fromStdWstring() here, it is not compatible with "treat wchar_t as embedded"
// option currently set for Framework.
const int LocalizationTableController::LOCALIZATION_KEY_INDEX = 0;
const int LocalizationTableController::LOCALIZATION_VALUE_INDEX = 1;

const QString LocalizationTableController::DEFAULT_LOCALIZATION_KEY = "LOCALIZATION_KEY_%1";
const QString LocalizationTableController::DEFAULT_LOCALIZATION_VALUE = "LOCALIZATION_VALUE_%1";


const int LocalizationFontsTableController::LOCALIZATION_FONT_SIZE_INDEX = 2;
const QString LocalizationFontsTableController::DEFAULT_LOCALIZATION_FONT_SIZE = "LOCALIZATION_FONT_SIZE_%1";

//---------------------------------------------------------------------

LocalizationTableController::LocalizationTableController(QTableView* view)
 : tableView(view)
 , tableModel(NULL)
 , sortOrder(Qt::AscendingOrder)
{
}

LocalizationTableController::~LocalizationTableController()
{
    SAFE_DELETE(tableModel);
}

void LocalizationTableController::ConnectToSignals()
{
    // Change key and value when selected table item is changed.
    connect(tableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,
            SLOT(OnLocalizationStringSelected(const QItemSelection &, const QItemSelection &))
            );

    // Connect to column click to change items order.
    tableView->horizontalHeader()->setSortIndicatorShown(true);
    connect(tableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(OnTableHeaderClicked(int)));

    // Connect to the table view to show custom menu.
    tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
}

void LocalizationTableController::DisconnectFromSignals()
{
    // Change key and value when selected table item is changed.
    disconnect(tableView->selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,
            SLOT(OnLocalizationStringSelected(const QItemSelection &, const QItemSelection &))
            );
    
    // Connect to column click to change items order.
    disconnect(tableView->horizontalHeader(), SIGNAL(sectionClicked(int)),
            this, SLOT(OnTableHeaderClicked(int)));
    
    // Connect to the table view to show custom menu.
    disconnect(tableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(OnShowCustomMenu(const QPoint&)));
}

void LocalizationTableController::SetupTable(QObject *parent)
{
    //Setup table appearence
    tableView->verticalHeader()->hide();
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->horizontalHeader()->setSectionResizeMode/*setResizeMode*/(QHeaderView::Stretch);
    //Disable editing of table
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    CreateTableModel(parent);
//    //TODO: Create and set table view model
}

void LocalizationTableController::ReloadTable()
{
	//Remember the selected item (if any) to restore it when the selection will be changed.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (selectionModel && selectionModel->hasSelection() && selectionModel->selectedIndexes().size() > 0)
	{
		selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	}
    
	// Do the cleanup.
	CleanupUIControls();
    if (tableModel->rowCount() > 0)
    {
        tableModel->removeRows(0, tableModel->rowCount());
    }
    
    if(LoadTable())
    {
        // Restore the selection if possible.
        if (selectedItemIndex.isValid())
        {
            tableView->selectRow(selectedItemIndex.row());
        }
    }
}

bool LocalizationTableController::IsValidSelection()
{
	QItemSelectionModel* selectionModel = tableView->selectionModel();
    if (selectionModel && selectionModel->hasSelection() && (selectionModel->selectedIndexes().size() > 0))
	{
		return true;
	}
    return false;
}

void LocalizationTableController::ProcessDeselectedItem(const QItemSelection & deselected)
{
	// Just do update.
	if (deselected.indexes().size() == 0)
	{
		// Nothing is selected - nothing to do more.
		return;
	}
	
	QModelIndex deselectedIndex = deselected.indexes().takeAt(0);
	if (!deselectedIndex.isValid())
	{
		return;
	}
    
    OnDeselectedItem(deselectedIndex);
}

void LocalizationTableController::OnDeselectedItem(const QModelIndex & deselectedIndex)
{
    //TODO: same deselect logic for strings and fonts?
}

void LocalizationTableController::ProcessSelectedItem(const QItemSelection & selected)
{
	CleanupUIControls();
	if (selected.indexes().size() == 0)
	{
		// Nothing is selected - nothing to do more.
		return;
	}
	
	QModelIndex selectedIndex = selected.indexes().takeAt(0);
	if (!selectedIndex.isValid())
	{
		return;
	}
	
    OnSelectedItem(selectedIndex);
}

void LocalizationTableController::OnSelectedItem(const QModelIndex & selectedIndex)
{
    UpdateUIControls(selectedIndex);
}

void LocalizationTableController::OnLocalizationStringSelected(const QItemSelection & selected, const QItemSelection & deselected)
{
	ProcessDeselectedItem(deselected);
	ProcessSelectedItem(selected);
}

void LocalizationTableController::OnTableHeaderClicked(int headerIndex)
{
	// Revert the current sort order and re-apply it.
	if (sortOrder == Qt::DescendingOrder)
	{
		sortOrder = Qt::AscendingOrder;
	}
	else
	{
		sortOrder = Qt::DescendingOrder;
	}
	
	ApplySortOrder(headerIndex);
}

void LocalizationTableController::ApplySortOrder(int headerIndex)
{
	tableView->horizontalHeader()->setSortIndicator(headerIndex, sortOrder);
	tableView->sortByColumn(headerIndex, sortOrder);
}

void LocalizationTableController::OnShowCustomMenu(const QPoint& /*pos*/)
{
	QMenu menu;
	QAction* addStringAction = new QAction("Add string", &menu);
	connect(addStringAction, SIGNAL(triggered()), this, SLOT(OnAddLocalizationStringAction()));
	menu.addAction(addStringAction);
    
	QAction* removeStringAction = new QAction("Remove selected string", &menu);
	connect(removeStringAction, SIGNAL(triggered()), this, SLOT(OnRemoveLocalizationStringAction()));
	menu.addAction(removeStringAction);
    
	menu.exec(QCursor::pos());
}

void LocalizationTableController::OnFilterTextChanged(const QString& value)
{
    filterValue = value;
    ReloadTable();
}

void LocalizationTableController::OnFilterTextCleared()
{
    filterValue.clear();
    QLineEdit* senderWidget = dynamic_cast<QLineEdit*>(QObject::sender());
    if(senderWidget)
    {
        senderWidget->clear();
    }
}


void LocalizationTableController::SelectStringItemByKey(const QString& keyToBeSelected)
{
	int rowsCount = tableModel->rowCount();
	for (int i = 0; i < rowsCount; i ++)
	{
		QModelIndex rowIndex = tableModel->index(i, LOCALIZATION_KEY_INDEX);
		if (!rowIndex.isValid())
		{
			continue;
		}
        
		QString localizationKey = tableModel->data(rowIndex, Qt::DisplayRole).toString();
		if (localizationKey == keyToBeSelected)
		{
			tableView->selectRow(i);
			break;
		}
	}
}

//---------------------------------------------------------------------

LocalizationStringsTableController::LocalizationStringsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog)
: LocalizationTableController(view)
, ui(dialog)
{
    
}

LocalizationStringsTableController::~LocalizationStringsTableController()
{

}

void LocalizationStringsTableController::CreateTableModel(QObject* parent)
{
    //Create and set table view model
    tableModel = new QStandardItemModel(parent); //0 Rows and 2 Columns
    
    //Setup column name
    tableModel->setHorizontalHeaderItem(LOCALIZATION_KEY_INDEX, new QStandardItem(QString("Key")));
    tableModel->setHorizontalHeaderItem(LOCALIZATION_VALUE_INDEX, new QStandardItem(QString("Localized Value")));
    
    tableView->setModel(tableModel);
}

bool LocalizationStringsTableController::LoadTable()
{
	Map<WideString, WideString> localizationTable;
	bool stringsFound = LocalizationSystem::Instance()->GetStringsForCurrentLocale(localizationTable);
	if (!stringsFound)
	{
		return false;
	}
    
	// Fill the values.
	for (Map<WideString, WideString>::iterator iter = localizationTable.begin(); iter != localizationTable.end(); iter ++)
	{
        // Add only strings which pass filter (or all strings if filter is not defined).
        QString keyValue = WideStringToQString(iter->first);
        if (filterValue.isEmpty() || keyValue.contains(filterValue, Qt::CaseInsensitive))
        {
            QList<QStandardItem *> itemsList;
            itemsList.append(new QStandardItem(keyValue));
            itemsList.append(new QStandardItem(WideStringToQString(iter->second)));
            tableModel->appendRow(itemsList);
        }
    }
    return true;
}

void LocalizationStringsTableController::UpdateUIControls(const QModelIndex &selectedIndex)
{
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
	QString localizationValue = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_VALUE_INDEX), Qt::DisplayRole).toString();
    
	ui->keyTextEdit->insertPlainText(localizationKey);
	ui->valueTextEdit->insertPlainText(localizationValue);
}

void LocalizationStringsTableController::CleanupUIControls()
{
	ui->keyTextEdit->clear();
	ui->valueTextEdit->clear();
}

void LocalizationStringsTableController::OnDeselectedItem(const QModelIndex & deselectedIndex)
{
	UpdateLocalizationValueForCurrentKey(deselectedIndex);
}

void LocalizationStringsTableController::UpdateLocalizationValueForCurrentKey()
{
	// Update the value for the item currently selected by default.
	QModelIndex selectedItemIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (!selectionModel || !selectionModel->hasSelection() ||
		!(selectionModel->selectedIndexes().size() > 0))
	{
		return;
	}
	
	selectedItemIndex = selectionModel->selectedIndexes().takeAt(0);
	UpdateLocalizationValueForCurrentKey(selectedItemIndex);
}

void LocalizationStringsTableController::UpdateLocalizationValueForCurrentKey(const QModelIndex& selectedItemIndex)
{
	if (!selectedItemIndex.isValid())
	{
		return;
	}
    
	// Firstly verify if something was changed.
	QString localizationKey = ui->keyTextEdit->toPlainText();
	QString localizationValue = ui->valueTextEdit->toPlainText();
	if (localizationKey.isEmpty())
	{
		return;
	}
	
	QString existingLocalizationValue = tableModel->data(tableModel->index(selectedItemIndex.row(), 1), Qt::DisplayRole).toString();
	if (existingLocalizationValue == localizationValue)
	{
		return;
	}
    
	// Change indeed happened - update the localized string.
	LocalizationSystem::Instance()->SetLocalizedString(QStringToWideString(localizationKey),
													   QStringToWideString(localizationValue));
	SaveLocalization();
    
	// Update the current localized string in the table.
	tableModel->setData(tableModel->index(selectedItemIndex.row(), 1), localizationValue);
    
	// Update the UI.
    //HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationStringsTableController::SaveLocalization()
{
	LocalizationSystem::Instance()->SaveLocalizedStrings();
}

QString LocalizationStringsTableController::GetSelectedLocalizationKey()
{
    QModelIndex selectedIndex = QModelIndex();
	QItemSelectionModel* selectionModel = tableView->selectionModel();
	if (selectionModel && selectionModel->hasSelection() && selectionModel->selectedIndexes().size() > 0)
	{
		selectedIndex = selectionModel->selectedIndexes().takeAt(0);
	}
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
    
    return localizationKey;
}

void LocalizationStringsTableController::AddNewLocalizationString(const QString &newLocalizationKey, const QString &newLocalizationValue)
{
    LocalizationSystem::Instance()->SetLocalizedString(QStringToWideString(newLocalizationKey),
                                                   QStringToWideString(newLocalizationValue));
    SaveLocalization();
    ReloadTable();

    SelectStringItemByKey(newLocalizationKey);
}

void LocalizationStringsTableController::RemoveSelectedLocalizationString(const QString &localizationKey)
{
    
	LocalizationSystem::Instance()->RemoveLocalizedString(QStringToWideString(localizationKey));
	SaveLocalization();
    
	ReloadTable();
}

//---------------------------------------------------------------------
LocalizationFontsTableController::LocalizationFontsTableController(QTableView* view, Ui::LocalizationEditorDialog *dialog)
: LocalizationTableController(view)
, ui(dialog)
{
    
}

LocalizationFontsTableController::~LocalizationFontsTableController()
{
    
}

void LocalizationFontsTableController::CreateTableModel(QObject* parent)
{
    //Create and set table view model
    tableModel = new QStandardItemModel(0, 2, parent); //0 Rows and 3 Columns
    
    //Setup column name
    tableModel->setHorizontalHeaderItem(LOCALIZATION_KEY_INDEX, new QStandardItem(QString("Font Preset Name")));
    tableModel->setHorizontalHeaderItem(LOCALIZATION_VALUE_INDEX, new QStandardItem(QString("Localized Font")));
    //tableModel->setHorizontalHeaderItem(LOCALIZATION_FONT_SIZE_INDEX, new QStandardItem(QString("Font Size")));
    
    tableView->setModel(tableModel);
}

bool LocalizationFontsTableController::LoadTable()
{
    const String &locale = LocalizationSystem::Instance()->GetCurrentLocale();
    Logger::Debug("LocalizationFontsTableController::LoadTable locale=%s", locale.c_str());
	const Map<String, Font*> &localizationFonts = EditorFontManager::Instance()->GetLocalizedFonts(locale);
	if (localizationFonts.empty())
	{
		return false;
	}
    
	// Fill the values.
    Map<String, Font*>::const_iterator it = localizationFonts.begin();
    Map<String, Font*>::const_iterator endIt = localizationFonts.end();
    
	for (; it != endIt; ++it)
	{
        // Add only strings which pass filter (or all strings if filter is not defined).
        QString keyValue(it->first.c_str());
        if (filterValue.isEmpty() || keyValue.contains(filterValue, Qt::CaseInsensitive))
        {
            Font* font = it->second;
            String fontDisplayName = EditorFontManager::Instance()->GetFontDisplayName(font);
            //QString fontName = QString::fromStdString(EditorFontManager::Instance()->GetFontDisplayName(font));
            QString fontName = QString("%1 %2").arg(font->GetSize()).arg(fontDisplayName.c_str());
            //QString fontSize = QString("%1").arg(font->GetSize());
            
            QList<QStandardItem *> itemsList;
            itemsList.append(new QStandardItem(keyValue));
            itemsList.append(new QStandardItem(fontName));
            //itemsList.append(new QStandardItem(fontSize));
            tableModel->appendRow(itemsList);
        }
    }
    return true;
}

void LocalizationFontsTableController::UpdateUIControls(const QModelIndex &selectedIndex)
{
    //TODO: update ui controls on fonts tab
    
	QString localizationKey = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_KEY_INDEX), Qt::DisplayRole).toString();
	QString localizationValue = tableModel->data(tableModel->index(selectedIndex.row(), LOCALIZATION_VALUE_INDEX), Qt::DisplayRole).toString();
//    
//	ui->keyTextEdit->insertPlainText(localizationKey);
//	ui->valueTextEdit->insertPlainText(localizationValue);
}

void LocalizationFontsTableController::CleanupUIControls()
{
    //TODO: cleanup ui controls on fonts tab
//	ui->keyTextEdit->clear();
//	ui->valueTextEdit->clear();
}

//---------------------------------------------------------------------

int LocalizationEditorDialog::addedStringsCount = 0;


LocalizationEditorDialog::LocalizationEditorDialog(QWidget *parent) :
    QDialog(parent)
,   ui(new Ui::LocalizationEditorDialog)
{
    ui->setupUi(this);
    
    stringsTable = new LocalizationStringsTableController(ui->stringsTableView, ui);
    fontsTable = new LocalizationFontsTableController(ui->fontsTableView, ui);
    
    stringsTable->SetupTable();
    fontsTable->SetupTable();
    
    FillLocaleComboBox();
    ConnectToSignals();
    SetLocalizationDirectoryPath();
    
//     HierarchyTreePlatformNode* platformNode = NULL;//HierarchyTreeController::Instance()->GetActivePlatform();
// 	if (platformNode)
// 	{
//     	QString platformName = platformNode ? platformNode->GetName() : "none";
//     	QString windowTitle = QString("Localization settings for platform \"%1\"").arg(platformName);
//     	setWindowTitle(windowTitle);
// 		// Enable open localization dialog button only if platfrom is available
// 		ui->openLocalizationFileButton->setEnabled(true);
// 
// 		// Apply default sort order when the loading is complete.
// 		stringsTable->ApplySortOrder(LocalizationTableController::LOCALIZATION_KEY_INDEX);
//         fontsTable->ApplySortOrder(LocalizationTableController::LOCALIZATION_KEY_INDEX);
// 	}
}

LocalizationEditorDialog::~LocalizationEditorDialog()
{
    DisconnectFromSignals();
    
    delete stringsTable;
    delete fontsTable;
    
    delete ui;
}

void LocalizationEditorDialog::FillLocaleComboBox()
{
    // Get count of supported languages
    int languagesCount = LocalizationSystemHelper::GetSupportedLanguagesCount();
    QString languageDescription;
    // Fill combobox with language values
    for (int i = 0; i < languagesCount; ++i) {
        languageDescription =  QString::fromStdString(LocalizationSystemHelper::GetSupportedLanguageDesc(i));
        ui->currentLocaleComboBox->addItem(languageDescription);
    }
    // Setup default locale
    SetDefaultLanguage();
}

void LocalizationEditorDialog::ConnectToSignals()
{
    stringsTable->ConnectToSignals();
    fontsTable->ConnectToSignals();
    
    // Open locale directory button clicked event
    connect(ui->openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    connect(ui->currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(CloseDialog()));
	
	connect(ui->addStringButton, SIGNAL(clicked()), this, SLOT(OnAddNewLocalizationString()));
	connect(ui->removeStringButton, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLocalizationString()));
    
    // Filter behaviour.
    connect(ui->filterLineEdit, SIGNAL(textChanged(const QString&)), this->stringsTable, SLOT(OnFilterTextChanged(const QString&)));
    connect(ui->clearFilterButton, SIGNAL(clicked()), this->stringsTable, SLOT(OnFilterTextCleared()));
    
    connect(ui->fontsFilterLineEdit, SIGNAL(textChanged(const QString&)), this->fontsTable, SLOT(OnFilterTextChanged(const QString&)));
    connect(ui->clearFontsFilterButton, SIGNAL(clicked()), this->fontsTable, SLOT(OnFilterTextCleared()));
}

void LocalizationEditorDialog::DisconnectFromSignals()
{
    stringsTable->DisconnectFromSignals();
    fontsTable->DisconnectFromSignals();
    
    // Open locale directory button clicked event
    disconnect(ui->openLocalizationFileButton, SIGNAL(clicked()), this, SLOT(OnOpenLocalizationFileButtonClicked()));
    // Locale combobox value changed event
    disconnect(ui->currentLocaleComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnCurrentLocaleChanged(int)));
    // Close dialog if ok button clicked
    disconnect(ui->closeButton, SIGNAL(clicked()), this, SLOT(CloseDialog()));
	
	disconnect(ui->addStringButton, SIGNAL(clicked()), this, SLOT(OnAddNewLocalizationString()));
	disconnect(ui->removeStringButton, SIGNAL(clicked()), this, SLOT(OnRemoveSelectedLocalizationString()));
    
    // Filter behaviour.
    disconnect(ui->filterLineEdit, SIGNAL(textChanged(const QString&)), this->stringsTable, SLOT(OnFilterTextChanged(const QString&)));
    disconnect(ui->clearFilterButton, SIGNAL(clicked()), this->stringsTable, SLOT(OnFilterTextCleared()));
    
    disconnect(ui->fontsFilterLineEdit, SIGNAL(textChanged(const QString&)), this->fontsTable, SLOT(OnFilterTextChanged(const QString&)));
    disconnect(ui->clearFontsFilterButton, SIGNAL(clicked()), this->fontsTable, SLOT(OnFilterTextCleared()));
}

void LocalizationEditorDialog::SetLocalizationDirectoryPath()
{
    QString defaultPath = QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath().GetAbsolutePathname());
    if (!defaultPath.isEmpty())
    {
        ui->localizationFilePathLineEdit->setText(defaultPath);
		stringsTable->ReloadTable();
        fontsTable->ReloadTable();
    }
}

void LocalizationEditorDialog::SetDefaultLanguage()
{
    // Get description for current language ID
    String currentLanguageID = LocalizationSystem::Instance()->GetCurrentLocale();
    String languageDescription = LocalizationSystemHelper::GetLanguageDescByLanguageID(currentLanguageID);

    // Setup combo box value
    int index = ui->currentLocaleComboBox->findText(QString::fromStdString(languageDescription));
    ui->currentLocaleComboBox->setCurrentIndex(index);
}

void LocalizationEditorDialog::OnOpenLocalizationFileButtonClicked()
{
	FilePath relativeLocalizationPath = LocalizationSystem::Instance()->GetDirectoryPath();
	QString absoluteLocalizationPath = QString::fromStdString(relativeLocalizationPath.GetAbsolutePathname());

	if (absoluteLocalizationPath.isEmpty())
	{
		absoluteLocalizationPath = ResourcesManageHelper::GetResourceRootDirectory();
	}

    QString fileDirectory = QFileDialog::getExistingDirectory(this, tr( "Select localization files directory" ), absoluteLocalizationPath);

	if(!fileDirectory.isNull() && !fileDirectory.isEmpty())
    {
		// Convert directory path into Unix-style path
		fileDirectory = ResourcesManageHelper::ConvertPathToUnixStyle(fileDirectory);

		if (ResourcesManageHelper::ValidateResourcePath(fileDirectory))
        {
			// Get localization relative path
			QString localizationRelativePath = ResourcesManageHelper::GetResourceRelativePath(fileDirectory);
      		// Update ui
      		ui->localizationFilePathLineEdit->setText(localizationRelativePath);
     		ReinitializeLocalizationSystem(localizationRelativePath);
        }
		else
		{
			ResourcesManageHelper::ShowErrorMessage(fileDirectory);
		}
     }
}

void LocalizationEditorDialog::OnCurrentLocaleChanged(int /*index*/)
{
    ReinitializeLocalizationSystem(QString::fromStdString(LocalizationSystem::Instance()->GetDirectoryPath().GetAbsolutePathname()));
}

void LocalizationEditorDialog::ReinitializeLocalizationSystem(const QString& localizationDirectory)
{
	// Store the latest changes before the reinitialization.
	stringsTable->UpdateLocalizationValueForCurrentKey();

    int languageItemID = this->ui->currentLocaleComboBox->currentIndex();
    if (languageItemID == -1)
    {
        return;
    }

    String languageId = LocalizationSystemHelper::GetSupportedLanguageID(languageItemID);
    
    // Re-initialize the Localization System with the new Locale.
    LocalizationSystem::Instance()->Cleanup();
    
    if (!localizationDirectory.isEmpty())
    {
        FilePath localizationFilePath(localizationDirectory.toStdString());
        localizationFilePath.MakeDirectoryPathname();

        Logger::Debug("HierarchyTreeController::ReinitializeLocalizationSystem LocalizationSystem::Instance()->SetCurrentLocale(%s);", languageId.c_str());
        LocalizationSystem::Instance()->SetCurrentLocale(languageId);
        LocalizationSystem::Instance()->InitWithDirectory(localizationFilePath);
    }
    
	stringsTable->ReloadTable();
    fontsTable->ReloadTable();
    
//    HierarchyTreeController::Instance()->UpdateLocalization(true);
}

void LocalizationEditorDialog::closeEvent(QCloseEvent* /*event*/)
{
	// Save the last-minute changes, if any.
//	UpdateLocalizationValueForCurrentKey();
    stringsTable->UpdateLocalizationValueForCurrentKey();
}

void LocalizationEditorDialog::CloseDialog()
{
//	UpdateLocalizationValueForCurrentKey();
    stringsTable->UpdateLocalizationValueForCurrentKey();
    
	accept();
}

void LocalizationEditorDialog::OnAddLocalizationStringAction()
{
	AddNewLocalizationString();
}

void LocalizationEditorDialog::OnRemoveLocalizationStringAction()
{
	RemoveSelectedLocalizationString();
}

void LocalizationEditorDialog::AddNewLocalizationString()
{
	addedStringsCount ++;
	QString newLocalizationKey = QString(LocalizationFontsTableController::DEFAULT_LOCALIZATION_KEY).arg(addedStringsCount);
	QString newLocalizationValue = QString(LocalizationFontsTableController::DEFAULT_LOCALIZATION_VALUE).arg(addedStringsCount);

	bool isOK = false;
	QRegExp asciiRegExp("[ -~]+");
	QString text = RegExpInputDialog::getText(this, "Localization Key",
										 "New Localization Key (ASCII characters only)",
										 newLocalizationKey, asciiRegExp, &isOK);
	if (isOK && !text.isEmpty())
	{
		newLocalizationKey = text;
        
        stringsTable->AddNewLocalizationString(newLocalizationKey, newLocalizationValue);
	}
	else
	{
		// String ID wasn't used, return it.
		addedStringsCount --;
	}
}

void LocalizationEditorDialog::RemoveSelectedLocalizationString()
{
    if(!stringsTable->IsValidSelection())
    {
        return;
    }
    
    QString localizationKey = stringsTable->GetSelectedLocalizationKey();

	int ret = QMessageBox::warning(this, qApp->applicationName(),
								   QString("Are you sure you want to delete localization string with key '%1'?").arg(localizationKey),
								   QMessageBox::Ok | QMessageBox::Cancel,
								   QMessageBox::Cancel);
	if (ret != QMessageBox::Ok)
	{
		return;
	}

    stringsTable->RemoveSelectedLocalizationString(localizationKey);
}

void LocalizationEditorDialog::OnAddNewLocalizationString()
{
	AddNewLocalizationString();
}

void LocalizationEditorDialog::OnRemoveSelectedLocalizationString()
{
	RemoveSelectedLocalizationString();
}

