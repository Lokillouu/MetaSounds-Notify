#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyIntNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyIntNode
    {
        METASOUND_PARAM(InParamNameSend, "Send", "Sends the notify.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameInt, "Value", "Int to notify.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyIntOperator : public TExecutableOperator<FNotifyIntOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyIntOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InSend,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InIDInput,
        const FInt32ReadRef& InIntInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef SendTrigger;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        FInt32ReadRef IntInput;

        FTriggerWriteRef SentTrigger;

        void SendMessageToListener();
    };

    FNotifyIntOperator::FNotifyIntOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InSend, 
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput,
    const FInt32ReadRef& InIntInput)
    :
    SendTrigger(InSend),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    IntInput(InIntInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    FDataReferenceCollection FNotifyIntOperator::GetInputs() const
    {
        using namespace NotifyIntNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameSend), SendTrigger);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameInt), IntInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyIntOperator::GetOutputs() const
    {
        using namespace NotifyIntNode;

        FDataReferenceCollection OutputDataReferences;
        
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyIntOperator::Execute()
    {
        SentTrigger->AdvanceBlock();
        
        SendTrigger->ExecuteBlock(
			[](int32, int32)
			{
			},
			[this](int32 StartFrame, int32 EndFrame)
			{
                SendMessageToListener();
                SentTrigger->TriggerFrame(StartFrame);
			}
		);
    }

    const FVertexInterface& FNotifyIntOperator::GetVertexInterface()
    {
        using namespace NotifyIntNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSend)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameInt))
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyIntOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyInt"), TEXT("NotifyInt") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyIntDisplayName", "Notify Int");
            Info.Description      = LOCTEXT("Metasound_NotifyIntNodeDescription", "Sends a notify to the string address if it implements the NodeInterface (only once per call). Optional int parameter.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyIntNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyIntOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyIntNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FTriggerReadRef SendTrigger = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameSend), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef NotifyIDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FInt32ReadRef IndexIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameInt), InParams.OperatorSettings);

        return MakeUnique<FNotifyIntOperator>(InParams.OperatorSettings, SendTrigger, AddressIn, NotifyIDIn, IndexIn);
    }

    void FNotifyIntOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyInt(Target, *IDInput, *IntInput);
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyIntNode : public FNodeFacade
    {
    public:
        FNotifyIntNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyIntOperator>())
        { 
        }
    };
    
    METASOUND_REGISTER_NODE(FNotifyIntNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE
