#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "E:\Unreal\Projects\Ethernal\Plugins\MetaSoundNotify\Source\MetaSoundNotify\Public\MetaSoundNotifyInterface.h"

// Define a localized namespace for the node!
#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyNode"

namespace Metasound
{
    // Specify your parameter names and descriptions here.
    #pragma region PARAMETERS
    // Create your own namespace for the node!
    namespace NotifyNode
    {
        // Every parameter must start with a fixed naming convention! InParamName for inputs and OutParamName for outputs! I.e.:
        // METASOUND_PARAM(InParamNameMyInput, "Name to display in the graph.", "Description to display in the graph.")
        // METASOUND_PARAM(OutParamNameMyOutput, "Name to display in the graph.", "Description to display in the graph.")
        
        // Inputs
        METASOUND_PARAM(InParamNameSend, "Send", "Sends the notify.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        // Outputs
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion
    
    // Create your operator and main behavior here.
    #pragma region OPERATOR
    /** 
     * @name FNotifyOperator Declaration
     * @brief Declare your node operator class here. It must start with F and end with Operator!
    */
    class FNotifyOperator : public TExecutableOperator<FNotifyOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        // Declare every input you want for your node in this constructor.
        FNotifyOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InSend,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InID);

        // Override GetInputs & GetOutputs
        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        // Function that will execute the node behavior.
        void Execute();

    private:
        // Declare any input parameters you want. For internal use only.
        FTriggerReadRef SendTrigger;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        
        // Declare any output parameters you want. For internal use only.
        FTriggerWriteRef SentTrigger;

        // Custom function for this specific node
        void SendMessageToListener();
    };

    /**
     * @name FNotifyOperator Implementation
     * @brief Set every input parameter to its constructor partner and create any outputs you have. Don't forget the ,!
    */
    FNotifyOperator::FNotifyOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InSend,
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput
    )
    :
    // Set inputs
    SendTrigger(InSend),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    // Create the output
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    /** 
     * @brief Specify the inputs in this function.
     * @warning This is obligatory to have! If we don't want any inputs (weird), just don't add any reference to the input collection.
    */
    FDataReferenceCollection FNotifyOperator::GetInputs() const
    {
        using namespace NotifyNode;

        FDataReferenceCollection InputDataReferences;

        // Create a data reference por each input, never forget ; at the end!
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameSend), SendTrigger);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);

        return InputDataReferences;
    }

    /**
     * @brief Specify your outputs here.
     * @warning This is obligatory to have! If we don't want any output, just don't add any reference to the output collection.
     */
    FDataReferenceCollection FNotifyOperator::GetOutputs() const
    {
        using namespace NotifyNode;

        FDataReferenceCollection OutputDataReferences;

        // Create a data reference por each output, never forget ; at the end!
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    /**
     * @brief Main executor function! Place your node logic here.
     * @warning Important information about triggers inside.
    */
    void FNotifyOperator::Execute()
    {
		// For every trigger output we have, we need to advance its block! This is OBLIGATORY if we want the output triggers to work.
        // I did not test if we need to do the same with non-trigger outputs, be cautious and test yourself.
        SentTrigger->AdvanceBlock();
        
        // This is the function that will execute once we call our input trigger.
        SendTrigger->ExecuteBlock(
			[](int32, int32)
			{
			},
			[this](int32 StartFrame, int32 EndFrame)
			{
                // Place your logic inside here!

                // Call custom function to execute message.             
                SendMessageToListener();
                // Execute the output trigger.
                SentTrigger->TriggerFrame(StartFrame);
			}
		);
    }

    /**
     * @brief Here, you will specify the interface for the node. Don't forget to do inputs and outputs!
    */
    const FVertexInterface& FNotifyOperator::GetVertexInterface()
    {
        using namespace NotifyNode;

        // Create the interface and fill it.
        static const FVertexInterface Interface(
            // Create an FInputVertexInterface and fill it with your inputs. Specify the correct data type for every parameter!
            // Do NOT forget comas!
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSend)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID))
            ),
            
            // Create an FOutputVertexInterface and fill it with your outputs. Specify the correct data type for every parameter!
            // Do NOT forget comas!
            FOutputVertexInterface(
                // You can leave this empty if you don't have any outputs.
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    /**
     * @brief Edit your node info here!
    */
    const FNodeClassMetadata& FNotifyOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("Notify"), TEXT("Notify") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyDisplayName", "Notify");
            Info.Description      = LOCTEXT("Metasound_NotifyNodeDescription", "Sends a notify to the string address if it implements the NodeInterface (only once per call).");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    /**
     * @brief Once again, specify your inputs here. Follow the function structure.
    */
    TUniquePtr<IOperator> FNotifyOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();
        
        FTriggerReadRef SendTrigger = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameSend), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef IDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);

        return MakeUnique<FNotifyOperator>(InParams.OperatorSettings, SendTrigger, AddressIn, IDIn);
    }

    /**
     * @brief Function to send the interface message.
    */
    void FNotifyOperator::SendMessageToListener(){
        // Try to convert string into object reference
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        // If object reference is valid and implements the NotifyInterface, execute interface call and disable TriggerIng.
        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotify(Target, *IDInput);
        }
    }
    #pragma endregion
    
    // Create your node class here
    #pragma region NODE
    /** 
     * @name FNotifyNode class.
     * @brief Use your custom class name. You can leave everything else untouched.
    */
    class FNotifyNode : public FNodeFacade
    {
    public:
        // Constructor used by the Metasound Frontend.
        FNotifyNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyOperator>())
        { 
        }
    };
    
    // Register the node with MetaSounds
    METASOUND_REGISTER_NODE(FNotifyNode)
    #pragma endregion
}

// Close localized namespace
#undef LOCTEXT_NAMESPACE