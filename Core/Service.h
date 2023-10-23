#pragma once

namespace Raptor
{
namespace Core
{
class Service 
{
    virtual void init(void* configuration) {}
    virtual void shutdown() {}
}; // class Service

#define RAPTOR_DECLARE_SERVICE(Type) static Type* instance();

} // namespace Core
} // namespace Raptor