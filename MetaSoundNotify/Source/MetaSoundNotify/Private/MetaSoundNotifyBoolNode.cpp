#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyBoolNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyBoolNode
    {
        METASOUND_PARAM(InParamNameSend, "Send", "Sends the notify.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameBool, "Value", "Bool to notify.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyBoolOperator : public TExecutableOperator<FNotifyBoolOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyBoolOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InSend,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InIDInput,
        const FBoolReadRef& InBoolInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef SendTrigger;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        FBoolReadRef BoolInput;

        FTriggerWriteRef SentTrigger;

        void SendMessageToListener();
    };

    FNotifyBoolOperator::FNotifyBoolOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InSend, 
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput,
    const FBoolReadRef& InBoolInput)
    :
    SendTrigger(InSend),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    BoolInput(InBoolInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    FDataReferenceCollection FNotifyBoolOperator::GetInputs() const
    {
        using namespace NotifyBoolNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameSend), SendTrigger);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameBool), BoolInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyBoolOperator::GetOutputs() const
    {
        using namespace NotifyBoolNode;

        FDataReferenceCollection OutputDataReferences;
        
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyBoolOperator::Execute()
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

    const FVertexInterface& FNotifyBoolOperator::GetVertexInterface()
    {
        using namespace NotifyBoolNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSend)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameBool))
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyBoolOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyBool"), TEXT("NotifyBool") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyBoolDisplayName", "Notify Bool");
            Info.Description      = LOCTEXT("Metasound_NotifyBoolNodeDescription", "Sends a notify to the string address if it implements the NodeInterface (only once per call). Optional Bool parameter.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyBoolNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyBoolOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyBoolNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FTriggerReadRef SendTrigger = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameSend), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef NotifyIDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FBoolReadRef BoolIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<bool>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameBool), InParams.OperatorSettings);

        return MakeUnique<FNotifyBoolOperator>(InParams.OperatorSettings, SendTrigger, AddressIn, NotifyIDIn, BoolIn);
    }

    void FNotifyBoolOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyBool(Target, *IDInput, *BoolInput);
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyBoolNode : public FNodeFacade
    {
    public:
        FNotifyBoolNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyBoolOperator>())
        { 
        }
    };
    
    METASOUND_REGISTER_NODE(FNotifyBoolNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE
