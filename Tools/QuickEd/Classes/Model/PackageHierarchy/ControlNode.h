#ifndef __UI_EDITOR_CONTROL_NODE__
#define __UI_EDITOR_CONTROL_NODE__

#include "PackageBaseNode.h"
#include "ControlsContainerNode.h"

class PackageSerializer;
class PackageNode;
class ControlPrototype;
class PackageRef;
class RootProperty;

class ControlNode : public ControlsContainerNode
{
public:
    enum eCreationType
    {
        CREATED_FROM_CLASS,
        CREATED_FROM_PROTOTYPE,
        CREATED_FROM_PROTOTYPE_CHILD
    };
    
private:
    ControlNode(DAVA::UIControl *control);
    ControlNode(ControlNode *node);
    ControlNode(ControlPrototype *prototype, eCreationType creationType);
    virtual ~ControlNode();

public:
    static ControlNode *CreateFromControl(DAVA::UIControl *control);
    static ControlNode *CreateFromPrototype(ControlNode *sourceNode, PackageRef *nodePackage);
    static ControlNode *CreateFromPrototypeChild(ControlNode *sourceNode, PackageRef *nodePackage);

public:
    ControlNode *Clone();
    
    void Add(ControlNode *node) override;
    void InsertAtIndex(int index, ControlNode *node) override;
    void Remove(ControlNode *node) override;
    int GetCount() const override;
    ControlNode *Get(int index) const override;
    ControlNode *FindByName(const DAVA::String &name) const;
    
    virtual DAVA::String GetName() const;
    DAVA::UIControl *GetControl() const;
    ControlPrototype *GetPrototype() const;
    const DAVA::Vector<ControlNode*> &GetInstances() const;

    virtual bool IsEditingSupported() const override;
    virtual bool IsInsertingSupported() const override;
    virtual bool CanInsertControl(ControlNode *node, DAVA::int32 pos) const override;
    virtual bool CanRemove() const override;
    virtual bool CanCopy() const override;

    eCreationType GetCreationType() const { return creationType; }

    RootProperty *GetRootProperty() const {return rootProperty; }

    void MarkAsRemoved();
    void MarkAsAlive();

    void Serialize(PackageSerializer *serializer) const;
    DAVA::String GetPathToPrototypeChild(bool withRootPrototypeName = false) const;

private:
    void CollectPrototypeChildrenWithChanges(DAVA::Vector<ControlNode*> &out) const;
    bool HasNonPrototypeChildren() const;
    bool IsInstancedFrom(const ControlNode *prototypeControl) const;
    
private:
    void AddControlToInstances(ControlNode *control);
    void RemoveControlFromInstances(ControlNode *control);

private:
    DAVA::UIControl *control;
    RootProperty *rootProperty;
    DAVA::Vector<ControlNode*> nodes;
    
    ControlPrototype *prototype;
    DAVA::Vector<ControlNode*> instances; // weak

    eCreationType creationType;
};


#endif // __UI_EDITOR_CONTROL_NODE__
