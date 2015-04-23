#include "PropertiesWidget.h"

#include <qitemeditorfactory>
#include <qstyleditemdelegate>
#include <QMenu>
#include <QItemSelection>

#include "UI/QtModelPackageCommandExecutor.h"
#include "Model/ControlProperties/ComponentPropertiesSection.h"

#include <QAbstractItemModel>
#include "SharedData.h"

#include "ui_PropertiesWidget.h"
#include "PropertiesModel.h"
#include "UI/Properties/PropertiesTreeItemDelegate.h"

#include "UI/Components/UIComponent.h"

using namespace DAVA;

PropertiesWidget::PropertiesWidget(QWidget *parent)
    : QDockWidget(parent)
    , sharedData(nullptr)
    , addComponentAction(nullptr)
    , removeComponentAction(nullptr)
{
    setupUi(this);
    treeView->setItemDelegate(new PropertiesTreeItemDelegate(this));
    
    QMenu *addComponentMenu = new QMenu(this);
    for (int32 i = 0; i < UIComponent::COMPONENT_COUNT; i++)
    {
        const char *name = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(i);
        QAction *componentAction = new QAction(name, this); // TODO: Localize name
        componentAction->setData(i);
        addComponentMenu->addAction(componentAction);
    }
    connect(addComponentMenu, &QMenu::triggered, this, &PropertiesWidget::OnAddComponent);

    addComponentAction = new QAction(tr("Add Component"), this);
    addComponentAction->setEnabled(false);
    addComponentAction->setMenu(addComponentMenu);
    
    removeComponentAction = new QAction(tr("Remove Component"), this);
    removeComponentAction->setEnabled(false);
    connect(removeComponentAction, &QAction::triggered, this, &PropertiesWidget::OnRemoveComponent);
    
    treeView->addAction(addComponentAction);
    treeView->addAction(removeComponentAction);
}

void PropertiesWidget::OnDocumentChanged(SharedData *arg)
{
    sharedData = arg;
    UpdateActivatedControls();
}

void PropertiesWidget::OnDataChanged(const QByteArray &role)
{
    if (role == "activatedControls")
    {
        UpdateActivatedControls();
    }
}

void PropertiesWidget::OnAddComponent(QAction *action)
{
    if (sharedData)
    {
        uint32 componentType = action->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->AddComponent(node, componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnRemoveComponent()
{
    if (sharedData)
    {
        uint32 componentType = removeComponentAction->data().toUInt();
        if (componentType < UIComponent::COMPONENT_COUNT)
        {
            ControlNode *node = GetSelectedControlNode();
            sharedData->GetDocument()->GetCommandExecutor()->RemoveComponent(node, componentType);
        }
        else
        {
            DVASSERT(componentType < UIComponent::COMPONENT_COUNT);
        }
    }
}

void PropertiesWidget::OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (!selected.empty())
    {
        const QItemSelectionRange &range = selected.first();
        const QPersistentModelIndex &index = range.topLeft();
        AbstractProperty *property = static_cast<AbstractProperty*>(index.internalPointer());
        
        bool enabled = property->CanRemove();

        if (enabled)
        {
            ComponentPropertiesSection *section = dynamic_cast<ComponentPropertiesSection*>(property);
            if (section)
            {
                int componentType = (int) section->GetComponent()->GetType();
                removeComponentAction->setData(componentType);
            }
            else
            {
                enabled = false;
            }
        }
        removeComponentAction->setEnabled(enabled);
    }
}

ControlNode *PropertiesWidget::GetSelectedControlNode() const
{
    if (!sharedData)
        return nullptr;
    
    const QList<ControlNode*> &activatedControls = sharedData->GetData("activatedControls").value<QList<ControlNode*> >();
    if (activatedControls.empty())
        return nullptr;
    
    return activatedControls.first();
    
}

void PropertiesWidget::UpdateActivatedControls()
{
    QAbstractItemModel *prevModel = treeView->model();
    if (nullptr == sharedData)
    {
        treeView->setModel(nullptr);
    }
    else
    {
        ControlNode *node = GetSelectedControlNode();
        if (node)
            treeView->setModel(new PropertiesModel(node, sharedData->GetDocument()->GetCommandExecutor())); //TODO this is ugly // -- why?
        else
            treeView->setModel(nullptr);
        
        treeView->expandToDepth(0);
        treeView->resizeColumnToContents(0);
    }
    
    addComponentAction->setEnabled(treeView->model() != nullptr);
    removeComponentAction->setEnabled(false);
    
    if (treeView->model() != nullptr)
    {
        connect(treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &PropertiesWidget::OnSelectionChanged);
    }
    
    delete prevModel;
}
