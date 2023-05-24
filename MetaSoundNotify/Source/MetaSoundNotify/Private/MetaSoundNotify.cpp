#include "MetaSoundNotify.h"
#include "MetasoundFrontendRegistries.h"

#define LOCTEXT_NAMESPACE "FMetaSoundNotifyModule"

void FMetaSoundNotifyModule::StartupModule()
{
    // Register nodes from the plugin
    FMetasoundFrontendRegistryContainer::Get()->RegisterPendingNodes();
}

void FMetaSoundNotifyModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FMetaSoundNotifyModule, MetaSoundNotify)
