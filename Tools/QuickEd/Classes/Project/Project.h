#ifndef QUICKED__PROJECT_H__
#define QUICKED__PROJECT_H__

#include <QObject>
#include "Model/LegacyEditorUIPackageLoader.h"
#include "Project/EditorFontSystem.h"
#include "Project/EditorLocalizationSystem.h"

class PackageNode;

class Project : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOpen READ IsOpen NOTIFY IsOpenChanged)

public:
    Project(QObject *parent = nullptr);
    virtual ~Project();
    bool Open(const QString &path);
    bool CheckAndUnlockProject(const QString& projectPath);

    DAVA::RefPtr<PackageNode> NewPackage(const QString &path);
    DAVA::RefPtr<PackageNode> OpenPackage(const QString &path);
    bool SavePackage(PackageNode *package);
    EditorFontSystem *GetEditorFontSystem() const;
    EditorLocalizationSystem *GetEditorLocalizationSystem() const;
signals:
    void ProjectOpened();

private:
    bool OpenInternal(const QString &path);
    DAVA::FilePath projectPath;
    
    LegacyControlData *legacyData;
    EditorFontSystem *editorFontSystem;
    EditorLocalizationSystem *editorLocalizationSystem;
    //properties
public:
    bool IsOpen() const;
signals:
    void IsOpenChanged(bool arg);
private:
    void SetIsOpen(bool arg);
    bool isOpen;
};

inline EditorFontSystem* Project::GetEditorFontSystem() const
{
    return editorFontSystem;
}

inline EditorLocalizationSystem* Project::GetEditorLocalizationSystem() const
{
    return editorLocalizationSystem;
}



#endif // QUICKED__PROJECT_H__
