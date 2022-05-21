#pragma once

#include "storage/page/page.h"
#include "container/vector.hpp"

namespace thomas {

class Transaction {
public:
    void AddIntoPageSet(Page *page) {
        page_set_.push_back(page);
    }
    void AddIntoDeletedPageSet(page_id_t page_id) {
        deleted_page_set_.push_back(page_id);
    }

    vector<Page *> *GetPageSet() {
        return &page_set_;
    }

    vector<page_id_t> *GetDeletedPageSet() {
        return &deleted_page_set_;
    }

private:
    vector<Page *> page_set_;
    vector<page_id_t> deleted_page_set_;
};

}