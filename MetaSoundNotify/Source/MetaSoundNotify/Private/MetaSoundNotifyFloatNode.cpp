#include "MetasoundParamHelper.h"
#include "MetasoundEnumRegistrationMacro.h"
#include "E:\Unreal\Projects\Ethernal\Plugins\MetaSoundNotify\Source\MetaSoundNotify\Public\MetaSoundNotifyInterface.h"

#define LOCTEXT_NAMESPACE "MetasoundStandardNodes_NotifyFloatNode"

namespace Metasound
{
    #pragma region PARAMETERS
    namespace NotifyFloatNode
    {
        METASOUND_PARAM(InParamNameSend, "Send", "Sends the notify.")
        METASOUND_PARAM(InParamNameAddress, "To Notify", "Soft reference of the object to notify passed into a string.")
        METASOUND_PARAM(InParamNameNotifyID, "Notify ID", "ID of this notify node. Useful when dealing with multiple nodes of the same kind notifying to the same listener.")
        METASOUND_PARAM(InParamNameFloat, "Value", "Float to notify.")
        METASOUND_PARAM(OutParamNameSent, "On Sent", "Triggered after we send the notify.")
    }
    #pragma endregion

    #pragma region OPERATOR
    class FNotifyFloatOperator : public TExecutableOperator<FNotifyFloatOperator>
    {
    public:
        static const FNodeClassMetadata& GetNodeInfo();
        static const FVertexInterface& GetVertexInterface();
        static TUniquePtr<IOperator> CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors);

        FNotifyFloatOperator(const FOperatorSettings& InSettings,
        const FTriggerReadRef& InSend,
        const FStringReadRef& InAddressInput,
        const FInt32ReadRef& InIDInput,
        const FFloatReadRef& InFloatInput);

        virtual FDataReferenceCollection GetInputs()  const override;
        virtual FDataReferenceCollection GetOutputs() const override;

        void Execute();

    private:
        FTriggerReadRef SendTrigger;
        FStringReadRef AddressInput;
        FInt32ReadRef IDInput;
        FFloatReadRef FloatInput;

        FTriggerWriteRef SentTrigger;

        void SendMessageToListener();
    };

    FNotifyFloatOperator::FNotifyFloatOperator(const FOperatorSettings& InSettings,
    const FTriggerReadRef& InSend, 
    const FStringReadRef& InAddressInput,
    const FInt32ReadRef& InIDInput,
    const FFloatReadRef& InFloatInput)
    :
    SendTrigger(InSend),
    AddressInput(InAddressInput),
    IDInput(InIDInput),
    FloatInput(InFloatInput),
    SentTrigger(FTriggerWriteRef::CreateNew(InSettings))
    {
    }

    FDataReferenceCollection FNotifyFloatOperator::GetInputs() const
    {
        using namespace NotifyFloatNode;

        FDataReferenceCollection InputDataReferences;

        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameSend), SendTrigger);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameAddress), AddressInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameNotifyID), IDInput);
        InputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(InParamNameFloat), FloatInput);

        return InputDataReferences;
    }

    FDataReferenceCollection FNotifyFloatOperator::GetOutputs() const
    {
        using namespace NotifyFloatNode;

        FDataReferenceCollection OutputDataReferences;
        
        OutputDataReferences.AddDataReadReference(METASOUND_GET_PARAM_NAME(OutParamNameSent), SentTrigger);

        return OutputDataReferences;
    }

    void FNotifyFloatOperator::Execute()
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

    const FVertexInterface& FNotifyFloatOperator::GetVertexInterface()
    {
        using namespace NotifyFloatNode;

        static const FVertexInterface Interface(
            FInputVertexInterface(
                TInputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameSend)),
                TInputDataVertexModel<FString>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameAddress)),
                TInputDataVertexModel<int32>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameNotifyID)),
                TInputDataVertexModel<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InParamNameFloat))
            ),
            FOutputVertexInterface(
                TOutputDataVertexModel<FTrigger>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutParamNameSent))
            )
        );

        return Interface;
    }

    const FNodeClassMetadata& FNotifyFloatOperator::GetNodeInfo()
    {
        auto InitNodeInfo = []() -> FNodeClassMetadata
        {
            FNodeClassMetadata Info;

            Info.ClassName        = { TEXT("UE"), TEXT("NotifyFloat"), TEXT("NotifyFloat") };
            Info.MajorVersion     = 1;
            Info.MinorVersion     = 0;
            Info.DisplayName      = LOCTEXT("Metasound_NotifyFloatDisplayName", "Notify Float");
            Info.Description      = LOCTEXT("Metasound_NotifyFloatNodeDescription", "Sends a notify to the string address if it implements the NodeInterface (only once per call). Optional float parameter.");
            Info.Author           = PluginAuthor;
            Info.PromptIfMissing  = PluginNodeMissingPrompt;
            Info.DefaultInterface = GetVertexInterface();
            Info.CategoryHierarchy = { LOCTEXT("Metasound_NotifyFloatNodeCategory", "Notify") };

            return Info;
        };

        static const FNodeClassMetadata Info = InitNodeInfo();

        return Info;
    }

    TUniquePtr<IOperator> FNotifyFloatOperator::CreateOperator(const FCreateOperatorParams& InParams, FBuildErrorArray& OutErrors)
    {
        using namespace NotifyFloatNode;

        const FDataReferenceCollection& InputCollection = InParams.InputDataReferences;
        const FInputVertexInterface& InputInterface = GetVertexInterface().GetInputInterface();

        FTriggerReadRef SendTrigger = InputCollection.GetDataReadReferenceOrConstruct<FTrigger>(METASOUND_GET_PARAM_NAME(InParamNameSend), InParams.OperatorSettings);
        FStringReadRef AddressIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<FString>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameAddress), InParams.OperatorSettings);
        FInt32ReadRef NotifyIDIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<int32>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameNotifyID), InParams.OperatorSettings);
        FFloatReadRef FloatIn = InputCollection.GetDataReadReferenceOrConstructWithVertexDefault<float>(InputInterface, METASOUND_GET_PARAM_NAME(InParamNameFloat), InParams.OperatorSettings);

        return MakeUnique<FNotifyFloatOperator>(InParams.OperatorSettings, SendTrigger, AddressIn, NotifyIDIn, FloatIn);
    }

    void FNotifyFloatOperator::SendMessageToListener(){
        FSoftObjectPath SoftTarget(*AddressInput);
        TSoftObjectPtr<UObject> SoftTargetPtr(SoftTarget);
        UObject* Target = SoftTargetPtr.Get();

        if (Target && Target->GetClass()->ImplementsInterface(UMetaSoundNotifyInterface::StaticClass()))
        {
            IMetaSoundNotifyInterface::Execute_MetaSoundsNotifyFloat(Target, *IDInput, *FloatInput);
        }
    }
    #pragma endregion
    
    #pragma region NODE
    class FNotifyFloatNode : public FNodeFacade
    {
    public:
        FNotifyFloatNode(const FNodeInitData& InitData)
        : FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FNotifyFloatOperator>())
        { 
        }
    };
    
    METASOUND_REGISTER_NODE(FNotifyFloatNode)
    #pragma endregion
}

#undef LOCTEXT_NAMESPACE