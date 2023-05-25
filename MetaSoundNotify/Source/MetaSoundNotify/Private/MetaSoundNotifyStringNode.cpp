#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyStringNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyStringNode
    {
        METASOUND_PARAM(InParamNameSend, "Send", "Sends the notify.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "String address of the object to notify.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameMsg, "Message", "Message to notify.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyStringOperator : public TExecutableOperator<FNotifyStringOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyStringOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InSend,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InIDInput,
        const FStringReadRef& InMessageInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef SendTrigger;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        FStringReadRef MessageInput;

        FTriggerWriteRef SentTrigger;

        void SendMessageToListener();
    };

    FNotifyStringOperator::FNotifyStringOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InSend, 
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput,
    const FStringReadRef& InMessageInput)
    :
    SendTrigger(InSend),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    MessageInput(InMessageInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    FDataReferenceCollection FNotifyStringOperator::GetInputs() const
    {
        using namespace NotifyStringNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameSend), SendTrigger);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameMsg), MessageInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyStringOperator::GetOutputs() const
    {
        using namespace NotifyStringNode;

        FDataReferenceCollection OutputDataReferences;
        
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyStringOperator::Execute()
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

    const FVertexInterface& FNotifyStringOperator::GetVertexInterface()
    {
        using namespace NotifyStringNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSend)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameMsg))
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyStringOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyString"), TEXT("NotifyString") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyStringDisplayName", "Notify String");
            Info.Description      = LOCTEXT("Metasound_NotifyStringNodeDescription", "Sends a notify to the string address if it implements the NodeInterface (only once per call). Optional message parameter.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyStringNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyStringOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyStringNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FTriggerReadRef SendTrigger = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameSend), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef NotifyIDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FStringReadRef MsgIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameMsg), InParams.OperatorSettings);

        return MakeUnique<FNotifyStringOperator>(InParams.OperatorSettings, SendTrigger, AddressIn, NotifyIDIn, MsgIn);
    }

    void FNotifyStringOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyString(Target, *IDInput, *MessageInput);
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyStringNode : public FNodeFacade
    {
    public:
        FNotifyStringNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyStringOperator>())
        { 
        }
    };
    
    METASOUND_REGISTER_NODE(FNotifyStringNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE
