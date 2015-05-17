#ifndef __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
#define __UI_EDITOR_IMPORTED_PACKAGES_NODE_H__

#include "PackageBaseNode.h"

#include "PackageControlsNode.h"

class PackageSerializer;

class ImportedPackagesNode : public PackageBaseNode
{
public:
    ImportedPackagesNode(PackageBaseNode *parent);
    virtual ~ImportedPackagesNode();

    void Add(PackageControlsNode *node);
    void InsertAtIndex(DAVA::int32 index, PackageControlsNode *node);
    void Remove(PackageControlsNode *node);
    virtual int GetCount() const override;
    virtual PackageControlsNode *Get(DAVA::int32 index) const override;
    
    virtual DAVA::String GetName() const override;
    bool CanInsertImportedPackage() const override;

    PackageControlsNode *FindPackageControlsNodeByName(const DAVA::String &name) const;

    
    void Serialize(PackageSerializer *serializer) const;
    void Serialize(PackageSerializer *serializer, const DAVA::Set<PackageRef*> &packageRefs) const;

protected:
    virtual bool IsReadOnly() const override;
    
private:
    DAVA::Vector<PackageControlsNode*> packageControlsNode;
};

#endif //__UI_EDITOR_IMPORTED_PACKAGES_NODE_H__
