#include "esinstalledmodmodel.h"
#include "esmodelement.h"
#include "esmodmodel.h"

ESInstalledModModel::ESInstalledModModel(bool inv, QObject *parent)
    : QSortFilterProxyModel(parent),
      inverseFilter(inv)
{

}

ESInstalledModModel::~ESInstalledModModel()
{

}

bool ESInstalledModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    int st = sourceModel()->data(ind, ESModModel::StateRole).toInt();

    switch (st) {
    case ESModElement::Downloading :
    case ESModElement::Unpacking :
    case ESModElement::Failed :
        return true;

    case ESModElement::InstalledAvailable :
    case ESModElement::InstalledHasUpdate :
    case ESModElement::Installed :
        return !inverseFilter;

    case ESModElement::Unknown :
    case ESModElement::Available :
    default :
        break;
    }

    return inverseFilter;
}

#define REMAP_SOURCE_SLOT(method) \
void ESInstalledModModel::method(int ind) \
{ \
    QModelIndex sind = mapToSource(index(ind, 0)); \
    ESModModel *smdl = static_cast<ESModModel *>(sourceModel()); \
    smdl->method(sind.row()); \
}

#define REMAP_SOURCE_SLOT2(method) \
void ESInstalledModModel::method(int ind, int l) \
{ \
    QModelIndex sind = mapToSource(index(ind, 0)); \
    ESModModel *smdl = static_cast<ESModModel *>(sourceModel()); \
    smdl->method(sind.row(), l); \
}

REMAP_SOURCE_SLOT(Download)
REMAP_SOURCE_SLOT(Abort)
REMAP_SOURCE_SLOT(Retry)
REMAP_SOURCE_SLOT(Update)
REMAP_SOURCE_SLOT(Delete)
REMAP_SOURCE_SLOT2(SendLike)
REMAP_SOURCE_SLOT(ShowError)

#undef REMAP_SOURCE_SLOT
#undef REMAP_SOURCE_SLOT2