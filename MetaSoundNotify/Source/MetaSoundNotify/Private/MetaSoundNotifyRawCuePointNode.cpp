#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyRawCuePointNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyRawCuePointNode
    {
        METASOUND_PARAM(InParamNameListen, "Listen", "Sets the node behaviour active.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameMsg, "Message", "Custom message to send. For example, the cue point ID, label, instructions...")
        METASOUND_PARAM(InParamNamePlayback, "Playback Position", "Current playback position of the sound.")
        METASOUND_PARAM(InParamNameCuePoint, "Cue Point Position", "Position for the virtual cue point.")
        METASOUND_PARAM(InParamNameStartListening, "Start Listening", "Whether the node should start listening or not.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyRawCuePointOperator : public TExecutableOperator<FNotifyRawCuePointOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyRawCuePointOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InListenInput,
        const FStringReadRef& InStrInput,
        const FInt32ReadRef& InIDInput,
        const FStringReadRef& InMsgInput,
        const FFloatReadRef& InPlaybackInput,
        const FFloatReadRef& InCuePointInput,
        const FBoolReadRef& InStartListeningInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef TriggerListenInput;
        FStringReadRef StrInput;
        FInt32ReadRef IDInput;
        FStringReadRef MsgInput;
        FFloatReadRef PlaybackInput;
        FFloatReadRef CuePointInput;
        FBoolReadRef StartListeningInput;

        FTriggerWriteRef SentTrigger;

        bool Listening;
        void SendMessageToListener();
    };
    
    FNotifyRawCuePointOperator::FNotifyRawCuePointOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InListenInput,
    const FStringReadRef& InStrInput,
    const FInt32ReadRef& InIDInput,
    const FStringReadRef& InMsgInput,
    const FFloatReadRef& InPlaybackInput,
    const FFloatReadRef& InCuePointInput,
    const FBoolReadRef& InStartListeningInput)
    :
    TriggerListenInput(InListenInput),
    StrInput(InStrInput),
    IDInput(InIDInput),
    MsgInput(InMsgInput),
    PlaybackInput(InPlaybackInput),
    CuePointInput(InCuePointInput),
    StartListeningInput(InStartListeningInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
        Listening = *InStartListeningInput;
    }

    FDataReferenceCollection FNotifyRawCuePointOperator::GetInputs() const
    {
        using namespace NotifyRawCuePointNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameListen), TriggerListenInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), StrInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameMsg), MsgInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNamePlayback), PlaybackInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameCuePoint), CuePointInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameStartListening), StartListeningInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyRawCuePointOperator::GetOutputs() const
    {
        using namespace NotifyRawCuePointNode;

        FDataReferenceCollection OutputDataReferences;

        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyRawCuePointOperator::Execute()
    {
		SentTrigger->AdvanceBlock();
        
        TriggerListenInput->ExecuteBlock(
			[](int32, int32)
			{
			},
			[this](int32 StartFrame, int32 EndFrame)
			{
                Listening = 1;
                SentTrigger->TriggerFrame(StartFrame);
			}
		);

        if (Listening && !StrInput->IsEmpty()){
            if (*PlaybackInput > *CuePointInput){
                SendMessageToListener();
            }
        }
    }

    const FVertexInterface& FNotifyRawCuePointOperator::GetVertexInterface()
    {
        using namespace NotifyRawCuePointNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameListen)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameMsg)),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNamePlayback)),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameCuePoint)),
                TInputDataVertexModel<bool>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameStartListening))
            ),
            
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyRawCuePointOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyRawCuePoint"), TEXT("Notify Raw Cue Point") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyRawCuePointDisplayName", "Notify Raw Cue Point");
            Info.Description      = LOCTEXT("Metasound_NotifyRawCuePointNodeDescription", "When triggered, waits for the playback position to reach the desired cue point position and sends a notify through the interface to the listener, after which it deactivate again. Use this if your audio file does not support or have cue points, and you need them. This simulates cue points.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyRawCuePointNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyRawCuePointOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyRawCuePointNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();
        
        FTriggerReadRef ListenIn = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameListen), InParams.OperatorSettings);
        FStringReadRef StrIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef IDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FStringReadRef MsgIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameMsg), InParams.OperatorSettings);
        FFloatReadRef PlaybackIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNamePlayback), InParams.OperatorSettings);
        FFloatReadRef CuePointIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameCuePoint), InParams.OperatorSettings);
        FBoolReadRef StartListeningIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<bool>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameStartListening), InParams.OperatorSettings);

        return MakeUnique<FNotifyRawCuePointOperator>(InParams.OperatorSettings, ListenIn, StrIn, IDIn, MsgIn, PlaybackIn, CuePointIn, StartListeningIn);
    }

    void FNotifyRawCuePointOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*StrInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyRawCuePoint(Target, *IDInput, *MsgInput);
            Listening = 0;
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyRawCuePointNode : public FNodeFacade
    {
    public:
        FNotifyRawCuePointNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyRawCuePointOperator>())
        {
        }
    };

    METASOUND_REGISTER_NODE(FNotifyRawCuePointNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE
