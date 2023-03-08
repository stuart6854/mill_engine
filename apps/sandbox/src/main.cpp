#include <mill/entry_point.hpp>
#include <mill/mill.hpp>

class SandboxApp : public mill::Application
{
public:
    void initialise() override {}
    void shutdown() override {}
};

auto mill::create_application() -> mill::Application*
{
    return new SandboxApp;
}