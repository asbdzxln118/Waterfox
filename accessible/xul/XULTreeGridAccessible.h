/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_a11y_XULTreeGridAccessible_h__
#define mozilla_a11y_XULTreeGridAccessible_h__

#include "XULTreeAccessible.h"
#include "TableAccessible.h"
#include "TableCellAccessible.h"
#include "xpcAccessibleTable.h"
#include "xpcAccessibleTableCell.h"

namespace mozilla {
namespace a11y {

class XULTreeGridCellAccessible;

/**
 * Represents accessible for XUL tree in the case when it has multiple columns.
 */
class XULTreeGridAccessible : public XULTreeAccessible,
                              public TableAccessible
{
public:
  XULTreeGridAccessible(nsIContent* aContent, DocAccessible* aDoc,
                        nsTreeBodyFrame* aTreeFrame) :
    XULTreeAccessible(aContent, aDoc, aTreeFrame)
    { mGenericTypes |= eTable; }

  // TableAccessible
  virtual uint32_t ColCount();
  virtual uint32_t RowCount();
  virtual Accessible* CellAt(uint32_t aRowIndex, uint32_t aColumnIndex);
  virtual void ColDescription(uint32_t aColIdx, nsString& aDescription);
  virtual bool IsColSelected(uint32_t aColIdx);
  virtual bool IsRowSelected(uint32_t aRowIdx);
  virtual bool IsCellSelected(uint32_t aRowIdx, uint32_t aColIdx);
  virtual uint32_t SelectedCellCount();
  virtual uint32_t SelectedColCount();
  virtual uint32_t SelectedRowCount();
  virtual void SelectedCells(nsTArray<Accessible*>* aCells);
  virtual void SelectedCellIndices(nsTArray<uint32_t>* aCells);
  virtual void SelectedColIndices(nsTArray<uint32_t>* aCols);
  virtual void SelectedRowIndices(nsTArray<uint32_t>* aRows);
  virtual void SelectRow(uint32_t aRowIdx);
  virtual void UnselectRow(uint32_t aRowIdx);
  virtual Accessible* AsAccessible() { return this; }

  // Accessible
  virtual TableAccessible* AsTable() { return this; }
  virtual a11y::role NativeRole() MOZ_OVERRIDE;

protected:
  virtual ~XULTreeGridAccessible();

  // XULTreeAccessible
  virtual already_AddRefed<Accessible>
    CreateTreeItemAccessible(int32_t aRow) const MOZ_OVERRIDE;
};


/**
 * Represents accessible for XUL tree item in the case when XUL tree has
 * multiple columns.
 */
class XULTreeGridRowAccessible MOZ_FINAL : public XULTreeItemAccessibleBase
{
public:
  using Accessible::GetChildAt;

  XULTreeGridRowAccessible(nsIContent* aContent, DocAccessible* aDoc,
                           Accessible* aParent, nsITreeBoxObject* aTree,
                           nsITreeView* aTreeView, int32_t aRow);

  // nsISupports and cycle collection
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(XULTreeGridRowAccessible,
                                           XULTreeItemAccessibleBase)

  // Accessible
  virtual void Shutdown();
  virtual a11y::role NativeRole() MOZ_OVERRIDE;
  virtual ENameValueFlag Name(nsString& aName);
  virtual Accessible* ChildAtPoint(int32_t aX, int32_t aY,
                                   EWhichChildAtPoint aWhichChild);

  virtual Accessible* GetChildAt(uint32_t aIndex) const MOZ_OVERRIDE;
  virtual uint32_t ChildCount() const MOZ_OVERRIDE;

  // XULTreeItemAccessibleBase
  virtual XULTreeGridCellAccessible* GetCellAccessible(nsITreeColumn* aColumn)
    const MOZ_OVERRIDE MOZ_FINAL;
  virtual void RowInvalidated(int32_t aStartColIdx, int32_t aEndColIdx);

protected:
  virtual ~XULTreeGridRowAccessible();

  // Accessible
  virtual void CacheChildren();

  // XULTreeItemAccessibleBase
  mutable nsRefPtrHashtable<nsPtrHashKey<const void>, XULTreeGridCellAccessible>
    mAccessibleCache;
};


/**
 * Represents an accessible for XUL tree cell in the case when XUL tree has
 * multiple columns.
 */

class XULTreeGridCellAccessible : public LeafAccessible,
                                  public TableCellAccessible
{
public:

  XULTreeGridCellAccessible(nsIContent* aContent, DocAccessible* aDoc,
                            XULTreeGridRowAccessible* aRowAcc,
                            nsITreeBoxObject* aTree, nsITreeView* aTreeView,
                            int32_t aRow, nsITreeColumn* aColumn);

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(XULTreeGridCellAccessible,
                                           LeafAccessible)

  // Accessible
  virtual TableCellAccessible* AsTableCell() { return this; }
  virtual nsIntRect Bounds() const MOZ_OVERRIDE;
  virtual ENameValueFlag Name(nsString& aName);
  virtual Accessible* FocusedChild();
  virtual already_AddRefed<nsIPersistentProperties> NativeAttributes() MOZ_OVERRIDE;
  virtual int32_t IndexInParent() const;
  virtual Relation RelationByType(RelationType aType) MOZ_OVERRIDE;
  virtual a11y::role NativeRole() MOZ_OVERRIDE;
  virtual uint64_t NativeState() MOZ_OVERRIDE;
  virtual uint64_t NativeInteractiveState() const MOZ_OVERRIDE;

  // ActionAccessible
  virtual uint8_t ActionCount() MOZ_OVERRIDE;
  virtual void ActionNameAt(uint8_t aIndex, nsAString& aName) MOZ_OVERRIDE;
  virtual bool DoAction(uint8_t aIndex) MOZ_OVERRIDE;

  // TableCellAccessible
  virtual TableAccessible* Table() const MOZ_OVERRIDE;
  virtual uint32_t ColIdx() const MOZ_OVERRIDE;
  virtual uint32_t RowIdx() const MOZ_OVERRIDE;
  virtual void ColHeaderCells(nsTArray<Accessible*>* aHeaderCells) MOZ_OVERRIDE;
  virtual void RowHeaderCells(nsTArray<Accessible*>* aCells) MOZ_OVERRIDE { }
  virtual bool Selected() MOZ_OVERRIDE;

  /**
   * Fire name or state change event if the accessible text or value has been
   * changed.
   * @return true if name has changed
   */
  bool CellInvalidated();

protected:
  virtual ~XULTreeGridCellAccessible();

  // Accessible
  virtual Accessible* GetSiblingAtOffset(int32_t aOffset,
                                         nsresult* aError = nullptr) const;
  virtual void DispatchClickEvent(nsIContent* aContent, uint32_t aActionIndex);

  // XULTreeGridCellAccessible

  /**
   * Return true if value of cell can be modified.
   */
  bool IsEditable() const;

  enum { eAction_Click = 0 };

  nsCOMPtr<nsITreeBoxObject> mTree;
  nsITreeView* mTreeView;

  int32_t mRow;
  nsCOMPtr<nsITreeColumn> mColumn;

  nsString mCachedTextEquiv;
};

} // namespace a11y
} // namespace mozilla

#endif