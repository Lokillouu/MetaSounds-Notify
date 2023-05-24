using UnrealBuildTool;

public class MetaSoundNotify : ModuleRules
{
    public MetaSoundNotify(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        
        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...

            }
            );
                
        
        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add private include paths required here ...

            }
            );
            
        
        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                // ... add public dependencies that you statically link with here ...
                "Core",
                "CoreUObject",
                "Serialization",
                "SignalProcessing",
                "MetasoundFrontend",
                "MetasoundGraphCore",
                "AudioExtensions",
            }
            );
            
        
        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                // ... add private dependencies that you statically link with here ...  
                "CoreUObject",
                "Engine",
                "SignalProcessing",
            }
            );
        
        PublicDefinitions.Add("WITH_METASOUND_FRONTEND=1");
        
        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...

            }
            );
    }
}
