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
        return true;

    case ESModElement::InstalledAvailable :
    case ESModElement::InstalledHasUpdate :
    case ESModElement::Installed :
        return !inverseFilter;

    case ESModElement::Unknown :
    case ESModElement::Available :
    case ESModElement::Failed :
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

ESIncompletedModModel::ESIncompletedModModel(QObject *parent) : ESInstalledModModel(false, parent)
{

}

ESIncompletedModModel::~ESIncompletedModModel()
{

}

bool ESIncompletedModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    int st = sourceModel()->data(ind, ESModModel::StateRole).toInt();
    int pr = sourceModel()->data(ind, ESModModel::ProgressRole).toInt();

    switch (st) {
    case ESModElement::Downloading :
    case ESModElement::Unpacking :
    case ESModElement::Failed :
        return true;

    case ESModElement::Available :
        return (pr != 100);

    default :
        break;
    }

    return false;
}
