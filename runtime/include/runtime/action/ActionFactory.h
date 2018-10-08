#include <runtime/action/ActionHandler.h>
#include <core/Transaction.h>
#include <storage/Repository.h>

using namespace core;

namespace runtime {

using namespace storage;

namespace action {

class ActionFactory {
public:
    ActionFactory(Transaction const& transaction, std::shared_ptr<Repository> repo);

    ~ActionFactory();

    void init();

    void execute();

    void finalize();

private:
    ActionHandler* m_handler;
};
}
}