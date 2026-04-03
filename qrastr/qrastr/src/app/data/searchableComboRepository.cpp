#include "searchableComboRepository.h"
#include "searchableComboEditor.h"

Qtitan::GridEditor* SearchableComboRepository::createEditor()
{
    return new SearchableComboEditor();
}