#ifndef JZNODE_META_H_
#define JZNODE_META_H_


class JZNodeMeta
{
public:

};

enum{
    DataValidator_none,
    DataValidator_minmax,
    DataValidator_list,
};

class JZNodeDataValidator
{
public:
    int type;
};

#endif
