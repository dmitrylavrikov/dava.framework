//
//  PackageTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include <QItemSelectionModel>

namespace Ui {
    class PackageDockWidget;
}

class PackageDocument;
class ControlNode;

class PackageDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit PackageDockWidget(QWidget *parent = 0);
    virtual ~PackageDockWidget();

    void SetDocument(PackageDocument *newDocument);

private:
    void RefreshActions(const QModelIndexList &indexList);
    void RefreshAction(QAction *action, bool enabled, bool visible);

private slots:
    void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void filterTextChanged(const QString &);
    void OnImport();
    void OnCopy();
    void OnPaste();
    void OnCut();
    void OnDelete();

signals:
    void SelectionRootControlChanged(const QList<ControlNode*> &activatedRootControls, const QList<ControlNode*> &deactivatedRootControls);
    void SelectionControlChanged(const QList<ControlNode*> &activatedControls, const QList<ControlNode*> &deactivatedControls);

private:
    Ui::PackageDockWidget *ui;
    PackageDocument *document;
    QAction *importPackageAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *cutAction;
    QAction *delAction;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
