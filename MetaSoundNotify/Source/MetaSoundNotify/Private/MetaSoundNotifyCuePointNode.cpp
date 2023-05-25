#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyCuePointNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyCuePointNode
    {
        METASOUND_PARAM(InParamNameTrigger, "On Cue Point", "Executes the node behaviour to send the message.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameID, "Cue Point ID", "Index of the cue point.")
        METASOUND_PARAM(InParamNameLabel, "Label", "Label of the cue point.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyCuePointOperator : public TExecutableOperator<FNotifyCuePointOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyCuePointOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InCuePointInput,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InIDInput,
        const FInt32ReadRef& InIndexInput,
        const FStringReadRef& InLabelInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef TriggerCuePointInput;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        FInt32ReadRef IndexInput;
        FStringReadRef LabelInput;
        
        FTriggerWriteRef SentTrigger;
        
        void SendMessageToListener();
    };
    
    FNotifyCuePointOperator::FNotifyCuePointOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InCuePointInput,
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput,
    const FInt32ReadRef& InIndexInput,
    const FStringReadRef& InLabelInput)
    :
    TriggerCuePointInput(InCuePointInput),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    IndexInput(InIndexInput),
    LabelInput(InLabelInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    FDataReferenceCollection FNotifyCuePointOperator::GetInputs() const
    {
        using namespace NotifyCuePointNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameTrigger), TriggerCuePointInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameID), IndexInput);        
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameLabel), LabelInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyCuePointOperator::GetOutputs() const
    {
        using namespace NotifyCuePointNode;

        FDataReferenceCollection OutputDataReferences;

        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyCuePointOperator::Execute()
    {
		SentTrigger->AdvanceBlock();
        
        TriggerCuePointInput->ExecuteBlock(
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

    const FVertexInterface& FNotifyCuePointOperator::GetVertexInterface()
    {
        using namespace NotifyCuePointNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameTrigger)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameID)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameLabel))
            ),
            
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyCuePointOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyCuePoint"), TEXT("Notify Cue Point") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyCuePointDisplayName", "Notify Cue Point");
            Info.Description      = LOCTEXT("Metasound_NotifyCuePointNodeDescription", "Useful to send a cue point notify, with optional cue point index and label. It does not check if the cue point is reached! This just sends the message out. If you don't have cue points in your audio file, you can try using Notify Raw Cue Point.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyCuePointNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyCuePointOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyCuePointNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();
        
        FTriggerReadRef TriggerIn = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameTrigger), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef IDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FInt32ReadRef IndexIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameID), InParams.OperatorSettings);
        FStringReadRef LabelIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameLabel), InParams.OperatorSettings);

        return MakeUnique<FNotifyCuePointOperator>(InParams.OperatorSettings, TriggerIn, AddressIn, IDIn, IndexIn, LabelIn);
    }

    void FNotifyCuePointOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyCuePoint(Target, *IDInput, *IndexInput, *LabelInput);
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyCuePointNode : public FNodeFacade
    {
    public:
        FNotifyCuePointNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyCuePointOperator>())
        {
        
        }
    };

    METASOUND_REGISTER_NODE(FNotifyCuePointNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE
