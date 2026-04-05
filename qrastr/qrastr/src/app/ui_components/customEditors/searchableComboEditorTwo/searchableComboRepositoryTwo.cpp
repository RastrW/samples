#include "searchableComboRepositoryTwo.h"
#include "searchableComboEditorTwo.h"

Qtitan::GridEditor* SearchableComboRepositoryTwo::createEditor()
{
    return new SearchableComboEditorTwo();
}