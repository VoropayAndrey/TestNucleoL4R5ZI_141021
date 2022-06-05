/*****< auddemo.c >************************************************************/
/*      Copyright 2011 - 2013 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  AUDDEMO - Simple sample application using Audio Sub-System Profile.       */
/*                                                                            */
/*  Author:  Matt Seabold                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/05/11  M. Seabold     Initial creation.                               */
/*   10/02/15  D. Keren       Fix bugs                                        */
/*   03/03/15  D. Horowitz    Adding Demo Application version.                */
/******************************************************************************/

#include "Main.h"                /* Application Interface Abstraction.        */
#include "SS1BTPS.h"             /* Main SS1 Bluetooth Stack Header.          */
#include "SS1BTAUD.h"            /* SS1 Audio Profile Sub-System Headers.     */
#include "BTPSKRNL.h"            /* BTPS Kernel Header.                       */
#include "BTVSAPI.h"             /* BTPS API Header.                          */

#include "AUDDemo.h"             /* Application Header.                       */
#include "AudioDecoder.h"        /* Audio Decoder sample.                     */
#include "AUDIO.h"

#define MAX_SUPPORTED_COMMANDS                     (36)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_INQUIRY_RESULTS                        (20)  /* Denotes the max   */
                                                         /* number of inquiry */
                                                         /* results.          */

#define MAX_SUPPORTED_LINK_KEYS                    (8)   /* Max supported Link*/
                                                         /* keys.             */

#define DEFAULT_IO_CAPABILITY          (icNoInputNoOutput)/* Denotes the      */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Secure  */
                                                         /* Simple Pairing.   */

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

#define ENDPOINT_TYPE_SRC                           (1)  /* Flags which denote*/
                                                         /* which type of     */

#define ENDPOINT_TYPE_SNK                           (2)  /* endpoints the app */
                                                         /* is initialized as.*/

#define NO_COMMAND_ERROR                           (-1)  /* Denotes that no   */
                                                         /* command was       */
                                                         /* specified to the  */
                                                         /* parser.           */

#define INVALID_COMMAND_ERROR                      (-2)  /* Denotes that the  */
                                                         /* Command does not  */
                                                         /* exist for         */
                                                         /* processing.       */

#define EXIT_CODE                                  (-3)  /* Denotes that the  */
                                                         /* Command specified */
                                                         /* was the Exit      */
                                                         /* Command.          */

#define FUNCTION_ERROR                             (-4)  /* Denotes that an   */
                                                         /* error occurred in */
                                                         /* execution of the  */
                                                         /* Command Function. */

#define TOO_MANY_PARAMS                            (-5)  /* Denotes that there*/
                                                         /* are more          */
                                                         /* parameters then   */
                                                         /* will fit in the   */
                                                         /* UserCommand.      */

#define INVALID_PARAMETERS_ERROR                   (-6)  /* Denotes that an   */
                                                         /* error occurred due*/
                                                         /* to the fact that  */
                                                         /* one or more of the*/
                                                         /* required          */
                                                         /* parameters were   */
                                                         /* invalid.          */

#define UNABLE_TO_INITIALIZE_STACK                 (-7)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* while Initializing*/
                                                         /* the Bluetooth     */
                                                         /* Protocol Stack.   */

#define INVALID_STACK_ID_ERROR                     (-8)  /* Denotes that an   */
                                                         /* occurred due to   */
                                                         /* attempted         */
                                                         /* execution of a    */
                                                         /* Command when a    */
                                                         /* Bluetooth Protocol*/
                                                         /* Stack has not been*/
                                                         /* opened.           */

#define UNABLE_TO_REGISTER_SERVER                  (-9)  /* Denotes that an   */
                                                         /* error occurred    */
                                                         /* when trying to    */
                                                         /* create a Serial   */
                                                         /* Port Server.      */

#define EXIT_TEST_MODE                             (-10) /* Flags exit from   */
                                                         /* Test Mode.        */

#define INDENT_LENGTH                                 3  /* Denotes the number*/
                                                         /* of character      */
                                                         /* spaces to be used */
                                                         /* for indenting when*/
                                                         /* displaying SDP    */
                                                         /* Data Elements.    */

/*** Define parameters for changing the Bluetooth ACL priority for 
     better audio quality. HCI Flow Spec command and DDIP command ****/

#define HIGH_AUDIO_CONNECTION_PRIORITY        (1)    
#define NORMAL_AUDIO_CONNECTION_PRIORITY      (0)

#define FLOW_SPEC_FLAGS                       (0)
#define FLOW_SPEC_FLOW_DIRECTION              (0x01)
#define FLOW_SPEC_SERVICE_TYPE_GUARANTEED     (0x02)
#define FLOW_SPEC_SERVICE_TYPE_BESTEFFORT     (0x02)
#define FLOW_SPEC_TOKEN_RATE                  (25000)
#define FLOW_SPEC_TOKEN_BUCKET_SIZE           (333)
#define FLOW_SPEC_PEAK_BANDWIDTH              (25000)
#define FLOW_SPEC_ACCESS_LATENCY              (13000)

#define BEST_EFFORT_HIGH_PRIORITY             (50)
#define BEST_EFFORT_NORMAL_PRIORITY           (20)
#define GUARANTEED_HIGH_PRIORITY              (90)
#define GUARANTEED_NORMAL_PRIORITY            (70)

/************************************************************************/

#define  DEFAULT_PASS_THROUGH_COMMAND_TIMEOUT      200


   /* Determine the Name we will use for this compilation.              */
#define APP_DEMO_NAME                              "AUDDemo"


   /* Following converts a Sniff Parameter in Milliseconds to frames.   */
#define MILLISECONDS_TO_BASEBAND_SLOTS(_x)   ((_x) / (0.625))

   /* The following type definition represents the container type which */
   /* holds the mapping between Bluetooth devices (based on the BD_ADDR)*/
   /* and the Link Key (BD_ADDR <-> Link Key Mapping).                  */
typedef struct _tagLinkKeyInfo_t
{
   BD_ADDR_t  BD_ADDR;
   Link_Key_t LinkKey;
} LinkKeyInfo_t;

   /* The following type definition represents the container type which */
   /* holds the mapping between Profile UUIDs and Profile Names (UUID   */
   /* <-> Name).                                                        */
typedef struct _tagUUIDInfo_t
{
   char       *Name;
   UUID_128_t  UUID;
} UUIDInfo_t;

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char     *strParam;
   SDWord_t  intParam;
} Parameter_t;

   /* The following type definition represents the structure which holds*/
   /* a list of parameters that are to be associated with a command The */
   /* NumberofParameters variable holds the value of the number of      */
   /* parameters in the list.                                           */
typedef struct _tagParameterList_t
{
   int         NumberofParameters;
   Parameter_t Params[MAX_NUM_OF_PARAMETERS];
} ParameterList_t;

   /* The following type definition represents the structure which holds*/
   /* the command and parameters to be executed.                        */
typedef struct _tagUserCommand_t
{
   char            *Command;
   ParameterList_t  Parameters;
} UserCommand_t;

   /* The following type definition represents the generic function     */
   /* pointer to be used by all commands that can be executed by the    */
   /* test program.                                                     */
typedef int (*CommandFunction_t)(ParameterList_t *TempParam);

   /* The following type definition represents the structure which holds*/
   /* information used in the interpretation and execution of Commands. */
typedef struct _tagCommandTable_t
{
   char              *CommandName;
   CommandFunction_t  CommandFunction;
} CommandTable_t;

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[15];

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static Boolean_t           Initialized;             /* Variable used to denote whether */
                                                    /* the AUD modules has been        */
                                                    /* initialized.                    */

static Boolean_t           Connection;              /* Variable used to indicate       */
                                                    /* whether the module currently has*/
                                                    /* a connection.                   */

static BD_ADDR_t           ConnectedBD_ADDR;        /* Variable used to store the      */
                                                    /* BD_ADDR of the currently        */
                                                    /* connected device.               */

static Boolean_t           RemoteControlConnection; /* Variable used to indicate       */
                                                    /* whether the module currently has*/
                                                    /* an on-going remote control      */
                                                    /* connection.                     */

static int                 HCIEventCallbackHandle;  /* Holds the handle of the         */
                                                    /* registered HCI Event Callback.  */

static BD_ADDR_t           InquiryResultList[MAX_INQUIRY_RESULTS]; /* Variable which   */
                                                    /* contains the inquiry result     */
                                                    /* received from the most recently */
                                                    /* preformed inquiry.              */

static unsigned int        NumberofValidResponses;  /* Variable which holds the number */
                                                    /* of valid inquiry results within */
                                                    /* the inquiry results array.      */

static LinkKeyInfo_t       LinkKeyInfo[MAX_SUPPORTED_LINK_KEYS]; /* Variable holds     */
                                                    /* BD_ADDR <-> Link Keys for       */
                                                    /* pairing.                        */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static GAP_IO_Capability_t IOCapability;            /* Variable which holds the        */
                                                    /* current I/O Capabilities that   */
                                                    /* are to be used for Secure Simple*/
                                                    /* Pairing.                        */

static Boolean_t           OOBSupport;              /* Variable which flags whether    */
                                                    /* or not Out of Band Secure Simple*/
                                                    /* Pairing exchange is supported.  */

static Boolean_t           MITMProtection;          /* Variable which flags whether or */
                                                    /* not Man in the Middle (MITM)    */
                                                    /* protection is to be requested   */
                                                    /* during a Secure Simple Pairing  */
                                                    /* procedure.                      */

static BD_ADDR_t           NullADDR;                /* Holds a NULL BD_ADDR for comp.  */
                                                    /* purposes.                       */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static BoardStr_t          Callback_BoardStr;       /* Holds a BD_ADDR string in the   */
                                                    /* Callbacks.                      */

static BoardStr_t          Function_BoardStr;       /* Holds a BD_ADDR string in the   */
                                                    /* various functions.              */

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

static Word_t   Connection_Handle;                  /* Holds the connection handle on
                                                       stream open indication          */

static Boolean_t           AbsoluteVolumeEnabled;
static int                 VolumeChangedEventTransactionID;
static unsigned int        CurrentVolume;

static BTPSCONST unsigned char DeviceName[]            = APP_DEMO_NAME;

static BTPSCONST unsigned char SinkDescription[]       = "A2DP Sink";
static BTPSCONST unsigned char ProviderName[]          = "Texas Instruments";
static BTPSCONST unsigned char ControllerDescription[] = "AVRCP Controller";

static AUD_Stream_Format_t AudioSNKSupportedFormats[] =
{
   { 48000, 2, 0 },
   { 44100, 2, 0 },
   { 48000, 1, 0 },
   { 44100, 1, 0 }
} ;


#define NUM_SNK_SUPPORTED_FORMATS      (sizeof(AudioSNKSupportedFormats)/sizeof(AUD_Stream_Format_t))

   /* The following string table is used to map HCI Version information */
   /* to an easily displayable version string.                          */
static BTPSCONST char *HCIVersionStrings[] =
{
   "1.0b",
   "1.1",
   "1.2",
   "2.0",
   "2.1",
   "3.0",
   "4.0",
   "4.1",
   "Unknown (greater 4.1)"
} ;

#define NUM_SUPPORTED_HCI_VERSIONS              (sizeof(HCIVersionStrings)/sizeof(char *) - 1)

   /* The following string table is used to map the API I/O Capabilities*/
   /* values to an easily displayable string.                           */
static BTPSCONST char *IOCapabilitiesStrings[] =
{
   "Display Only",
   "Display Yes/No",
   "Keyboard Only",
   "No Input/Output"
} ;

static UUIDInfo_t UUIDTable[] =
{
   { "L2CAP",                 { 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Advanced Audio",        { 0x00, 0x00, 0x11, 0x0D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "A/V Remote Control",    { 0x00, 0x00, 0x11, 0x0E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Basic Imaging",         { 0x00, 0x00, 0x11, 0x1A, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Basic Printing",        { 0x00, 0x00, 0x11, 0x22, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Dial-up Networking",    { 0x00, 0x00, 0x11, 0x03, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "FAX",                   { 0x00, 0x00, 0x11, 0x11, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "File Transfer",         { 0x00, 0x00, 0x11, 0x06, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Hard Copy Cable Repl.", { 0x00, 0x00, 0x11, 0x25, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Health Device",         { 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Headset",               { 0x00, 0x00, 0x11, 0x08, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Handsfree",             { 0x00, 0x00, 0x11, 0x1E, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "HID",                   { 0x00, 0x00, 0x11, 0x24, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "LAN Access",            { 0x00, 0x00, 0x11, 0x02, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Message Access",        { 0x00, 0x00, 0x11, 0x34, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Object Push",           { 0x00, 0x00, 0x11, 0x05, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Personal Area Network", { 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Phonebook Access",      { 0x00, 0x00, 0x11, 0x30, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "SIM Access",            { 0x00, 0x00, 0x11, 0x2D, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "Serial Port",           { 0x00, 0x00, 0x11, 0x01, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } },
   { "IrSYNC",                { 0x00, 0x00, 0x11, 0x04, 0x00, 0x00, 0x10, 0x00, 0x80, 0x00, 0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB } }
} ;

#define NUM_UUIDS                               (sizeof(UUIDTable)/sizeof(UUIDInfo_t))

   /* Internal function prototypes.                                     */
static void UserInterface(void);
static Boolean_t CommandLineInterpreter(char *Command);
static unsigned int StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *Input);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

static void BD_ADDRToStr(BD_ADDR_t Board_Address, char *BoardStr);
static void DisplayIOCapabilities(void);
static void DisplayClassOfDevice(Class_of_Device_t Class_of_Device);
static void DisplayPrompt(void);
static void DisplayUsage(char *UsageString);
static void DisplayFunctionError(char *Function,int Status);
static void DisplayFunctionSuccess(char *Function);


static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
static int CloseStack(void);

static int Initialize_Sink(void);

static void Change_connection_priority(int high_normal);
static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);
static int DeleteLinkKey(BD_ADDR_t BD_ADDR);

static int DisplayHelp(ParameterList_t *TempParam);
static int Inquiry(ParameterList_t *TempParam);
static int DisplayInquiryList(ParameterList_t *TempParam);
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangeSimplePairingParameters(ParameterList_t *TempParam);
static int Pair(ParameterList_t *TempParam);
static int EndPairing(ParameterList_t *TempParam);
static int PINCodeResponse(ParameterList_t *TempParam);
static int PassKeyResponse(ParameterList_t *TempParam);
static int UserConfirmationResponse(ParameterList_t *TempParam);
static int GetLocalAddress(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int GetLocalName(ParameterList_t *TempParam);
static int SetClassOfDevice(ParameterList_t *TempParam);
static int GetClassOfDevice(ParameterList_t *TempParam);
static int GetRemoteName(ParameterList_t *TempParam);
static int ServiceDiscovery(ParameterList_t *TempParam);

static int OpenRequestResponse(ParameterList_t *TempParam);
static int OpenRemoteStream(ParameterList_t *TempParam);
static int CloseStream(ParameterList_t *TempParam);
static int OpenRemoteControl(ParameterList_t *TempParam);
static int CloseRemoteControl(ParameterList_t *TempParam);
static int ChangeStreamState(ParameterList_t *TempParam);
static int QueryStreamState(ParameterList_t *TempParam);
static int ChangeStreamFormat(ParameterList_t *TempParam);
static int QueryStreamFormat(ParameterList_t *TempParam);
static int QuerySupportedFormats(ParameterList_t *TempParam);
static int QueryStreamConfiguration(ParameterList_t *TempParam);
static int ChangeConnectionMode(ParameterList_t *TempParam);
static int QueryConnectionMode(ParameterList_t *TempParam);
static int SendPassThroughCommand(ParameterList_t *TempParam);
static int SetVolume(ParameterList_t *TempParam);
static int QueryMemory(ParameterList_t *TempParam);

static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel);
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse);
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter);
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter);

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function for SNK mode.    */
   /* First, this function retrieves a String of user input, parses the */
   /* user input into Command and Parameters, and finally executes the  */
   /* Command or Displays an Error Message if the input is not a valid  */
   /* Command.                                                          */
static void UserInterface(void)
{
    /* Next display the available commands.                           */
    DisplayHelp(NULL);

    ClearCommands();

    AddCommand("INQUIRY", Inquiry);
    AddCommand("DISPLAYINQUIRYLIST", DisplayInquiryList);
    AddCommand("PAIR", Pair);
    AddCommand("ENDPAIRING", EndPairing);
    AddCommand("PINCODERESPONSE", PINCodeResponse);
    AddCommand("PASSKEYRESPONSE", PassKeyResponse);
    AddCommand("USERCONFIRMATIONRESPONSE", UserConfirmationResponse);
    AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
    AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
    AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
    AddCommand("CHANGESIMPLEPAIRINGPARAMETERS", ChangeSimplePairingParameters);
    AddCommand("GETLOCALADDRESS", GetLocalAddress);
    AddCommand("SETLOCALNAME", SetLocalName);
    AddCommand("GETLOCALNAME", GetLocalName);
    AddCommand("SETCLASSOFDEVICE", SetClassOfDevice);
    AddCommand("GETCLASSOFDEVICE", GetClassOfDevice);
    AddCommand("GETREMOTENAME", GetRemoteName);
    AddCommand("SERVICEDISCOVERY", ServiceDiscovery);
    AddCommand("OPENREQUESTRESPONSE", OpenRequestResponse);
    AddCommand("OPENREMOTECONTROL", OpenRemoteControl);
    AddCommand("CLOSEREMOTECONTROL", CloseRemoteControl);
    AddCommand("OPENREMOTESTREAM", OpenRemoteStream);
    AddCommand("CLOSESTREAM", CloseStream);
    AddCommand("CHANGESTREAMSTATE", ChangeStreamState);
    AddCommand("QUERYSTREAMSTATE", QueryStreamState);
    AddCommand("CHANGESTREAMFORMAT", ChangeStreamFormat);
    AddCommand("QUERYSTREAMFORMAT", QueryStreamFormat);
    AddCommand("QUERYSUPPORTEDFORMATS", QuerySupportedFormats);
    AddCommand("QUERYSTREAMCONFIG", QueryStreamConfiguration);
    AddCommand("CHANGECONNECTIONMODE", ChangeConnectionMode);
    AddCommand("QUERYCONNECTIONMODE", QueryConnectionMode);
    AddCommand("SENDPASSTHROUGHCOMMAND", SendPassThroughCommand);
    AddCommand("SETVOLUME", SetVolume);
    AddCommand("MEMORYUSAGE", QueryMemory);
    AddCommand("HELP", DisplayHelp);
}


   /* The following function is responsible for parsing user input      */
   /* and call appropriate command function.                            */
static Boolean_t CommandLineInterpreter(char *Command)
{
   int           Result = !EXIT_CODE;
   Boolean_t     ret_val = FALSE;
   UserCommand_t TempCommand;

   /* The string input by the user contains a value, now run the string */
   /* through the Command Parser.                                       */
       if(CommandParser(&TempCommand, Command) >= 0)
         {
            Display(("\r\n"));

            /* The Command was successfully parsed run the Command.           */
            Result = CommandInterpreter(&TempCommand);
            switch(Result)
            {
               case INVALID_COMMAND_ERROR:
                  Display(("Invalid Command: %s.\r\n",TempCommand.Command));
                  break;
               case FUNCTION_ERROR:
                  Display(("Function Error.\r\n"));
                  break;
               case EXIT_CODE:           
                  break;
            }

          /* Display a prompt.                                              */
          DisplayPrompt();

          ret_val = TRUE;
         }
       else
        {
          /* Display a prompt.                                              */
          DisplayPrompt();

          Display(("\r\nInvalid Command.\r\n"));
        }
    
   return(ret_val);
}
  


   /* The following function is responsible for converting number       */
   /* strings to there unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned int StringToUnsignedInteger(char *StringInteger)
{
   int          IsHex;
   unsigned int Index;
   unsigned int ret_val = 0;

   /* Before proceeding make sure that the parameter that was passed as */
   /* an input appears to be at least semi-valid.                       */
   if((StringInteger) && (BTPS_StringLength(StringInteger)))
   {
      /* Initialize the variable.                                       */
      Index = 0;

      /* Next check to see if this is a hexadecimal number.             */
      if(BTPS_StringLength(StringInteger) > 2)
      {
         if((StringInteger[0] == '0') && ((StringInteger[1] == 'x') || (StringInteger[1] == 'X')))
         {
            IsHex = 1;

            /* Increment the String passed the Hexadecimal prefix.      */
            StringInteger += 2;
         }
         else
            IsHex = 0;
      }
      else
         IsHex = 0;

      /* Process the value differently depending on whether or not a    */
      /* Hexadecimal Number has been specified.                         */
      if(!IsHex)
      {
         /* Decimal Number has been specified.                          */
         while(1)
         {
            /* First check to make sure that this is a valid decimal    */
            /* digit.                                                   */
            if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               ret_val += (StringInteger[Index] & 0xF);

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9'))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 10.                                */
                  ret_val *= 10;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
      else
      {
         /* Hexadecimal Number has been specified.                      */
         while(1)
         {
            /* First check to make sure that this is a valid Hexadecimal*/
            /* digit.                                                   */
            if(((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9')) || ((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f')) || ((StringInteger[Index] >= 'A') && (StringInteger[Index] <= 'F')))
            {
               /* This is a valid digit, add it to the value being      */
               /* built.                                                */
               if((StringInteger[Index] >= '0') && (StringInteger[Index] <= '9'))
                  ret_val += (StringInteger[Index] & 0xF);
               else
               {
                  if((StringInteger[Index] >= 'a') && (StringInteger[Index] <= 'f'))
                     ret_val += (StringInteger[Index] - 'a' + 10);
                  else
                     ret_val += (StringInteger[Index] - 'A' + 10);
               }

               /* Determine if the next digit is valid.                 */
               if(((Index + 1) < BTPS_StringLength(StringInteger)) && (((StringInteger[Index+1] >= '0') && (StringInteger[Index+1] <= '9')) || ((StringInteger[Index+1] >= 'a') && (StringInteger[Index+1] <= 'f')) || ((StringInteger[Index+1] >= 'A') && (StringInteger[Index+1] <= 'F'))))
               {
                  /* The next digit is valid so multiply the current    */
                  /* return value by 16.                                */
                  ret_val *= 16;
               }
               else
               {
                  /* The next value is invalid so break out of the loop.*/
                  break;
               }
            }

            Index++;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for parsing strings into    */
   /* components.  The first parameter of this function is a pointer to */
   /* the String to be parsed.  This function will return the start of  */
   /* the string upon success and a NULL pointer on all errors.         */
static char *StringParser(char *String)
{
   int   Index;
   char *ret_val = NULL;

   /* Before proceeding make sure that the string passed in appears to  */
   /* be at least semi-valid.                                           */
   if((String) && (BTPS_StringLength(String)))
   {
      /* The string appears to be at least semi-valid.  Search for the  */
      /* first space character and replace it with a NULL terminating   */
      /* character.                                                     */
      for(Index=0, ret_val=String;Index < BTPS_StringLength(String);Index++)
      {
         /* Is this the space character.                                */
         if((String[Index] == ' ') || (String[Index] == '\r') || (String[Index] == '\n'))
         {
            /* This is the space character, replace it with a NULL      */
            /* terminating character and set the return value to the    */
            /* beginning character of the string.                       */
            String[Index] = '\0';
            break;
         }
      }
   }

   return(ret_val);
}

   /* This function is responsible for taking command strings and       */
   /* parsing them into a command, param1, and param2.  After parsing   */
   /* this string the data is stored into a UserCommand_t structure to  */
   /* be used by the interpreter.  The first parameter of this function */
   /* is the structure used to pass the parsed command string out of the*/
   /* function.  The second parameter of this function is the string    */
   /* that is parsed into the UserCommand structure.  Successful        */
   /* execution of this function is denoted by a return value of zero.  */
   /* Negative return values denote an error in the parsing of the      */
   /* string parameter.                                                 */
static int CommandParser(UserCommand_t *TempCommand, char *Input)
{
   int            ret_val;
   int            StringLength;
   char          *LastParameter;
   unsigned int   Count         = 0;

   /* Before proceeding make sure that the passed parameters appear to  */
   /* be at least semi-valid.                                           */
   if((TempCommand) && (Input) && (BTPS_StringLength(Input)))
   {
      /* First get the initial string length.                           */
      StringLength = BTPS_StringLength(Input);

      /* Retrieve the first token in the string, this should be the     */
      /* command.                                                       */
      TempCommand->Command = StringParser(Input);

      /* Flag that there are NO Parameters for this Command Parse.      */
      TempCommand->Parameters.NumberofParameters = 0;

       /* Check to see if there is a Command                            */
      if(TempCommand->Command)
      {
         /* Initialize the return value to zero to indicate success on  */
         /* commands with no parameters.                                */
         ret_val    = 0;

         /* Adjust the UserInput pointer and StringLength to remove the */
         /* Command from the data passed in before parsing the          */
         /* parameters.                                                 */
         Input        += BTPS_StringLength(TempCommand->Command) + 1;
         StringLength  = BTPS_StringLength(Input);

         /* There was an available command, now parse out the parameters*/
         while((StringLength > 0) && ((LastParameter = StringParser(Input)) != NULL))
         {
            /* There is an available parameter, now check to see if     */
            /* there is room in the UserCommand to store the parameter  */
            if(Count < (sizeof(TempCommand->Parameters.Params)/sizeof(Parameter_t)))
            {
               /* Save the parameter as a string.                       */
               TempCommand->Parameters.Params[Count].strParam = LastParameter;

               /* Save the parameter as an unsigned int intParam will   */
               /* have a value of zero if an error has occurred.        */
               TempCommand->Parameters.Params[Count].intParam = StringToUnsignedInteger(LastParameter);

               Count++;
               Input        += BTPS_StringLength(LastParameter) + 1;
               StringLength -= BTPS_StringLength(LastParameter) + 1;

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               StringLength = 0;

               ret_val      = TOO_MANY_PARAMS;
            }
         }

         /* Set the number of parameters in the User Command to the     */
         /* number of found parameters.                                 */
         TempCommand->Parameters.NumberofParameters = Count;
      }
      else
      {
         /* No command was specified                                    */
         ret_val = NO_COMMAND_ERROR;
      }
   }
   else
   {
      /* One or more of the passed parameters appear to be invalid.     */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* This function is responsible for determining the command in which */
   /* the user entered and running the appropriate function associated  */
   /* with that command.  The first parameter of this function is a     */
   /* structure containing information about the command to be issued.  */
   /* This information includes the command name and multiple parameters*/
   /* which maybe be passed to the function to be executed.  Successful */
   /* execution of this function is denoted by a return value of zero.  */
   /* A negative return value implies that command was not found and is */
   /* invalid.                                                          */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invalid   */
   /* command error                                                     */
   ret_val = INVALID_COMMAND_ERROR;

   /* Let's make sure that the data passed to us appears semi-valid.    */
   if((TempCommand) && (TempCommand->Command))
   {
      /* Now, let's make the Command string all upper case so that we   */
      /* compare against it.                                            */
      for(i=0;i<BTPS_StringLength(TempCommand->Command);i++)
      {
         if((TempCommand->Command[i] >= 'a') && (TempCommand->Command[i] <= 'z'))
            TempCommand->Command[i] -= ('a' - 'A');
      }

      /* The command entered is not exit so search for command in    */
      /* table.                                                      */
      if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
      {
         /* The command was found in the table so call the command.  */
         ret_val = (*CommandFunction)(&TempCommand->Parameters);
         if(!ret_val)
         {
            /* Return success to the caller.                         */
            ret_val = 0;
         }
         else
         {
            if((ret_val != EXIT_CODE) && (ret_val != EXIT_TEST_MODE))
               ret_val = FUNCTION_ERROR;
         }
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a means to            */
   /* programatically add Commands the Global (to this module) Command  */
   /* Table.  The Command Table is simply a mapping of Command Name     */
   /* (NULL terminated ASCII string) to a command function.  This       */
   /* function returns zero if successful, or a non-zero value if the   */
   /* command could not be added to the list.                           */
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction)
{
   int ret_val = 0;

   /* First, make sure that the parameters passed to us appear to be    */
   /* semi-valid.                                                       */
   if((CommandName) && (CommandFunction))
   {
      /* Next, make sure that we still have room in the Command Table   */
      /* to add commands.                                               */
      if(NumberCommands < MAX_SUPPORTED_COMMANDS)
      {
         /* Simply add the command data to the command table and        */
         /* increment the number of supported commands.                 */
         CommandTable[NumberCommands].CommandName       = CommandName;
         CommandTable[NumberCommands++].CommandFunction = CommandFunction;

         /* Return success to the caller.                               */
         ret_val                                        = 0;
      }
      else
         ret_val = 1;
   }
   else
      ret_val = 1;

   return(ret_val);
}

   /* The following function searches the Command Table for the         */
   /* specified Command.  If the Command is found, this function returns*/
   /* a NON-NULL Command Function Pointer.  If the command is not found */
   /* this function returns NULL.                                       */
static CommandFunction_t FindCommand(char *Command)
{
   unsigned int      Index;
   CommandFunction_t ret_val;

   /* First, make sure that the command specified is semi-valid.        */
   if(Command)
   {
      /* Now loop through each element in the table to see if there is  */
      /* a match.                                                       */
      for(Index=0,ret_val=NULL;((Index<NumberCommands) && (!ret_val));Index++)
      {
         if(BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(CommandTable[Index].CommandName)) == 0)
            ret_val = CommandTable[Index].CommandFunction;
      }
   }
   else
      ret_val = NULL;

   return(ret_val);
}

   /* The following function is provided to allow a means to clear out  */
   /* all available commands from the command table.                    */
static void ClearCommands(void)
{
   /* Simply flag that there are no commands present in the table.      */
   NumberCommands = 0;
}

   /* The following function is responsible for converting data of type */
   /* BD_ADDR to a string.  The first parameter of this function is the */
   /* BD_ADDR to be converted to a string.  The second parameter of this*/
   /* function is a pointer to the string in which the converted BD_ADDR*/
   /* is to be stored.                                                  */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr)
{
   BTPS_SprintF((char *)BoardStr, "0x%02X%02X%02X%02X%02X%02X", Board_Address.BD_ADDR5, Board_Address.BD_ADDR4, Board_Address.BD_ADDR3, Board_Address.BD_ADDR2, Board_Address.BD_ADDR1, Board_Address.BD_ADDR0);
}

   /* Displays the current I/O Capabilities.                            */
static void DisplayIOCapabilities(void)
{
   Display(("I/O Capabilities: %s, MITM: %s.\r\n", IOCapabilitiesStrings[(unsigned int)IOCapability], MITMProtection?"TRUE":"FALSE"));
}

   /* Utility function to display a Class of Device Structure.          */
static void DisplayClassOfDevice(Class_of_Device_t Class_of_Device)
{
   Display(("Class of Device: 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2));
}

   /* Displays the correct prompt depening on the Server/Client Mode.   */
static void DisplayPrompt(void)
{
   Display(("\r\nSink> \b"));
}

   /* Displays a usage string..                                         */
static void DisplayUsage(char *UsageString)
{
   Display(("\nUsage: %s.\r\n", UsageString));
}

   /* Displays a function error.                                        */
static void DisplayFunctionError(char *Function,int Status)
{
   Display(("\n%s Failed: %d.\r\n", Function, Status));
}

static void DisplayFunctionSuccess(char *Function)
{
   Display(("\n%s success.\r\n", Function));
}

   /* The following function is for displaying The FW Version by reading*/
   /* The Local version information form the FW.                        */
void DisplayFWVersion (void)
{
    typedef struct 
    {
        Byte_t StatusResult; 
        Byte_t HCI_VersionResult;
        Word_t HCI_RevisionResult;
        Byte_t LMP_VersionResult; 
        Word_t Manufacturer_NameResult; 
        Word_t LMP_SubversionResult;
    }FW_Version;
    FW_Version FW_Version_Details;
    
    /* This function retrieves the Local Version Information of the FW. */    
    HCI_Read_Local_Version_Information(BluetoothStackID, &FW_Version_Details.StatusResult, &FW_Version_Details.HCI_VersionResult, &FW_Version_Details.HCI_RevisionResult, &FW_Version_Details.LMP_VersionResult, &FW_Version_Details.Manufacturer_NameResult, &FW_Version_Details.LMP_SubversionResult);
    if (!FW_Version_Details.StatusResult)
    {
        /* This function prints The project type from firmware, Bits    */
        /* 10 to 14 (5 bits) from LMP_SubversionResult parameter.       */
        Display(("Project Type  : %d \r\n", ((FW_Version_Details.LMP_SubversionResult >> 10)) & 0x1F));
        /* This function prints The version of the firmware. The first  */
        /* number is the Major version, Bits 7 to 9 and 15 (4 bits) from*/
        /* LMP_SubversionResult parameter, the second number is the     */
        /* Minor Version, Bits 0 to 6 (7 bits) from LMP_SubversionResult*/
        /* parameter.                                                   */
        Display(("FW Version    : %d.%d \r\n", ((FW_Version_Details.LMP_SubversionResult >> 7) & 0x07) + ((FW_Version_Details.LMP_SubversionResult >> 12) & 0x08), FW_Version_Details.LMP_SubversionResult & 0x7F));
    }
    else
        /* There was an error with HCI_Read_Local_Version_Information.  */
        /* Function.                                                    */
        DisplayFunctionError("HCI_Read_Local_Version_Information", FW_Version_Details.StatusResult);
}


   /* The following function is responsible for opening the SS1         */
   /* Bluetooth Protocol Stack.  This function accepts a pre-populated  */
   /* HCI Driver Information structure that contains the HCI Driver     */
   /* Transport Information.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization)
{
   int                        Result;
   int                        ret_val = 0;
   char                       BluetoothAddress[16];
   Byte_t                     Status;
   BD_ADDR_t                  BD_ADDR;
   HCI_Version_t              HCIVersion;
   Class_of_Device_t          Class_of_Device;
   L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;
  

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
       if((HCI_DriverInformation) && (BTPS_Initialization))
      {

         /* Initialize BTPSKNRl.                                        */
         BTPS_Init(BTPS_Initialization);

         Display(("\r\nOpenStack().\r\n"));

         /* Initialize the Stack                                        */
         Result = BSC_Initialize(HCI_DriverInformation, 0);

         /* Next, check the return value of the initialization to see if*/
         /* it was successful.                                          */
         if(Result > 0)
         {
            /* The Stack was initialized successfully, inform the user  */
            /* and set the return value of the initialization function  */
            /* to the Bluetooth Stack ID.                               */
            BluetoothStackID = Result;
            Display(("Bluetooth Stack ID: %d\r\n", BluetoothStackID));

            /* Initialize the default Secure Simple Pairing parameters. */
            IOCapability     = DEFAULT_IO_CAPABILITY;
            OOBSupport       = FALSE;
            MITMProtection   = DEFAULT_MITM_PROTECTION;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
               Display(("Device Chipset: %s\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]));

            /* Printing the BTPS version                                */
            Display(("BTPS Version  : %s \r\n", BTPS_VERSION_VERSION_STRING));
            /* Printing the FW version                                  */
            DisplayFWVersion();

            /* Printing the Demo Application version                    */
            Display(("App Name      : %s \r\n", APP_DEMO_NAME));
            Display(("App Version   : %s \r\n", DEMO_APPLICATION_VERSION_STRING));            
            
            /* Let's output the Bluetooth Device Address so that the    */
            /* user knows what the Device Address is.                   */
            if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
            {
               BD_ADDRToStr(BD_ADDR, BluetoothAddress);

               Display(("LOCAL BD_ADDR: %s\r\n", BluetoothAddress));
            }

            /* Set the Name for the initial use.              */
            GAP_Set_Local_Device_Name (BluetoothStackID,APP_DEMO_NAME);


            /* Go ahead and allow Master/Slave Role Switch.             */
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config  = cqAllowRoleSwitch;
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config = csMaintainCurrentRole;

            L2CA_Set_Link_Connection_Configuration(BluetoothStackID, &L2CA_Link_Connect_Params);

            if(HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER) > 0)
               HCI_Write_Default_Link_Policy_Settings(BluetoothStackID, (HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH|HCI_LINK_POLICY_SETTINGS_ENABLE_SNIFF_MODE), &Status);

            /* Delete all Stored Link Keys.                             */
            ASSIGN_BD_ADDR(BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

            DeleteLinkKey(BD_ADDR);

            GAP_Set_Local_Device_Name(BluetoothStackID, (char *)DeviceName);

            ASSIGN_CLASS_OF_DEVICE(Class_of_Device, 0x24, 0x04, 0x04);
            GAP_Set_Class_Of_Device(BluetoothStackID, Class_of_Device);
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            DisplayFunctionError("Stack Init", Result);

            BluetoothStackID = 0;

            ret_val          = UNABLE_TO_INITIALIZE_STACK;
         }
      }
      else
      {
         /* One or more of the necessary parameters are invalid.        */
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }

   return(ret_val);
}

   /* The following function is responsible for closing the SS1         */
   /* Bluetooth Protocol Stack.  This function requires that the        */
   /* Bluetooth Protocol stack previously have been initialized via the */
   /* OpenStack() function.  This function returns zero on successful   */
   /* execution and a negative value on all errors.                     */
static int CloseStack(void)
{
   int ret_val = 0;

   /* First check to see if the Stack has been opened.                  */
   if(BluetoothStackID)
   {
      /* Clean up Audio profile sub-system.                             */
      AUD_Un_Initialize(BluetoothStackID);

      AbsoluteVolumeEnabled = FALSE;

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      /* Free BTPSKRNL allocated memory.                                */
      BTPS_DeInit();

      Display(("Stack Shutdown.\r\n"));

      /* Flag that the Stack is no longer initialized.                  */
      BluetoothStackID = 0;

      /* Flag success to the caller.                                    */
      ret_val          = 0;
   }
   else
   {
      /* A valid Stack ID does not exist, inform to user.               */
      ret_val = UNABLE_TO_INITIALIZE_STACK;
   }

   return(ret_val);
}

   /* The following function is responsible for initializing the Audio  */
   /* Manager with Sink endpoints.                                      */
static int Initialize_Sink(void)
{
   int                                      ret_val;
   AUD_Initialization_Info_t                InitializationInfo;
   AUD_Stream_Initialization_Info_t         InitializationInfoSNK;
   AUD_Remote_Control_Initialization_Info_t InitializationInfoAVR;
   AUD_Remote_Control_Role_Info_t           RemoteControlRoleInfo;

   /* First, check to make sure that a valid Bluetooth Stack ID exists. */
   if(BluetoothStackID)
   {
      /* Next, check to make sure that the Audio Manager has not already*/
      /* been initialized.                                              */
      if(!Initialized)
      {
         VolumeChangedEventTransactionID = -1;

         /* Audio has not been initialized, now let's attempt to        */
         /* initialize it.                                              */
         BTPS_MemInitialize(&InitializationInfo, 0, sizeof(AUD_Initialization_Info_t));
         BTPS_MemInitialize(&InitializationInfoSNK, 0, sizeof(AUD_Stream_Initialization_Info_t));
         BTPS_MemInitialize(&InitializationInfoAVR, 0, sizeof(AUD_Remote_Control_Initialization_Info_t));
         BTPS_MemInitialize(&RemoteControlRoleInfo, 0, sizeof(AUD_Remote_Control_Role_Info_t));

         InitializationInfo.SNKInitializationInfo           = &InitializationInfoSNK;
         InitializationInfo.RemoteControlInitializationInfo = &InitializationInfoAVR;

         InitializationInfoSNK.EndpointSDPDescription       = (char *)SinkDescription;
         InitializationInfoSNK.NumberConcurrentStreams      = 1;
         InitializationInfoSNK.NumberSupportedStreamFormats = NUM_SNK_SUPPORTED_FORMATS;
         BTPS_MemCopy(InitializationInfoSNK.StreamFormat, AudioSNKSupportedFormats, sizeof(AudioSNKSupportedFormats));

         InitializationInfoAVR.ControllerRoleInfo           = &RemoteControlRoleInfo;
         InitializationInfoAVR.SupportedVersion             = apvVersion1_0;

         RemoteControlRoleInfo.ProviderName                 = (char *)ProviderName;
         RemoteControlRoleInfo.ServiceName                  = (char *)ControllerDescription;
         RemoteControlRoleInfo.SupportedFeaturesFlags       = SDP_AVRCP_SUPPORTED_FEATURES_CONTROLLER_CATEGORY_1;

         /* Everything has been initialized, now attemp to initialize   */
         /* the Audio Manager.                                          */
         ret_val = AUD_Initialize(BluetoothStackID, &InitializationInfo, AUD_Event_Callback, 0);
         if(!ret_val)
         {
            DisplayFunctionSuccess("AUD_Initialize() Sink");

            Initialized = TRUE;
         }
         else
            DisplayFunctionError("AUD_Initialize()", ret_val);
      }
      else
         ret_val = -1;
   }
   else
      ret_val = -1;

   return(ret_val);
}


static void Change_connection_priority(int high_normal)
{  
   /*
     The HCI flow specifications parameters: 1 Flags=0x00, 1 For incoming=0x01, 1 Guaranteed=0x02,
     4 Token Rate=25000, 4 Token Bucket Size=333, 4 Peak Bandwidth=25000, 4 Access Latency=13000 .
     0x00, 0x01, 0x02, 25000, 333, 25000, 13000
   */
   Byte_t  Flags = FLOW_SPEC_FLAGS;             
   Byte_t  Flow_Direction = FLOW_SPEC_FLOW_DIRECTION;
   Byte_t  Service_Type = FLOW_SPEC_SERVICE_TYPE_GUARANTEED;
   /* The rest are the defaults for A2DP- */
   DWord_t Token_Rate = FLOW_SPEC_TOKEN_RATE;
   DWord_t Token_Bucket_Size = FLOW_SPEC_TOKEN_BUCKET_SIZE;
   DWord_t Peak_Bandwidth = FLOW_SPEC_PEAK_BANDWIDTH;
   DWord_t Access_Latency = FLOW_SPEC_ACCESS_LATENCY; 
   Byte_t StatusResult;
   
  if(HIGH_AUDIO_CONNECTION_PRIORITY == high_normal)
  {
     Display(("\r\n Send HCI Flow Spec for guarenteed ACL connection\r\n"));
     /* For A2DP streaming - Set the ACL to guarenteed with 90% to the ACL over scans priority */
     /* Send Flow spec command for setting the ACL connection to guarenteed */
     HCI_Flow_Specification(BluetoothStackID, Connection_Handle, Flags, Flow_Direction, Service_Type, 
                            Token_Rate, Token_Bucket_Size, Peak_Bandwidth, Access_Latency, &StatusResult);
     Display(("\r\n Send HCI VS DDIP for ACL priority over scans\r\n"));
     /* Send Vendor DDIP command to change the ACL priority over scans to 90% */
     VS_Send_DDIP(BluetoothStackID, BEST_EFFORT_HIGH_PRIORITY, GUARANTEED_HIGH_PRIORITY);
  }
  else
  {
     /* Change back the priority to Best effort=20% and Guarenteed = 70%    */ 
     /* Change back the ACL to Best Effort */
     Service_Type = FLOW_SPEC_SERVICE_TYPE_BESTEFFORT;
     
     Display(("\r\n Send HCI Flow Spec for best effort ACL connection\r\n"));
     HCI_Flow_Specification(BluetoothStackID, Connection_Handle, Flags, Flow_Direction, Service_Type, 
                            Token_Rate, Token_Bucket_Size, Peak_Bandwidth, Access_Latency, &StatusResult);
     Display(("\r\n Send HCI VS DDIP for ACL priority over scans\r\n"));
     /* Change back the priority to Best effort=20% and Guarenteed = 70%    */ 
     VS_Send_DDIP(BluetoothStackID, BEST_EFFORT_NORMAL_PRIORITY, GUARANTEED_NORMAL_PRIORITY);
  }
}
   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into General Discoverablity Mode.  Once in this  */
   /* mode the Device will respond to Inquiry Scans from other Bluetooth*/
   /* Devices.  This function requires that a valid Bluetooth Stack ID  */
   /* exists before running.  This function returns zero on successful  */
   /* execution and a negative value if an error occurred.              */
static int SetDisc(void)
{
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* A semi-valid Bluetooth Stack ID exists, now attempt to set the */
      /* attached Devices Discoverablity Mode to General.               */
      ret_val = GAP_Set_Discoverability_Mode(BluetoothStackID, dmGeneralDiscoverableMode, 0);

      /* Next, check the return value of the GAP Set Discoverability    */
      /* Mode command for successful execution.                         */
      if(ret_val)
      {
         /* An error occurred while trying to set the Discoverability   */
         /* Mode of the Device.                                         */
         DisplayFunctionError("Set Discoverable Mode", ret_val);
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the Local       */
   /* Bluetooth Device into Connectable Mode.  Once in this mode the    */
   /* Device will respond to Page Scans from other Bluetooth Devices.   */
   /* This function requires that a valid Bluetooth Stack ID exists     */
   /* before running.  This function returns zero on success and a      */
   /* negative value if an error occurred.                              */
static int SetConnect(void)
{
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached Device to be Connectable.          */
      ret_val = GAP_Set_Connectability_Mode(BluetoothStackID, cmConnectableMode);

      /* Next, check the return value of the                            */
      /* GAP_Set_Connectability_Mode() function for successful          */
      /* execution.                                                     */
      if(ret_val)
      {
         /* An error occurred while trying to make the Device           */
         /* Connectable.                                                */
         DisplayFunctionError("Set Connectability Mode", ret_val);
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for placing the local       */
   /* Bluetooth device into Pairable mode.  Once in this mode the device*/
   /* will response to pairing requests from other Bluetooth devices.   */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetPairable(void)
{
   int Result;
   int ret_val = 0;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to set the attached device to be pairable.             */
      Result = GAP_Set_Pairability_Mode(BluetoothStackID, pmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_Register_Remote_Authentication(BluetoothStackID, GAP_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(Result)
         {
            /* An error occurred while trying to execute this function. */
            DisplayFunctionError("Auth", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         DisplayFunctionError("Set Pairability Mode", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to delete*/
   /* the specified Link Key from the Local Bluetooth Device.  If a NULL*/
   /* Bluetooth Device Address is specified, then all Link Keys will be */
   /* deleted.                                                          */
static int DeleteLinkKey(BD_ADDR_t BD_ADDR)
{
   int       Result;
   Byte_t    Status_Result;
   Word_t    Num_Keys_Deleted = 0;
   BD_ADDR_t NULL_BD_ADDR;

   Result = HCI_Delete_Stored_Link_Key(BluetoothStackID, BD_ADDR, TRUE, &Status_Result, &Num_Keys_Deleted);

   /* Any stored link keys for the specified address (or all) have been */
   /* deleted from the chip.  Now, let's make sure that our stored Link */
   /* Key Array is in sync with these changes.                          */

   /* First check to see all Link Keys were deleted.                    */
   ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

   if(COMPARE_BD_ADDR(BD_ADDR, NULL_BD_ADDR))
      BTPS_MemInitialize(LinkKeyInfo, 0, sizeof(LinkKeyInfo));
   else
   {
      /* Individual Link Key.  Go ahead and see if know about the entry */
      /* in the list.                                                   */
      for(Result=0;(Result<sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Result++)
      {
         if(COMPARE_BD_ADDR(BD_ADDR, LinkKeyInfo[Result].BD_ADDR))
         {
            LinkKeyInfo[Result].BD_ADDR = NULL_BD_ADDR;

            break;
         }
      }
   }

   return(Result);
}

   /* The following function is responsible for displaying the current  */
   /* Command Options for either Serial Port Client or Serial Port      */
   /* Server.  The input parameter to this function is completely       */
   /* ignored, and only needs to be passed in because all Commands that */
   /* can be entered at the Prompt pass in the parsed information.  This*/
   /* function displays the current Command Options that are available  */
   /* and always returns zero.                                          */
static int DisplayHelp(ParameterList_t *TempParam)
{
   Display(("\r\n"));
   Display(("******************************************************************\r\n"));
   Display(("* Command Options: Inquiry, DisplayInquiryList, Pair,            *\r\n"));
   Display(("*                  EndPairing, PINCodeResponse, PassKeyResponse, *\r\n"));
   Display(("*                  UserConfirmationResponse,                     *\r\n"));
   Display(("*                  SetDiscoverabilityMode, SetConnectabilityMode,*\r\n"));
   Display(("*                  SetPairabilityMode,                           *\r\n"));
   Display(("*                  ChangeSimplePairingParameters,                *\r\n"));
   Display(("*                  GetLocalAddress, GetLocalName, SetLocalName,  *\r\n"));
   Display(("*                  GetClassOfDevice, SetClassOfDevice,           *\r\n"));
   Display(("*                  GetRemoteName, ServiceDiscovery,              *\r\n"));
   Display(("*                  OpenRequestResponse, OpenRemoteStream,        *\r\n"));
   Display(("*                  CloseStream, OpenRemoteControl,               *\r\n"));
   Display(("*                  CloseRemoteControl, ChangeStreamState,        *\r\n"));
   Display(("*                  QueryStreamState, ChangeStreamFormat,         *\r\n"));
   Display(("*                  QueryStreamFormat, QuerySupportedFormats,     *\r\n"));
   Display(("*                  QueryStreamConfig, ChangeConnectionMode,      *\r\n"));
   Display(("*                  QueryConnectionMode, SendPassThroughCommand,  *\r\n"));
   Display(("*                  Help                                          *\r\n"));
   Display(("******************************************************************\r\n"));

   return(0);
}

   /* The following function is responsible for performing a General    */
   /* Inquiry for discovering Bluetooth Devices.  This function requires*/
   /* that a valid Bluetooth Stack ID exists before running.  This      */
   /* function returns zero is successful or a negative value if there  */
   /* was an error.                                                     */
static int Inquiry(ParameterList_t *TempParam)
{
   int Result;
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Use the GAP_Perform_Inquiry() function to perform an Inquiry.  */
      /* The Inquiry will last 10 seconds or until 1 Bluetooth Device is*/
      /* found.  When the Inquiry Results become available the          */
      /* GAP_Event_Callback is called.                                  */
      Result = GAP_Perform_Inquiry(BluetoothStackID, itGeneralInquiry, 0, 0, 10, MAX_INQUIRY_RESULTS, GAP_Event_Callback, (unsigned long)NULL);

      /* Next, check to see if the GAP_Perform_Inquiry() function was   */
      /* successful.                                                    */
      if(!Result)
      {
         /* The Inquiry appears to have been sent successfully.         */
         /* Processing of the results returned from this command occurs */
         /* within the GAP_Event_Callback() function.                   */

         /* Flag that we have found NO Bluetooth Devices.               */
         NumberofValidResponses = 0;

         ret_val                = 0;
      }
      else
      {
         /* A error occurred while performing the Inquiry.              */
         DisplayFunctionError("Inquiry", Result);

         ret_val = Result;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is a utility function that exists to       */
   /* display the current Inquiry List (with Indexes).  This is useful  */
   /* in case the user has forgotten what Inquiry Index a particular    */
   /* Bluteooth Device was located in.  This function returns zero on   */
   /* successful execution and a negative value on all errors.          */
static int DisplayInquiryList(ParameterList_t *TempParam)
{
   int          ret_val = 0;
   unsigned int Index;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply display all of the items in the Inquiry List.           */
      Display(("Inquiry List: %d Devices%s\r\n\r\n", NumberofValidResponses, NumberofValidResponses?":":"."));

      for(Index=0;Index<NumberofValidResponses;Index++)
      {
         BD_ADDRToStr(InquiryResultList[Index], Function_BoardStr);

         Display((" Inquiry Result: %d, %s.\r\n", (Index+1), Function_BoardStr));
      }

      if(NumberofValidResponses)
         Display(("\r\n"));

      /* All finished, flag success to the caller.                      */
      ret_val = 0;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int SetDiscoverabilityMode(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   GAP_Discoverability_Mode_t DiscoverabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 1)
            DiscoverabilityMode = dmLimitedDiscoverableMode;
         else
         {
            if(TempParam->Params[0].intParam == 2)
               DiscoverabilityMode = dmGeneralDiscoverableMode;
            else
               DiscoverabilityMode = dmNonDiscoverableMode;
         }

         /* Parameters mapped, now set the Discoverability Mode.        */
         Result = GAP_Set_Discoverability_Mode(BluetoothStackID, DiscoverabilityMode, (DiscoverabilityMode == dmLimitedDiscoverableMode)?60:0);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            Display(("Discoverability: %s.\r\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited")));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Discoverability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetDiscoverabilityMode [Mode(0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the             */
   /* Connectability Mode of the local device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static int SetConnectabilityMode(ParameterList_t *TempParam)
{
   int                       Result;
   int                       ret_val;
   GAP_Connectability_Mode_t ConnectableMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            ConnectableMode = cmNonConnectableMode;
         else
            ConnectableMode = cmConnectableMode;

         /* Parameters mapped, now set the Connectabilty Mode.          */
         Result = GAP_Set_Connectability_Mode(BluetoothStackID, ConnectableMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            Display(("Connectability Mode: %s.\r\n", (ConnectableMode == cmNonConnectableMode)?"Non Connectable":"Connectable"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Connectability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetConnectabilityMode [(0 = NonConectable, 1 = Connectable)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Pairability */
   /* Mode of the local device.  This function returns zero on          */
   /* successful execution and a negative value on all errors.          */
static int SetPairabilityMode(ParameterList_t *TempParam)
{
   int                     Result;
   int                     ret_val;
   char                   *Mode;
   GAP_Pairability_Mode_t  PairabilityMode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 2))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
         {
            PairabilityMode = pmNonPairableMode;
            Mode            = "pmNonPairableMode";
         }
         else
         {
            if(TempParam->Params[0].intParam == 1)
            {
               PairabilityMode = pmPairableMode;
               Mode            = "pmPairableMode";
            }
            else
            {
               PairabilityMode = pmPairableMode_EnableSecureSimplePairing;
               Mode            = "pmPairableMode_EnableSecureSimplePairing";
            }
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            Display(("Pairability Mode Changed to %s.\r\n", Mode));

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == pmPairableMode_EnableSecureSimplePairing)
               DisplayIOCapabilities();

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error setting the Mode.                     */
            DisplayFunctionError("GAP_Set_Pairability_Mode", Result);

            /* Flag that an error occurred while submitting the command.*/
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetPairabilityMode [Mode (0 = Non Pairable, 1 = Pairable, 2 = Pairable (Secure Simple Pairing)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for changing the Secure     */
   /* Simple Pairing Parameters that are exchanged during the Pairing   */
   /* procedure when Secure Simple Pairing (Security Level 4) is used.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int ChangeSimplePairingParameters(ParameterList_t *TempParam)
{
   int ret_val = 0;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 2) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 3))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            IOCapability = icDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               IOCapability = icDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  IOCapability = icKeyboardOnly;
               else
                  IOCapability = icNoInputNoOutput;
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection valid.  */
         MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capabilities.                */
         DisplayIOCapabilities();

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("ChangeSimplePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output)] [MITM Requirement (0 = No, 1 = Yes)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for initiating bonding with */
   /* a remote device.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int Pair(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   GAP_Bonding_Type_t BondingType;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that we are not already connected.             */
      if(!Connection)
      {
         /* There are currently no active connections, make sure that   */
         /* all of the parameters required for this function appear to  */
         /* be at least semi-valid.                                     */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
         {
            /* Check to see if General Bonding was specified.           */
            if(TempParam->NumberofParameters > 1)
               BondingType = TempParam->Params[1].intParam?btGeneral:btDedicated;
            else
               BondingType = btDedicated;

            /* Before we submit the command to the stack, we need to    */
            /* make sure that we clear out any Link Key we have stored  */
            /* for the specified device.                                */
            DeleteLinkKey(InquiryResultList[(TempParam->Params[0].intParam - 1)]);

            /* Attempt to submit the command.                           */
            Result = GAP_Initiate_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], BondingType, GAP_Event_Callback, (unsigned long)0);

            /* Check the return value of the submitted command for      */
            /* success.                                                 */
            if(!Result)
            {
               /* Display a message indicating that Bonding was         */
               /* initiated successfully.                               */
               Display(("GAP_Initiate_Bonding(%s): Success.\r\n", (BondingType == btDedicated)?"Dedicated":"General"));

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Display a message indicating that an error occurred   */
               /* while initiating bonding.                             */
               DisplayFunctionError("GAP_Initiate_Bonding", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("Pair [Inquiry Index] [0 = Dedicated, 1 = General (optional)]");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* Display an error to the user describing that Pairing can    */
         /* only occur when we are not connected.                       */
         Display(("Only valid when not connected.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for ending a previously     */
   /* initiated bonding session with a remote device.  This function    */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int EndPairing(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_End_Bonding(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the End bonding was    */
            /* successfully submitted.                                  */
            DisplayFunctionSuccess("GAP_End_Bonding");

            /* Flag success to the caller.                              */
            ret_val = 0;

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* ending bonding.                                          */
            DisplayFunctionError("GAP_End_Bonding", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("EndPairing [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a PIN Code value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PINCodeResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   PIN_Code_t                       PINCode;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam) && (BTPS_StringLength(TempParam->Params[0].strParam) > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= sizeof(PIN_Code_t)))
         {
            /* Parameters appear to be valid, go ahead and convert the  */
            /* input parameter into a PIN Code.                         */

            /* Initialize the PIN code.                                 */
            ASSIGN_PIN_CODE(PINCode, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

            BTPS_MemCopy(&PINCode, TempParam->Params[0].strParam, BTPS_StringLength(TempParam->Params[0].strParam));

            /* Populate the response structure.                         */
            GAP_Authentication_Information.GAP_Authentication_Type      = atPINCode;
            GAP_Authentication_Information.Authentication_Data_Length   = (Byte_t)(BTPS_StringLength(TempParam->Params[0].strParam));
            GAP_Authentication_Information.Authentication_Data.PIN_Code = PINCode;

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               Display(("GAP_Authentication_Response(), Pin Code Response Success.\r\n"));

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               Display(("GAP_Authentication_Response() Failure: %d.\r\n", Result));

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("PINCodeResponse [PIN Code]");

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("PIN Code Authentication Response: Authentication not in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int PassKeyResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= GAP_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type     = atPassKey;
            GAP_Authentication_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
            GAP_Authentication_Information.Authentication_Data.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               DisplayFunctionSuccess("Passkey Response Success");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_Authentication_Response", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            Display(("PassKeyResponse[Numeric Passkey(0 - 999999)].\r\n"));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("Pass Key Authentication Response: Authentication not in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a GAP           */
   /* Authentication Response with a User Confirmation value specified  */
   /* via the input parameter.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int UserConfirmationResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   GAP_Authentication_Information_t GAP_Authentication_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
            GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)(sizeof(Byte_t));
            GAP_Authentication_Information.Authentication_Data.Confirmation = (Boolean_t)(TempParam->Params[0].intParam?TRUE:FALSE);

            /* Submit the Authentication Response.                      */
            Result = GAP_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_Authentication_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               DisplayFunctionSuccess("User Confirmation Response");

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_Authentication_Response", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            DisplayUsage("UserConfirmationResponse [Confirmation(0 = No, 1 = Yes)]");
            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("User Confirmation Authentication Response: Authentication is not currently in progress.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetLocalAddress(ParameterList_t *TempParam)
{
   int       Result;
   int       ret_val;
   BD_ADDR_t BD_ADDR;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         BD_ADDRToStr(BD_ADDR, Function_BoardStr);

         Display(("BD_ADDR of Local Device is: %s.\r\n", Function_BoardStr));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Device Address.               */
         Display(("GAP_Query_Local_BD_ADDR() Failure: %d.\r\n", Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the name of the */
   /* local Bluetooth Device to a specified name.  This function returns*/
   /* zero on successful execution and a negative value on all errors.  */
static int SetLocalName(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].strParam))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Set_Local_Device_Name(BluetoothStackID, TempParam->Params[0].strParam);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the Device Name was    */
            /* successfully submitted.                                  */
            Display(("Local Device Name: %s.\r\n", TempParam->Params[0].strParam));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Device Name.                 */
            DisplayFunctionError("GAP_Set_Local_Device_Name", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetLocalName [Local Name]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the local Bluetooth Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int GetLocalName(ParameterList_t *TempParam)
{
   int   Result;
   int   ret_val;
   char *LocalName;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Allocate a Buffer to hold the Local Name.                      */
      if((LocalName = BTPS_AllocateMemory(257)) != NULL)
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Local_Device_Name(BluetoothStackID, 257, (char *)LocalName);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            Display(("Name of Local Device is: %s.\r\n", LocalName));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to query the Local Device Name.               */
            Display(("GAP_Query_Local_Device_Name() Failure: %d.\r\n", Result));

            ret_val = FUNCTION_ERROR;
         }

         BTPS_FreeMemory(LocalName);
      }
      else
      {
         Display(("Failed to allocate buffer to hold Local Name.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Class of    */
   /* Device of the local Bluetooth Device to a Class Of Device value.  */
   /* This function returns zero on successful execution and a negative */
   /* value on all errors.                                              */
static int SetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to submit the command.                              */
         ASSIGN_CLASS_OF_DEVICE(Class_of_Device, (Byte_t)((TempParam->Params[0].intParam) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 8) & 0xFF), (Byte_t)(((TempParam->Params[0].intParam) >> 16) & 0xFF));

         Result = GAP_Set_Class_Of_Device(BluetoothStackID, Class_of_Device);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that the Class of Device was*/
            /* successfully submitted.                                  */
            DisplayClassOfDevice(Class_of_Device);

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* attempting to set the local Class of Device.             */
            DisplayFunctionError("GAP_Set_Class_Of_Device", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetClassOfDevice [Class of Device]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Class of Device of the local Bluetooth Device.  This function     */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetClassOfDevice(ParameterList_t *TempParam)
{
   int               Result;
   int               ret_val;
   Class_of_Device_t Class_of_Device;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Class_Of_Device(BluetoothStackID, &Class_of_Device);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         Display(("Local Class of Device is: 0x%02X%02X%02X.\r\n", Class_of_Device.Class_of_Device0, Class_of_Device.Class_of_Device1, Class_of_Device.Class_of_Device2));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occurred while   */
         /* attempting to query the Local Class of Device.              */
         Display(("GAP_Query_Class_Of_Device() Failure: %d.\r\n", Result));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the Bluetooth  */
   /* Device Name of the specified remote Bluetooth Device.  This       */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int GetRemoteName(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to submit the command.                              */
         Result = GAP_Query_Remote_Device_Name(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], GAP_Event_Callback, (unsigned long)0);

         /* Check the return value of the submitted command for success.*/
         if(!Result)
         {
            /* Display a message indicating that Remote Name request was*/
            /* initiated successfully.                                  */
            DisplayFunctionSuccess("GAP_Query_Remote_Device_Name");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Display a message indicating that an error occurred while*/
            /* initiating the Remote Name request.                      */
            DisplayFunctionError("GAP_Query_Remote_Device_Name", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("GetRemoteName [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for issuing a Service Search*/
   /* Attribute Request to a Remote SDP Server.  This function returns  */
   /* zero if successful and a negative value if an error occurred.     */
static int ServiceDiscovery(ParameterList_t *TempParam)
{
   int                           Result;
   int                           ret_val;
   int                           Index;
   SDP_UUID_Entry_t              SDPUUIDEntry;
   SDP_Attribute_ID_List_Entry_t AttributeID;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Now let's make sure that all of the parameters required for    */
      /* this function appear to be at least semi-valid.                */
      if((TempParam) && (TempParam->NumberofParameters > 1) && (((TempParam->Params[1].intParam) && (TempParam->Params[1].intParam <= NUM_UUIDS)) || ((!TempParam->Params[1].intParam) && (TempParam->NumberofParameters > 2))) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* OK, parameters appear to be semi-valid, now let's attempt to*/
         /* issue the SDP Service Attribute Request.                    */
         if(!TempParam->Params[1].intParam)
         {
            /* First let's build the UUID 32 value(s).                  */
            SDPUUIDEntry.SDP_Data_Element_Type = deUUID_32;

            ASSIGN_SDP_UUID_32(SDPUUIDEntry.UUID_Value.UUID_32, ((TempParam->Params[2].intParam & 0xFF000000) >> 24), ((TempParam->Params[2].intParam & 0x00FF0000) >> 16), ((TempParam->Params[2].intParam & 0x0000FF00) >> 8), (TempParam->Params[2].intParam & 0x000000FF));
         }
         else
         {
            SDPUUIDEntry.SDP_Data_Element_Type = deUUID_128;

            SDPUUIDEntry.UUID_Value.UUID_128   = UUIDTable[TempParam->Params[1].intParam - 1].UUID;
         }

         AttributeID.Attribute_Range    = (Boolean_t)TRUE;
         AttributeID.Start_Attribute_ID = 0;
         AttributeID.End_Attribute_ID   = 65335;

         /* Finally submit the SDP Request.                             */
         Result = SDP_Service_Search_Attribute_Request(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], 1, &SDPUUIDEntry, 1, &AttributeID, SDP_Event_Callback, (unsigned long)0);

         if(Result > 0)
         {
            /* The SDP Request was submitted successfully.              */
            Display(("SDP_Service_Search_Attribute_Request(%s) Success.\r\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* There was an error submitting the SDP Request.           */
            Display(("SDP_Service_Search_Attribute_Request(%s) Failure: %d.\r\n", TempParam->Params[1].intParam?UUIDTable[TempParam->Params[1].intParam - 1].Name:"Manual", Result));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         Display(("Usage: ServiceDiscovery [Inquiry Index] [Profile Index] [16/32 bit UUID (Manual only)].\r\n"));
         Display(("\r\n   Profile Index:\r\n"));
         Display(("       0) Manual (MUST specify 16/32 bit UUID)\r\n"));
         for(Index=0;Index<NUM_UUIDS;Index++)
            Display(("      %2d) %s\r\n", Index + 1, UUIDTable[Index].Name));
         Display(("\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for responding to a request */
   /* to connect from a remote Bluetooth Device.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int OpenRequestResponse(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going connection request */
      /* operation active.                                              */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
         {
            /* Parameters appear to be valid.                           */

            /* Attempt to submit the response.                          */
            Result = AUD_Open_Request_Response(BluetoothStackID, CurrentRemoteBD_ADDR, TempParam->Params[0].intParam?acrRemoteControl:acrStream, (Boolean_t)TempParam->Params[1].intParam);

            if(!Result)
            {
               /* Function was successful, inform the user.             */
               Display(("AUD_Open_Request_Response(%s): Successful.\r\n", TempParam->Params[1].intParam?"TRUE":"FALSE"));

               /* Flag that there is no longer a current connection     */
               /* procedure in progress.                                */
               ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Function failed, inform the user.                     */
               DisplayFunctionError("AUD_Open_Request_Response()", Result);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            /* One of more of the parameters is/are invalid.            */
            DisplayUsage("OpenRequestResponse [Connection type: Stram/Remote_Control (0/1)] [Reject/Accept (0/1)]");

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("Unable to issue Open Request Response: Connection Request is not currently in progress.\r\n"));


         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for opening a stream        */
   /* endpoint on a remote Bluetooth Device. This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int OpenRemoteStream(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)], NullADDR)))
      {
         /* Attempt to open the remote stream.                          */
         Result = AUD_Open_Remote_Stream(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)], astSNK);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Open_Remote_Stream(SNK): Successful.\r\n"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Open_Remote_Stream()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("OpenRemoteStream [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for closing an opened       */
   /* stream.  This function returns zero on successful execution and a */
   /* negative value on all errors.                                     */
static int CloseStream(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam))
      {
         /* Attempt to close the stream.                                */
         Result = AUD_Close_Stream(BluetoothStackID, ConnectedBD_ADDR, astSNK);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Close_Stream(SNK): Successful.\r\n"));

            ASSIGN_BD_ADDR(ConnectedBD_ADDR, 0, 0, 0, 0, 0, 0);

            /* Flag success to the caller.                              */
            ret_val    = 0;

            Connection = FALSE;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Close_Stream()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         Display(("Usage: CloseStream.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for opening a remote control*/
   /* connection to a remote Bluetooth Device.  This function returns   */
   /* zero on successful execution and a negative value on all errors.  */
static int OpenRemoteControl(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_NULL_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)])))
      {
         /* Attempt to open the remote control connection.              */
         Result = AUD_Open_Remote_Control(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            DisplayFunctionSuccess("AUD_Open_Remote_Control()");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Open_Remote_Control()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("OpenRemoteControl [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for closing an opened remote*/
   /* control connection.  This function returns zero on successful     */
   /* execution and a negative value on all errors.                     */
static int CloseRemoteControl(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && (NumberofValidResponses) && (TempParam->Params[0].intParam <= NumberofValidResponses) && (!COMPARE_NULL_BD_ADDR(InquiryResultList[(TempParam->Params[0].intParam - 1)])))
      {
         /* Attempt to close the remote control connection.             */
         Result = AUD_Close_Remote_Control(BluetoothStackID, InquiryResultList[(TempParam->Params[0].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            DisplayFunctionSuccess("AUD_Close_Remote_Control()");

            ASSIGN_BD_ADDR(ConnectedBD_ADDR, 0, 0, 0, 0, 0, 0);

            /* Flag success to the caller.                              */
            ret_val                 = 0;

            RemoteControlConnection = FALSE;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Close_Remote_Control", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("CloseRemoteControl [Inquiry Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing the state of an*/
   /* open stream.  This function returns zero on successful execution  */
   /* and a negative value on all errors.                               */
static int ChangeStreamState(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) &&
	  	  (TempParam->Params[0].intParam <= 1))
      {
         /* Attempt to change the stream state.                         */
         Result = AUD_Change_Stream_State(BluetoothStackID, ConnectedBD_ADDR, astSNK, (TempParam->Params[0].intParam)?astStreamStarted:astStreamStopped);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Change_Stream_State(%s): Successful.\r\n", TempParam->Params[0].intParam?"Started":"Stopped"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Change_Stream_State()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("ChangeStreamState [Stream State (0 = Stopped, 1 = Started)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying an open stream's current state.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int QueryStreamState(ParameterList_t *TempParam)
{
   int                Result;
   int                ret_val;
   AUD_Stream_State_t StreamState;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream state.                          */
         Result = AUD_Query_Stream_State(BluetoothStackID, ConnectedBD_ADDR, astSNK, &StreamState);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            DisplayFunctionSuccess("AUD_Query_Stream_State()");
            Display(("    State: %s.\r\n", (StreamState == astStreamStopped)?"Stopped":"Started"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Query_Stream_State()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         Display(("Usage: QueryStreamState.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing the format of a*/
   /* currently open stream.  The Supported Formats Index should be     */
   /* determined by the displayed list from a call to                   */
   /* QuerySupportedFormats.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int ChangeStreamFormat(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters == 1) && (TempParam->Params[0].intParam) && (TempParam->Params[0].intParam <= NUM_SNK_SUPPORTED_FORMATS))
      {
         /* Attempt to change the stream format.                        */
         Result = AUD_Change_Stream_Format(BluetoothStackID, ConnectedBD_ADDR, astSNK, &AudioSNKSupportedFormats[(TempParam->Params[0].intParam - 1)]);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Change_Stream_Format(%s): Successful.\r\n", "SNK"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Change_Stream_Format()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("ChangeStreamFormat [Supported Formats Index]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying the format of a currently opened stream.  This function*/
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int QueryStreamFormat(ParameterList_t *TempParam)
{
   int                 Result;
   int                 ret_val;
   AUD_Stream_Format_t StreamFormat;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream format.                         */
         Result = AUD_Query_Stream_Format(BluetoothStackID, ConnectedBD_ADDR, astSNK, &StreamFormat);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Query_Stream_Format(%s): Successful.\r\n", "SNK"));
            Display(("    Format: %u, %u.\r\n", (unsigned int)StreamFormat.SampleFrequency, (unsigned int)StreamFormat.NumberChannels));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Query_Stream_Format()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         Display(("Usage: QueryStreamFormat.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for displaying the local    */
   /* supported formats to the user.  The indices displayed should be   */
   /* used with the ChangeStreamFormat function.  This function returns */
   /* zero on successful execution and a negative value on all errors.  */
static int QuerySupportedFormats(ParameterList_t *TempParam)
{
   int Index;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      Display(("Supported Sink Formats:\r\n"));
      for(Index=0;Index<NUM_SNK_SUPPORTED_FORMATS;Index++)
         Display(("%d: {%u, %u}\r\n", Index+1, (unsigned int)AudioSNKSupportedFormats[Index].SampleFrequency, (unsigned int)AudioSNKSupportedFormats[Index].NumberChannels));

      /* Flag success to the caller.                                    */
      ret_val = 0;
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   /* Finally return the result to the caller.                          */
   return(ret_val);
}

   /* The following function is responsible for determining and         */
   /* displaying the configuration of a currently opened stream.  This  */
   /* function returns zero on successful execution and a negative value*/
   /* on all errors.                                                    */
static int QueryStreamConfiguration(ParameterList_t *TempParam)
{
   int                        Result;
   int                        ret_val;
   int                        Index;
   AUD_Stream_Configuration_t StreamConfiguration;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to query the stream configuration.                  */
         Result = AUD_Query_Stream_Configuration(BluetoothStackID, ConnectedBD_ADDR, astSNK, &StreamConfiguration);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            Display(("AUD_Query_Stream_Configuration(%s): Successful.\r\n", "SNK"));
            Display(("    Format:              %u, %u.\r\n", (unsigned int)StreamConfiguration.StreamFormat.SampleFrequency, (unsigned int)StreamConfiguration.StreamFormat.NumberChannels));
            Display(("    MTU:                 %d.\r\n", StreamConfiguration.MediaMTU));
            Display(("    Codec Type (Length): %d (%d).\r\n", StreamConfiguration.MediaCodecType, StreamConfiguration.MediaCodecInfoLength));
            Display(("    Codec Info:          0x"));

            for(Index=0;Index<StreamConfiguration.MediaCodecInfoLength;Index++)
               Display(("%02X", (unsigned int)(StreamConfiguration.MediaCodecInformation[Index] & 0xFF)));

            Display((".\r\n"));

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Query_Stream_Configuration()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         Display(("Usage: QueryStreamConfig.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for changing how incoming   */
   /* connections are handled.  This function returns zero on successful*/
   /* execution and a negative value on all errors.                     */
static int ChangeConnectionMode(ParameterList_t *TempParam)
{
   int Result;
   int ret_val;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters > 0))
      {
         /* Attempt to set the connection mode.                         */
         Result = AUD_Set_Server_Connection_Mode(BluetoothStackID, (!(TempParam->Params[0].intParam))?ausAutomaticAccept:(TempParam->Params[0].intParam == 1)?ausAutomaticReject:ausManualAccept);

         if(!Result)
         {
            /* Function was successful, inform the user.                */
            DisplayFunctionSuccess("AUD_Set_Server_Connection_Mode()");

            /* Flag success to the caller.                              */
            ret_val = 0;
         }
         else
         {
            /* Function failed, inform the user.                        */
            DisplayFunctionError("AUD_Set_Server_Connection_Mode()", Result);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One of more of the parameters is/are invalid.               */
         DisplayUsage("ChangeConnectionMode [Connection Mode (0=Automatic Accept, 1=Automatic Reject, 2=Manual Accept)]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for determing and displaying*/
   /* how incoming connections are handled.  This function returns zero */
   /* on successful execution and a negative value on all errors.       */
static int QueryConnectionMode(ParameterList_t *TempParam)
{
   int                          Result;
   int                          ret_val;
   AUD_Server_Connection_Mode_t ServerConnectionMode;

   /* First, check that a valid Bluetooth Stack ID exists.              */
   if(BluetoothStackID)
   {
      /* Attempt to get the connection mode.                            */
      Result = AUD_Get_Server_Connection_Mode(BluetoothStackID, &ServerConnectionMode);

      if(!Result)
      {
         /* Function was successful, inform the user.                   */
         DisplayFunctionSuccess("AUD_Get_Server_Connection_Mode()");
         Display(("    Connection Mode: %s.\r\n", 
          (ServerConnectionMode == ausAutomaticAccept)?"Automatic Accept":\
          (ServerConnectionMode == ausAutomaticReject)?"Automatic Reject":"Manual Accept"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Function failed, inform the user.                           */
         DisplayFunctionError("AUD_Get_Server_Connection_Mode()", Result);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
      ret_val = INVALID_STACK_ID_ERROR;

   return(ret_val);
}

   /* The following function is responsible for sending Remote Control  */
   /* Pass Through Commands.  This function returns zero on successful  */
   /* execution and a negative value on all errors.                     */
static int SendPassThroughCommand(ParameterList_t *TempParam)
{
   int                               ret_val;
   int                               Result;
   Byte_t                            OperationID;
   AUD_Remote_Control_Command_Data_t RemoteControlCommandData;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         /* Now check to make sure that a Remote Control session is     */
         /* connected.                                                  */
         if(RemoteControlConnection)
         {
            /* Now let's parse the data that was specified.             */
            Result            = 0;

            /* Determine which command was specified.                   */
            switch(TempParam->Params[0].intParam)
            {
               case 0:
                  /* Pause.                                             */
                  OperationID = AVRCP_PASS_THROUGH_ID_PAUSE;
                  Display(("Pause Pass-through Command specified.\r\n"));
                  break;
               case 1:
                  /* Play.                                              */
                  OperationID = AVRCP_PASS_THROUGH_ID_PLAY;
                  Display(("Play Pass-through Command specified.\r\n"));
                  break;
               case 2:
                  /* Stop.                                              */
                  OperationID = AVRCP_PASS_THROUGH_ID_STOP;
                  Display(("Stop Pass-through Command specified.\r\n"));
                  break;
               case 3:
                  /* Volume Up.                                         */
                  OperationID = AVRCP_PASS_THROUGH_ID_VOLUME_UP;
                  Display(("Volume Up Pass-through Command specified.\r\n"));
                  break;
               case 4:
                  /* Volume Down.                                       */
                  OperationID = AVRCP_PASS_THROUGH_ID_VOLUME_DOWN;
                  Display(("Volume Down Pass-through Command specified.\r\n"));
                  break;
               case 5:
                  /* Specify Command.                                   */
                  if((TempParam->NumberofParameters >= 2) && (TempParam->Params[1].intParam) && (TempParam->Params[1].intParam != 0xFF))
                  {
                     OperationID = TempParam->Params[1].intParam;
                     Display(("Specific Pass-through Command specified: 0x%02X.\r\n", OperationID));
                  }
                  else
                     Result = INVALID_PARAMETERS_ERROR;
                  break;
               default:
                  Result = INVALID_PARAMETERS_ERROR;
                  Display(("Unknown Pass-through Command Option specified: %d\r\n", TempParam->Params[0].intParam));
                  break;
            }

            if(!Result)
            {
               /* Format up a Pass-through Command. StateFlag is FALSE  */
               /* for "Button Down".                                    */
               RemoteControlCommandData.MessageType                                            = amtPassThrough;
               RemoteControlCommandData.MessageData.PassThroughCommandData.CommandType         = AVRCP_CTYPE_CONTROL;
               RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitType         = AVRCP_SUBUNIT_TYPE_PANEL;
               RemoteControlCommandData.MessageData.PassThroughCommandData.SubunitID           = AVRCP_SUBUNIT_ID_INSTANCE_0;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID         = OperationID;
               RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag           = FALSE;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationDataLength = 0;
               RemoteControlCommandData.MessageData.PassThroughCommandData.OperationData       = NULL;

               /* Check that the AVRCP command is supported (with timeout=0) */
               Result = AUD_Send_Remote_Control_Command(BluetoothStackID, ConnectedBD_ADDR, &RemoteControlCommandData, 0);
               if(Result > 0)
               {
                   /* Now we must send the button-up command to finish up (with the default timeout parameter) */
                  RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag = TRUE;
                    ret_val = AUD_Send_Remote_Control_Command(BluetoothStackID, ConnectedBD_ADDR, &RemoteControlCommandData,
                        DEFAULT_PASS_THROUGH_COMMAND_TIMEOUT);

                    if(ret_val > 0)
                  {
                        Display(("AVRCP command sent successfully.\r\n"));
                        ret_val = 0;
                  }
                  else
                  {
                        DisplayFunctionError("AUD_Send_Remote_Control_Command()", ret_val);
                        ret_val = FUNCTION_ERROR;
                    }
                  }
               else
               {
                    DisplayFunctionError("AUD_Send_Remote_Control_Command()", Result);
                    ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               DisplayUsage("SendPassThroughCommand [Command] \r\n Commands: 0 = Pause, 1 = Play, 2 = Stop, 3 = Vol. Up, 4 = Vol. Down,\r\n 5 = Specify Command with value parameter (the second parameter)\r\n");

               ret_val = Result;
            }
         }
         else
         {
            /* Profile is either not Registered or not connected.       */
            Display(("No Profile Registered or Profile not connected.\r\n"));
            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SendPassThroughCommand [Command] \r\n Commands: 0 = Pause, 1 = Play, 2 = Stop, 3 = Vol. Up, 4 = Vol. Down,\r\n 5 = Specify Command with value parameter (the second parameter)\r\n");
         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      Display(("Stack ID Invalid.\r\n"));
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

static int SetVolume(ParameterList_t *TempParam)
{
   int                                ret_val;
   int                                Result;
   AUD_Remote_Control_Response_Data_t RemoteControlResponseData;

   /* First check to see if a valid Bluetooth Stack ID exists.          */
   if(BluetoothStackID)
   {
      /* Now, check that the input parameters appear to be at least     */
      /* semi-valid.                                                    */
      if((TempParam) && (TempParam->NumberofParameters >= 1))
      {
         if(RemoteControlConnection)
         {
            if(AbsoluteVolumeEnabled)
            {
               CurrentVolume = TempParam->Params[0].intParam;

               /* As a sink, a volume changed notification will be   */
               /* sent if it has be registered.                      */
               if(VolumeChangedEventTransactionID >= 0)
               {
                  RemoteControlResponseData.MessageType                                                                                    = amtRegisterNotification;
                  RemoteControlResponseData.MessageData.RegisterNotificationResponseData.EventID                                           = AVRCP_EVENT_VOLUME_CHANGED;
                  RemoteControlResponseData.MessageData.RegisterNotificationResponseData.ResponseCode                                      = AVRCP_RESPONSE_CHANGED;
                  RemoteControlResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume = CurrentVolume;
                  if((Result = AUD_Send_Remote_Control_Response(BluetoothStackID, ConnectedBD_ADDR, VolumeChangedEventTransactionID, &RemoteControlResponseData)) == 0)
                  {
                     DisplayFunctionSuccess("AUD_Send_Remote_Control_Response");
                     ret_val = 0;

                     /* Volume changed event is no longer registered */
                     /* so invalidate the transaction ID.            */
                     VolumeChangedEventTransactionID = -1;
                  }
                  else
                  {
                     DisplayFunctionError("AUD_Send_Remote_Control_Response", Result);
                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
               {
                  Display(("Volume changed event has not been registered.\r\n"));
                  ret_val = FUNCTION_ERROR;
               }
            }
            else
            {
               Display(("Absolute volume not supported\r\n"));

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            Display(("Remote control not connected\r\n"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SetVolume [Volume]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for querying the memory heap usage.              */  
   /* This function will return zero on successful execution and a negative value on errors. */
static int QueryMemory(ParameterList_t *TempParam)
{
   BTPS_MemoryStatistics_t MemoryStatistics;
   int ret_val;

   /* Get current memory buffer usage                                   */
   ret_val = BTPS_QueryMemoryUsage(&MemoryStatistics, TRUE);
   if(!ret_val)
   {
      Display(("\r\n"));
      Display(("Heap Size:                %5d bytes\r\n", MemoryStatistics.HeapSize));
      Display(("Current Memory Usage:\r\n"));
      Display(("   Used:                  %5d bytes\r\n", MemoryStatistics.CurrentHeapUsed));
      Display(("   Free:                  %5d bytes\r\n", MemoryStatistics.HeapSize - MemoryStatistics.CurrentHeapUsed));
      Display(("Maximum Memory Usage:\r\n"));
      Display(("   Used:                  %5d bytes\r\n", MemoryStatistics.MaximumHeapUsed));
      Display(("   Free:                  %5d bytes\r\n", MemoryStatistics.HeapSize - MemoryStatistics.MaximumHeapUsed));
      Display(("Framentation:\r\n"));
      Display(("   Largest Free Fragment: %5d bytes\r\n", MemoryStatistics.LargestFreeFragment));
      Display(("   Free Fragment Cound:   %5d\r\n",       MemoryStatistics.FreeFragmentCount));
   }
   else
   {
      Display(("Failed to get memory usage\r\n"));
   }

   return(ret_val);
}

   /* The following function is responsible for Displaying the contents */
   /* of an SDP Service Attribute Response to the display.              */
static void DisplaySDPAttributeResponse(SDP_Service_Attribute_Response_Data_t *SDPServiceAttributeResponse, unsigned int InitLevel)
{
   int Index;

   /* First, check to make sure that there were Attributes returned.    */
   if(SDPServiceAttributeResponse->Number_Attribute_Values)
   {
      /* Loop through all returned SDP Attribute Values.                */
      for(Index = 0; Index < SDPServiceAttributeResponse->Number_Attribute_Values; Index++)
      {
         /* First Print the Attribute ID that was returned.             */
         Display(("%*s Attribute ID 0x%04X\r\n", (InitLevel*INDENT_LENGTH), "", SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].Attribute_ID));

         /* Now Print out all of the SDP Data Elements that were        */
         /* returned that are associated with the SDP Attribute.        */
         DisplayDataElement(SDPServiceAttributeResponse->SDP_Service_Attribute_Value_Data[Index].SDP_Data_Element, (InitLevel + 1));
      }
   }
   else
      Display(("No SDP Attributes Found.\r\n"));
}

   /* The following function is responsible for displaying the contents */
   /* of an SDP Service Search Attribute Response to the display.       */
static void DisplaySDPSearchAttributeResponse(SDP_Service_Search_Attribute_Response_Data_t *SDPServiceSearchAttributeResponse)
{
   int Index;

   /* First, check to see if Service Records were returned.             */
   if(SDPServiceSearchAttributeResponse->Number_Service_Records)
   {
      /* Loop through all returned SDP Service Records.                 */
      for(Index = 0; Index < SDPServiceSearchAttributeResponse->Number_Service_Records; Index++)
      {
         /* First display the number of SDP Service Records we are      */
         /* currently processing.                                       */
         Display(("Service Record: %u:\r\n", (Index + 1)));

         /* Call Display SDPAttributeResponse for all SDP Service       */
         /* Records received.                                           */
         DisplaySDPAttributeResponse(&(SDPServiceSearchAttributeResponse->SDP_Service_Attribute_Response_Data[Index]), 1);
      }
   }
   else
      Display(("No SDP Service Records Found.\r\n"));
}

   /* The following function is responsible for actually displaying an  */
   /* individual SDP Data Element to the Display.  The Level Parameter  */
   /* is used in conjunction with the defined INDENT_LENGTH constant to */
   /* make readability easier when displaying Data Element Sequences    */
   /* and Data Element Alternatives.  This function will recursively    */
   /* call itself to display the contents of Data Element Sequences and */
   /* Data Element Alternatives when it finds these Data Types (and     */
   /* increments the Indent Level accordingly).                         */
static void DisplayDataElement(SDP_Data_Element_t *SDPDataElement, unsigned int Level)
{
   unsigned int Index;
   char         Buffer[256];

   switch(SDPDataElement->SDP_Data_Element_Type)
   {
      case deNIL:
         /* Display the NIL Type.                                       */
         Display(("%*s Type: NIL\r\n", (Level*INDENT_LENGTH), ""));
         break;
      case deNULL:
         /* Display the NULL Type.                                      */
         Display(("%*s Type: NULL\r\n", (Level*INDENT_LENGTH), ""));
         break;
      case deUnsignedInteger1Byte:
         /* Display the Unsigned Integer (1 Byte) Type.                 */
         Display(("%*s Type: Unsigned Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger1Byte));
         break;
      case deUnsignedInteger2Bytes:
         /* Display the Unsigned Integer (2 Bytes) Type.                */
         Display(("%*s Type: Unsigned Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger2Bytes));
         break;
      case deUnsignedInteger4Bytes:
         /* Display the Unsigned Integer (4 Bytes) Type.                */
         Display(("%*s Type: Unsigned Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.UnsignedInteger4Bytes));
         break;
      case deUnsignedInteger8Bytes:
         /* Display the Unsigned Integer (8 Bytes) Type.                */
         Display(("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[7],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[6],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[5],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[4],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[3],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[2],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[1],
                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger8Bytes[0]));
         break;
      case deUnsignedInteger16Bytes:
         /* Display the Unsigned Integer (16 Bytes) Type.               */
         Display(("%*s Type: Unsigned Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[15],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[14],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[13],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[12],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[11],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[10],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[9],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[8],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[7],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[6],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[5],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[4],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[3],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[2],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[1],
                                                                                                                   SDPDataElement->SDP_Data_Element.UnsignedInteger16Bytes[0]));
         break;
      case deSignedInteger1Byte:
         /* Display the Signed Integer (1 Byte) Type.                   */
         Display(("%*s Type: Signed Int = 0x%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger1Byte));
         break;
      case deSignedInteger2Bytes:
         /* Display the Signed Integer (2 Bytes) Type.                  */
         Display(("%*s Type: Signed Int = 0x%04X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger2Bytes));
         break;
      case deSignedInteger4Bytes:
         /* Display the Signed Integer (4 Bytes) Type.                  */
         Display(("%*s Type: Signed Int = 0x%08X\r\n", (Level*INDENT_LENGTH), "", (unsigned int)SDPDataElement->SDP_Data_Element.SignedInteger4Bytes));
         break;
      case deSignedInteger8Bytes:
         /* Display the Signed Integer (8 Bytes) Type.                  */
         Display(("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[7],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[6],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[5],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[4],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[3],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[2],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[1],
                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger8Bytes[0]));
         break;
      case deSignedInteger16Bytes:
         /* Display the Signed Integer (16 Bytes) Type.                 */
         Display(("%*s Type: Signed Int = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[15],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[14],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[13],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[12],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[11],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[10],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[9],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[8],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[7],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[6],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[5],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[4],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[3],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[2],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[1],
                                                                                                                 SDPDataElement->SDP_Data_Element.SignedInteger16Bytes[0]));
         break;
      case deTextString:
         /* First retrieve the Length of the Text String so that we can */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the Text String into the Buffer and then NULL terminate*/
         /* it.                                                         */
         BTPS_MemCopy(Buffer, SDPDataElement->SDP_Data_Element.TextString, Index);
         Buffer[Index] = '\0';

         Display(("%*s Type: Text String = %s\r\n", (Level*INDENT_LENGTH), "", Buffer));
         break;
      case deBoolean:
         Display(("%*s Type: Boolean = %s\r\n", (Level*INDENT_LENGTH), "", (SDPDataElement->SDP_Data_Element.Boolean)?"TRUE":"FALSE"));
         break;
      case deURL:
         /* First retrieve the Length of the URL String so that we can  */
         /* copy the Data into our Buffer.                              */
         Index = (SDPDataElement->SDP_Data_Element_Length < sizeof(Buffer))?SDPDataElement->SDP_Data_Element_Length:(sizeof(Buffer)-1);

         /* Copy the URL String into the Buffer and then NULL terminate */
         /* it.                                                         */
         BTPS_MemCopy(Buffer, SDPDataElement->SDP_Data_Element.URL, Index);
         Buffer[Index] = '\0';

         Display(("%*s Type: URL = %s\r\n", (Level*INDENT_LENGTH), "", Buffer));
         break;
      case deUUID_16:
         Display(("%*s Type: UUID_16 = 0x%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte0,
                                                                                 SDPDataElement->SDP_Data_Element.UUID_16.UUID_Byte1));
         break;
      case deUUID_32:
         Display(("%*s Type: UUID_32 = 0x%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte0,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte1,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte2,
                                                                                         SDPDataElement->SDP_Data_Element.UUID_32.UUID_Byte3));
         break;
      case deUUID_128:
         Display(("%*s Type: UUID_128 = 0x%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X\r\n", (Level*INDENT_LENGTH), "", SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte0,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte1,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte2,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte3,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte4,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte5,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte6,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte7,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte8,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte9,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte10,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte11,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte12,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte13,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte14,
                                                                                                               SDPDataElement->SDP_Data_Element.UUID_128.UUID_Byte15));
         break;
      case deSequence:
         /* Display that this is a SDP Data Element Sequence.           */
         Display(("%*s Type: Data Element Sequence\r\n", (Level*INDENT_LENGTH), ""));

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Sequence.                                           */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Sequence.              */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Sequence[Index]), (Level + 1));
         }
         break;
      case deAlternative:
         /* Display that this is a SDP Data Element Alternative.        */
         Display(("%*s Type: Data Element Alternative\r\n", (Level*INDENT_LENGTH), ""));

         /* Loop through each of the SDP Data Elements in the SDP Data  */
         /* Element Alternative.                                        */
         for(Index = 0; Index < SDPDataElement->SDP_Data_Element_Length; Index++)
         {
            /* Call this function again for each of the SDP Data        */
            /* Elements in this SDP Data Element Alternative.           */
            DisplayDataElement(&(SDPDataElement->SDP_Data_Element.SDP_Data_Element_Alternative[Index]), (Level + 1));
         }
         break;
      default:
         Display(("%*s Unknown SDP Data Element Type\r\n", (Level*INDENT_LENGTH), ""));
         break;
   }
}

   /*********************************************************************/
   /*                         Event Callbacks                           */
   /*********************************************************************/

   /* The following function is for the GAP Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified GAP Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the GAP  */
   /* Event Data of the specified Event and the GAP Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the GAP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other GAP Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other GAP Events.  A  */
   /*          Deadlock WILL occur because NO GAP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI GAP_Event_Callback(unsigned int BluetoothStackID, GAP_Event_Data_t *GAP_Event_Data, unsigned long CallbackParameter)
{
   int                               Result;
   int                               Index;
   BD_ADDR_t                         NULL_BD_ADDR;
   Boolean_t                         OOB_Data;
   Boolean_t                         MITM;
   GAP_IO_Capability_t               RemoteIOCapability;
   GAP_Inquiry_Event_Data_t         *GAP_Inquiry_Event_Data;
   GAP_Remote_Name_Event_Data_t     *GAP_Remote_Name_Event_Data;
   GAP_Authentication_Information_t  GAP_Authentication_Information;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (GAP_Event_Data))
   {
      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming event is.                                    */
      switch(GAP_Event_Data->Event_Data_Type)
      {
         case etInquiry_Result:
            /* The GAP event received was of type Inquiry_Result.       */
            GAP_Inquiry_Event_Data = GAP_Event_Data->Event_Data.GAP_Inquiry_Event_Data;

            /* Next, Check to see if the inquiry event data received    */
            /* appears to be semi-valid.                                */
            if(GAP_Inquiry_Event_Data)
            {
               /* Now, check to see if the gap inquiry event data's     */
               /* inquiry data appears to be semi-valid.                */
               if(GAP_Inquiry_Event_Data->GAP_Inquiry_Data)
               {
                  Display(("\r\n"));

                  /* Display a list of all the devices found from       */
                  /* performing the inquiry.                            */
                  for(Index=0;(Index<GAP_Inquiry_Event_Data->Number_Devices) && (Index<MAX_INQUIRY_RESULTS);Index++)
                  {
                     InquiryResultList[Index] = GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR;
                     BD_ADDRToStr(GAP_Inquiry_Event_Data->GAP_Inquiry_Data[Index].BD_ADDR, Callback_BoardStr);

                     Display(("Result: %d,%s.\r\n", (Index+1), Callback_BoardStr));
                  }

                  NumberofValidResponses = GAP_Inquiry_Event_Data->Number_Devices;
               }
            }
            break;
         case etInquiry_Entry_Result:
            /* Next convert the BD_ADDR to a string.                    */
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Inquiry_Entry_Event_Data->BD_ADDR, Callback_BoardStr);

            /* Display this GAP Inquiry Entry Result.                   */
            Display(("\r\n"));
            Display(("Inquiry Entry: %s.\r\n", Callback_BoardStr));
            break;
         case etAuthentication:
            /* An authentication event occurred, determine which type of*/
            /* authentication event occurred.                           */
            switch(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->GAP_Authentication_Event_Type)
            {
               case atLinkKeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atLinkKeyRequest: %s\r\n", Callback_BoardStr));

                  /* Setup the authentication information response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atLinkKey;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  /* See if we have stored a Link Key for the specified */
                  /* device.                                            */
                  for(Index=0;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                     {
                        /* Link Key information stored, go ahead and    */
                        /* respond with the stored Link Key.            */
                        GAP_Authentication_Information.Authentication_Data_Length   = sizeof(Link_Key_t);
                        GAP_Authentication_Information.Authentication_Data.Link_Key = LinkKeyInfo[Index].LinkKey;

                        break;
                     }
                  }

                  /* Submit the authentication response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  if(!Result)
                     DisplayFunctionSuccess("GAP_Authentication_Response");
                  else
                     DisplayFunctionError("GAP_Authentication_Response", Result);
                  break;
               case atPINCodeRequest:
                  /* A pin code request event occurred, first display   */
                  /* the BD_ADD of the remote device requesting the pin.*/
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atPINCodeRequest: %s\r\n", Callback_BoardStr));

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the PIN Code.                                      */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a PIN Code Response.                               */
                  Display(("Respond with: PINCodeResponse\r\n"));
                  break;
               case atAuthenticationStatus:
                  /* An authentication status event occurred, display   */
                  /* all relevant information.                          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atAuthenticationStatus: %d for %s\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Authentication_Status, Callback_BoardStr));

                  /* Flag that there is no longer a current             */
                  /* Authentication procedure in progress.              */
                  ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  break;
               case atLinkKeyCreation:
                  /* A link key creation event occurred, first display  */
                  /* the remote device that caused this event.          */
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atLinkKeyCreation: %s\r\n", Callback_BoardStr));

                  /* Now store the link Key in either a free location OR*/
                  /* over the old key location.                         */
                  ASSIGN_BD_ADDR(NULL_BD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                  for(Index=0,Result=-1;Index<(sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t));Index++)
                  {
                     if(COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device))
                        break;
                     else
                     {
                        if((Result == (-1)) && (COMPARE_BD_ADDR(LinkKeyInfo[Index].BD_ADDR, NULL_BD_ADDR)))
                           Result = Index;
                     }
                  }

                  /* If we didn't find a match, see if we found an empty*/
                  /* location.                                          */
                  if(Index == (sizeof(LinkKeyInfo)/sizeof(LinkKeyInfo_t)))
                     Index = Result;

                  /* Check to see if we found a location to store the   */
                  /* Link Key information into.                         */
                  if(Index != (-1))
                  {
                     LinkKeyInfo[Index].BD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;
                     LinkKeyInfo[Index].LinkKey = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key;

                     Display(("Link Key Stored.\r\n"));
                     /* The BD_ADDR of the remote device has been displayed before */
                     /* now display the link key being created.                    */
                     Display(("Link Key: 0x"));

                     for(Index = 0;Index<sizeof(Link_Key_t);Index++)
                     {
                        Display(("%02X", ((Byte_t *)(&(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Link_Key_Info.Link_Key)))[Index]));
                     }
                     Display(("\r\n"));
                  }
                  else
                     Display(("Link Key array full.\r\n"));
                  break;
               case atIOCapabilityRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atIOCapabilityRequest: %s\r\n", Callback_BoardStr));

                  /* Setup the Authentication Information Response      */
                  /* structure.                                         */
                  GAP_Authentication_Information.GAP_Authentication_Type                                      = atIOCapabilities;
                  GAP_Authentication_Information.Authentication_Data_Length                                   = sizeof(GAP_IO_Capabilities_t);
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.IO_Capability            = (GAP_IO_Capability_t)IOCapability;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.MITM_Protection_Required = MITMProtection;
                  GAP_Authentication_Information.Authentication_Data.IO_Capabilities.OOB_Data_Present         = OOBSupport;

                  /* Submit the Authentication Response.                */
                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  /* Check the result of the submitted command.         */
                  /* Check the result of the submitted command.         */
                  if(!Result)
                     DisplayFunctionSuccess("Auth");
                  else
                     DisplayFunctionError("Auth", Result);
                  break;
               case atIOCapabilityResponse:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atIOCapabilityResponse: %s\r\n", Callback_BoardStr));

                  RemoteIOCapability = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.IO_Capability;
                  MITM               = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.MITM_Protection_Required;
                  OOB_Data           = (Boolean_t)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.IO_Capabilities.OOB_Data_Present;

                  Display(("Capabilities: %s%s%s\r\n", IOCapabilitiesStrings[RemoteIOCapability], ((MITM)?", MITM":""), ((OOB_Data)?", OOB Data":"")));
                  break;
               case atUserConfirmationRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atUserConfirmationRequest: %s\r\n", Callback_BoardStr));

                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  if(IOCapability != icDisplayYesNo)
                  {
                     /* Invoke JUST Works Process...                    */
                     GAP_Authentication_Information.GAP_Authentication_Type          = atUserConfirmation;
                     GAP_Authentication_Information.Authentication_Data_Length       = (Byte_t)sizeof(Byte_t);
                     GAP_Authentication_Information.Authentication_Data.Confirmation = TRUE;

                     /* Submit the Authentication Response.             */
                     Display(("\r\nAuto Accepting: %l\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));

                     Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                     if(!Result)
                        DisplayFunctionSuccess("GAP_Authentication_Response");
                     else
                        DisplayFunctionError("GAP_Authentication_Response", Result);

                     /* Flag that there is no longer a current          */
                     /* Authentication procedure in progress.           */
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
                  }
                  else
                  {
                     Display(("User Confirmation: %l\r\n", (unsigned long)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));

                     /* Inform the user that they will need to respond  */
                     /* with a PIN Code Response.                       */
                     Display(("Respond with: UserConfirmationResponse\r\n"));
                  }
                  break;
               case atPasskeyRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atPasskeyRequest: %s\r\n", Callback_BoardStr));

                  /* Note the current Remote BD_ADDR that is requesting */
                  /* the Passkey.                                       */
                  CurrentRemoteBD_ADDR = GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device;

                  /* Inform the user that they will need to respond with*/
                  /* a Passkey Response.                                */
                  Display(("Respond with: PassKeyResponse\r\n"));
                  break;
               case atRemoteOutOfBandDataRequest:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atRemoteOutOfBandDataRequest: %s\r\n", Callback_BoardStr));

                  /* This application does not support OOB data so      */
                  /* respond with a data length of Zero to force a      */
                  /* negative reply.                                    */
                  GAP_Authentication_Information.GAP_Authentication_Type    = atOutOfBandData;
                  GAP_Authentication_Information.Authentication_Data_Length = 0;

                  Result = GAP_Authentication_Response(BluetoothStackID, GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, &GAP_Authentication_Information);

                  if(!Result)
                     DisplayFunctionSuccess("GAP_Authentication_Response");
                  else
                     DisplayFunctionError("GAP_Authentication_Response", Result);
                  break;
               case atPasskeyNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atPasskeyNotification: %s\r\n", Callback_BoardStr));

                  Display(("Passkey Value: %d\r\n", GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Numeric_Value));
                  break;
               case atKeypressNotification:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atKeypressNotification: %s\r\n", Callback_BoardStr));

                  Display(("Keypress: %d\r\n", (int)GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Authentication_Event_Data.Keypress_Type));
                  break;
               case atSecureSimplePairingComplete:
                  BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Authentication_Event_Data->Remote_Device, Callback_BoardStr);
                  Display(("\r\n"));
                  Display(("atSecureSimplePairingComplete: %s\r\n", Callback_BoardStr));
                  break;
               default:
                  Display(("Un-handled Auth. Event.\r\n"));
                  break;
            }
            break;
         case etRemote_Name_Result:
            /* Bluetooth Stack has responded to a previously issued     */
            /* Remote Name Request that was issued.                     */
            GAP_Remote_Name_Event_Data = GAP_Event_Data->Event_Data.GAP_Remote_Name_Event_Data;
            if(GAP_Remote_Name_Event_Data)
            {
               /* Inform the user of the Result.                        */
               BD_ADDRToStr(GAP_Remote_Name_Event_Data->Remote_Device, Callback_BoardStr);

               Display(("\r\n"));
               Display(("BD_ADDR: %s.\r\n", Callback_BoardStr));

               if(GAP_Remote_Name_Event_Data->Remote_Name)
                  Display(("Name: %s.\r\n", GAP_Remote_Name_Event_Data->Remote_Name));
               else
                  Display(("Name: NULL.\r\n"));
            }
            break;
         case etEncryption_Change_Result:
            BD_ADDRToStr(GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Remote_Device, Callback_BoardStr);
            Display(("\r\netEncryption_Change_Result for %s, Status: 0x%02X, Mode: %s.\r\n", Callback_BoardStr,
                                                                                             GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Change_Status,
                                                                                             ((GAP_Event_Data->Event_Data.GAP_Encryption_Mode_Event_Data->Encryption_Mode == emDisabled)?"Disabled": "Enabled")));
            break;
         default:
            /* An unknown/unexpected GAP event was received.            */
            Display(("\r\nUnknown Event: %d.\r\n", GAP_Event_Data->Event_Data_Type));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));
      Display(("Null Event\r\n"));
   }

   DisplayPrompt();
}

   /* The following function is for the SDP Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified SDP Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the SDP  */
   /* Request ID of the SDP Request, the SDP Response Event Data of the */
   /* specified Response Event and the SDP Response Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the SDP Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other SDP Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other SDP Events.  A  */
   /*          Deadlock WILL occur because NO SDP Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI SDP_Event_Callback(unsigned int BluetoothStackID, unsigned int SDPRequestID, SDP_Response_Data_t *SDP_Response_Data, unsigned long CallbackParameter)
{
   int Index;

   /* First, check to see if the required parameters appear to be       */
   /* semi-valid.                                                       */
   if((SDP_Response_Data != NULL) && (BluetoothStackID))
   {
      /* The parameters appear to be semi-valid, now check to see what  */
      /* type the incoming Event is.                                    */
      switch(SDP_Response_Data->SDP_Response_Data_Type)
      {
         case rdTimeout:
            /* A SDP Timeout was received, display a message indicating */
            /* this.                                                    */
            Display(("\r\n"));
            Display(("SDP Timeout Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t)));
            break;
         case rdConnectionError:
            /* A SDP Connection Error was received, display a message   */
            /* indicating this.                                         */
            Display(("\r\n"));
            Display(("SDP Connection Error Received (Size = 0x%04X).\r\n", sizeof(SDP_Response_Data_t)));
            break;
         case rdErrorResponse:
            /* A SDP error response was received, display all relevant  */
            /* information regarding this event.                        */
            Display(("\r\n"));
            Display(("SDP Error Response Received (Size = 0x%04X) - Error Code: %d.\r\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Error_Response_Data.Error_Code));
            break;
         case rdServiceSearchResponse:
            /* A SDP Service Search Response was received, display all  */
            /* relevant information regarding this event                */
            Display(("\r\n"));
            Display(("SDP Service Search Response Received (Size = 0x%04X) - Record Count: %d\r\n", sizeof(SDP_Response_Data_t), SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count));

            /* First, check to see if any SDP Service Records were      */
            /* found.                                                   */
            if(SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count)
            {
               Display(("Record Handles:\r\n"));

               for(Index = 0; (Word_t)Index < SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Total_Service_Record_Count; Index++)
               {
                  Display(("Record %u: 0x%08X\r\n", (Index + 1), (unsigned int)SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Response_Data.Service_Record_List[Index]));
               }
            }
            else
               Display(("No SDP Service Records Found.\r\n"));
            break;
         case rdServiceAttributeResponse:
            /* A SDP Service Attribute Response was received, display   */
            /* all relevant information regarding this event            */
            Display(("\r\n"));
            Display(("SDP Service Attribute Response Received (Size = 0x%04X)\r\n", sizeof(SDP_Response_Data_t)));

            DisplaySDPAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Attribute_Response_Data, 0);
            break;
         case rdServiceSearchAttributeResponse:
            /* A SDP Service Search Attribute Response was received,    */
            /* display all relevant information regarding this event    */
            Display(("\r\n"));
            Display(("SDP Service Search Attribute Response Received (Size = 0x%04X)\r\n", sizeof(SDP_Response_Data_t)));

            DisplaySDPSearchAttributeResponse(&SDP_Response_Data->SDP_Response_Data.SDP_Service_Search_Attribute_Response_Data);
            break;
         default:
            /* An unknown/unexpected SDP event was received.            */
            Display(("\r\n"));
            Display(("Unknown SDP Event.\r\n"));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));
      Display(("SDP callback data: Response_Data = NULL.\r\n"));
   }

   DisplayPrompt();
}

   /* The following function is responsible for processing HCI Mode     */
   /* change events.                                                    */
static void BTPSAPI HCI_Event_Callback(unsigned int BluetoothStackID, HCI_Event_Data_t *HCI_Event_Data, unsigned long CallbackParameter)
{
   char *Mode;

   /* Make sure that the input parameters that were passed to us are    */
   /* semi-valid.                                                       */
   if((BluetoothStackID) && (HCI_Event_Data))
   {
      /* Process the Event Data.                                        */
      switch(HCI_Event_Data->Event_Data_Type)
      {
         case etMode_Change_Event:
            if(HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data)
            {
               switch(HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Current_Mode)
               {
                  case HCI_CURRENT_MODE_HOLD_MODE:
                     Mode = "Hold";
                     break;
                  case HCI_CURRENT_MODE_SNIFF_MODE:
                     Mode = "Sniff";
                     break;
                  case HCI_CURRENT_MODE_PARK_MODE:
                     Mode = "Park";
                     break;
                  case HCI_CURRENT_MODE_ACTIVE_MODE:
                  default:
                     Mode = "Active";
                     break;
               }

               Display(("\r\n"));
               Display(("HCI Mode Change Event, Status: 0x%02X, Connection Handle: %d, Mode: %s, Interval: %d\r\n", HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Status,
                                                                                                                    HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Connection_Handle,
                                                                                                                    Mode,
                                                                                                                    HCI_Event_Data->Event_Data.HCI_Mode_Change_Event_Data->Interval));
               DisplayPrompt();
            }
            break;
      }
   }
}

   /* The following function is for the AUD Event Receive Data Callback.*/
   /* This function will be called whenever a Callback has been         */
   /* registered for the specified AUD Action that is associated with   */
   /* the Bluetooth Stack.  This function passes to the caller the AUD  */
   /* Event Data of the specified Event and the AUD Event Callback      */
   /* Parameter that was specified when this Callback was installed.    */
   /* The caller is free to use the contents of the AUD Event Data ONLY */
   /* in the context of this callback.  If the caller requires the Data */
   /* for a longer period of time, then the callback function MUST copy */
   /* the data into another Data Buffer.  This function is guaranteed   */
   /* NOT to be invoked more than once simultaneously for the specified */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* other AUD Events will not be processed while this function call is*/
   /* outstanding).                                                     */
   /* * NOTE * This function MUST NOT Block and wait for events that    */
   /*          can only be satisfied by Receiving other AUD Events.  A  */
   /*          Deadlock WILL occur because NO AUD Event Callbacks will  */
   /*          be issued while this function is currently outstanding.  */
static void BTPSAPI AUD_Event_Callback(unsigned int BluetoothStackID, AUD_Event_Data_t *AUD_Event_Data, unsigned long CallbackParameter)
{
   char                               BoardStr[13];
   Boolean_t                          _DisplayPrompt;
   static unsigned int                FrameCount;
   AUD_Remote_Control_Response_Data_t RemoteControlResponseData;
   AVRCP_Capability_Info_t            CapabilityInfoList;
   int                                Result;

   if((BluetoothStackID) && (AUD_Event_Data))
   {
      _DisplayPrompt = TRUE;

      switch(AUD_Event_Data->Event_Data_Type)
      {
         case etAUD_Open_Request_Indication:
            Display(("\r\nAUD Open Request Indication, Type: %s\r\n", (AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->ConnectionRequestType == acrStream)?"Audio Stream":"Remote Control"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR: %s\r\n", BoardStr));

            Display(("\r\nRespond with the command: OpenRequestResponse\r\n"));

            /* Note the current Remote BD_ADDR that is requesting the   */
            /* connection.                                              */
            CurrentRemoteBD_ADDR = AUD_Event_Data->Event_Data.AUD_Open_Request_Indication_Data->BD_ADDR;
            break;
         case etAUD_Stream_Open_Indication:
            /* A local stream endpoint has been opened, display the     */
            /* information.                                             */
            Display(("\r\nAUD Stream Open Indication, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR:  %s.\r\n", BoardStr));
            Display(("MediaMTU: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->MediaMTU));
            Display(("Format:   %u, %u.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->StreamFormat.NumberChannels));

            Connection = TRUE;
            if(NumberofValidResponses < (MAX_INQUIRY_RESULTS-1))
               InquiryResultList[NumberofValidResponses++] = AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR;

            ConnectedBD_ADDR = AUD_Event_Data->Event_Data.AUD_Stream_Open_Indication_Data->BD_ADDR;
            break;
         case etAUD_Stream_Open_Confirmation:
            /* An OpenRemote request has been completed, display the    */
            /* information.                                             */
            Display(("\r\nAUD Stream Open Confirmation, Type: Sink.\r\n"));

            Display(("Status:   %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->OpenStatus));
            Display(("MediaMTU: %u.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->MediaMTU));
            Display(("Format:   %u, %u.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->StreamFormat.NumberChannels));

            if(!AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->OpenStatus)
               Connection = TRUE;

            ConnectedBD_ADDR = AUD_Event_Data->Event_Data.AUD_Stream_Open_Confirmation_Data->BD_ADDR;
            break;
         case etAUD_Stream_Close_Indication:
            /* A local stream endpoint has been closed, display the     */
            /* information.                                             */
            Display(("\r\nAUD Stream Close Indication, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR: %s.\r\n", BoardStr));
            Display(("Reason:  %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->DisconnectReason == adrRemoteDeviceDisconnect)?"Disconnect":(AUD_Event_Data->Event_Data.AUD_Stream_Close_Indication_Data->DisconnectReason == adrRemoteDeviceLinkLoss)?"Link Loss":"Timeout"));

            Connection = FALSE;

            CleanupAudioDecoder();
            Display(("\r\nAUDDemo Clean HW CODEC\r\n"));
            (void) Un_Initialize_AUDIO();
            Change_connection_priority(NORMAL_AUDIO_CONNECTION_PRIORITY);
            ASSIGN_BD_ADDR(ConnectedBD_ADDR, 0, 0, 0, 0, 0, 0);
            break;
         case etAUD_Remote_Control_Open_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Indication_Data->BD_ADDR, BoardStr);

            Display(("\r\nAUD Open Remote Control Indication: %s.\r\n", BoardStr));

            RemoteControlConnection = TRUE;
            ConnectedBD_ADDR        = AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Indication_Data->BD_ADDR;

            /* If using absolute volume and currently a source, send the*/
            /* get capabilities command now.                            */
            break;
         case etAUD_Remote_Control_Open_Confirmation:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->BD_ADDR, BoardStr);

            Display(("\r\nAUD Open Remote Control Confirmation: %s, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->OpenStatus));

            if(!AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->OpenStatus)
               RemoteControlConnection = TRUE;

            ConnectedBD_ADDR = AUD_Event_Data->Event_Data.AUD_Remote_Control_Open_Confirmation_Data->BD_ADDR;
            break;
         case etAUD_Remote_Control_Close_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Close_Indication_Data->BD_ADDR, BoardStr);

            Display(("\r\nAUD Close Remote Control Indication: %s, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Close_Indication_Data->DisconnectReason));

            CleanupAudioDecoder();
            Display(("\r\nAUDDemo Clean HW CODEC\r\n"));
            (void) Un_Initialize_AUDIO();
            RemoteControlConnection = FALSE;
            Change_connection_priority(NORMAL_AUDIO_CONNECTION_PRIORITY);
            ASSIGN_BD_ADDR(ConnectedBD_ADDR, 0, 0, 0, 0, 0, 0);
            break;
         case etAUD_Remote_Control_Command_Indication:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->BD_ADDR, BoardStr);

            switch(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageType)
            {
               case amtPassThrough:
                  Display(("\r\nPassthrough command received: %s, %d, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.PassThroughCommandData.OperationID, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.PassThroughCommandData.StateFlag));
                  break;
               case amtGetCapabilities:
                  RemoteControlResponseData.MessageType                                                   = amtGetCapabilities;
                  RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.CapabilityID          = AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.GetCapabilitiesCommandData.CapabilityID;
                  RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.ResponseCode          = AVRCP_RESPONSE_STABLE;

                  if((AbsoluteVolumeEnabled) && (AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.GetCapabilitiesCommandData.CapabilityID == AVRCP_GET_CAPABILITIES_CAPABILITY_ID_EVENTS_SUPPORTED))
                  {
                     RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.NumberCapabilities = 1;
                     RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.CapabilityInfoList = &CapabilityInfoList;
                     CapabilityInfoList.CapabilityInfo.EventID                                            = AVRCP_EVENT_VOLUME_CHANGED;
                  }
                  else
                  {
                     RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.NumberCapabilities = 0;
                     RemoteControlResponseData.MessageData.GetCapabilitiesResponseData.CapabilityInfoList = NULL;
                  }

                  if((Result = AUD_Send_Remote_Control_Response(BluetoothStackID, ConnectedBD_ADDR, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID, &RemoteControlResponseData)) == 0)
                     Display(("\r\nGet Capabilities response sent.\r\n"));
                  else
                     Display(("\r\nFailed to send Get Capabilities response: %d.\r\n", Result));
                  break;
               case amtRegisterNotification:
                  if((AbsoluteVolumeEnabled) && (AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.RegisterNotificationCommandData.EventID == AVRCP_EVENT_VOLUME_CHANGED))
                  {
                     Display(("\r\nVolume Changed Notification Registered: Transaction ID = %d\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID));
                     RemoteControlResponseData.MessageData.RegisterNotificationResponseData.NotificationData.VolumeChangedData.AbsoluteVolume = CurrentVolume;
                     RemoteControlResponseData.MessageData.RegisterNotificationResponseData.ResponseCode = AVRCP_RESPONSE_INTERIM;

                     VolumeChangedEventTransactionID = AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID;
                  }
                  else
                     RemoteControlResponseData.MessageData.RegisterNotificationResponseData.ResponseCode = AVRCP_RESPONSE_REJECTED;

                  RemoteControlResponseData.MessageType                                          = amtRegisterNotification;
                  RemoteControlResponseData.MessageData.RegisterNotificationResponseData.EventID = AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.RegisterNotificationCommandData.EventID;
                  if((Result = AUD_Send_Remote_Control_Response(BluetoothStackID, ConnectedBD_ADDR, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID, &RemoteControlResponseData)) < 0)
                     Display(("\r\nFailed to send Register Notification response: %d.\r\n", Result));
                  break;
               case amtSetAbsoluteVolume:
                  if(AbsoluteVolumeEnabled)
                  {
                     CurrentVolume = AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageData.SetAbsoluteVolumeCommandData.AbsoluteVolume;
                     Display(("\r\nVolume set to %d\r\n", CurrentVolume));

                     RemoteControlResponseData.MessageType                                              = amtSetAbsoluteVolume;
                     RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.AbsoluteVolume = CurrentVolume;
                     RemoteControlResponseData.MessageData.SetAbsoluteVolumeResponseData.ResponseCode   = AVRCP_RESPONSE_ACCEPTED;

                     if((Result = AUD_Send_Remote_Control_Response(BluetoothStackID, ConnectedBD_ADDR, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->TransactionID, &RemoteControlResponseData)) < 0)
                        Display(("\r\nFailed to send Set Absolute Volume response: %d.\r\n", Result));
                  }
                  break;
               default:
                  Display(("Unknown remote control command: %d\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageType));
            }
            break;
         case etAUD_Remote_Control_Command_Confirmation:
            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->BD_ADDR, BoardStr);

            switch(AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->RemoteControlResponseData.MessageType)
            {
               case amtPassThrough:
                  Display(("\r\nPassthrough response received: %s, %d, %d.\r\n", BoardStr, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->RemoteControlResponseData.MessageData.PassThroughResponseData.OperationID, AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Confirmation_Data->RemoteControlResponseData.MessageData.PassThroughResponseData.StateFlag));
                  break;
               default:
                  Display(("Unknown remote control command: %d\r\n", AUD_Event_Data->Event_Data.AUD_Remote_Control_Command_Indication_Data->RemoteControlCommandData.MessageType));
            }
            break;
         case etAUD_Stream_State_Change_Indication:
            /* A local stream endpoint has had the state changed,       */
            /* display the information.                                 */
            Display(("\r\nAUD Stream State Change Indication, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR: %s.\r\n", BoardStr));
            Display(("State:   %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamState == astStreamStarted)?"Started":"Stopped"));

            if(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Indication_Data->StreamState == astStreamStopped)
            {
               Display(("\r\nAUD Clean Decoder thread\r\n"));
               CleanupAudioDecoder();
               Change_connection_priority(NORMAL_AUDIO_CONNECTION_PRIORITY);
            }
            else
            {
               Display(("\r\n Initialize HW CODEC\r\n"));
               InitializeAudioDecoder(BluetoothStackID, ConnectedBD_ADDR);
               Change_connection_priority(HIGH_AUDIO_CONNECTION_PRIORITY);
            }

            break;
         case etAUD_Stream_State_Change_Confirmation:
            /* A request to change the stream state has returned,       */
            /* display the information.                                 */
            Display(("\r\nAUD Stream State Change Confirmation, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR:    %s.\r\n", BoardStr));
            Display(("State:      %s.\r\n", (AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->StreamState == astStreamStarted)?"Started":"Stopped"));
            Display(("Successful: %s.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_State_Change_Confirmation_Data->Successful?"TRUE":"FALSE"));
            break;
         case etAUD_Stream_Format_Change_Indication:
            /* A local stream endpoint has had the format changed,      */
            /* display the information.                                 */
            Display(("\r\nAUD Stream Format Change Indication, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR: %s.\r\n", BoardStr));
            Display(("Format:  %u, %u.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Indication_Data->StreamFormat.NumberChannels));
            break;
         case etAUD_Stream_Format_Change_Confirmation:
            /* A request to change the stream format has returned,      */
            /* display the information.                                 */
            Display(("\r\nAUD Stream Format Change Confirmation, Type: Sink.\r\n"));

            BD_ADDRToStr(AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->BD_ADDR, BoardStr);
            Display(("BD_ADDR:    %s.\r\n", BoardStr));
            Display(("Format:     %u, %u.\r\n", (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamFormat.SampleFrequency, (unsigned int)AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->StreamFormat.NumberChannels));
            Display(("Successful: %s.\r\n", AUD_Event_Data->Event_Data.AUD_Stream_Format_Change_Confirmation_Data->Successful?"TRUE":"FALSE"));
            break;
         case etAUD_Encoded_Audio_Data_Indication:
            /* Encoded Audio Data has been received.                    */

            /* Pass the audio data to the decoder.                      */
            ProcessAudioData(AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrameLength, AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrame);

            if(FrameCount++ > 100)
            {
//               Display(("\r\nAUD Encoded Audio Data Indication, Length %d.\r\n", AUD_Event_Data->Event_Data.AUD_Encoded_Audio_Data_Indication_Data->RawAudioDataFrameLength));

               FrameCount = 0;
            }

            _DisplayPrompt = FALSE;
            break;
         case etAUD_Signalling_Channel_Open_Indication:
            Display(("Signaling channel openned... \r\n"));
            /* Query the connection handle */
            Result = GAP_Query_Connection_Handle(BluetoothStackID, AUD_Event_Data->Event_Data.AUD_Signalling_Channel_Open_Indication_Data->BD_ADDR, &Connection_Handle);
            if(Result)
            {
               /* Failed to Query the Connection Handle.                */
               DisplayFunctionError(" Faile! GAP_Query_Connection_Handle()", Result);
               Connection_Handle = 0;
            }
            else
               Display(("HCI Connection Handle: 0x%04X.\r\n", Connection_Handle));
        
            break;
         default:
            /* An unknown/unexpected Audio event was received.          */
            Display(("\r\nUnknown/Unhandled Audio Event: %d.\n", AUD_Event_Data->Event_Data_Type));
            break;
      }

      if(_DisplayPrompt)
         DisplayPrompt();
   }
}

/* The following function is used to initialize the application      */
   /* instance.  This function should open the stack and prepare to     */
   /* execute commands based on user input.  The first parameter passed */
   /* to this function is the HCI Driver Information that will be used  */
   /* when opening the stack and the second parameter is used to pass   */
   /* parameters to BTPS_Init.  This function returns the               */
   /* BluetoothStackID returned from BSC_Initialize on success or a     */
   /* negative error code (of the form APPLICATION_ERROR_XXX).          */

int InitializeApplication(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization)
{
   int                           ret_val;

   /* Initiailize some defaults.                                        */
   NumberofValidResponses  = 0;
   HCIEventCallbackHandle  = 0;

/* Next, makes sure that the Driver Information passed appears to be */
   /* semi-valid.                                                       */
   if((HCI_DriverInformation) && (BTPS_Initialization))
   {
           /* Try to Open the stack and check if it was successful.             */
           if(!OpenStack(HCI_DriverInformation, BTPS_Initialization))
           {
              /* The stack was opened successfully.  Now set some defaults.     */

              /* First, attempt to set the Device to be Connectable.            */
              ret_val = SetConnect();

              /* Next, check to see if the Device was successfully made         */
              /* Connectable.                                                   */
              if(!ret_val)
              {
                 /* Now that the device is Connectable attempt to make it       */
                 /* Discoverable.                                               */
                 ret_val = SetDisc();

                 /* Next, check to see if the Device was successfully made      */
                 /* Discoverable.                                               */
                 if(!ret_val)
                 {
                    /* Now that the device is discoverable attempt to make it   */
                    /* pairable.                                                */
                    ret_val = SetPairable();
                    if(!ret_val)
                    {
                       /* Attempt to register a HCI Event Callback.             */
                       ret_val = HCI_Register_Event_Callback(BluetoothStackID, HCI_Event_Callback, (unsigned long)NULL);
                       if(ret_val > 0)
                       {
                        /* Assign the Callback Handle.                        */
                        HCIEventCallbackHandle = ret_val;

                        ASSIGN_BD_ADDR(NullADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

                        if((Initialize_Sink()) < 0)
                        {                                                  
                            Display(("Failed to initialize Audio subsystem.\r\n"));

                            /* Unregister the previously registered HCI     */
                            /* Event Callback.                              */
                            HCI_Un_Register_Callback(BluetoothStackID, HCIEventCallbackHandle);

                            HCIEventCallbackHandle = 0;
                        }
                        /* Set up the Selection Interface.                 */
                        UserInterface();

                        /* Display the first command prompt.               */
                        DisplayPrompt();                        

                        /* Return success to the caller.                   */
                        ret_val = (int)BluetoothStackID;                        

                       }
                       else
                          DisplayFunctionError("HCI_Register_Event_Callback()", ret_val);
                    }
                    else
                       DisplayFunctionError("SetPairable", ret_val);
                 }
                 else
                    DisplayFunctionError("SetDisc", ret_val);
              }
              else
               DisplayFunctionError("SetConnect", ret_val);

            /* In some error occurred then close the stack.                */
            if(ret_val < 0)
            {
                /* Close the Bluetooth Stack.                                     */
                CloseStack();
            }

           }
           else
           {
              /* There was an error while attempting to open the Stack.         */
              Display(("Unable to open the stack.\r\n"));
           }

    }
    else
    {
      ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;
    }
 return (ret_val);
}


   /* The following function is used to process a command line string.  */
    /* This function takes as it's only parameter the command line string*/
    /* to be parsed and returns TRUE if a command was parsed and executed*/
    /* or FALSE otherwise.                                               */
Boolean_t ProcessCommandLine(char *String)
{
   return(CommandLineInterpreter(String));
}


