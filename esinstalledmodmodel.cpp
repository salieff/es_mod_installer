#include "esinstalledmodmodel.h"
#include "esmodelement.h"
#include "esmodmodel.h"


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
REMAP_SOURCE_SLOT(ToggleFavorite)

#undef REMAP_SOURCE_SLOT
#undef REMAP_SOURCE_SLOT2


bool ESInstalledModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    int st = sourceModel()->data(ind, ESModModel::StateRole).toInt();

    switch (st) {
    case ESModElement::Downloading :
    case ESModElement::Unpacking :
        return true;

    case ESModElement::Failed :
        return false;

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

bool ESIncompletedModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    int st = sourceModel()->data(ind, ESModModel::StateRole).toInt();
    int pr = sourceModel()->data(ind, ESModModel::ProgressRole).toInt();

    switch (st) {
    case ESModElement::Downloading :
    case ESModElement::Unpacking :
        return true;

    case ESModElement::Available :
        return (pr != 100);

    case ESModElement::Failed :
    default :
        break;
    }

    return false;
}

bool ESBrokenModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->data(ind, ESModModel::StateRole).toInt() == ESModElement::Failed;
}

bool ESFavoriteModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->data(ind, ESModModel::FavoriteRole).toBool();
}

bool ESReleasedModModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    QModelIndex ind = sourceModel()->index(source_row, 0, source_parent);
    return sourceModel()->data(ind, ESModModel::StatusRole).toString().startsWith("окончен", Qt::CaseInsensitive);
}
