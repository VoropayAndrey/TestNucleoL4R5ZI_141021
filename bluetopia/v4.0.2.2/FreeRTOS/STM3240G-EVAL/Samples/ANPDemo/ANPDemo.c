/*****< ANPDemo.c >************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright 2015 Texas Instruments Incorporated.                        */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  ANPDemo - Embedded Bluetooth Alert Notification Profile using GATT (LE)   */
/*            application.                                                    */
/*                                                                            */
/*  Author:  Ryan Byrne                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   05/28/12  Ryan Byrne     Initial creation.                               */
/*   03/03/15  D. Horowitz    Adding Demo Application version.                */
/******************************************************************************/
#include "Main.h"                /* Application Interface Abstraction.        */
#include "ANPDemo.h"             /* Application Header.                       */

#include "SS1BTPS.h"             /* Main SS1 BT Stack Header.                 */
#include "SS1BTGAT.h"            /* Main SS1 GATT Header.                     */
#include "SS1BTGAP.h"            /* Main SS1 GAP Service Header.              */
#include "SS1BTANS.h"            /* Main SS1 ANP Service Header.              */
#include "SS1BTDIS.h"            /* Main SS1 DIS Service Header.              */
#include "BTPSKRNL.h"            /* BTPS Kernel Header.                       */
#include "SS1BTVS.h"             /* Vendor Specific Header.                   */

#define MAX_SUPPORTED_COMMANDS                     (46)  /* Denotes the       */
                                                         /* maximum number of */
                                                         /* User Commands that*/
                                                         /* are supported by  */
                                                         /* this application. */

#define MAX_NUM_OF_PARAMETERS                       (5)  /* Denotes the max   */
                                                         /* number of         */
                                                         /* parameters a      */
                                                         /* command can have. */

#define MAX_SIZE_ALERT_STRING                      (32)  /* Denotes the max   */
                                                         /* string size for an*/
                                                         /* alert message.    */

#define DEFAULT_IO_CAPABILITY      (licNoInputNoOutput)  /* Denotes the       */
                                                         /* default I/O       */
                                                         /* Capability that is*/
                                                         /* used with Pairing.*/

#define DEFAULT_MITM_PROTECTION                  (TRUE)  /* Denotes the       */
                                                         /* default value used*/
                                                         /* for Man in the    */
                                                         /* Middle (MITM)     */
                                                         /* protection used   */
                                                         /* with Secure Simple*/
                                                         /* Pairing.          */

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

#define TO_MANY_PARAMS                             (-5)  /* Denotes that there*/
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

#define EXIT_TEST_MODE                            (-10)  /* Flags exit from   */
                                                         /* Test Mode.        */

                                                         /* Mask for all      */
                                                         /* categories that   */
                                                         /* The ANP Service   */
                                                         /* supports.         */
#define SUPPORTED_CATEGORIES_MASK ((ANS_SUPPORTED_CATEGORIES_SIMPLE_ALERT)        | \
                                   (ANS_SUPPORTED_CATEGORIES_EMAIL)               | \
                                   (ANS_SUPPORTED_CATEGORIES_NEWS)                | \
                                   (ANS_SUPPORTED_CATEGORIES_CALL)                | \
                                   (ANS_SUPPORTED_CATEGORIES_MISSED_CALL)         | \
                                   (ANS_SUPPORTED_CATEGORIES_SMS_MMS)             | \
                                   (ANS_SUPPORTED_CATEGORIES_VOICE_MAIL)          | \
                                   (ANS_SUPPORTED_CATEGORIES_SCHEDULE)            | \
                                   (ANS_SUPPORTED_CATEGORIES_HIGH_PRIORITY_ALERT) | \
                                   (ANS_SUPPORTED_CATEGORIES_INSTANT_MESSAGE))

   /* The following MACRO is used to convert an ASCII character into the*/
   /* equivalent decimal value.  The MACRO converts lower case          */
   /* characters to upper case before the conversion.                   */
#define ToInt(_x)                                  (((_x) > 0x39)?(((_x) & ~0x20)-0x37):((_x)-0x30))

   /* Determine the Name we will use for this compilation.              */
#define LE_APP_DEMO_NAME                        "ANPDemo"

   /* The following type definition represents the structure which holds*/
   /* all information about the parameter, in particular the parameter  */
   /* as a string and the parameter as an unsigned int.                 */
typedef struct _tagParameter_t
{
   char         *strParam;
   SDWord_t      intParam;
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

   /* The following enumerated type definition defines the different    */
   /* types of service discovery that can be performed.                 */
typedef enum
{
   sdGAPS,
   sdANS
} Service_Discovery_Type_t;

   /* Structure used to hold all of the GAP LE Parameters.              */
typedef struct _tagGAPLE_Parameters_t
{
   GAP_LE_Connectability_Mode_t ConnectableMode;
   GAP_Discoverability_Mode_t   DiscoverabilityMode;
   GAP_LE_IO_Capability_t       IOCapability;
   Boolean_t                    MITMProtection;
   Boolean_t                    OOBDataPresent;
} GAPLE_Parameters_t;

#define GAPLE_PARAMETERS_DATA_SIZE                       (sizeof(GAPLE_Parameters_t))

   /* The following structure represents the information we will store  */
   /* on a Discovered GAP Service.                                      */
typedef struct _tagGAPS_Client_Info_t
{
   Word_t DeviceNameHandle;
   Word_t DeviceAppearanceHandle;
} GAPS_Client_Info_t;

   /* The following structure holds information on known Device         */
   /* Appearance Values.                                                */
typedef struct _tagGAPS_Device_Appearance_Mapping_t
{
   Word_t  Appearance;
   char   *String;
} GAPS_Device_Appearance_Mapping_t;

   /* The following structure for is used to hold a list of information */
   /* on all paired devices.                                            */
typedef struct _tagDeviceInfo_t
{
   Byte_t                    Flags;
   Byte_t                    EncryptionKeySize;
   GAP_LE_Address_Type_t     ConnectionAddressType;
   BD_ADDR_t                 ConnectionBD_ADDR;
   Long_Term_Key_t           LTK;
   Random_Number_t           Rand;
   Word_t                    EDIV;
   GAPS_Client_Info_t        GAPSClientInfo;
   ANS_Client_Information_t  ClientInfo;
   ANS_Server_Information_t  ServerInfo;
   struct _tagDeviceInfo_t  *NextDeviceInfoPtr;
} DeviceInfo_t;

#define DEVICE_INFO_DATA_SIZE                            (sizeof(DeviceInfo_t))

   /* Defines the bitmask flags that may be set in the DeviceInfo_t     */
   /* structure.                                                        */
#define DEVICE_INFO_FLAGS_LTK_VALID                         0x01
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING     0x02
#define DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE        0x04
#define DEVICE_INFO_FLAGS_LINK_ENCRYPTED                    0x08

   /* The following structure represents the structure of an ANP New    */
   /* Alert Status.                                                     */
typedef struct _tagNewAlertStatus_t
{
   Byte_t NumberOfAlerts;
   char   AlertString[MAX_SIZE_ALERT_STRING];
} NewAlertStatus_t;


   /* The following structure is used to hold information of the 		*/
   /* FIRMWARE version.                                                 */
typedef struct FW_Version_t
{
   Byte_t StatusResult; 
   Byte_t HCI_VersionResult;
   Word_t HCI_RevisionResult;
   Byte_t LMP_VersionResult; 
   Word_t Manufacturer_NameResult; 
   Word_t LMP_SubversionResult;
} FW_Version;

   /* User to represent a structure to hold a BD_ADDR return from       */
   /* BD_ADDRToStr.                                                     */
typedef char BoardStr_t[16];

                        /* The Encryption Root Key should be generated  */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t ER = {0x28, 0xBA, 0xE1, 0x37, 0x13, 0xB2, 0x20, 0x45, 0x16, 0xB2, 0x19, 0xD0, 0x80, 0xEE, 0x4A, 0x51};

                        /* The Identity Root Key should be generated    */
                        /* in such a way as to guarantee 128 bits of    */
                        /* entropy.                                     */
static BTPSCONST Encryption_Key_t IR = {0x41, 0x09, 0xA0, 0x88, 0x09, 0x6B, 0x70, 0xC0, 0x95, 0x23, 0x3C, 0x8C, 0x48, 0xFC, 0xC9, 0xFE};

                        /* The following keys can be regenerated on the */
                        /* fly using the constant IR and ER keys and    */
                        /* are used globally, for all devices.          */
static Encryption_Key_t DHK;
static Encryption_Key_t IRK;

   /* Internal Variables to this Module (Remember that all variables    */
   /* declared static are initialized to 0 automatically by the         */
   /* compiler as part of standard C/C++).                              */
static unsigned int        ANSInstanceID;           /* The following holds the ANS     */
                                                    /* Instance ID that is returned    */
                                                    /* from ANS_Initialize_Service().  */

static unsigned int        GAPSInstanceID;          /* Holds the Instance ID for the   */
                                                    /* GAP Service.                    */

static unsigned int        DISInstanceID;           /* Holds the Instance ID for the   */
                                                    /* DIS Service.                    */

static GAPLE_Parameters_t  LE_Parameters;           /* Holds GAP Parameters like       */
                                                    /* Discoverability, Connectability */
                                                    /* Modes.                          */

static DeviceInfo_t       *DeviceInfoList;          /* Holds the list head for the     */
                                                    /* device info list.               */

static unsigned int        BluetoothStackID;        /* Variable which holds the Handle */
                                                    /* of the opened Bluetooth Protocol*/
                                                    /* Stack.                          */

static BD_ADDR_t           ConnectionBD_ADDR;       /* Holds the BD_ADDR of the        */
                                                    /* currently connected device.     */

static unsigned int        ConnectionID;            /* Holds the Connection ID of the  */
                                                    /* currently connected device.     */

static Boolean_t           LocalDeviceIsMaster;     /* Boolean that tells if the local */
                                                    /* device is the master of the     */
                                                    /* current connection.             */

static BD_ADDR_t           CurrentRemoteBD_ADDR;    /* Variable which holds the        */
                                                    /* current BD_ADDR of the device   */
                                                    /* which is currently pairing or   */
                                                    /* authenticating.                 */

static unsigned int        NumberCommands;          /* Variable which is used to hold  */
                                                    /* the number of Commands that are */
                                                    /* supported by this application.  */
                                                    /* Commands are added individually.*/

static CommandTable_t      CommandTable[MAX_SUPPORTED_COMMANDS]; /* Variable which is  */
                                                    /* used to hold the actual Commands*/
                                                    /* that are supported by this      */
                                                    /* application.                    */

                                                    /* Variable which is used to hold  */
                                                    /* the New Alert Status Data for   */
                                                    /* each Alert Category.            */
static NewAlertStatus_t    NewAlertStatuses[ANS_ALERT_CATEGORY_ID_INSTANT_MESSAGE+1];

                                                    /* Variable which is used to hold  */
                                                    /* the Unread Alert Status Data for*/
                                                    /* each Alert Category.            */
static Byte_t              UnreadAlertStatuses[ANS_ALERT_CATEGORY_ID_INSTANT_MESSAGE+1];

   /* The following is used to map from ATT Error Codes to a printable  */
   /* string.                                                           */
static char *ErrorCodeStr[] = {
   "ATT_PROTOCOL_ERROR_CODE_NO_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_HANDLE",
   "ATT_PROTOCOL_ERROR_CODE_READ_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_WRITE_NOT_PERMITTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_PDU",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHENTICATION",
   "ATT_PROTOCOL_ERROR_CODE_REQUEST_NOT_SUPPORTED",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_OFFSET",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_AUTHORIZATION",
   "ATT_PROTOCOL_ERROR_CODE_PREPARE_QUEUE_FULL",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_FOUND",
   "ATT_PROTOCOL_ERROR_CODE_ATTRIBUTE_NOT_LONG",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION_KEY_SIZE",
   "ATT_PROTOCOL_ERROR_CODE_INVALID_ATTRIBUTE_VALUE_LENGTH",
   "ATT_PROTOCOL_ERROR_CODE_UNLIKELY_ERROR",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_ENCRYPTION",
   "ATT_PROTOCOL_ERROR_CODE_UNSUPPORTED_GROUP_TYPE",
   "ATT_PROTOCOL_ERROR_CODE_INSUFFICIENT_RESOURCES",
};

#define NUMBER_OF_ERROR_CODES     (sizeof(ErrorCodeStr)/sizeof(char *))

   /* The following array is used to map Device Appearance Values to    */
   /* strings.                                                          */
static GAPS_Device_Appearance_Mapping_t AppearanceMappings[] =
{
   {GAP_DEVICE_APPEARENCE_VALUE_UNKNOWN,                        "Unknown"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_PHONE,                  "Generic Phone"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER,               "Generic Computer"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_WATCH,                  "Generic Watch"},
   {GAP_DEVICE_APPEARENCE_VALUE_SPORTS_WATCH,                   "Sports Watch"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_CLOCK,                  "Generic Clock"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_DISPLAY,                "Generic Display"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GENERIC_REMOTE_CONTROL, "Generic Remote Control"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_EYE_GLASSES,            "Eye Glasses"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_TAG,                    "Generic Tag"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_KEYRING,                "Generic Keyring"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_MEDIA_PLAYER,           "Generic Media Player"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BARCODE_SCANNER,        "Generic Barcode Scanner"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_THERMOMETER,            "Generic Thermometer"},
   {GAP_DEVICE_APPEARENCE_VALUE_THERMOMETER_EAR,                "Ear Thermometer"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_HEART_RATE_SENSOR,      "Generic Heart Rate Sensor"},
   {GAP_DEVICE_APPEARENCE_VALUE_BELT_HEART_RATE_SENSOR,         "Belt Heart Rate Sensor"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_BLOOD_PRESSURE,         "Generic Blood Pressure"},
   {GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_ARM,             "Blood Pressure: ARM"},
   {GAP_DEVICE_APPEARENCE_VALUE_BLOOD_PRESSURE_WRIST,           "Blood Pressure: Wrist"},
   {GAP_DEVICE_APPEARENCE_VALUE_HUMAN_INTERFACE_DEVICE,         "Human Interface Device"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_KEYBOARD,                   "HID Keyboard"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_MOUSE,                      "HID Mouse"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_JOYSTICK,                   "HID Joystick"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_GAMEPAD,                    "HID Gamepad"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITIZER_TABLET,           "HID Digitizer Tablet"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_CARD_READER,                "HID Card Reader"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_DIGITAL_PEN,                "HID Digitizer Pen"},
   {GAP_DEVICE_APPEARENCE_VALUE_HID_BARCODE_SCANNER,            "HID Bardcode Scanner"},
   {GAP_DEVICE_APPEARENCE_VALUE_GENERIC_GLUCOSE_METER,          "Generic Glucose Meter"}
};

#define NUMBER_OF_APPEARANCE_MAPPINGS     (sizeof(AppearanceMappings)/sizeof(GAPS_Device_Appearance_Mapping_t))

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
   "No Input/Output",
   "Keyboard/Display"
} ;

   /* The following string table is used to map the Alert Category      */
   /* values to an easily displayable string.                           */
static BTPSCONST char *AlertCategories[] =
{
   "Simple Alert",
   "E-Mail",
   "News",
   "Call",
   "Missed Call",
   "SMS MMS",
   "Voice Mail",
   "Schedule",
   "High Priority Alert",
   "Instant Message",
   "All Categories"
} ;

#define NUM_ALERT_CATEGORIES              ((sizeof(AlertCategories)/sizeof(char *)) - 1)

   /* Internal function prototypes.                                     */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR);
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR);
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree);
static void FreeDeviceInfoList(DeviceInfo_t **ListHead);

   /* Command line functions                                            */
static void UserInterface(void);
static Boolean_t CommandLineInterpreter(char *Command);
static unsigned long StringToUnsignedInteger(char *StringInteger);
static char *StringParser(char *String);
static int CommandParser(UserCommand_t *TempCommand, char *Input);
static int CommandInterpreter(UserCommand_t *TempCommand);
static int AddCommand(char *CommandName, CommandFunction_t CommandFunction);
static CommandFunction_t FindCommand(char *Command);
static void ClearCommands(void);

   /* Display functions                                                 */
static void DisplayIOCapabilities(void);
static void DisplayAdvertisingData(GAP_LE_Advertising_Data_t *Advertising_Data);
static void DisplayPairingInformation(GAP_LE_Pairing_Capabilities_t Pairing_Capabilities);
static void DisplayUUID(GATT_UUID_t *UUID);
static void DisplayPrompt(void);
static void DisplayUsage(char *UsageString);
static void DisplayFunctionError(char *Function,int Status);
static void DisplayConnectLEUsage(char *CharacteristicName);
static void DumpAppearanceMappings(void);
static void DisplayFWVersion (void);

   /* Generic Function Commands                                         */
static int DisplayHelp(ParameterList_t *TempParam);
static int QueryMemory(ParameterList_t *TempParam);
static int SetBaudRate(ParameterList_t *TempParam);

   /* Generic Helper Functions                                          */
static void BD_ADDRToStr(BD_ADDR_t Board_Address, BoardStr_t BoardStr);
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address);

   /* Bluetooth Function Commands                                       */
static int SetDiscoverabilityMode(ParameterList_t *TempParam);
static int SetConnectabilityMode(ParameterList_t *TempParam);
static int SetPairabilityMode(ParameterList_t *TempParam);
static int ChangePairingParameters(ParameterList_t *TempParam);
static int GetLocalAddress(ParameterList_t *TempParam);
static int GetLocalName(ParameterList_t *TempParam);
static int SetLocalName(ParameterList_t *TempParam);
static int ReadRemoteName(ParameterList_t *TempParam);

   /* Bluetooth Helper Functions                                        */
static int OpenStack(HCI_DriverInformation_t *HCI_DriverInformation, BTPS_Initialization_t *BTPS_Initialization);
static int CloseStack(void);
static int SetDisc(void);
static int SetConnect(void);
static int SetPairable(void);

   /* Bluetooth LE Function Commands                                    */
static int LEPassKeyResponse(ParameterList_t *TempParam);
static int LEQueryEncryption(ParameterList_t *TempParam);
static int LESetPasskey(ParameterList_t *TempParam);
static int AdvertiseLE(ParameterList_t *TempParam);
static int StartScanning(ParameterList_t *TempParam);
static int StopScanning(ParameterList_t *TempParam);
static int ConnectLE(ParameterList_t *TempParam);
static int DisconnectLE(ParameterList_t *TempParam);
static int PairLE(ParameterList_t *TempParam);
static int DiscoverGAPS(ParameterList_t *TempParam);
static int GetLocalAppearance(ParameterList_t *TempParam);
static int SetLocalAppearance(ParameterList_t *TempParam);
static int GetRemoteAppearance(ParameterList_t *TempParam);

   /* Bluetooth LE helper functions                                     */
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance);
static Boolean_t AppearanceToString(Word_t Appearance, char **String);
static void ConfigureCapabilities(GAP_LE_Pairing_Capabilities_t *Capabilities);
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo);
static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback);
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information);
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR);
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster);
static int DisconnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR);
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t RemoteAddressType ,GAP_LE_Address_Type_t OwnAddressType, Boolean_t UseWhiteList);
static int StartScan(unsigned int BluetoothStackID);
static int StopScan(unsigned int BluetoothStackID);
static void ResolveRemoteAddressHelper(BD_ADDR_t BD_ADDR);


   /* Alert Notification Profile Function Commands                      */
static int RegisterANS(ParameterList_t *TempParam);
static int UnregisterANS(ParameterList_t *TempParam);
static int DiscoverANS(ParameterList_t *TempParam);
static int ConfigureRemoteANS(ParameterList_t *TempParam);
static int NotifyNewAlerts(ParameterList_t *TempParam);
static int NotifyUnreadAlerts(ParameterList_t *TempParam);
static int GetNewAlertStatus(ParameterList_t *TempParam);
static int SetNewAlertStatus(ParameterList_t *TempParam);
static int GetUnreadAlertStatus(ParameterList_t *TempParam);
static int SetUnreadAlertStatus(ParameterList_t *TempParam);
static int SetSupportedNewAlertCategories(ParameterList_t *TempParam);
static int GetSupportedNewAlertCategories(ParameterList_t *TempParam);
static int GetSupportedUnreadAlertCategories(ParameterList_t *TempParam);
static int SetSupportedUnreadAlertCategories(ParameterList_t *TempParam);
static int EnableNewAlertNotifications(ParameterList_t *TempParam);
static int DisableNewAlertNotifications(ParameterList_t *TempParam);
static int EnableUnreadAlertNotifications(ParameterList_t *TempParam);
static int DisableUnreadAlertNotifications(ParameterList_t *TempParam);
static int NotifyImmediatelyNewAlert(ParameterList_t *TempParam);
static int NotifyImmediatelyUnreadAlert(ParameterList_t *TempParam);

   /* Alert Notification Helper Functions                               */
static void DisplayUsageAlertCategoryNumericalValue(Boolean_t DisplayAllCategories);
static void DisplayUsageAlertCategoryBitMask(void);
static void DisplaySupportedCategories(Word_t SupportedCategories, char *Prefix, char *CategoryType);
static void DisplayNewAlertStatus(ANS_Category_Identification_t Category, Byte_t NumAlerts, char *AlertString, char *Prefix);
static void DisplayUnreadAlertStatus(ANS_Category_Identification_t Category, Byte_t NumAlerts, char *Prefix);
static int WriteControlPointCommand(ANS_Control_Point_Command_t Command, ANS_Category_Identification_t Category);
static int SendNewAlertNotification(Word_t BitMask, char *Prefix);
static int SendUnreadAlertNotification(Word_t BitMask, char *Prefix);

   /* BTPS Callback function prototypes.                                */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI ANS_EventCallback(unsigned int BluetoothStackID, ANS_Event_Data_t *ANS_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_ANP(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter);
static void BTPSAPI GATT_Service_Discovery_Event_Callback(unsigned int BluetoothStackID, GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, unsigned long CallbackParameter);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as parameters to this function.  This     */
   /* function will return FALSE if NO Entry was added.  This can occur */
   /* if the element passed in was deemed invalid or the actual List    */
   /* Head was invalid.                                                 */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            Connection BD_ADDR.  When this occurs, this function   */
   /*            returns NULL.                                          */
static Boolean_t CreateNewDeviceInfoEntry(DeviceInfo_t **ListHead, GAP_LE_Address_Type_t ConnectionAddressType, BD_ADDR_t ConnectionBD_ADDR)
{
   Boolean_t     ret_val = FALSE;
   DeviceInfo_t *DeviceInfoPtr;

   /* Verify that the passed in parameters seem semi-valid.             */
   if((ListHead) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
   {
      /* Allocate the memory for the entry.                             */
      if((DeviceInfoPtr = BTPS_AllocateMemory(sizeof(DeviceInfo_t))) != NULL)
      {
         /* Initialize the entry.                                       */
         BTPS_MemInitialize(DeviceInfoPtr, 0, sizeof(DeviceInfo_t));
         DeviceInfoPtr->ConnectionAddressType = ConnectionAddressType;
         DeviceInfoPtr->ConnectionBD_ADDR     = ConnectionBD_ADDR;

         ret_val = BSC_AddGenericListEntry_Actual(ekBD_ADDR_t, BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead), (void *)(DeviceInfoPtr));
         if(!ret_val)
         {
            /* Failed to add to list so we should free the memory that  */
            /* we allocated for the entry.                              */
            BTPS_FreeMemory(DeviceInfoPtr);
         }
      }
   }

   return(ret_val);
}

   /* The following function searches the specified List for the        */
   /* specified Connection BD_ADDR.  This function returns NULL if      */
   /* either the List Head is invalid, the BD_ADDR is invalid, or the   */
   /* Connection BD_ADDR was NOT found.                                 */
static DeviceInfo_t *SearchDeviceInfoEntryByBD_ADDR(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return(BSC_SearchGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
}

   /* The following function searches the specified Key Info List for   */
   /* the specified BD_ADDR and removes it from the List.  This function*/
   /* returns NULL if either the List Head is invalid, the BD_ADDR is   */
   /* invalid, or the specified Entry was NOT present in the list.  The */
   /* entry returned will have the Next Entry field set to NULL, and    */
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling the FreeKeyEntryMemory() function.          */
static DeviceInfo_t *DeleteDeviceInfoEntry(DeviceInfo_t **ListHead, BD_ADDR_t BD_ADDR)
{
   return(BSC_DeleteGenericListEntry(ekBD_ADDR_t, (void *)(&BD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, ConnectionBD_ADDR), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr), (void **)(ListHead)));
}

   /* This function frees the specified Key Info Information member     */
   /* memory.                                                           */
static void FreeDeviceInfoEntryMemory(DeviceInfo_t *EntryToFree)
{
   BSC_FreeGenericListEntryMemory((void *)(EntryToFree));
}

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Key Info List. Upon return of this       */
   /* function, the Head Pointer is set to NULL.                        */
static void FreeDeviceInfoList(DeviceInfo_t **ListHead)
{
   BSC_FreeGenericListEntryList((void **)(ListHead), BTPS_STRUCTURE_OFFSET(DeviceInfo_t, NextDeviceInfoPtr));
}

   /* This function is responsible for taking the input from the user   */
   /* and dispatching the appropriate Command Function.  First, this    */
   /* function retrieves a String of user input, parses the user input  */
   /* into Command and Parameters, and finally executes the Command or  */
   /* Displays an Error Message if the input is not a valid Command.    */
static void UserInterface(void)
{
   /* Clear the installed command.                                      */
   ClearCommands();

   /* Install the commands relevant for this UI.                        */
   AddCommand("SETDISCOVERABILITYMODE", SetDiscoverabilityMode);
   AddCommand("SETCONNECTABILITYMODE", SetConnectabilityMode);
   AddCommand("SETPAIRABILITYMODE", SetPairabilityMode);
   AddCommand("CHANGEPAIRINGPARAMETERS", ChangePairingParameters);
   AddCommand("GETLOCALADDRESS", GetLocalAddress);
   AddCommand("ADVERTISELE", AdvertiseLE);
   AddCommand("STARTSCANNING", StartScanning);
   AddCommand("STOPSCANNING", StopScanning);
   AddCommand("CONNECTLE", ConnectLE);
   AddCommand("DISCONNECTLE", DisconnectLE);
   AddCommand("PAIRLE", PairLE);
   AddCommand("SETBAUDRATE", SetBaudRate);
   AddCommand("LEPASSKEYRESPONSE", LEPassKeyResponse);
   AddCommand("QUERYENCRYPTIONMODE", LEQueryEncryption);
   AddCommand("SETPASSKEY", LESetPasskey);
   AddCommand("DISCOVERGAPS", DiscoverGAPS);
   AddCommand("GETLOCALNAME", GetLocalName);
   AddCommand("SETLOCALNAME", SetLocalName);
   AddCommand("GETREMOTENAME", ReadRemoteName);
   AddCommand("GETLOCALAPPEARANCE", GetLocalAppearance);
   AddCommand("SETLOCALAPPEARANCE", SetLocalAppearance);
   AddCommand("GETREMOTEAPPEARANCE", GetRemoteAppearance);
   AddCommand("REGISTERANS", RegisterANS);
   AddCommand("UNREGISTERANS", UnregisterANS);
   AddCommand("DISCOVERANS", DiscoverANS);
   AddCommand("CONFIGUREREMOTEANS", ConfigureRemoteANS);
   AddCommand("NOTIFYNEWALERTS", NotifyNewAlerts);
   AddCommand("NOTIFYUNREADALERTS", NotifyUnreadAlerts);
   AddCommand("GETNEWALERTSTATUS", GetNewAlertStatus);
   AddCommand("SETNEWALERTSTATUS", SetNewAlertStatus);
   AddCommand("GETUNREADALERTSTATUS", GetUnreadAlertStatus);
   AddCommand("SETUNREADALERTSTATUS", SetUnreadAlertStatus);
   AddCommand("GETSUPPORTEDNEWALERTCATEGORIES", GetSupportedNewAlertCategories);
   AddCommand("SETSUPPORTEDNEWALERTCATEGORIES", SetSupportedNewAlertCategories);
   AddCommand("GETSUPPORTEDUNREADALERTCATEGORIES", GetSupportedUnreadAlertCategories);
   AddCommand("SETSUPPORTEDUNREADALERTCATEGORIES", SetSupportedUnreadAlertCategories);
   AddCommand("ENABLENEWALERTNOTIFICATIONS", EnableNewAlertNotifications);
   AddCommand("DISABLENEWALERTNOTIFICATIONS", DisableNewAlertNotifications);
   AddCommand("ENABLEUNREADALERTNOTIFICATIONS", EnableUnreadAlertNotifications);
   AddCommand("DISABLEUNREADALERTNOTIFICATIONS", DisableUnreadAlertNotifications);
   AddCommand("NOTIFYIMMEDIATELYNEWALERT", NotifyImmediatelyNewAlert);
   AddCommand("NOTIFYIMMEDIATELYUNREADALERT", NotifyImmediatelyUnreadAlert);
   AddCommand("HELP", DisplayHelp);
   AddCommand("QUERYMEMORY", QueryMemory);
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
   /* strings to their unsigned integer equivalent.  This function can  */
   /* handle leading and tailing white space, however it does not handle*/
   /* signed or comma delimited values.  This function takes as its     */
   /* input the string which is to be converted.  The function returns  */
   /* zero if an error occurs otherwise it returns the value parsed from*/
   /* the string passed as the input parameter.                         */
static unsigned long StringToUnsignedInteger(char *StringInteger)
{
   int           IsHex;
   unsigned long Index;
   unsigned long ret_val = 0;

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
            /* begining character of the string.                        */
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
         ret_val = 0;

         /* Adjust the UserInput pointer and StringLength to remove the */
         /* Command from the data passed in before parsing the          */
         /* parameters.                                                 */
         if(BTPS_StringLength(TempCommand->Command) == StringLength)
            Input     += StringLength;
         else
            Input     += BTPS_StringLength(TempCommand->Command) + 1;

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

               if(BTPS_StringLength(LastParameter) == StringLength)
                  Input     += StringLength;
               else
                  Input     += BTPS_StringLength(LastParameter) + 1;

               StringLength  = BTPS_StringLength(Input);

               ret_val = 0;
            }
            else
            {
               /* Be sure we exit out of the Loop.                      */
               StringLength = 0;

               ret_val      = TO_MANY_PARAMS;
            }
         }

         /* Set the number of parameters in the User Command to the     */
         /* number of found parameters                                  */
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
   /* A negative return value implies that that command was not found   */
   /* and is invalid.                                                   */
static int CommandInterpreter(UserCommand_t *TempCommand)
{
   int               i;
   int               ret_val;
   CommandFunction_t CommandFunction;

   /* If the command is not found in the table return with an invaild   */
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

      /* Check to see if the command which was entered was exit.        */
      if(BTPS_MemCompare(TempCommand->Command, "QUIT", BTPS_StringLength("QUIT")) != 0)
      {
         /* The command entered is not exit so search for command in    */
         /* table.                                                      */
         if((CommandFunction = FindCommand(TempCommand->Command)) != NULL)
         {
            /* The command was found in the table so call the command.  */
            if((ret_val = ((*CommandFunction)(&TempCommand->Parameters))) == 0)
            {
               /* Return success to the caller.                         */
               ret_val = 0;
            }
            else
            {
               if ((ret_val != EXIT_CODE) && (ret_val != EXIT_TEST_MODE))
                  ret_val = FUNCTION_ERROR;
            }
         }
      }
      else
      {
         /* The command entered is exit, set return value to EXIT_CODE  */
         /* and return.                                                 */
         ret_val = EXIT_CODE;
      }
   }
   else
      ret_val = INVALID_PARAMETERS_ERROR;

   return(ret_val);
}

   /* The following function is provided to allow a means to            */
   /* programmatically add Commands the Global (to this module) Command */
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
         if((BTPS_StringLength(CommandTable[Index].CommandName) == BTPS_StringLength(Command)) && (BTPS_MemCompare(Command, CommandTable[Index].CommandName, BTPS_StringLength(CommandTable[Index].CommandName)) == 0))
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
   /* Flag that there are no commands present in the table and clear the*/
   /* memory                                                            */
   BTPS_MemInitialize(CommandTable, 0, sizeof(CommandTable));
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

   /* The following function is responsible for the specified string    */
   /* into data of type BD_ADDR.  The first parameter of this function  */
   /* is the BD_ADDR string to be converted to a BD_ADDR.  The second   */
   /* parameter of this function is a pointer to the BD_ADDR in which   */
   /* the converted BD_ADDR String is to be stored.                     */
static void StrToBD_ADDR(char *BoardStr, BD_ADDR_t *Board_Address)
{
   char          *TempPtr;
   unsigned int   StringLength;
   unsigned int   Index;
   unsigned char  Value;

   if((BoardStr) && ((StringLength = BTPS_StringLength(BoardStr)) >= (sizeof(BD_ADDR_t) * 2)) && (Board_Address))
   {
      TempPtr = BoardStr;
      if((StringLength >= (sizeof(BD_ADDR_t) * 2) + 2) && (TempPtr[0] == '0') && ((TempPtr[1] == 'x') || (TempPtr[1] == 'X')))
         TempPtr += 2;

      for(Index=0;Index<6;Index++)
      {
         Value  = (char)(ToInt(*TempPtr) * 0x10);
         TempPtr++;
         Value += (char)ToInt(*TempPtr);
         TempPtr++;
         ((char *)Board_Address)[5-Index] = (Byte_t)Value;
      }
   }
   else
   {
      if(Board_Address)
         BTPS_MemInitialize(Board_Address, 0, sizeof(BD_ADDR_t));
   }
}

   /* Displays the current I/O Capabilities.                            */
static void DisplayIOCapabilities(void)
{
   Display(("I/O Capabilities: %s, MITM: %s.\r\n", IOCapabilitiesStrings[(unsigned int)(LE_Parameters.IOCapability - licDisplayOnly)], LE_Parameters.MITMProtection?"TRUE":"FALSE"));
}

   /* Utility function to display advertising data.                     */
static void DisplayAdvertisingData(GAP_LE_Advertising_Data_t *Advertising_Data)
{
   unsigned int Index;
   unsigned int Index2;

   /* Verify that the input parameters seem semi-valid.                 */
   if(Advertising_Data)
   {
      for(Index = 0; Index < Advertising_Data->Number_Data_Entries; Index++)
      {
         Display(("  AD Type: 0x%02X.\r\n", Advertising_Data->Data_Entries[Index].AD_Type));
         Display(("  AD Length: 0x%02X.\r\n", Advertising_Data->Data_Entries[Index].AD_Data_Length));
         if(Advertising_Data->Data_Entries[Index].AD_Data_Buffer)
         {
            Display(("  AD Data: "));
            for(Index2 = 0; Index2 < Advertising_Data->Data_Entries[Index].AD_Data_Length; Index2++)
            {
               Display(("0x%02X ", Advertising_Data->Data_Entries[Index].AD_Data_Buffer[Index2]));
            }
            Display(("\r\n"));
         }
      }
   }
}

   /* The following function displays the pairing capabilities that is  */
   /* passed into this function.                                        */
static void DisplayPairingInformation(GAP_LE_Pairing_Capabilities_t Pairing_Capabilities)
{
   /* Display the IO Capability.                                        */
   switch(Pairing_Capabilities.IO_Capability)
   {
      case licDisplayOnly:
         Display(("   IO Capability:       lcDisplayOnly.\r\n"));
         break;
      case licDisplayYesNo:
         Display(("   IO Capability:       lcDisplayYesNo.\r\n"));
         break;
      case licKeyboardOnly:
         Display(("   IO Capability:       lcKeyboardOnly.\r\n"));
         break;
      case licNoInputNoOutput:
         Display(("   IO Capability:       lcNoInputNoOutput.\r\n"));
         break;
      case licKeyboardDisplay:
         Display(("   IO Capability:       lcKeyboardDisplay.\r\n"));
         break;
   }

   Display(("   MITM:                %s.\r\n", (Pairing_Capabilities.MITM == TRUE)?"TRUE":"FALSE"));
   Display(("   Bonding Type:        %s.\r\n", (Pairing_Capabilities.Bonding_Type == lbtBonding)?"Bonding":"No Bonding"));
   Display(("   OOB:                 %s.\r\n", (Pairing_Capabilities.OOB_Present == TRUE)?"OOB":"OOB Not Present"));
   Display(("   Encryption Key Size: %d.\r\n", Pairing_Capabilities.Maximum_Encryption_Key_Size));
   Display(("   Sending Keys: \r\n"));
   Display(("      LTK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Encryption_Key == TRUE)?"YES":"NO")));
   Display(("      IRK:              %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Identification_Key == TRUE)?"YES":"NO")));
   Display(("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Sending_Keys.Signing_Key == TRUE)?"YES":"NO")));
   Display(("   Receiving Keys: \r\n"));
   Display(("      LTK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Encryption_Key == TRUE)?"YES":"NO")));
   Display(("      IRK:              %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Identification_Key == TRUE)?"YES":"NO")));
   Display(("      CSRK:             %s.\r\n", ((Pairing_Capabilities.Receiving_Keys.Signing_Key == TRUE)?"YES":"NO")));
}

   /* The following function is provided to properly print a UUID.      */
static void DisplayUUID(GATT_UUID_t *UUID)
{
   if(UUID)
   {
      if(UUID->UUID_Type == guUUID_16)
         Display(("%02X%02X", UUID->UUID.UUID_16.UUID_Byte1, UUID->UUID.UUID_16.UUID_Byte0));
      else
      {
         if(UUID->UUID_Type == guUUID_128)
         {
            Display(("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", UUID->UUID.UUID_128.UUID_Byte15, UUID->UUID.UUID_128.UUID_Byte14, UUID->UUID.UUID_128.UUID_Byte13,
                                                                                         UUID->UUID.UUID_128.UUID_Byte12, UUID->UUID.UUID_128.UUID_Byte11, UUID->UUID.UUID_128.UUID_Byte10,
                                                                                         UUID->UUID.UUID_128.UUID_Byte9,  UUID->UUID.UUID_128.UUID_Byte8,  UUID->UUID.UUID_128.UUID_Byte7,
                                                                                         UUID->UUID.UUID_128.UUID_Byte6,  UUID->UUID.UUID_128.UUID_Byte5,  UUID->UUID.UUID_128.UUID_Byte4,
                                                                                         UUID->UUID.UUID_128.UUID_Byte3,  UUID->UUID.UUID_128.UUID_Byte2,  UUID->UUID.UUID_128.UUID_Byte1,
                                                                                         UUID->UUID.UUID_128.UUID_Byte0));
         }
      }
   }

   Display((".\r\n"));
}

   /* Displays the correct prompt depending on the Server/Client Mode.  */
static void DisplayPrompt(void)
{
   Display(("\r\nLE>"));
}

   /* Displays a usage string..                                         */
static void DisplayUsage(char *UsageString)
{
   Display(("Usage: %s.\r\n",UsageString));
}

   /* Displays a function error message.                                */
static void DisplayFunctionError(char *Function,int Status)
{
   Display(("Error - %s returned %d.\r\n", Function, Status));
}

   /* The following function is for displaying The FW Version by reading*/
   /* The Local version information form the FW.                        */
static void DisplayFWVersion (void)
{
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

   /* The following function is responsible for querying the memory     */
   /* heap usage. This function will return zero on successful          */  
   /* execution and a negative value on errors.                         */
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

 /* This function displays the usage of DisplayConnectLEUsage command */
static void DisplayConnectLEUsage(char *CharacteristicName)
{
   Display(("Usage: %s \t", CharacteristicName));
   Display((" [BD_ADDR] (default Public Addresses)\r\n"));
   Display(("[RemoteDeviceAddressType (0 = Public Address, 1 = Random Address )(Optional)]\r\n"));
   Display(("[OwnAddressType          (0 = Public Address, 1 = Random Address )(Optional)].\r\n"));
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
   unsigned int               ServiceID;
   HCI_Version_t              HCIVersion;
   L2CA_Link_Connect_Params_t L2CA_Link_Connect_Params;

   /* First check to see if the Stack has already been opened.          */
   if(!BluetoothStackID)
   {
      /* Next, makes sure that the Driver Information passed appears to */
      /* be semi-valid.                                                 */
      if(HCI_DriverInformation)
      {

         /* Initialize BTPSKNRl.                                        */
         BTPS_Init((void *)BTPS_Initialization);

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

            ret_val          = 0;

            /* Attempt to enable the WBS feature.                       */
            Result = BSC_EnableFeature(BluetoothStackID, BSC_FEATURE_BLUETOOTH_LOW_ENERGY);
            if(!Result)
            {
               Display(("LOW ENERGY Support initialized.\r\n"));
            }
            else
            {
               Display(("LOW ENERGY Support not initialized %d.\r\n", Result));
            }            


            /* Initialize the Default Pairing Parameters.               */
            LE_Parameters.IOCapability   = DEFAULT_IO_CAPABILITY;
            LE_Parameters.MITMProtection = DEFAULT_MITM_PROTECTION;
            LE_Parameters.OOBDataPresent = FALSE;

            if(!HCI_Version_Supported(BluetoothStackID, &HCIVersion))
               Display(("Device Chipset: %s\r\n", (HCIVersion <= NUM_SUPPORTED_HCI_VERSIONS)?HCIVersionStrings[HCIVersion]:HCIVersionStrings[NUM_SUPPORTED_HCI_VERSIONS]));

            /* Printing the BTPS version                                */
            Display(("BTPS Version  : %s \r\n", BTPS_VERSION_VERSION_STRING));
            /* Printing the FW version                                  */
            DisplayFWVersion();

            /* Printing the Demo Application name and version           */
            Display(("App Name      : %s \r\n", LE_APP_DEMO_NAME));
            Display(("App Version   : %s \r\n", DEMO_APPLICATION_VERSION_STRING));            
            
            /* Let's output the Bluetooth Device Address so that the    */
            /* user knows what the Device Address is.                   */
            if(!GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR))
            {
               BD_ADDRToStr(BD_ADDR, BluetoothAddress);

               Display(("Local BD_ADDR: %s\r\n", BluetoothAddress));
            }

            /* Go ahead and allow Master/Slave Role Switch.             */
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Request_Config  = cqAllowRoleSwitch;
            L2CA_Link_Connect_Params.L2CA_Link_Connect_Response_Config = csMaintainCurrentRole;

            L2CA_Set_Link_Connection_Configuration(BluetoothStackID, &L2CA_Link_Connect_Params);

            if(HCI_Command_Supported(BluetoothStackID, HCI_SUPPORTED_COMMAND_WRITE_DEFAULT_LINK_POLICY_BIT_NUMBER) > 0)
               HCI_Write_Default_Link_Policy_Settings(BluetoothStackID, (HCI_LINK_POLICY_SETTINGS_ENABLE_MASTER_SLAVE_SWITCH|HCI_LINK_POLICY_SETTINGS_ENABLE_SNIFF_MODE), &Status);

            /* Flag that no connection is currently active.             */
            ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);
            LocalDeviceIsMaster = FALSE;

            /* Regenerate IRK and DHK from the constant Identity Root   */
            /* Key.                                                     */
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 1,0, &IRK);
            GAP_LE_Diversify_Function(BluetoothStackID, (Encryption_Key_t *)(&IR), 3, 0, &DHK);

            /* Flag that we have no Key Information in the Key List.    */
            DeviceInfoList = NULL;

            /* Initialize the GATT Service.                             */
            if((Result = GATT_Initialize(BluetoothStackID, GATT_INITIALIZATION_FLAGS_SUPPORT_LE, GATT_Connection_Event_Callback, 0)) == 0)
            {
               /* Initialize the GAPS Service.                          */
               Result = GAPS_Initialize_Service(BluetoothStackID, &ServiceID);
               if(Result > 0)
               {
                  /* Save the Instance ID of the GAP Service.           */
                  GAPSInstanceID = (unsigned int)Result;

                  /* Set the GAP Device Name and Device Appearance.     */
                  GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, LE_APP_DEMO_NAME);
                  GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, GAP_DEVICE_APPEARENCE_VALUE_GENERIC_COMPUTER);

                  /* Initialize the DIS Service.                        */
                  Result = DIS_Initialize_Service(BluetoothStackID, &ServiceID);
                  if(Result > 0)
                  {
                     /* Save the Instance ID of the GAP Service.        */
                     DISInstanceID = (unsigned int)Result;

                     /* Set the discoverable attributes                 */
                     DIS_Set_Manufacturer_Name(BluetoothStackID, DISInstanceID, BTPS_VERSION_COMPANY_NAME_STRING);
                     DIS_Set_Model_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);
                     DIS_Set_Serial_Number(BluetoothStackID, DISInstanceID, BTPS_VERSION_VERSION_STRING);

                     /* Return success to the caller.                   */
                     ret_val        = 0;
                  }
               }
               else
               {
                  /* The Stack was NOT initialized successfully, inform */
                  /* the user and set the return value of the           */
                  /* initialization function to an error.               */
                  DisplayFunctionError("GAPS_Initialize_Service", Result);

                  /* Cleanup GATT Module.                               */
                  GATT_Cleanup(BluetoothStackID);

                  BluetoothStackID = 0;

                  ret_val          = UNABLE_TO_INITIALIZE_STACK;
               }
            }
            else
            {
               /* The Stack was NOT initialized successfully, inform the*/
               /* user and set the return value of the initialization   */
               /* function to an error.                                 */
               DisplayFunctionError("GATT_Initialize", Result);

               BluetoothStackID = 0;

               ret_val          = UNABLE_TO_INITIALIZE_STACK;
            }
         }
         else
         {
            /* The Stack was NOT initialized successfully, inform the   */
            /* user and set the return value of the initialization      */
            /* function to an error.                                    */
            DisplayFunctionError("BSC_Initialize", Result);

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
      /* Cleanup GAP Service Module.                                    */
      if(GAPSInstanceID)
      {
         GAPS_Cleanup_Service(BluetoothStackID, GAPSInstanceID);

         GAPSInstanceID = 0;
      }

      /* Cleanup ANP Service.                                           */
      if(ANSInstanceID)
      {
         ANS_Cleanup_Service(BluetoothStackID, ANSInstanceID);

         ANSInstanceID = 0;
      }

      /* Cleanup DIS Service Module.                                    */
      if(DISInstanceID)
      {
         DIS_Cleanup_Service(BluetoothStackID, DISInstanceID);

         DISInstanceID = 0;
      }

      /* Cleanup GATT Module.                                           */
      GATT_Cleanup(BluetoothStackID);

      /* Simply close the Stack                                         */
      BSC_Shutdown(BluetoothStackID);

      /* Free BTPSKRNL allocated memory.                                */
      BTPS_DeInit();

      Display(("Stack Shutdown.\r\n"));

      /* Free the Key List.                                             */
      FreeDeviceInfoList(&DeviceInfoList);

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
      /* * NOTE * Discoverability is only applicable when we are        */
      /*          advertising so save the default Discoverability Mode  */
      /*          for later.                                            */
      LE_Parameters.DiscoverabilityMode = dmGeneralDiscoverableMode;
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
      /* * NOTE * Connectability is only an applicable when advertising */
      /*          so we will just save the default connectability for   */
      /*          the next time we enable advertising.                  */
      LE_Parameters.ConnectableMode = lcmConnectable;
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
      Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, lpmPairableMode);

      /* Next, check the return value of the GAP Set Pairability mode   */
      /* command for successful execution.                              */
      if(!Result)
      {
         /* The device has been set to pairable mode, now register an   */
         /* Authentication Callback to handle the Authentication events */
         /* if required.                                                */
         Result = GAP_LE_Register_Remote_Authentication(BluetoothStackID, GAP_LE_Event_Callback, (unsigned long)0);

         /* Next, check the return value of the GAP Register Remote     */
         /* Authentication command for successful execution.            */
         if(Result)
         {
            /* An error occurred while trying to execute this function. */
            DisplayFunctionError("GAP_LE_Register_Remote_Authentication", Result);

            ret_val = Result;
         }
      }
      else
      {
         /* An error occurred while trying to make the device pairable. */
         DisplayFunctionError("GAP_LE_Set_Pairability_Mode", Result);

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

   /* The following function is a utility function that is used to dump */
   /* the Appearance to String Mapping Table.                           */
static void DumpAppearanceMappings(void)
{
   unsigned int Index;

   for(Index=0;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      Display(("   %u = %s.\r\n", Index, AppearanceMappings[Index].String));
}

   /* The following function is used to map a Appearance Value to it's  */
   /* string representation.  This function returns TRUE on success or  */
   /* FALSE otherwise.                                                  */
static Boolean_t AppearanceToString(Word_t Appearance, char **String)
{
   Boolean_t    ret_val;
   unsigned int Index;

   /* Verify that the input parameters are semi-valid.                  */
   if(String)
   {
      for(Index=0,ret_val=FALSE;Index<NUMBER_OF_APPEARANCE_MAPPINGS;++Index)
      {
         if(AppearanceMappings[Index].Appearance == Appearance)
         {
            *String = AppearanceMappings[Index].String;
            ret_val = TRUE;
            break;
         }
      }
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is used to map an Index into the Appearance*/
   /* Mapping table to it's Appearance Value.  This function returns    */
   /* TRUE on success or FALSE otherwise.                               */
static Boolean_t AppearanceIndexToAppearance(unsigned int Index, Word_t *Appearance)
{
   Boolean_t ret_val;

   if((Index < NUMBER_OF_APPEARANCE_MAPPINGS) && (Appearance))
   {
      *Appearance = AppearanceMappings[Index].Appearance;
      ret_val     = TRUE;
   }
   else
      ret_val = FALSE;

   return(ret_val);
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating discovered GAP Service Handles.           */
static void GAPSPopulateHandles(GAPS_Client_Info_t *ClientInfo, GATT_Service_Discovery_Indication_Data_t *ServiceInfo)
{
   unsigned int                       Index1;
   GATT_Characteristic_Information_t *CurrentCharacteristic;

   /* Verify that the input parameters are semi-valid.                  */
   if((ClientInfo) && (ServiceInfo) && (ServiceInfo->ServiceInformation.UUID.UUID_Type == guUUID_16) && (GAP_COMPARE_GAP_SERVICE_UUID_TO_UUID_16(ServiceInfo->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceInfo->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1=0;Index1<ServiceInfo->NumberOfCharacteristics;Index1++,CurrentCharacteristic++)
         {
            /* All GAP Service UUIDs are defined to be 16 bit UUIDs.    */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               /* Determine which characteristic this is.               */
               if(!GAP_COMPARE_GAP_DEVICE_NAME_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  if(!GAP_COMPARE_GAP_DEVICE_APPEARANCE_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     continue;
                  else
                  {
                     ClientInfo->DeviceAppearanceHandle = CurrentCharacteristic->Characteristic_Handle;
                     continue;
                  }
               }
               else
               {
                  ClientInfo->DeviceNameHandle = CurrentCharacteristic->Characteristic_Handle;
                  continue;
               }
            }
         }
      }
   }
}

   /* The following function is a utility function that provides a      */
   /* mechanism of populating a ANP Client Information structure with   */
   /* the information discovered from a GATT Discovery operation.       */
static void ANSPopulateHandles(DeviceInfo_t *DeviceInfo, GATT_Service_Discovery_Indication_Data_t *ServiceDiscoveryData)
{
   Word_t                                       *ClientConfigurationHandle;
   unsigned int                                  Index1;
   unsigned int                                  Index2;
   GATT_Characteristic_Information_t            *CurrentCharacteristic;
   GATT_Characteristic_Descriptor_Information_t *CurrentDescriptor;

   /* Verify that the input parameters are semi-valid.                  */
   if((DeviceInfo) && (ServiceDiscoveryData) && (ServiceDiscoveryData->ServiceInformation.UUID.UUID_Type == guUUID_16) && (ANS_COMPARE_ANS_SERVICE_UUID_TO_UUID_16(ServiceDiscoveryData->ServiceInformation.UUID.UUID.UUID_16)))
   {
      /* Loop through all characteristics discovered in the service     */
      /* and populate the correct entry.                                */
      CurrentCharacteristic = ServiceDiscoveryData->CharacteristicInformationList;
      if(CurrentCharacteristic)
      {
         for(Index1 = 0; Index1 < ServiceDiscoveryData->NumberOfCharacteristics; Index1++, CurrentCharacteristic++)
         {
            /* All ANS UUIDs are defined to be 16 bit UUIDs.            */
            if(CurrentCharacteristic->Characteristic_UUID.UUID_Type == guUUID_16)
            {
               ClientConfigurationHandle = NULL;

               /* Determine which characteristic this is.               */
               if(!ANS_COMPARE_ANS_SUPORTED_NEW_ALERT_CATEGORY_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
               {
                  if(!ANS_COMPARE_ANS_NEW_ALERT_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                  {
                     if(!ANS_COMPARE_ANS_SUPPORTED_UNREAD_ALERT_CATEGORY_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                     {
                        if(!ANS_COMPARE_ANS_UNREAD_ALERT_STATUS_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                        {
                           if(!ANS_COMPARE_ANS_CONTROL_POINT_UUID_TO_UUID_16(CurrentCharacteristic->Characteristic_UUID.UUID.UUID_16))
                           {
                              continue;
                           }
                           else
                           {
                              DeviceInfo->ClientInfo.Control_Point = CurrentCharacteristic->Characteristic_Handle;

                              /* Verify that write is supported.        */
                              if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_WRITE))
                                 Display(("Warning - Mandatory write property of Ringer Control Point characteristic not supported!\r\n"));

                              continue;
                           }
                        }
                        else
                        {
                           DeviceInfo->ClientInfo.Unread_Alert_Status = CurrentCharacteristic->Characteristic_Handle;
                           ClientConfigurationHandle                  = &(DeviceInfo->ClientInfo.Unread_Alert_Status_Client_Configuration);

                           /* Verify that notify is supported.          */
                           if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                              Display(("Warning - Mandatory notify property of Ringer Control Point characteristic not supported!\r\n"));
                        }
                     }
                     else
                     {
                        DeviceInfo->ClientInfo.Supported_Unread_Alert_Category = CurrentCharacteristic->Characteristic_Handle;

                        /* Verify that read is supported.               */
                        if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                           Display(("Warning - Mandatory read property of Ringer Control Point characteristic not supported!\r\n"));

                        continue;
                     }
                  }
                  else
                  {
                     DeviceInfo->ClientInfo.New_Alert = CurrentCharacteristic->Characteristic_Handle;
                     ClientConfigurationHandle        = &(DeviceInfo->ClientInfo.New_Alert_Client_Configuration);

                     /* Verify that notify is supported.                */
                     if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_NOTIFY))
                        Display(("Warning - Mandatory notify property of Ringer Setting characteristic not supported!\r\n"));
                  }
               }
               else
               {
                  DeviceInfo->ClientInfo.Supported_New_Alert_Category = CurrentCharacteristic->Characteristic_Handle;

                  /* Verify that read is supported.                     */
                  if(!(CurrentCharacteristic->Characteristic_Properties & GATT_CHARACTERISTIC_PROPERTIES_READ))
                     Display(("Warning - Mandatory read property of Alert Status characteristic not supported!\r\n"));

                  continue;
               }

               /* Loop through the Descriptor List.                     */
               CurrentDescriptor = CurrentCharacteristic->DescriptorList;
               if(ClientConfigurationHandle && CurrentDescriptor)
               {
                  for(Index2 = 0; Index2 < CurrentCharacteristic->NumberOfDescriptors; Index2++, CurrentDescriptor++)
                  {
                     if(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID_Type == guUUID_16)
                     {
                        if(GATT_COMPARE_CLIENT_CHARACTERISTIC_CONFIGURATION_ATTRIBUTE_TYPE_TO_BLUETOOTH_UUID_16(CurrentDescriptor->Characteristic_Descriptor_UUID.UUID.UUID_16))
                        {
                           *ClientConfigurationHandle = CurrentDescriptor->Characteristic_Descriptor_Handle;
                           break;
                        }
                     }
                  }
               }
            }
         }
      }
   }
}

   /* The following function function is used to enable/disable         */
   /* notifications on a specified handle.  This function returns the   */
   /* positive non-zero Transaction ID of the Write Request or a        */
   /* negative error code.                                              */
static int EnableDisableNotificationsIndications(Word_t ClientConfigurationHandle, Word_t ClientConfigurationValue, GATT_Client_Event_Callback_t ClientEventCallback)
{
   int              ret_val;
   NonAlignedWord_t Buffer;

   /* Verify the input parameters.                                      */
   if((BluetoothStackID) && (ConnectionID) && (ClientConfigurationHandle))
   {
      ASSIGN_HOST_WORD_TO_LITTLE_ENDIAN_UNALIGNED_WORD(&Buffer, ClientConfigurationValue);

      ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, ClientConfigurationHandle, sizeof(Buffer), &Buffer, ClientEventCallback, ClientConfigurationHandle);
   }
   else
      ret_val = BTPS_ERROR_INVALID_PARAMETER;

   return(ret_val);
}

   /* The following function is responsible for starting a scan.        */
static int StartScan(unsigned int BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      /* Not currently scanning, go ahead and attempt to perform the    */
      /* scan.                                                          */
      Result = GAP_LE_Perform_Scan(BluetoothStackID, stActive, 10, 10, latPublic, fpNoFilter, TRUE, GAP_LE_Event_Callback, 0);

      if(!Result)
      {
         Display(("Scan started successfully.\r\n"));
      }
      else
      {
         /* Unable to start the scan.                                   */
         Display(("Unable to perform scan: %d\r\n", Result));
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible for stopping on on-going    */
   /* scan.                                                             */
static int StopScan(unsigned int BluetoothStackID)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      Result = GAP_LE_Cancel_Scan(BluetoothStackID);
      if(!Result)
      {
         Display(("Scan stopped successfully.\r\n"));
      }
      else
      {
         /* Error stopping scan.                                        */
         Display(("Unable to stop scan: %d\r\n", Result));
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is responsible to determining that Remote  */
   /* Device address is resolvable private address or not. If it is     */
   /* resolvable address then it resolve this address.                  */
static void ResolveRemoteAddressHelper(BD_ADDR_t BD_ADDR)
{
   if(GAP_LE_TEST_RESOLVABLE_ADDRESS_BITS(BD_ADDR))
   {
      if(GAP_LE_Resolve_Address(BluetoothStackID,&IRK,BD_ADDR))
         Display(("GAP_LE_Resolve_Address Success: \r\n\n"));
      else
         Display(("GAP_LE_Resolve_Address Failure: \r\n\n"));
   }
}

   /* The following function is responsible for creating an LE          */
   /* connection to the specified Remote Device.                        */
static int ConnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR, GAP_LE_Address_Type_t RemoteAddressType ,GAP_LE_Address_Type_t OwnAddressType, Boolean_t UseWhiteList)
{
   int                            Result;
   unsigned int                   WhiteListChanged;
   GAP_LE_White_List_Entry_t      WhiteListEntry;
   GAP_LE_Connection_Parameters_t ConnectionParameters;

   /* First, determine if the input parameters appear to be semi-valid. */
   if((BluetoothStackID) && (!COMPARE_NULL_BD_ADDR(BD_ADDR)))
   {
      if(COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Remove any previous entries for this device from the White  */
         /* List.                                                       */
         WhiteListEntry.Address_Type = RemoteAddressType;
         WhiteListEntry.Address      = BD_ADDR;

         GAP_LE_Remove_Device_From_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);

         if(UseWhiteList)
            Result = GAP_LE_Add_Device_To_White_List(BluetoothStackID, 1, &WhiteListEntry, &WhiteListChanged);
         else
            Result = 1;

         /* If everything has been successful, up until this point, then*/
         /* go ahead and attempt the connection.                        */
         if(Result >= 0)
         {
            /* Initialize the connection parameters.                    */
            ConnectionParameters.Connection_Interval_Min    = 50;
            ConnectionParameters.Connection_Interval_Max    = 200;
            ConnectionParameters.Minimum_Connection_Length  = 0;
            ConnectionParameters.Maximum_Connection_Length  = 10000;
            ConnectionParameters.Slave_Latency              = 0;
            ConnectionParameters.Supervision_Timeout        = 20000;

            /* Everything appears correct, go ahead and attempt to make */
            /* the connection.                                          */
            Result = GAP_LE_Create_Connection(BluetoothStackID, 100, 100, Result?fpNoFilter:fpWhiteList, RemoteAddressType, Result?&BD_ADDR:NULL, OwnAddressType, &ConnectionParameters, GAP_LE_Event_Callback, 0);
            if(!Result)
            {
               Display(("Connection Request successful.\r\n"));
            }
            else
            {
               /* Unable to create connection.                          */
               Display(("Unable to create connection: %d.\r\n", Result));
            }
         }
         else
         {
            /* Unable to add device to White List.                      */
            Display(("Unable to add device to White List.\r\n"));
         }
      }
      else
      {
         /* Device already connected.                                   */
         Display(("Device is already connected.\r\n"));

         Result = -2;
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function is provided to allow a mechanism to        */
   /* disconnect a currently connected device.                          */
static int DisconnectLEDevice(unsigned int BluetoothStackID, BD_ADDR_t BD_ADDR)
{
   int Result;

   /* First, determine if the input parameters appear to be semi-valid. */
   if(BluetoothStackID)
   {
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         Result = GAP_LE_Disconnect(BluetoothStackID, BD_ADDR);

         if(!Result)
         {
            Display(("Disconnect Request successful.\r\n"));
         }
         else
         {
            /* Unable to disconnect device.                             */
            Display(("Unable to disconnect device: %d.\r\n", Result));
         }
      }
      else
      {
         /* Device not connected.                                       */
         Display(("Device is not connected.\r\n"));

         Result = 0;
      }
   }
   else
      Result = -1;

   return(Result);
}

   /* The following function provides a mechanism to configure a        */
   /* Pairing Capabilities structure with the application's pairing     */
   /* parameters.                                                       */
static void ConfigureCapabilities(GAP_LE_Pairing_Capabilities_t *Capabilities)
{
   /* Make sure the Capabilities pointer is semi-valid.                 */
   if(Capabilities)
   {
      /* Configure the Pairing Capabilities structure.                  */
      Capabilities->Bonding_Type                    = lbtBonding;
      Capabilities->IO_Capability                   = LE_Parameters.IOCapability;
      Capabilities->MITM                            = LE_Parameters.MITMProtection;
      Capabilities->OOB_Present                     = LE_Parameters.OOBDataPresent;

      /* ** NOTE ** This application always requests that we use the    */
      /*            maximum encryption because this feature is not a    */
      /*            very good one, if we set less than the maximum we   */
      /*            will internally in GAP generate a key of the        */
      /*            maximum size (we have to do it this way) and then   */
      /*            we will zero out how ever many of the MSBs          */
      /*            necessary to get the maximum size.  Also as a slave */
      /*            we will have to use Non-Volatile Memory (per device */
      /*            we are paired to) to store the negotiated Key Size. */
      /*            By requesting the maximum (and by not storing the   */
      /*            negotiated key size if less than the maximum) we    */
      /*            allow the slave to power cycle and regenerate the   */
      /*            LTK for each device it is paired to WITHOUT storing */
      /*            any information on the individual devices we are    */
      /*            paired to.                                          */
      Capabilities->Maximum_Encryption_Key_Size        = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;

      /* This application only demonstrates using Long Term Key's (LTK) */
      /* for encryption of a LE Link, however we could request and send */
      /* all possible keys here if we wanted to.                        */
      Capabilities->Receiving_Keys.Encryption_Key     = TRUE;
      Capabilities->Receiving_Keys.Identification_Key = FALSE;
      Capabilities->Receiving_Keys.Signing_Key        = FALSE;

      Capabilities->Sending_Keys.Encryption_Key       = TRUE;
      Capabilities->Sending_Keys.Identification_Key   = FALSE;
      Capabilities->Sending_Keys.Signing_Key          = FALSE;
   }
}

   /* The following function provides a mechanism for sending a pairing */
   /* request to a device that is connected on an LE Link.              */
static int SendPairingRequest(BD_ADDR_t BD_ADDR, Boolean_t ConnectionMaster)
{
   int                           ret_val;
   BoardStr_t                    BoardStr;
   GAP_LE_Pairing_Capabilities_t Capabilities;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the BD_ADDR is valid.                                */
      if(!COMPARE_NULL_BD_ADDR(BD_ADDR))
      {
         /* Configure the application pairing parameters.               */
         ConfigureCapabilities(&Capabilities);

         /* Set the BD_ADDR of the device that we are attempting to pair*/
         /* with.                                                       */
         CurrentRemoteBD_ADDR = BD_ADDR;

         BD_ADDRToStr(BD_ADDR, BoardStr);
         Display(("Attempting to Pair to %s.\r\n", BoardStr));

         /* Attempt to pair to the remote device.                       */
         if(ConnectionMaster)
         {
            /* Start the pairing process.                               */
            ret_val = GAP_LE_Pair_Remote_Device(BluetoothStackID, BD_ADDR, &Capabilities, GAP_LE_Event_Callback, 0);

            if(ret_val == 0)
               Display(("GAP_LE_Pair_Remote_Device success.\r\n", ret_val));
            else
               DisplayFunctionError("     GAP_LE_Pair_Remote_Device", ret_val);

         }
         else
         {
            /* As a slave we can only request that the Master start     */
            /* the pairing process.                                     */
            ret_val = GAP_LE_Request_Security(BluetoothStackID, BD_ADDR, Capabilities.Bonding_Type, Capabilities.MITM, GAP_LE_Event_Callback, 0);

            if(ret_val == 0)
               Display(("GAP_LE_Request_Security success.\r\n", ret_val));
            else
               DisplayFunctionError("     GAP_LE_Request_Security", ret_val);
         }
      }
      else
      {
         Display(("Invalid Parameters.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function provides a mechanism of sending a Slave    */
   /* Pairing Response to a Master's Pairing Request.                   */
static int SlavePairingRequestResponse(BD_ADDR_t BD_ADDR)
{
   int                                          ret_val;
   BoardStr_t                                   BoardStr;
   GAP_LE_Authentication_Response_Information_t AuthenticationResponseData;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      BD_ADDRToStr(BD_ADDR, BoardStr);
      Display(("Sending Pairing Response to %s.\r\n", BoardStr));

      /* We must be the slave if we have received a Pairing Request     */
      /* thus we will respond with our capabilities.                    */
      AuthenticationResponseData.GAP_LE_Authentication_Type = larPairingCapabilities;
      AuthenticationResponseData.Authentication_Data_Length = GAP_LE_PAIRING_CAPABILITIES_SIZE;

      /* Configure the Application Pairing Parameters.                  */
      ConfigureCapabilities(&(AuthenticationResponseData.Authentication_Data.Pairing_Capabilities));

      /* Attempt to pair to the remote device.                          */
      ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, &AuthenticationResponseData);

      Display(("GAP_LE_Authentication_Response returned %d.\r\n", ret_val));
   }
   else
   {
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* responding to a request for Encryption Information to send to a   */
   /* remote device.                                                    */
static int EncryptionInformationRequestResponse(BD_ADDR_t BD_ADDR, Byte_t KeySize, GAP_LE_Authentication_Response_Information_t *GAP_LE_Authentication_Response_Information)
{
   int    ret_val;
   Word_t LocalDiv;

   /* Make sure a Bluetooth Stack is open.                              */
   if(BluetoothStackID)
   {
      /* Make sure the input parameters are semi-valid.                 */
      if((!COMPARE_NULL_BD_ADDR(BD_ADDR)) && (GAP_LE_Authentication_Response_Information))
      {
         Display(("   Calling GAP_LE_Generate_Long_Term_Key.\r\n"));

         /* Generate a new LTK, EDIV and Rand tuple.                    */
         ret_val = GAP_LE_Generate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.LTK), &LocalDiv, &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.EDIV), &(GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Rand));
         if(!ret_val)
         {
            Display(("   Encryption Information Request Response.\r\n"));

            /* Response to the request with the LTK, EDIV and Rand      */
            /* values.                                                  */
            GAP_LE_Authentication_Response_Information->GAP_LE_Authentication_Type                                     = larEncryptionInformation;
            GAP_LE_Authentication_Response_Information->Authentication_Data_Length                                     = GAP_LE_ENCRYPTION_INFORMATION_DATA_SIZE;
            GAP_LE_Authentication_Response_Information->Authentication_Data.Encryption_Information.Encryption_Key_Size = KeySize;

            ret_val = GAP_LE_Authentication_Response(BluetoothStackID, BD_ADDR, GAP_LE_Authentication_Response_Information);
            if(!ret_val)
            {
               Display(("   GAP_LE_Authentication_Response (larEncryptionInformation) success.\r\n", ret_val));
            }
            else
            {
               DisplayFunctionError("GAP_LE_Authentication_Response", ret_val);
            }
         }
         else
         {
            DisplayFunctionError("GAP_LE_Generate_Long_Term_Key", ret_val);
         }
      }
      else
      {
         Display(("Invalid Parameters.\r\n"));

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      Display(("Stack ID Invalid.\r\n"));

      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for displaying usage        */
   /* information for a command line Alert Categories numerical input   */
   /* field.                                                            */
static void DisplayUsageAlertCategoryNumericalValue(Boolean_t DisplayAllCategories)
{
   unsigned int i;

   Display(("   Where Category is one of the corresponding numerical values:\r\n"));
   Display(("\r\n   Value  Category\r\n   -----  --------\r\n"));

   for(i = 0; i < NUM_ALERT_CATEGORIES; i++)
      Display(("     %u    %s\r\n", i, AlertCategories[i]));

   if(DisplayAllCategories)
      Display(("     %u    %s\r\n", i, AlertCategories[i]));

   Display(("\r\n"));
}

   /* The following function is responsible for displaying usage        */
   /* information for a command line Alert Categories bitmask input     */
   /* field.                                                            */
static void DisplayUsageAlertCategoryBitMask(void)
{
   unsigned int i;

   Display(("   Where setting a bit BitMask enables the category.\r\n"));
   Display(("\r\n   Bit  Category\r\n   ---  --------\r\n"));

   for(i = 0; i < NUM_ALERT_CATEGORIES; i++)
      Display(("     %d  %s\r\n", i, AlertCategories[i]));

   Display(("\r\n"));
}

   /* The following function is responsible for displaying the Supported*/
   /* Categories for a server.  The first parameter is a bitmask        */
   /* defining which categories are supported.  The second parameter is */
   /* a prefix string that will be prepended to each printed line.  The */
   /* last parameter is the category type string that will be displayed */
   /* as part of the information.                                       */
static void DisplaySupportedCategories(Word_t SupportedCategories, char *Prefix, char *CategoryType)
{
   unsigned int i;
   unsigned int j;

   if((Prefix) && (CategoryType))
   {
      Display(("\r\n%sSupported %s Alerts Categories\r\n", Prefix, CategoryType));
      Display(("%s----------------------------", Prefix));

      for(i = 0; i < BTPS_StringLength(CategoryType); i++)
         Display(("-"));
      Display(("\r\n"));

      for(i = 0; i < NUM_ALERT_CATEGORIES; i++)
      {
         Display(("%s   %s: ", Prefix, AlertCategories[i]));

         for(j = 0; j < (BTPS_StringLength(AlertCategories[8]) - BTPS_StringLength(AlertCategories[i])); j++)
            Display((" "));

         Display(("%sSupported\r\n", ((SupportedCategories & (0x1 << i))               ? "" : "Not ")));
      }

      Display(("\r\n"));
   }
}

   /* The following function is responsible for displaying a New Alert  */
   /* Status message.  The first parameter is a valid category          */
   /* identifier.  The second parameter specifies the number of alerts  */
   /* available for this category.  The third parameter is the alert    */
   /* status message to be displayed.  The last parameter is a prefix   */
   /* string that will be prepended to each printed line.               */
static void DisplayNewAlertStatus(ANS_Category_Identification_t Category, Byte_t NumAlerts, char *AlertString, char *Prefix)
{
   /* Validate parameters.                                              */
   if(Prefix)
   {
      Display(("\r\n%sNew Alert Status\r\n", Prefix));
      Display(("%s----------------\r\n", Prefix));
      Display(("%s   Category:         %s\r\n", Prefix, AlertCategories[Category]));
      Display(("%s   Number of Alerts: %u\r\n", Prefix, NumAlerts));

      if(AlertString)
         Display(("%s   Alert String:     '%s'\r\n\r\n", Prefix, AlertString));
   }
}

   /* The following function is responsible for displaying an Unread    */
   /* Alert Status message.  The first parameter is a valid category    */
   /* identifier.  The second parameter specifies the number of alerts  */
   /* available for this category.  The last parameter is a prefix      */
   /* string that will be prepended to each printed line.               */
static void DisplayUnreadAlertStatus(ANS_Category_Identification_t Category, Byte_t NumAlerts, char *Prefix)
{
   /* Validate parameters.                                              */
   if(Prefix)
   {
      Display(("\r\n%sUnread Alert Status\r\n", Prefix));
      Display(("%s----------------\r\n", Prefix));
      Display(("%s   Category:         %s\r\n", Prefix, AlertCategories[Category]));
      Display(("%s   Number of Alerts: %u\r\n\r\n", Prefix, NumAlerts));
   }
}

   /* The following function is responsible for sending a control point */
   /* command to the connected server.  The first parameter specifies   */
   /* what control point command to send.  The caller should also verify*/
   /* that the remote client has registered for New Alert Notifications */
   /* and has enabled all of the categories in the bitmask.  The second */
   /* parameter is the alert category to send as part of the command.   */
   /* This function returns zero on success or a negative value on      */
   /* error.                                                            */
static int WriteControlPointCommand(ANS_Control_Point_Command_t Command, ANS_Category_Identification_t Category)
{
   ANS_Control_Point_Command_Value_t  CommandBuffer;
   DeviceInfo_t                      *DeviceInfo;
   int                                ret_val = FUNCTION_ERROR;

   /* Verify that there is a valid connection                           */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that the client has received a valid Control Point   */
         /* Attribute Handle.                                           */
         if(DeviceInfo->ClientInfo.Control_Point != 0)
         {
            /* Format the command                                       */
            if((ret_val = ANS_Format_Control_Point_Command(&CommandBuffer, Command, Category)) == 0)
            {
               /* Finally, submit a write request to the server.        */
               if((ret_val = GATT_Write_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Control_Point, ANS_CONTROL_POINT_COMMAND_VALUE_DATA_SIZE, ((void *)&CommandBuffer), GATT_ClientEventCallback_ANP, DeviceInfo->ClientInfo.Control_Point)) > 0)
               {
                  ret_val = 0;
               }
               else
                  DisplayFunctionError("GATT_Write_Request", ret_val);
            }
            else
            {
               DisplayFunctionError("ANS_Format_Control_Point_Command", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
            Display(("Error - Control Point not supported on remote service!\r\n"));
      }
      else
         Display(("Error getting device info.\r\n"));
   }
   else
      Display(("Connection is not established.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for sending New Alert       */
   /* Notifications to a connected client.  The first parameter is a    */
   /* bitmask defining which notification categories are to be sent.  It*/
   /* is assumed that only supported notifications are part of the      */
   /* bitmask.  The caller should also verify that the remote client has*/
   /* registered for New Alert Notifications and has enabled all of the */
   /* categories in the bitmask.  The second parameter is a prefix      */
   /* string that will be prepended to each printed line.  This function*/
   /* returns zero on success or a negative value on error.             */
static int SendNewAlertNotification(Word_t BitMask, char *Prefix)
{
   ANS_New_Alert_Data_t NewAlertData;
   int                  ret_val = FUNCTION_ERROR;
   int                  i;

   /* Verify Parameters                                                 */
   if(BitMask && Prefix)
   {
      /* First, check that valid Bluetooth Stack ID exists.             */
      if(BluetoothStackID)
      {
         /* Check for a valid connection.                               */
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            /* Shift through each bit to check if a notification should */
            /* be sent.                                                 */
            i = 0;
            while(BitMask)
            {
               if(BitMask & 0x1)
               {
                  /* Populate the New Alert data to send.               */
                  NewAlertData.CategoryID        = (ANS_Category_Identification_t)i;
                  NewAlertData.NumberOfNewAlerts = NewAlertStatuses[i].NumberOfAlerts;
                  NewAlertData.LastAlertString   = NewAlertStatuses[i].AlertString;

                  /* Go ahead and send the Alert Status.                */
                  Display(("%sSending New Alert Notification for '%s'... ", Prefix, AlertCategories[i]));
                  if((ret_val = ANS_New_Alert_Notification(BluetoothStackID, ANSInstanceID, ConnectionID, &NewAlertData)) == 0)
                  {
                     Display(("Success.\r\n"));
                  }
                  else
                     Display(("Failure.\r\n"));
               }

               BitMask >>= 1;
               i        += 1;
            }

            ret_val = 0;
         }
      }
   }

   return(ret_val);
}

   /* The following function is responsible for sending Unread Alert    */
   /* Notifications to a connected client.  The first parameter is a    */
   /* bitmask defining which notification categories are to be sent.  It*/
   /* is assumed that only supported notifications are part of the      */
   /* bitmask.  The caller should also verify that the remote client has*/
   /* registered for Unread Alert Notifications and has enabled all of  */
   /* the categories in the bitmask.  The second parameter is a prefix  */
   /* string that will be prepended to each printed line.  This function*/
   /* returns zero on success or a negative value on error.             */
static int SendUnreadAlertNotification(Word_t BitMask, char *Prefix)
{
   ANS_Un_Read_Alert_Data_t UnreadAlertData;
   int                      ret_val = FUNCTION_ERROR;
   int                      i;

   /* Verify Parameters                                                 */
   if(BitMask && Prefix)
   {
      /* First, check that valid Bluetooth Stack ID exists.             */
      if(BluetoothStackID)
      {
         /* Check for a valid connection.                               */
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            /* Shift through each bit to check if a notification should */
            /* be sent.                                                 */
            i = 0;
            while(BitMask)
            {
               if(BitMask & 0x1)
               {
                  /* Populate the Unread Alert data to send.            */
                  UnreadAlertData.CategoryID           = (ANS_Category_Identification_t)i;
                  UnreadAlertData.NumberOfUnreadAlerts = UnreadAlertStatuses[i];

                  /* Go ahead and send the Alert Status.                */
                  Display(("%sSending Unread Alert Notification for '%s'... ", Prefix, AlertCategories[i]));
                  if((ret_val = ANS_Un_Read_Alert_Status_Notification(BluetoothStackID, ANSInstanceID, ConnectionID, &UnreadAlertData)) == 0)
                  {
                     Display(("Success.\r\n"));
                  }
                  else
                     Display(("Failure.\r\n"));
               }

               BitMask >>= 1;
               i        += 1;
            }

            ret_val = 0;
         }
      }
   }

   return(ret_val);
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
   Display(("* Command Options General: Help, GetLocalAddress                 *\r\n"));
   Display(("*                          SetDiscoverabilityMode,               *\r\n"));
   Display(("*                          SetConnectabilityMode,                *\r\n"));
   Display(("*                          SetPairabilityMode,                   *\r\n"));
   Display(("*                          ChangePairingParameters,              *\r\n"));
   Display(("*                          AdvertiseLE, StartScanning,           *\r\n"));
   Display(("*                          StopScanning, ConnectLE,              *\r\n"));
   Display(("*                          DisconnectLE, PairLE,                 *\r\n"));
   Display(("*                          LEPasskeyResponse,                    *\r\n"));
   Display(("*                          QueryEncryptionMode, SetPasskey,      *\r\n"));
   Display(("*                          DiscoverGAPS, GetLocalName,           *\r\n"));
   Display(("*                          SetLocalName, GetRemoteName,          *\r\n"));
   Display(("*                          SetLocalAppearance,                   *\r\n"));
   Display(("*                          GetLocalAppearance,                   *\r\n"));
   Display(("*                          GetRemoteAppearance,                  *\r\n"));
   Display(("* Command Options ANP:     RegisterANS, UnregisterANS,           *\r\n"));
   Display(("*                          DiscoverANS, ConfigureRemoteANS,      *\r\n"));
   Display(("*                          NotifyNewAlerts, NotifyUnreadAlerts,  *\r\n"));
   Display(("*                          GetNewAlertStatus, SetNewAlertStatus, *\r\n"));
   Display(("*                          GetUnreadAlertStatus,                 *\r\n"));
   Display(("*                          SetUnreadAlertStatus,                 *\r\n"));
   Display(("*                          GetSupportedNewAlertCategories,       *\r\n"));
   Display(("*                          SetSupportedNewAlertCategories,       *\r\n"));
   Display(("*                          GetSupportedUnreadAlertCategories,    *\r\n"));
   Display(("*                          SetSupportedUnreadAlertCategories,    *\r\n"));
   Display(("*                          EnableNewAlertNotifications,          *\r\n"));
   Display(("*                          DisableNewAlertNotifications,         *\r\n"));
   Display(("*                          EnableUnreadAlertNotifications,       *\r\n"));
   Display(("*                          DisableUnreadAlertNotifications,      *\r\n"));
   Display(("*                          NotifyImmediatelyNewAlert,            *\r\n"));
   Display(("*                          NotifyImmediatelyUnreadAlert          *\r\n"));
   Display(("******************************************************************\r\n"));

   return(0);
}

   /* The following function is responsible for setting the             */
   /* Discoverability Mode of the local device.  This function returns  */
   /* zero on successful execution and a negative value on all errors.  */
static int SetDiscoverabilityMode(ParameterList_t *TempParam)
{
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

         /* Set the LE Discoverability Mode.                            */
         LE_Parameters.DiscoverabilityMode = DiscoverabilityMode;

         /* The Mode was changed successfully.                          */
         Display(("Discoverability: %s.\r\n", (DiscoverabilityMode == dmNonDiscoverableMode)?"Non":((DiscoverabilityMode == dmGeneralDiscoverableMode)?"General":"Limited")));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetDiscoverabilityMode [(0 = Non Discoverable, 1 = Limited Discoverable, 2 = General Discoverable)]");

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
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 1))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         /* * NOTE * The Connectability Mode in LE is only applicable   */
         /*          when advertising, if a device is not advertising   */
         /*          it is not connectable.                             */
         if(TempParam->Params[0].intParam == 0)
            LE_Parameters.ConnectableMode = lcmNonConnectable;
         else
            LE_Parameters.ConnectableMode = lcmConnectable;

         /* The Mode was changed successfully.                          */
         Display(("Connectability Mode: %s.\r\n", (LE_Parameters.ConnectableMode == lcmNonConnectable)?"Non Connectable":"Connectable"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
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
   int                        Result;
   int                        ret_val;
   char                      *Mode;
   GAP_LE_Pairability_Mode_t  PairabilityMode;

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
         {
            PairabilityMode = lpmNonPairableMode;
            Mode            = "lpmNonPairableMode";
         }
         else
         {
            PairabilityMode = lpmPairableMode;
            Mode            = "lpmPairableMode";
         }

         /* Parameters mapped, now set the Pairability Mode.            */
         Result = GAP_LE_Set_Pairability_Mode(BluetoothStackID, PairabilityMode);

         /* Next, check the return value to see if the command was      */
         /* issued successfully.                                        */
         if(Result >= 0)
         {
            /* The Mode was changed successfully.                       */
            Display(("Pairability Mode Changed to %s.\r\n", Mode));

            /* If Secure Simple Pairing has been enabled, inform the    */
            /* user of the current Secure Simple Pairing parameters.    */
            if(PairabilityMode == lpmPairableMode)
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
         DisplayUsage("SetPairabilityMode [(0 = Non Pairable, 1 = Pairable]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      Display(("Invalid Stack ID.\r\n"));

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
static int ChangePairingParameters(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if((TempParam) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam <= 4))
      {
         /* Parameters appear to be valid, map the specified parameters */
         /* into the API specific parameters.                           */
         if(TempParam->Params[0].intParam == 0)
            LE_Parameters.IOCapability = licDisplayOnly;
         else
         {
            if(TempParam->Params[0].intParam == 1)
               LE_Parameters.IOCapability = licDisplayYesNo;
            else
            {
               if(TempParam->Params[0].intParam == 2)
                  LE_Parameters.IOCapability = licKeyboardOnly;
               else
               {
                  if(TempParam->Params[0].intParam == 3)
                     LE_Parameters.IOCapability = licNoInputNoOutput;
                  else
                     LE_Parameters.IOCapability = licKeyboardDisplay;
               }
            }
         }

         /* Finally map the Man in the Middle (MITM) Protection value.  */
         LE_Parameters.MITMProtection = (Boolean_t)(TempParam->Params[1].intParam?TRUE:FALSE);

         /* Inform the user of the New I/O Capabilities.                */
         DisplayIOCapabilities();

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("ChangePairingParameters [I/O Capability (0 = Display Only, 1 = Display Yes/No, 2 = Keyboard Only, 3 = No Input/Output, 4 = Keyboard/Display)] [MITM Requirement (0 = No, 1 = Yes)]");

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
   /* Authentication Response with a Pass Key value specified via the   */
   /* input parameter.  This function returns zero on successful        */
   /* execution and a negative value on all errors.                     */
static int LEPassKeyResponse(ParameterList_t *TempParam)
{
   int                              Result;
   int                              ret_val;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(CurrentRemoteBD_ADDR))
      {
         /* Make sure that all of the parameters required for this      */
         /* function appear to be at least semi-valid.                  */
         if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS))
         {
            /* Parameters appear to be valid, go ahead and populate the */
            /* response structure.                                      */
            GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type  = larPasskey;
            GAP_LE_Authentication_Response_Information.Authentication_Data_Length  = (Byte_t)(sizeof(DWord_t));
            GAP_LE_Authentication_Response_Information.Authentication_Data.Passkey = (DWord_t)(TempParam->Params[0].intParam);

            /* Submit the Authentication Response.                      */
            Result = GAP_LE_Authentication_Response(BluetoothStackID, CurrentRemoteBD_ADDR, &GAP_LE_Authentication_Response_Information);

            /* Check the return value for the submitted command for     */
            /* success.                                                 */
            if(!Result)
            {
               /* Operation was successful, inform the user.            */
               Display(("Passkey Response Success."));

               /* Flag success to the caller.                           */
               ret_val = 0;
            }
            else
            {
               /* Inform the user that the Authentication Response was  */
               /* not successful.                                       */
               DisplayFunctionError("GAP_LE_Authentication_Response", Result);

               ret_val = FUNCTION_ERROR;
            }

            /* Flag that there is no longer a current Authentication    */
            /* procedure in progress.                                   */
            ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
         }
         else
         {
            /* One or more of the necessary parameters is/are invalid.  */
            Display(("PassKeyResponse [Numeric Passkey(0 - 999999)].\r\n"));

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
   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static int LEQueryEncryption(ParameterList_t *TempParam)
{
   int                   ret_val;
   GAP_Encryption_Mode_t GAP_Encryption_Mode;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* First, check to see if there is an on-going Pairing operation  */
      /* active.                                                        */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         /* Query the current Encryption Mode for this Connection.      */
         ret_val = GAP_LE_Query_Encryption_Mode(BluetoothStackID, ConnectionBD_ADDR, &GAP_Encryption_Mode);
         if(!ret_val)
            Display(("Current Encryption Mode: %s.\r\n", (GAP_Encryption_Mode == emEnabled)?"Enabled":"Disabled"));
         else
         {
            DisplayFunctionError("GAP_LE_Query_Encryption_Mode", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* There is not currently an on-going authentication operation,*/
         /* inform the user of this error condition.                    */
         Display(("Not Connected.\r\n"));

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

   /* The following function is responsible for querying the Encryption */
   /* Mode for an LE Connection.  This function returns zero on         */
   /* successful execution and a negative value on all errors.          */
static int LESetPasskey(ParameterList_t *TempParam)
{
   int     ret_val;
   DWord_t Passkey;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this         */
      /* function appear to be at least semi-valid.                     */
      if((TempParam) && (TempParam->NumberofParameters >= 1) && ((TempParam->Params[0].intParam == 0) || (TempParam->Params[0].intParam == 1)))
      {
         if(TempParam->Params[0].intParam == 1)
         {
            /* We are setting the passkey so make sure it is valid.     */
            if(BTPS_StringLength(TempParam->Params[1].strParam) <= GAP_LE_PASSKEY_MAXIMUM_NUMBER_OF_DIGITS)
            {
               Passkey = (DWord_t)(TempParam->Params[1].intParam);

               ret_val = GAP_LE_Set_Fixed_Passkey(BluetoothStackID, &Passkey);
               if(!ret_val)
                  Display(("Fixed Passkey set to %u.\r\n", Passkey));
            }
            else
            {
               Display(("Error - Invalid Passkey.\r\n"));

               ret_val = INVALID_PARAMETERS_ERROR;
            }
         }
         else
         {
            /* Un-set the fixed passkey that we previously configured.  */
            ret_val = GAP_LE_Set_Fixed_Passkey(BluetoothStackID, NULL);
            if(!ret_val)
               Display(("Fixed Passkey no longer configured.\r\n"));
         }

         /* If GAP_LE_Set_Fixed_Passkey returned an error display this. */
         if((ret_val) && (ret_val != INVALID_PARAMETERS_ERROR))
         {
            DisplayFunctionError("GAP_LE_Set_Fixed_Passkey", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         /* One or more of the necessary parameters is/are invalid.     */
         DisplayUsage("SetPasskey [(0 = UnSet Passkey, 1 = Set Fixed Passkey)] [6 Digit Passkey (optional)]");

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
   /* Device Address of the local Bluetooth Device.  This function      */
   /* returns zero on successful execution and a negative value on all  */
   /* errors.                                                           */
static int GetLocalAddress(ParameterList_t *TempParam)
{
   int        Result;
   int        ret_val;
   BD_ADDR_t  BD_ADDR;
   BoardStr_t BoardStr;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Attempt to submit the command.                                 */
      Result = GAP_Query_Local_BD_ADDR(BluetoothStackID, &BD_ADDR);

      /* Check the return value of the submitted command for success.   */
      if(!Result)
      {
         BD_ADDRToStr(BD_ADDR, BoardStr);

         Display(("BD_ADDR of Local Device is: %s.\r\n", BoardStr));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
      else
      {
         /* Display a message indicating that an error occured while    */
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

   /* The following thread is responsible for checking changing the     */
   /* current Baud Rate used to talk to the Radio.                      */
   /* * NOTE * This function ONLY configures the Baud Rate for a TI     */
   /*          Bluetooth chipset.                                       */
static int SetBaudRate(ParameterList_t *TempParam)
{
   int ret_val;

   /* First check to see if the parameters required for the execution of*/
   /* this function appear to be semi-valid.                            */
   if(BluetoothStackID)
   {
      /* Next check to see if the parameters required for the execution */
      /* of this function appear to be semi-valid.                      */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam))
      {
         /* Next, write the command to the device.                      */
         ret_val = VS_Update_UART_Baud_Rate(BluetoothStackID, (DWord_t)TempParam->Params[0].intParam);
         if(!ret_val)
         {
            Display(("VS_Update_UART_Baud_Rate(%u): Success.\r\n", TempParam->Params[0].intParam));
         }
         else
         {
            /* Unable to write vendor specific command to chipset.      */
            Display(("VS_Update_UART_Baud_Rate(%u): Failure %d, %d.\r\n", TempParam->Params[0].intParam, ret_val));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         DisplayUsage("SetBaudRate [BaudRate]");

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
   {
      /* One or more of the necessary parameters are invalid.           */
      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for enabling LE             */
   /* Advertisements.  This function returns zero on successful         */
   /* execution and a negative value on all errors.                     */
static int AdvertiseLE(ParameterList_t *TempParam)
{
   int                                 ret_val = 0;
   int                                 Length;
   GAP_LE_Advertising_Parameters_t     AdvertisingParameters;
   GAP_LE_Connectability_Parameters_t  ConnectabilityParameters;
   GAP_LE_Address_Type_t               OwnAddressType = latPublic;
   BD_ADDR_t                           BD_ADDR;
   Byte_t                              StatusResult;
   union
   {
      Advertising_Data_t               AdvertisingData;
      Scan_Response_Data_t             ScanResponseData;
   } Advertisement_Data_Buffer;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Make sure that all of the parameters required for this function*/
      /* appear to be at least semi-valid.                              */
      if(TempParam && (TempParam->Params[0].strParam)) 
      {
         if  ((TempParam->NumberofParameters >= 1) && ((TempParam->Params[0].intParam == 0) || (TempParam->Params[0].intParam == 1)))
         {
      
             /* Determine whether to enable or disable Advertising.         */
             if(TempParam->Params[0].intParam == 0)
             {
                /* Disable Advertising.                                     */
                ret_val = GAP_LE_Advertising_Disable(BluetoothStackID);
                if(!ret_val)
                {
                   Display(("   GAP_LE_Advertising_Disable success.\r\n"));
                }
                else
                {
                   DisplayFunctionError("GAP_LE_Advertising_Disable", ret_val);

                   ret_val = FUNCTION_ERROR;
                }
             }
             else
             {
                /* Verifying enable Advertising.parameters                  */
                if(TempParam->NumberofParameters >= 2) 
				{
                    if (TempParam->Params[1].intParam == 1)
                    {
                       OwnAddressType = latRandom;
                       if (BTPS_StringLength(TempParam->Params[2].strParam) >= (sizeof(BD_ADDR_t)*2))
    		           {
    		                /* Convert the parameter to a Bluetooth Device Address.        */
                       	        StrToBD_ADDR(TempParam->Params[2].strParam, &BD_ADDR);
    	  	           }
    		           else
    		   	           ret_val = INVALID_PARAMETERS_ERROR;
                    }   
                    else 
                       if ((TempParam->Params[1].intParam != 0) || (TempParam->NumberofParameters != 2))
      		   	           ret_val = INVALID_PARAMETERS_ERROR;
	            }
                if (!ret_val)
                {
                    /* Enable Advertising.                                      */
                    /* Set the Advertising Data.                                */
                    BTPS_MemInitialize(&(Advertisement_Data_Buffer.AdvertisingData), 0, sizeof(Advertising_Data_t));

                    /* Set the Flags A/D Field (1 byte type and 1 byte Flags.   */
                    Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] = 2;
                    Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_FLAGS;

                    /* Configure the flags field based on the Discoverability   */
                    /* Mode.                                                    */
                    if(LE_Parameters.DiscoverabilityMode == dmGeneralDiscoverableMode)
                       Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_GENERAL_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                    else
                    {
                       if(LE_Parameters.DiscoverabilityMode == dmLimitedDiscoverableMode)
                          Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[2] = HCI_LE_ADVERTISING_FLAGS_LIMITED_DISCOVERABLE_MODE_FLAGS_BIT_MASK;
                    }

                    if(ANSInstanceID)
                    {
                       /* Advertise the Health Thermometer Server (1 byte type  */
                       /* and 2 bytes UUID.                                     */
                       Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] = 3;
                       Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[4] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_16_BIT_SERVICE_UUID_COMPLETE;
                       ANS_ASSIGN_ANS_SERVICE_UUID_16(&(Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[5]));
                    }

                    /* Write the advertising data to the chip.                  */
                    ret_val = GAP_LE_Set_Advertising_Data(BluetoothStackID, (Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[0] + Advertisement_Data_Buffer.AdvertisingData.Advertising_Data[3] + 2), &(Advertisement_Data_Buffer.AdvertisingData));
                    if(!ret_val)
                    {
                       BTPS_MemInitialize(&(Advertisement_Data_Buffer.ScanResponseData), 0, sizeof(Scan_Response_Data_t));

                       /* Set the Scan Response Data.                           */
                       Length = BTPS_StringLength(LE_APP_DEMO_NAME);
                       if(Length < (ADVERTISING_DATA_MAXIMUM_SIZE - 2))
                       {
                          Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_COMPLETE;
                       }
                       else
                       {
                          Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[1] = HCI_LE_ADVERTISING_REPORT_DATA_TYPE_LOCAL_NAME_SHORTENED;
                          Length = (ADVERTISING_DATA_MAXIMUM_SIZE - 2);
                       }

                       Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] = (Byte_t)(1 + Length);
                       BTPS_MemCopy(&(Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[2]),LE_APP_DEMO_NAME,Length);

                       ret_val = GAP_LE_Set_Scan_Response_Data(BluetoothStackID, (Advertisement_Data_Buffer.ScanResponseData.Scan_Response_Data[0] + 1), &(Advertisement_Data_Buffer.ScanResponseData));
                       if(!ret_val)
                       {
                          /* Set up the advertising parameters.                 */
                          AdvertisingParameters.Advertising_Channel_Map   = HCI_LE_ADVERTISING_CHANNEL_MAP_DEFAULT;
                          AdvertisingParameters.Scan_Request_Filter       = fpNoFilter;
                          AdvertisingParameters.Connect_Request_Filter    = fpNoFilter;
                          AdvertisingParameters.Advertising_Interval_Min  = 100;
                          AdvertisingParameters.Advertising_Interval_Max  = 200;

                          /* Configure the Connectability Parameters.           */
                          /* * NOTE * Since we do not ever put ourselves to be  */
                          /*          direct connectable then we will set the   */
                          /*          DirectAddress to all 0s.                  */
                          ConnectabilityParameters.Connectability_Mode   = LE_Parameters.ConnectableMode;
                          ConnectabilityParameters.Own_Address_Type      = OwnAddressType;
                          ConnectabilityParameters.Direct_Address_Type   = latPublic;
                          ASSIGN_BD_ADDR(ConnectabilityParameters.Direct_Address, 0, 0, 0, 0, 0, 0);

                         /*If its a Random Adrees if So set the Random address first   */                
                        if((OwnAddressType) && (!ret_val))
                        {
                            ret_val = HCI_LE_Set_Random_Address(BluetoothStackID, BD_ADDR, &StatusResult);
                            ret_val += StatusResult;
                        }

                         /* Now enable advertising.                         */
                        if(!ret_val)
                        {
                             ret_val = GAP_LE_Advertising_Enable(BluetoothStackID, TRUE, &AdvertisingParameters, &ConnectabilityParameters, GAP_LE_Event_Callback, 0);
                             if(!ret_val)
                             {
                                Display(("   GAP_LE_Advertising_Enable success.\r\n"));
                             }
                             else
                             {
                                DisplayFunctionError("GAP_LE_Advertising_Enable", ret_val);

                                ret_val = FUNCTION_ERROR;
                             }
                        }
                        else
                        {
                            DisplayFunctionError("HCI_LE_Set_Random_Address", ret_val);
                            ret_val = FUNCTION_ERROR;
                        }

                      }
                      else
                      {
                         DisplayFunctionError("GAP_LE_Set_Advertising_Data(dtScanResponse)", ret_val);

                         ret_val = FUNCTION_ERROR;
                      }

                   }
                   else
                   {
                      DisplayFunctionError("GAP_LE_Set_Advertising_Data(dtAdvertising)", ret_val);

                      ret_val = FUNCTION_ERROR;
                   }
                }
                else
                {
                   DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)]");
                   DisplayUsage("AdvertiseLE [1 = Enable] [0 = Public Address]");
                   DisplayUsage("AdvertiseLE [1 = Enable] [1 = Random Address] [Random BD Address]");
                   ret_val = INVALID_PARAMETERS_ERROR;
                }
             }
         }
         else
         {
             DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)]");
             DisplayUsage("AdvertiseLE [1 = Enable] [0 = Public Address]");
             DisplayUsage("AdvertiseLE [1 = Enable] [1 = Random Address] [Random BD Address]");
             ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
          DisplayUsage("AdvertiseLE [0 = Disable, 1 = Enable(Default - Public Address)]");
          DisplayUsage("AdvertiseLE [1 = Enable] [0 = Public Address]");
          DisplayUsage("AdvertiseLE [1 = Enable] [1 = Random Address] [Random BD Address]");
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


   /* The following function is responsible for starting an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int StartScanning(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply start scanning.                                         */
      if(!StartScan(BluetoothStackID))
         ret_val = 0;
      else
         ret_val = FUNCTION_ERROR;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for stopping an LE scan     */
   /* procedure.  This function returns zero if successful and a        */
   /* negative value if an error occurred.                              */
static int StopScanning(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Simply stop scanning.                                          */
      if(!StopScan(BluetoothStackID))
         ret_val = 0;
      else
         ret_val = FUNCTION_ERROR;
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for connecting to an LE     */
   /* device. This function need to receive at least a parameter of     */
   /* BD_ADDRESS [0xBD_Addr or BD_Addr], the next two parameters are    */    
   /* Optional Remote Device Address Type and Own Address Type, The     */  
   /* default value for this parameters is Public address type.  This   */
   /* function returns zero if successful and a negative value if an    */
   /* error occurred.                                                   */
static int ConnectLE(ParameterList_t *TempParam)
{
   int                   ret_val;
   BD_ADDR_t             BD_ADDR;
   GAP_LE_Address_Type_t OwnAddressType = latPublic;
   GAP_LE_Address_Type_t RemoteAddressType = latPublic;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, make sure that a valid device address exists.            */
      if (TempParam && (BTPS_StringLength(TempParam->Params[0].strParam) >= (sizeof(BD_ADDR_t)*2)) && (TempParam->Params[0].strParam))
      {
          StrToBD_ADDR(TempParam->Params[0].strParam, &BD_ADDR);
          switch(TempParam->NumberofParameters)
          {
              case 3:
                      if ((TempParam->Params[2].intParam >= 0) && (TempParam->Params[2].intParam <= 1))
                          OwnAddressType  = (TempParam->Params[2].intParam == 0) ?latPublic :latRandom;
                      else
                      {
                          ret_val = INVALID_PARAMETERS_ERROR;	
                          break;
                      }
              case 2:
                      if ((TempParam->Params[1].intParam >= 0) && (TempParam->Params[1].intParam <= 1))
                      {
                          if (TempParam->Params[1].intParam  == 1)
                          {
                              RemoteAddressType  = latRandom;
                              ResolveRemoteAddressHelper(BD_ADDR);
                          }
                      }
                      else
                      {
                          ret_val = INVALID_PARAMETERS_ERROR;
                          break;
                      }
              case 1:
                      if(!ConnectLEDevice(BluetoothStackID, BD_ADDR, RemoteAddressType, OwnAddressType, FALSE))
                          ret_val = 0;
                      else
                          ret_val = FUNCTION_ERROR;
                      break;
              default:
                      ret_val = INVALID_PARAMETERS_ERROR;
                      break;
          }
          if (ret_val)
          {
              /* Invalid parameters specified so flag an error to the user.  */
              DisplayConnectLEUsage("ConnectLE");
              /* Flag that an error occurred while submitting the command.   */
              ret_val = INVALID_PARAMETERS_ERROR;
          }
      }
      else
      {
          /* Invalid parameters specified so flag an error to the user.  */
          DisplayConnectLEUsage("ConnectLE");
          /* Flag that an error occurred while submitting the command.   */
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

   /* The following function is responsible for disconnecting to an LE  */
   /* device.  This function returns zero if successful and a negative  */
   /* value if an error occurred.                                       */
static int DisconnectLE(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         if(!DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR))
            ret_val = 0;
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         Display(("Device is not connected.\r\n"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is provided to allow a mechanism of        */
   /* Pairing (or requesting security if a slave) to the connected      */
   /* device.                                                           */
static int PairLE(ParameterList_t *TempParam)
{
   int ret_val;

   /* First, check that valid Bluetooth Stack ID exists.                */
   if(BluetoothStackID)
   {
      /* Next, check to make sure we are currently connected.           */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
      {
         if(!SendPairingRequest(ConnectionBD_ADDR, LocalDeviceIsMaster))
         {
            Display(("Pairing request sent.\r\n"));

            ret_val = 0;
         }
         else
            ret_val = FUNCTION_ERROR;
      }
      else
      {
         Display(("Device is not connected.\r\n"));

         /* Flag success to the caller.                                 */
         ret_val = 0;
      }
   }
   else
   {
      /* No valid Bluetooth Stack ID exists.                            */
      ret_val = INVALID_STACK_ID_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for performing a GAP Service*/
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverGAPS(ParameterList_t *TempParam)
{
   int           ret_val;
   GATT_UUID_t   UUID[1];
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that no service discovery is outstanding for this    */
         /* device.                                                     */
         if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
         {
            /* Configure the filter so that only the GAP Service is     */
            /* discovered.                                              */
            UUID[0].UUID_Type = guUUID_16;
            GAP_ASSIGN_GAP_SERVICE_UUID_16(UUID[0].UUID.UUID_16);

            /* Start the service discovery process.                     */
            ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdGAPS);
            if(!ret_val)
            {
               /* Display success message.                              */
               Display(("GATT_Start_Service_Discovery() success.\r\n"));

               /* Flag that a Service Discovery Operation is            */
               /* outstanding.                                          */
               DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
            }
            else
            {
               /* An error occur so just clean-up.                      */
               DisplayFunctionError("GATT_Start_Service_Discovery()", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            Display(("Service Discovery Operation Outsanding for Device.\r\n"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         Display(("No Device Info.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("No Connection Established\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static int GetLocalName(ParameterList_t *TempParam)
{
   int  ret_val;
   char NameBuffer[BTPS_CONFIGURATION_GAPS_MAXIMUM_SUPPORTED_DEVICE_NAME+1];

   /* Verify that the GAP Service is registered.                        */
   if(GAPSInstanceID)
   {
      /* Initialize the Name Buffer to all ZEROs.                       */
      BTPS_MemInitialize(NameBuffer, 0, sizeof(NameBuffer));

      /* Query the Local Name.                                          */
      ret_val = GAPS_Query_Device_Name(BluetoothStackID, GAPSInstanceID, NameBuffer);
      if(!ret_val)
         Display(("Device Name: %s.\r\n", NameBuffer));
      else
      {
         DisplayFunctionError("GAPS_Query_Device_Name", ret_val);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("GAP Service not registered.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the current     */
   /* Local Device Name.  This function will return zero on successful  */
   /* execution and a negative value on errors.                         */
static int SetLocalName(ParameterList_t *TempParam)
{
   int  ret_val;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) > 0) && (BTPS_StringLength(TempParam->Params[0].strParam) <= BTPS_CONFIGURATION_GAPS_MAXIMUM_SUPPORTED_DEVICE_NAME))
   {
      /* Verify that the GAP Service is registered.                     */
      if(GAPSInstanceID)
      {
         /* Query the Local Name.                                       */
         ret_val = GAPS_Set_Device_Name(BluetoothStackID, GAPSInstanceID, TempParam->Params[0].strParam);
         if(!ret_val)
            Display(("GAPS_Set_Device_Name success.\r\n"));
         else
         {
            DisplayFunctionError("GAPS_Set_Device_Name", ret_val);

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         Display(("GAP Service not registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      DisplayUsage("SetLocalName [NameString]");

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int ReadRemoteName(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceNameHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->GAPSClientInfo.DeviceNameHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceNameHandle);
            if(ret_val > 0)
            {
               Display(("Attempting to read Remote Device Name.\r\n"));

               ret_val = 0;
            }
            else
            {
               DisplayFunctionError("GATT_Read_Value_Request", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            Display(("GAP Service Device Name Handle not discovered.\r\n"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         Display(("No Device Info.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("No Connection Established\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static int GetLocalAppearance(ParameterList_t *TempParam)
{
   int     ret_val;
   char   *AppearanceString;
   Word_t  Appearance;

   /* Verify that the GAP Service is registered.                        */
   if(GAPSInstanceID)
   {
      /* Query the Local Name.                                          */
      ret_val = GAPS_Query_Device_Appearance(BluetoothStackID, GAPSInstanceID, &Appearance);
      if(!ret_val)
      {
         /* Map the Appearance to a String.                             */
         if(AppearanceToString(Appearance, &AppearanceString))
            Display(("Device Appearance: %s(%u).\r\n", AppearanceString, Appearance));
         else
            Display(("Device Appearance: Unknown(%u).\r\n", Appearance));
      }
      else
      {
         DisplayFunctionError("GAPS_Query_Device_Appearance", ret_val);

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("GAP Service not registered.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for setting the Local Device*/
   /* Appearance value.  This function will return zero on successful   */
   /* execution and a negative value on errors.                         */
static int SetLocalAppearance(ParameterList_t *TempParam)
{
   int    ret_val;
   Word_t Appearance;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= 0) && (TempParam->Params[0].intParam < NUMBER_OF_APPEARANCE_MAPPINGS))
   {
      /* Verify that the GAP Service is registered.                     */
      if(GAPSInstanceID)
      {
         /* Map the Appearance Index to the GAP Appearance Value.       */
         if(AppearanceIndexToAppearance(TempParam->Params[0].intParam, &Appearance))
         {
            /* Set the Local Appearance.                                */
            ret_val = GAPS_Set_Device_Appearance(BluetoothStackID, GAPSInstanceID, Appearance);
            if(!ret_val)
               Display(("GAPS_Set_Device_Appearance success.\r\n"));
            else
            {
               DisplayFunctionError("GAPS_Set_Device_Appearance", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            Display(("Invalid Appearance Index.\r\n"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         Display(("GAP Service not registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      DisplayUsage("SetLocalName [Index]");
      Display(("Where Index = \r\n"));
      DumpAppearanceMappings();

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Device Name */
   /* for the currently connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int GetRemoteAppearance(ParameterList_t *TempParam)
{
   int           ret_val;
   DeviceInfo_t *DeviceInfo;

   /* Verify that there is a connection that is established.            */
   if(ConnectionID)
   {
      /* Get the device info for the connection device.                 */
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         /* Verify that we discovered the Device Name Handle.           */
         if(DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
         {
            /* Attempt to read the remote device name.                  */
            ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle, GATT_ClientEventCallback_GAPS, (unsigned long)DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle);
            if(ret_val > 0)
            {
               Display(("Attempting to read Remote Device Appearance.\r\n"));

               ret_val = 0;
            }
            else
            {
               DisplayFunctionError("GATT_Read_Value_Request", ret_val);

               ret_val = FUNCTION_ERROR;
            }
         }
         else
         {
            Display(("GAP Service Device Appearance Handle not discovered.\r\n"));

            ret_val = FUNCTION_ERROR;
         }
      }
      else
      {
         Display(("No Device Info.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("No Connection Established\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for registering a ANP       */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int RegisterANS(ParameterList_t *TempParam)
{
   int ret_val;

   /* Verify that there is no active connection.                        */
   if(!ConnectionID)
   {
      /* Verify that the Service is not already registered.             */
      if(!ANSInstanceID)
      {
         /* Register the ANP Service with GATT.                         */
         ret_val = ANS_Initialize_Service(BluetoothStackID, ANS_EventCallback, 0, &ANSInstanceID);
         if((ret_val > 0) && (ANSInstanceID > 0))
         {
            /* Display success message.                                 */
            Display(("Successfully registered ANP Service.\r\n"));

            /* Save the ServiceID of the registered service.            */
            ANSInstanceID = (unsigned int)ret_val;

            /* Initialize internal ANS variables                        */
            BTPS_MemInitialize(NewAlertStatuses, 0, sizeof(NewAlertStatuses));
            BTPS_MemInitialize(UnreadAlertStatuses, 0, sizeof(UnreadAlertStatuses));

            /* Set the supported categories                             */
            ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, SUPPORTED_CATEGORIES_MASK);
            ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, SUPPORTED_CATEGORIES_MASK);

            /* Return success to the caller.                            */
            ret_val        = 0;
         }
      }
      else
      {
         Display(("ANP Service already registered.\r\n"));

         ret_val = FUNCTION_ERROR;
      }
   }
   else
   {
      Display(("Connection current active.\r\n"));

      ret_val = FUNCTION_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for unregistering a ANP     */
   /* Service.  This function will return zero on successful execution  */
   /* and a negative value on errors.                                   */
static int UnregisterANS(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that a service is registered.                              */
   if(ANSInstanceID)
   {
      /* If there is a connected device, then first disconnect it.      */
      if(!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR))
         DisconnectLEDevice(BluetoothStackID, ConnectionBD_ADDR);

      /* Unregister the ANP Service with GATT.                          */
      ret_val = ANS_Cleanup_Service(BluetoothStackID, ANSInstanceID);
      if(ret_val == 0)
      {
         /* Display success message.                                    */
         Display(("Successfully unregistered ANP Service.\r\n"));

         /* Clear the ServiceID of the registered service.              */
         ANSInstanceID = 0;
      }
      else
         DisplayFunctionError("ANS_Cleanup_Service", ret_val);
   }
   else
      Display(("ANP Service not registered.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for performing a ANP        */
   /* Service Discovery Operation.  This function will return zero on   */
   /* successful execution and a negative value on errors.              */
static int DiscoverANS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   GATT_UUID_t   UUID[1];
   int           ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Verify that there is a connection that is established.         */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that no service discovery is outstanding for this */
            /* device.                                                  */
            if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING))
            {
               /* Configure the filter so that only the ANP Service is  */
               /* discovered.                                           */
               UUID[0].UUID_Type = guUUID_16;
               ANS_ASSIGN_ANS_SERVICE_UUID_16(&(UUID[0].UUID.UUID_16));

               /* Start the service discovery process.                  */
               ret_val = GATT_Start_Service_Discovery(BluetoothStackID, ConnectionID, (sizeof(UUID)/sizeof(GATT_UUID_t)), UUID, GATT_Service_Discovery_Event_Callback, sdANS);
               if(!ret_val)
               {
                  /* Display success message.                           */
                  Display(("GATT_Start_Service_Discovery() success.\r\n"));

                  /* Flag that a Service Discovery Operation is         */
                  /* outstanding.                                       */
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;
               }
               else
                  DisplayFunctionError("GATT_Start_Service_Discovery()", ret_val);
            }
            else
               Display(("Service Discovery Operation Outsanding for Device.\r\n"));
         }
         else
            Display(("No Device Info.\r\n"));
      }
      else
         Display(("No Connection Established\r\n"));
   }
   else
      Display(("Cannot discover ANP Services when registered as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for configure a ANP Service */
   /* on a remote device.  This function will return zero on successful */
   /* execution and a negative value on errors.                         */
static int ConfigureRemoteANS(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if(TempParam && (TempParam->NumberofParameters > 1))
   {
      /* Verify that we are not configured as a server                  */
      if(!ANSInstanceID)
      {
         /* Verify that there is a connection that is established.      */
         if(ConnectionID)
         {
            /* Get the device info for the connection device.           */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Determine if service discovery has been performed on  */
               /* this device                                           */
               if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE)
               {
                  ret_val = 0;

                  Display(("Attempting to configure CCCDs...\r\n"));

                  /* Determine if New Alert CC is supported (mandatory).*/
                  if(DeviceInfo->ClientInfo.New_Alert_Client_Configuration)
                  {
                     ret_val = EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.New_Alert_Client_Configuration, (TempParam->Params[0].intParam ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE : 0), GATT_ClientEventCallback_ANP);
                  }
                  else
                     Display(("   Error - New Alert CC not found on this device.\r\n"));

                  if(ret_val > 0)
                  {

                     /* Determine if Unread Alert Status CC is supported*/
                     /* (mandatory).                                    */
                     if(DeviceInfo->ClientInfo.Unread_Alert_Status_Client_Configuration)
                     {
                        ret_val = EnableDisableNotificationsIndications(DeviceInfo->ClientInfo.Unread_Alert_Status_Client_Configuration, (TempParam->Params[1].intParam ? GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE : 0), GATT_ClientEventCallback_ANP);
                     }
                     else
                     {
                        Display(("   Error - Unread Alert Status CC not found on this device.\r\n"));

                        ret_val = 0;
                     }
                  }

                  /* Check for CC Configuration success                 */
                  if(ret_val > 0)
                  {
                     Display(("CCCD Configuration Success.\r\n"));

                     ret_val = 0;
                  }
                  else
                  {
                     /* CC Configuration failed, check to see if it was */
                     /* from a call to                                  */
                     /* EnableDisableNotificationsIndications           */
                     if(ret_val < 0)
                     {
                        DisplayFunctionError("EnableDisableNotificationsIndications", ret_val);
                     }

                     ret_val = FUNCTION_ERROR;
                  }
               }
               else
                  Display(("Service discovery has not been performed on this device.\r\n"));
            }
            else
               Display(("No Device Info.\r\n"));
         }
         else
            Display(("No Connection Established.\r\n"));
      }
      else
         Display(("Cannot configure remote ANP Services when registered as a service.\r\n"));
   }
   else
   {
      Display(("Usage: ConfigureRemoteANS [New Alert Notify (0 = disable, 1 = enable)]\r\n"));
      Display(("                          [Unread Alert Status Notify (0 = disable, 1 = enable)]\r\n"));
   }

   return(ret_val);
}

   /* The following function is responsible for performing a New Alert  */
   /* notification to a connected remote device.  This function will    */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int NotifyNewAlerts(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  i;
   Word_t        EnabledNotifications;
   Word_t        DesiredNotifications;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam) && ((TempParam->Params[0].intParam & (~SUPPORTED_CATEGORIES_MASK)) == 0))
   {
      /* Verify that we have an open server.                            */
      if(ANSInstanceID)
      {
         /* Verify that we have a valid connection.                     */
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that the client has registered for New Alert   */
               /* notifications.                                        */
               if(DeviceInfo->ServerInfo.New_Alert_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
               {
                  /* Verify that the client has enabled the requested   */
                  /* notification categories.                           */
                  EnabledNotifications = DeviceInfo->ServerInfo.Enabled_New_Alert_Categories;
                  DesiredNotifications = TempParam->Params[0].intParam;
                  ret_val              = 0;
                  i                    = 0;

                  while(DesiredNotifications)
                  {
                     if(DesiredNotifications & 0x1)
                     {
                        if(!(EnabledNotifications & 0x1))
                        {
                           Display(("Error - Client has not enabled '%s' for New Alert notifications.\r\n", AlertCategories[i]));

                           ret_val = INVALID_PARAMETERS_ERROR;
                        }
                     }

                     EnabledNotifications >>= 1;
                     DesiredNotifications >>= 1;
                     i                     += 1;
                  }

                  /* Send the notifications if all requested categories */
                  /* are enabled.                                       */
                  if(ret_val == 0)
                  {
                     SendNewAlertNotification(TempParam->Params[0].intParam, "");
                  }
               }
               else
               {
                  Display(("Client has not registered for New Alert notifications.\r\n"));

                  ret_val = 0;
               }
            }
            else
               Display(("Error - Unknown Client.\r\n"));
         }
         else
            Display(("Connection not established.\r\n"));
      }
      else
      {
         if(ConnectionID)
            Display(("Error - Only a server can notify.\r\n"));
         else
            Display(("Error - ANP server not registered\r\n"));
      }
   }
   else
   {
      DisplayUsage("NotifyNewAlerts [BitMask (0xXXXX)]");
      DisplayUsageAlertCategoryBitMask();

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for performing an Unread    */
   /* Alert Status notification to a connected remote device.  This     */
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int NotifyUnreadAlerts(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  i;
   Word_t        EnabledNotifications;
   Word_t        DesiredNotifications;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && ((TempParam->Params[0].intParam & (~SUPPORTED_CATEGORIES_MASK)) == 0))
   {
      /* Verify that we have an open server.                            */
      if(ANSInstanceID)
      {
         /* Verify that we have a valid connection.                     */
         if((ConnectionID) && (!COMPARE_NULL_BD_ADDR(ConnectionBD_ADDR)))
         {
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
            {
               /* Verify that the client has registered for Unread Alert*/
               /* notifications.                                        */
               if(DeviceInfo->ServerInfo.Unread_Alert_Status_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
               {
                  /* Verify that the client has enabled the requested   */
                  /* notification categories.                           */
                  EnabledNotifications = DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories;
                  DesiredNotifications = TempParam->Params[0].intParam;
                  ret_val              = 0;
                  i                    = 0;

                  while(DesiredNotifications)
                  {
                     if(DesiredNotifications & 0x1)
                     {
                        if(!(EnabledNotifications & 0x1))
                        {
                           ret_val = INVALID_PARAMETERS_ERROR;

                           Display(("Error - Client has not enabled '%s' for Unread Alert notifications.\r\n", AlertCategories[i]));
                        }
                     }

                     EnabledNotifications >>= 1;
                     DesiredNotifications >>= 1;
                     i                     += 1;
                  }

                  /* Send the notifications if all requested categories */
                  /* are enabled.                                       */
                  if(ret_val == 0)
                  {
                     ret_val = SendUnreadAlertNotification(TempParam->Params[0].intParam, "");
                  }
               }
               else
               {
                  Display(("Client has not registered for Unread Alert notifications.\r\n"));

                  ret_val = 0;
               }
            }
            else
               Display(("Error - Unknown Client.\r\n"));
         }
         else
            Display(("Connection not established.\r\n"));
      }
      else
      {
         if(ConnectionID)
            Display(("Error - Only a server can notify.\r\n"));
         else
            Display(("Error - ANP server not registered\r\n"));
      }
   }
   else
   {
      DisplayUsage("NotifyUnreadAlerts [BitMask (0xXXXX)]");
      DisplayUsageAlertCategoryBitMask();

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the New Alert   */
   /* Status for a specified category.  It can be executed only by a    */
   /* server.  This function will return zero on successful execution   */
   /* and a negative value on errors.                                   */
static int GetNewAlertStatus(ParameterList_t *TempParam)
{
   Word_t SupportedCategories;
   int    ret_val = 0;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= ciInstantMessage))
   {
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, &SupportedCategories)) == 0)
      {
         if(SupportedCategories & (0x1 << TempParam->Params[0].intParam))
         {
            /* First, check that valid Bluetooth Stack ID exists.       */
            if(BluetoothStackID)
            {
               /* Check for a registered ANP Server                     */
               if(ANSInstanceID)
               {
                  DisplayNewAlertStatus((ANS_Category_Identification_t)TempParam->Params[0].intParam, NewAlertStatuses[TempParam->Params[0].intParam].NumberOfAlerts, NewAlertStatuses[TempParam->Params[0].intParam].AlertString, "");

                  ret_val = 0;
               }
               else
               {
                  if(ConnectionID)
                     Display(("Cannot get new alert statuses as a client.\r\n"));
                  else
                     Display(("ANP server not registered\r\n"));
               }
            }
            else
            {
               /* No valid Bluetooth Stack ID exists.                   */
               ret_val = INVALID_STACK_ID_ERROR;
            }
         }
         else
         {
            Display(("Error - '%s' is currently not supported.\r\n", AlertCategories[TempParam->Params[0].intParam]));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
      }
   }
   else
   {
      DisplayUsage("GetNewAlertStatus [Category]");
      DisplayUsageAlertCategoryNumericalValue(FALSE);

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for writing the New Alert   */
   /* Status for a specified category.  It can be executed only by a    */
   /* server.  This function will return zero on successful execution   */
   /* and a negative value on errors.                                   */
static int SetNewAlertStatus(ParameterList_t *TempParam)
{
   unsigned int  i;
   unsigned int  j;
   Word_t        SupportedCategories;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= ciInstantMessage))
   {
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, &SupportedCategories)) == 0)
      {
         if(SupportedCategories & (0x1 << TempParam->Params[0].intParam))
         {
            /* First, check that valid Bluetooth Stack ID exists.       */
            if(BluetoothStackID)
            {
               /* First check for a registered ANP Server               */
               if(ANSInstanceID)
               {
                  /* Set the number of alerts.                          */
                  NewAlertStatuses[TempParam->Params[0].intParam].NumberOfAlerts = TempParam->Params[1].intParam;

                  /* Use the rest of the parameters as the alert string */
                  /* (if they exist).                                   */
                  NewAlertStatuses[TempParam->Params[0].intParam].AlertString[0] = '\0';
                  i                                                              = 0;
                  j                                                              = 2;

                  while((j < TempParam->NumberofParameters) && (i < (MAX_SIZE_ALERT_STRING - 1)))
                  {
                     if(i)
                     {
                        /* Put a space in-between each parameter.       */
                        NewAlertStatuses[TempParam->Params[0].intParam].AlertString[i++] = ' ';
                        NewAlertStatuses[TempParam->Params[0].intParam].AlertString[i]   = '\0';
                     }

                     if((MAX_SIZE_ALERT_STRING - i - 1) < BTPS_StringLength(TempParam->Params[j].strParam))
                        TempParam->Params[j].strParam[MAX_SIZE_ALERT_STRING - i - 1] = '\0';

                     BTPS_StringCopy(&(NewAlertStatuses[TempParam->Params[0].intParam].AlertString[i]), TempParam->Params[j].strParam);

                     i += BTPS_StringLength(TempParam->Params[j++].strParam);
                  }

                  Display(("Set New Alert Status success.\r\n"));

                  ret_val = 0;
               }
               else
               {
                  if(ConnectionID)
                     Display(("Cannot set new alert statuses as a client.\r\n"));
                  else
                     Display(("ANP server not registered\r\n"));
               }
            }
            else
            {
               /* No valid Bluetooth Stack ID exists.                   */
               ret_val = INVALID_STACK_ID_ERROR;
            }
         }
         else
         {
            Display(("Error - '%s' is currently not supported.\r\n", AlertCategories[TempParam->Params[0].intParam]));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
      }
   }
   else
   {
      DisplayUsage("SetNewAlertStatus [Category] [Num Alerts] [Alert String (Optional)]");
      DisplayUsageAlertCategoryNumericalValue(FALSE);

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Unread Alert*/
   /* Status for a specified category.  It can be executed only by a    */
   /* server.  This function will return zero on successful execution   */
   /* and a negative value on errors.                                   */
static int GetUnreadAlertStatus(ParameterList_t *TempParam)
{
   Word_t SupportedCategories;
   int    ret_val = 0;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && (TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= ciInstantMessage))
   {
      /* Verify that the category is currently supported.               */
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, &SupportedCategories)) == 0)
      {
         if(SupportedCategories & (0x1 << TempParam->Params[0].intParam))
         {
            /* First, check that valid Bluetooth Stack ID exists.       */
            if(BluetoothStackID)
            {
               /* First check for a registered ANP Server               */
               if(ANSInstanceID)
               {
                  DisplayUnreadAlertStatus((ANS_Category_Identification_t)TempParam->Params[0].intParam, UnreadAlertStatuses[TempParam->Params[0].intParam], "");

                  ret_val = 0;
               }
               else
               {
                  if(ConnectionID)
                     Display(("Cannot get unread alert statuses as a client.\r\n"));
                  else
                     Display(("ANP server not registered\r\n"));
               }
            }
            else
            {
               /* No valid Bluetooth Stack ID exists.                   */
               ret_val = INVALID_STACK_ID_ERROR;
            }
         }
         else
         {
            Display(("Error - '%s' is currently not supported.\r\n", AlertCategories[TempParam->Params[0].intParam]));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
      }
   }
   else
   {
      DisplayUsage("GetUnreadAlertStatus [Category]");
      DisplayUsageAlertCategoryNumericalValue(FALSE);

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for writing the Unread Alert*/
   /* Status for a specified category.  It can be executed only by a    */
   /* server.  This function will return zero on successful execution   */
   /* and a negative value on errors.                                   */
static int SetUnreadAlertStatus(ParameterList_t *TempParam)
{
   Word_t SupportedCategories;
   int    ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 1) && (TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= ciInstantMessage))
   {
      /* Verify that the category is currently supported.               */
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, &SupportedCategories)) == 0)
      {
         if(SupportedCategories & (0x1 << TempParam->Params[0].intParam))
         {
            /* First, check that valid Bluetooth Stack ID exists.       */
            if(BluetoothStackID)
            {
               /* First check for a registered ANP Server               */
               if(ANSInstanceID)
               {
                  /* Set the number of alerts.                          */
                  UnreadAlertStatuses[TempParam->Params[0].intParam] = TempParam->Params[1].intParam;

                  Display(("Set Unread Alert Status success.\r\n"));

                  ret_val = 0;
               }
               else
               {
                  if(ConnectionID)
                     Display(("Cannot set unread alert statuses as a client.\r\n"));
                  else
                     Display(("ANP server not registered\r\n"));
               }
            }
            else
            {
               /* No valid Bluetooth Stack ID exists.                   */
               ret_val = INVALID_STACK_ID_ERROR;
            }
         }
         else
         {
            Display(("Error - '%s' is currently not supported.\r\n", AlertCategories[TempParam->Params[0].intParam]));

            ret_val = INVALID_PARAMETERS_ERROR;
         }
      }
      else
      {
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
      }
   }
   else
   {
      DisplayUsage("SetUnreadAlertStatus [Category] [Num Alerts]");
      DisplayUsageAlertCategoryNumericalValue(FALSE);

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Supported   */
   /* New Alert Category.  It can be executed by a server or a client   */
   /* with an open connection to a remote server.  If executed as a     */
   /* client, a GATT read request will be generated, and the results    */
   /* will be returned as a response in the GATT client event callback. */
   /* This function will return zero on successful execution and a      */
   /* negative value on errors.                                         */
static int GetSupportedNewAlertCategories(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   Word_t        SupportedCategories;
   int           ret_val = FUNCTION_ERROR;

   /* First check for a registered ANP Server                           */
   if(ANSInstanceID)
   {
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, &SupportedCategories)) == 0)
      {
         DisplaySupportedCategories(SupportedCategories, "", "New");
      }
      else
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
   }
   else
   {
      /* Check to see if we are configured as a client with an active   */
      /* connection                                                     */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that the client has received a valid Supported New*/
            /* Alert Category Attribute Handle.                         */
            if(DeviceInfo->ClientInfo.Supported_New_Alert_Category != 0)
            {
               /* Finally, submit a read request to the server          */
               if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Supported_New_Alert_Category, GATT_ClientEventCallback_ANP, DeviceInfo->ClientInfo.Supported_New_Alert_Category)) > 0)
               {
                  Display(("Get Supported New Alert Category Request sent, Transaction ID = %u", ret_val));

                  ret_val = 0;
               }
               else
                  DisplayFunctionError("GATT_Read_Value_Request", ret_val);
            }
            else
               Display(("Error - Supported New Alert Category not supported on remote service.\r\n"));
         }
         else
            Display(("No Device Info.\r\n"));
      }
      else
         Display(("Either a ANP server must be registered or a ANP client must be connected.\r\n"));
   }

   return(ret_val);
}

   /* The following function is responsible for writing the Supported   */
   /* New Alert Category.  It can be executed only by a server.  This   */
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int SetSupportedNewAlertCategories(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;
   Word_t        SupportedCategories;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && ((TempParam->Params[0].intParam == 0) || ((TempParam->Params[0].intParam & (~SUPPORTED_CATEGORIES_MASK)) == 0)))
   {
      /* Check for a registered server                                  */
      if(ANSInstanceID)
      {
         /* Set the supported categories                                */
         SupportedCategories = TempParam->Params[0].intParam;

         if((ret_val = ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, SupportedCategories)) == 0)
         {
            /* Check if a currently enabled category became unsupported.*/
            DeviceInfo = DeviceInfoList;
            while(DeviceInfo)
            {
               if(DeviceInfo->ServerInfo.Enabled_New_Alert_Categories != SupportedCategories)
               {
                  BD_ADDRToStr(DeviceInfo->ConnectionBD_ADDR, BoardStr);
                  Display(("Warning - Disabling previously supported notification(s) for device %s.\r\n", BoardStr));

                  DeviceInfo->ServerInfo.Enabled_New_Alert_Categories = SupportedCategories;
               }

               DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
            }
            Display(("Supported New Alert Category successfully set.\r\n"));
         }
         else
            DisplayFunctionError("ANS_Set_Supported_Categories", ret_val);
      }
      else
      {
         if(ConnectionID)
            Display(("Cannot write to Supported New Alert Category as a client.\r\n"));
         else
            Display(("ANP server not registered\r\n"));
      }
   }
   else
   {
      DisplayUsage("SetSupportedNewAlertCategories [BitMask (0xXXXX)]");
      DisplayUsageAlertCategoryBitMask();

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for reading the Supported   */
   /* New Alert Category.  It can be executed by a server or a client   */
   /* with an open connection to a remote server.  If executed as a     */
   /* client, a GATT read request will be generated, and the results    */
   /* will be returned as a response in the GATT client event callback. */
   /* This function will return zero on successful execution and a      */
   /* negative value on errors.                                         */
static int GetSupportedUnreadAlertCategories(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   Word_t        SupportedCategories;
   int           ret_val = FUNCTION_ERROR;

   /* First check for a registered ANP Server                           */
   if(ANSInstanceID)
   {
      if((ret_val = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, &SupportedCategories)) == 0)
      {
         DisplaySupportedCategories(SupportedCategories, "", "Unread");
      }
      else
         DisplayFunctionError("ANS_Query_Supported_Categories", ret_val);
   }
   else
   {
      /* Check to see if we are configured as a client with an active   */
      /* connection                                                     */
      if(ConnectionID)
      {
         /* Get the device info for the connection device.              */
         if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
         {
            /* Verify that the client has received a valid Supported    */
            /* Unread Alert Category Attribute Handle.                  */
            if(DeviceInfo->ClientInfo.Supported_Unread_Alert_Category != 0)
            {
               /* Finally, submit a read request to the server          */
               if((ret_val = GATT_Read_Value_Request(BluetoothStackID, ConnectionID, DeviceInfo->ClientInfo.Supported_Unread_Alert_Category, GATT_ClientEventCallback_ANP, DeviceInfo->ClientInfo.Supported_Unread_Alert_Category)) > 0)
               {
                  Display(("Get Supported Unread Alert Category Request sent, Transaction ID = %u", ret_val));

                  ret_val = 0;
               }
               else
                  DisplayFunctionError("GATT_Read_Value_Request", ret_val);
            }
            else
               Display(("Error - Supported Unread Alert Category not supported on remote service.\r\n"));
         }
         else
            Display(("No Device Info.\r\n"));
      }
      else
         Display(("Either a ANP server must be registered or a ANP client must be connected.\r\n"));
   }

   return(ret_val);
}

   /* The following function is responsible for writing the Supported   */
   /* Unread Alert Category.  It can be executed only by a server.  This*/
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int SetSupportedUnreadAlertCategories(ParameterList_t *TempParam)
{
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;
   Word_t        SupportedCategories;
   int           ret_val = FUNCTION_ERROR;

   /* Verify that the input parameters are semi-valid.                  */
   if((TempParam) && (TempParam->NumberofParameters > 0) && ((TempParam->Params[0].intParam == 0) || ((TempParam->Params[0].intParam & (~SUPPORTED_CATEGORIES_MASK)) == 0)))
   {
      /* Check for a registered server                                  */
      if(ANSInstanceID)
      {
         /* Set the supported categories                                */
         SupportedCategories = TempParam->Params[0].intParam;

         if((ret_val = ANS_Set_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, SupportedCategories)) == 0)
         {
            /* Check if a currently enabled category became unsupported.*/
            DeviceInfo = DeviceInfoList;
            while(DeviceInfo)
            {
               if(DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories != SupportedCategories)
               {
                  BD_ADDRToStr(DeviceInfo->ConnectionBD_ADDR, BoardStr);
                  Display(("Warning - Disabling previously supported notifications for device %s.\r\n", BoardStr));

                  DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories = SupportedCategories;
               }

               DeviceInfo = DeviceInfo->NextDeviceInfoPtr;
            }

            Display(("Supported Unread Alert Category successfully set.\r\n"));
         }
         else
            DisplayFunctionError("ANS_Set_Supported_Categories", ret_val);
      }
      else
      {
         if(ConnectionID)
            Display(("Cannot write to Supported Unread Alert Category as a client.\r\n"));
         else
            Display(("ANP server not registered\r\n"));
      }
   }
   else
   {
      DisplayUsage("SetSupportedUnreadAlertCategories [BitMask (0xXXXX)]");
      DisplayUsageAlertCategoryBitMask();

      ret_val = INVALID_PARAMETERS_ERROR;
   }

   return(ret_val);
}

   /* The following function is responsible for writing the Enable New  */
   /* Incoming Alert Notifications command to a remote server Control   */
   /* Point.  It can be executed only by a client.  This function will  */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int EnableNewAlertNotifications(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcEnable_New_Alert_Notifications, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Enable New Alert Notifications command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("EnableNewAlertNotifications [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Enable New Alert Notifications as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for writing the Disable New */
   /* Incoming Alert Notifications command to a remote server Control   */
   /* Point.  It can be executed only by a client.  This function will  */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int DisableNewAlertNotifications(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcDisable_New_Alert_Notifications, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Disable New Alert Notifications command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("DisableNewAlertNotifications [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Disable New Alert Notifications as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for writing the Enable      */
   /* Unread Category Status Notifications command to a remote server   */
   /* Control Point.  It can be executed only by a client.  This        */
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int EnableUnreadAlertNotifications(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcEnable_Unread_Category_Notifications, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Enable Unread Category Notifications command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("EnableUnreadAlertNotifications [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Enable Unread Category Notifications as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for writing the Disable     */
   /* Unread Category Status Notifications command to a remote server   */
   /* Control Point.  It can be executed only by a client.  This        */
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int DisableUnreadAlertNotifications(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcDisable_Unread_Category_Notifications, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Disable Unread Category Notifications command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("DisableUnreadAlertNotifications [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Disable Unread Alert Notifications as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for writing the Notify New  */
   /* Incoming Alert Immediately command to a remote server Control     */
   /* Point.  It can be executed only by a client.  This function will  */
   /* return zero on successful execution and a negative value on       */
   /* errors.                                                           */
static int NotifyImmediatelyNewAlert(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcNotify_New_Alert_Immediately, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Notify New Alert Immediately command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("NotifyImmediatelyNewAlert [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Notify New Alert Immediately as a service.\r\n"));

   return(ret_val);
}

   /* The following function is responsible for writing the Notify New  */
   /* Unread Category Status Immediately command to a remote server     */
   /* Control Point.  It can be executed only by a client.  This        */
   /* function will return zero on successful execution and a negative  */
   /* value on errors.                                                  */
static int NotifyImmediatelyUnreadAlert(ParameterList_t *TempParam)
{
   int ret_val = FUNCTION_ERROR;

   /* Verify that we are not configured as a server                     */
   if(!ANSInstanceID)
   {
      /* Validate parameters.                                           */
      if((TempParam) && (TempParam->NumberofParameters > 0) && (((TempParam->Params[0].intParam >= ciSimpleAlert) && (TempParam->Params[0].intParam <= (ciInstantMessage + 1)))))
      {
         /* Check for all condition.                                    */
         if(TempParam->Params[0].intParam == (ciInstantMessage + 1))
            TempParam->Params[0].intParam = ciAllCategories;

         if((ret_val = WriteControlPointCommand(pcNotify_Unread_Category_Immediately, ((ANS_Category_Identification_t)TempParam->Params[0].intParam))) == 0)
            Display(("Notify Unread Category Immediately command request sent.\r\n"));
      }
      else
      {
         DisplayUsage("NotifyImmediatelyUnreadAlert [Category]");
         DisplayUsageAlertCategoryNumericalValue(TRUE);

         ret_val = INVALID_PARAMETERS_ERROR;
      }
   }
   else
      Display(("Cannot Notify Unread Category Immediately as a service.\r\n"));

   return(ret_val);
}

   /* ***************************************************************** */
   /*                         Event Callbacks                           */
   /* ***************************************************************** */

   /* The following function is for the GAP LE Event Receive Data       */
   /* Callback.  This function will be called whenever a Callback has   */
   /* been registered for the specified GAP LE Action that is associated*/
   /* with the Bluetooth Stack.  This function passes to the caller the */
   /* GAP LE Event Data of the specified Event and the GAP LE Event     */
   /* Callback Parameter that was specified when this Callback was      */
   /* installed.  The caller is free to use the contents of the GAP LE  */
   /* Event Data ONLY in the context of this callback.  If the caller   */
   /* requires the Data for a longer period of time, then the callback  */
   /* function MUST copy the data into another Data Buffer.  This       */
   /* function is guaranteed NOT to be invoked more than once           */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because other GAP Events will not be  */
   /* processed while this function call is outstanding).               */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GAP_LE_Event_Callback(unsigned int BluetoothStackID, GAP_LE_Event_Data_t *GAP_LE_Event_Data, unsigned long CallbackParameter)
{
   int                                           Result;
   BoardStr_t                                    BoardStr;
   unsigned int                                  Index;
   DeviceInfo_t                                 *DeviceInfo;
   Long_Term_Key_t                               GeneratedLTK;
   GAP_LE_Security_Information_t                 GAP_LE_Security_Information;
   GAP_LE_Connection_Parameters_t                ConnectionParameters;
   GAP_LE_Advertising_Report_Data_t             *DeviceEntryPtr;
   GAP_LE_Authentication_Event_Data_t           *Authentication_Event_Data;
   GAP_LE_Authentication_Response_Information_t  GAP_LE_Authentication_Response_Information;

   /* Verify that all parameters to this callback are semi-Valid.       */
   if((BluetoothStackID) && (GAP_LE_Event_Data))
   {
      switch(GAP_LE_Event_Data->Event_Data_Type)
      {
         case etLE_Advertising_Report:
            Display(("\r\netLE_Advertising_Report with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));
            Display(("  %d Responses.\r\n",GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries));

            for(Index = 0; Index < GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Number_Device_Entries; Index++)
            {
               DeviceEntryPtr = &(GAP_LE_Event_Data->Event_Data.GAP_LE_Advertising_Report_Event_Data->Advertising_Data[Index]);

               /* Display the packet type for the device                */
               switch(DeviceEntryPtr->Advertising_Report_Type)
               {
                  case rtConnectableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtConnectableUndirected"));
                     break;
                  case rtConnectableDirected:
                     Display(("  Advertising Type: %s.\r\n", "rtConnectableDirected"));
                     break;
                  case rtScannableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtScannableUndirected"));
                     break;
                  case rtNonConnectableUndirected:
                     Display(("  Advertising Type: %s.\r\n", "rtNonConnectableUndirected"));
                     break;
                  case rtScanResponse:
                     Display(("  Advertising Type: %s.\r\n", "rtScanResponse"));
                     break;
               }

               /* Display the Address Type.                             */
               if(DeviceEntryPtr->Address_Type == latPublic)
               {
                  Display(("  Address Type: %s.\r\n","atPublic"));
               }
               else
               {
                  Display(("  Address Type: %s.\r\n","atRandom"));
               }

               /* Display the Device Address.                           */
               Display(("  Address: 0x%02X%02X%02X%02X%02X%02X.\r\n", DeviceEntryPtr->BD_ADDR.BD_ADDR5, DeviceEntryPtr->BD_ADDR.BD_ADDR4, DeviceEntryPtr->BD_ADDR.BD_ADDR3, DeviceEntryPtr->BD_ADDR.BD_ADDR2, DeviceEntryPtr->BD_ADDR.BD_ADDR1, DeviceEntryPtr->BD_ADDR.BD_ADDR0));
               Display(("  RSSI: %d.\r\n", (int)DeviceEntryPtr->RSSI));
               Display(("  Data Length: %d.\r\n", DeviceEntryPtr->Raw_Report_Length));

               DisplayAdvertisingData(&(DeviceEntryPtr->Advertising_Data));
            }
            break;
         case etLE_Connection_Complete:
            Display(("\r\netLE_Connection_Complete with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address, BoardStr);

               Display(("   Status:       0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status));
               Display(("   Role:         %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master)?"Master":"Slave"));
               Display(("   Address Type: %s.\r\n", (GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type == latPublic)?"Public":"Random"));
               Display(("   BD_ADDR:      %s.\r\n", BoardStr));

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               {
                  ConnectionBD_ADDR   = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address;
                  LocalDeviceIsMaster = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Master;

                  /* Make sure that no entry already exists.            */
                  if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) == NULL)
                  {
                     /* No entry exists so create one.                  */
                     if(!CreateNewDeviceInfoEntry(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Complete_Event_Data->Peer_Address_Type, ConnectionBD_ADDR))
                        Display(("Failed to add device to Device Info List.\r\n"));
                  }
                  else
                  {
                     /* If we are the Master of the connection we will  */
                     /* attempt to Re-Establish Security if a LTK for   */
                     /* this device exists (i.e.  we previously paired).*/
                     if(LocalDeviceIsMaster)
                     {
                        /* Re-Establish Security if there is a LTK that */
                        /* is stored for this device.                   */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           /* Re-Establish Security with this LTK.      */
                           Display(("Attempting to Re-Establish Security.\r\n"));

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), RANDOM_NUMBER_DATA_SIZE);

                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, ConnectionBD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              DisplayFunctionError("GAP_LE_Reestablish_Security", Result);
                           }
                        }
                     }
                  }
               }
            }
            break;
         case etLE_Disconnection_Complete:
            Display(("\r\netLE_Disconnection_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data)
            {
               Display(("   Status: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Status));
               Display(("   Reason: 0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Reason));

               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Disconnection_Complete_Event_Data->Peer_Address, BoardStr);
               Display(("   BD_ADDR: %s.\r\n", BoardStr));

               /* Check to see if the device info is present in the     */
               /* list.                                                 */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* If this device is not paired, then delete it.  The */
                  /* link will be encrypted if the device is paired.    */
                  if(!(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LINK_ENCRYPTED))
                  {
                     if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
                        FreeDeviceInfoEntryMemory(DeviceInfo);
                  }
                  else
                  {
                     /* Flag that the Link is no longer encrypted since */
                     /* we have disconnected.                           */
                     DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
                  }
               }
               else
                  Display(("Warning - Disconnect from unknown device.\r\n"));

               /* Clear the saved Connection BD_ADDR.                   */
               ASSIGN_BD_ADDR(ConnectionBD_ADDR, 0, 0, 0, 0, 0, 0);
               LocalDeviceIsMaster = FALSE;
            }
            break;
         case etLE_Connection_Parameter_Update_Request:
            Display(("\r\netLE_Connection_Parameter_Update_Request with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, BoardStr);
               Display(("   BD_ADDR:             %s.\r\n", BoardStr));
               Display(("   Minimum Interval:    %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min));
               Display(("   Maximum Interval:    %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max));
               Display(("   Slave Latency:       %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency));
               Display(("   Supervision Timeout: %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout));

               /* Initialize the connection parameters.                 */
               ConnectionParameters.Connection_Interval_Min    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Min;
               ConnectionParameters.Connection_Interval_Max    = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Interval_Max;
               ConnectionParameters.Slave_Latency              = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Slave_Latency;
               ConnectionParameters.Supervision_Timeout        = GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->Conn_Supervision_Timeout;
               ConnectionParameters.Minimum_Connection_Length  = 0;
               ConnectionParameters.Maximum_Connection_Length  = 10000;

               Display(("\r\nAttempting to accept connection parameter update request.\r\n"));

               /* Go ahead and accept whatever the slave has requested. */
               Result = GAP_LE_Connection_Parameter_Update_Response(BluetoothStackID, GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Update_Request_Event_Data->BD_ADDR, TRUE, &ConnectionParameters);
               if(!Result)
               {
                  Display(("      GAP_LE_Connection_Parameter_Update_Response() success.\r\n"));
               }
               else
               {
                  Display(("      GAP_LE_Connection_Parameter_Update_Response() error %d.\r\n", Result));
               }
            }
            break;
         case etLE_Connection_Parameter_Updated:
            Display(("\r\netLE_Connection_Parameter_Updated with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data)
            {
               BD_ADDRToStr(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->BD_ADDR, BoardStr);
               Display(("   Status:              0x%02X.\r\n", GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status));
               Display(("   BD_ADDR:             %s.\r\n", BoardStr));

               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
               {
                  Display(("   Connection Interval: %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Connection_Interval));
                  Display(("   Slave Latency:       %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Slave_Latency));
                  Display(("   Supervision Timeout: %u.\r\n", (unsigned int)GAP_LE_Event_Data->Event_Data.GAP_LE_Connection_Parameter_Updated_Event_Data->Current_Connection_Parameters.Supervision_Timeout));
               }
            }
            break;
         case etLE_Encryption_Change:
            Display(("\r\netLE_Encryption_Change with size %d.\r\n",(int)GAP_LE_Event_Data->Event_Data_Size));

            /* Search for the device entry to see flag if the link is   */
            /* encrypted.                                               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->BD_ADDR)) != NULL)
            {
               /* Check to see if the encryption change was successful. */
               if((GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Change_Status == HCI_ERROR_CODE_NO_ERROR) && (GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Change_Event_Data->Encryption_Mode == emEnabled))
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               else
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
            }
            break;
         case etLE_Encryption_Refresh_Complete:
            Display(("\r\netLE_Encryption_Refresh_Complete with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            /* Search for the device entry to see flag if the link is   */
            /* encrypted.                                               */
            if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->BD_ADDR)) != NULL)
            {
               /* Check to see if the refresh was successful.           */
               if(GAP_LE_Event_Data->Event_Data.GAP_LE_Encryption_Refresh_Complete_Event_Data->Status == HCI_ERROR_CODE_NO_ERROR)
                  DeviceInfo->Flags |= DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
               else
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_LINK_ENCRYPTED;
            }
            break;
         case etLE_Authentication:
            Display(("\r\netLE_Authentication with size %d.\r\n", (int)GAP_LE_Event_Data->Event_Data_Size));

            /* Make sure the authentication event data is valid before  */
            /* continuing.                                              */
            if((Authentication_Event_Data = GAP_LE_Event_Data->Event_Data.GAP_LE_Authentication_Event_Data) != NULL)
            {
               BD_ADDRToStr(Authentication_Event_Data->BD_ADDR, BoardStr);

               switch(Authentication_Event_Data->GAP_LE_Authentication_Event_Type)
               {
                  case latLongTermKeyRequest:
                     Display(("    latKeyRequest: \r\n"));
                     Display(("      BD_ADDR: %s.\r\n", BoardStr));

                     /* The other side of a connection is requesting    */
                     /* that we start encryption. Thus we should        */
                     /* regenerate LTK for this connection and send it  */
                     /* to the chip.                                    */
                     Result = GAP_LE_Regenerate_Long_Term_Key(BluetoothStackID, (Encryption_Key_t *)(&DHK), (Encryption_Key_t *)(&ER), Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.EDIV, &(Authentication_Event_Data->Authentication_Event_Data.Long_Term_Key_Request.Rand), &GeneratedLTK);
                     if(!Result)
                     {
                        Display(("      GAP_LE_Regenerate_Long_Term_Key Success.\r\n"));

                        /* Respond with the Re-Generated Long Term Key. */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type                                        = larLongTermKey;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length                                        = GAP_LE_LONG_TERM_KEY_INFORMATION_DATA_SIZE;
                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Encryption_Key_Size = GAP_LE_MAXIMUM_ENCRYPTION_KEY_SIZE;
                        GAP_LE_Authentication_Response_Information.Authentication_Data.Long_Term_Key_Information.Long_Term_Key       = GeneratedLTK;
                     }
                     else
                     {
                        DisplayFunctionError("      GAP_LE_Regenerate_Long_Term_Key", Result);

                        /* Since we failed to generate the requested key*/
                        /* we should respond with a negative response.  */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larLongTermKey;
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = 0;
                     }

                     /* Send the Authentication Response.               */
                     Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                     if(Result)
                     {
                        DisplayFunctionError("      GAP_LE_Authentication_Response", Result);
                     }
                     break;
                  case latSecurityRequest:
                     /* Display the data for this event.                */
                     /* * NOTE * This is only sent from Slave to Master.*/
                     /*          Thus we must be the Master in this     */
                     /*          connection.                            */
                     Display(("    latSecurityRequest:.\r\n"));
                     Display(("      BD_ADDR: %s.\r\n", BoardStr));
                     Display(("      Bonding Type: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.Bonding_Type == lbtBonding)?"Bonding":"No Bonding")));
                     Display(("      MITM: %s.\r\n", ((Authentication_Event_Data->Authentication_Event_Data.Security_Request.MITM == TRUE)?"YES":"NO")));

                     /* Determine if we have previously paired with the */
                     /* device. If we have paired we will attempt to    */
                     /* re-establish security using a previously        */
                     /* exchanged LTK.                                  */
                     if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                     {
                        /* Determine if a Valid Long Term Key is stored */
                        /* for this device.                             */
                        if(DeviceInfo->Flags & DEVICE_INFO_FLAGS_LTK_VALID)
                        {
                           Display(("Attempting to Re-Establish Security.\r\n"));

                           /* Attempt to re-establish security to this  */
                           /* device.                                   */
                           GAP_LE_Security_Information.Local_Device_Is_Master                                      = TRUE;

                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.LTK), &(DeviceInfo->LTK), LONG_TERM_KEY_SIZE);
                           BTPS_MemCopy(&(GAP_LE_Security_Information.Security_Information.Master_Information.Rand), &(DeviceInfo->Rand), RANDOM_NUMBER_DATA_SIZE);

                           GAP_LE_Security_Information.Security_Information.Master_Information.EDIV                = DeviceInfo->EDIV;
                           GAP_LE_Security_Information.Security_Information.Master_Information.Encryption_Key_Size = DeviceInfo->EncryptionKeySize;

                           Result = GAP_LE_Reestablish_Security(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Security_Information, GAP_LE_Event_Callback, 0);
                           if(Result)
                           {
                              DisplayFunctionError("GAP_LE_Reestablish_Security", Result);
                           }
                        }
                        else
                        {
                           CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                           /* We do not have a stored Link Key for this */
                           /* device so go ahead and pair to this       */
                           /* device.                                   */
                           SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                        }
                     }
                     else
                     {
                        CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                        /* There is no Key Info Entry for this device   */
                        /* so we will just treat this as a slave        */
                        /* request and initiate pairing.                */
                        SendPairingRequest(Authentication_Event_Data->BD_ADDR, TRUE);
                     }

                     break;
                  case latPairingRequest:
                     CurrentRemoteBD_ADDR = Authentication_Event_Data->BD_ADDR;

                     Display(("Pairing Request: %s.\r\n",BoardStr));
                     DisplayPairingInformation(Authentication_Event_Data->Authentication_Event_Data.Pairing_Request);

                     /* This is a pairing request. Respond with a       */
                     /* Pairing Response.                               */
                     /* * NOTE * This is only sent from Master to Slave.*/
                     /*          Thus we must be the Slave in this      */
                     /*          connection.                            */

                     /* Send the Pairing Response.                      */
                     SlavePairingRequestResponse(Authentication_Event_Data->BD_ADDR);
                     break;
                  case latConfirmationRequest:
                     Display(("latConfirmationRequest.\r\n"));

                     if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtNone)
                     {
                        Display(("Invoking Just Works.\r\n"));

                        /* Just Accept Just Works Pairing.              */
                        GAP_LE_Authentication_Response_Information.GAP_LE_Authentication_Type = larConfirmation;

                        /* By setting the Authentication_Data_Length to */
                        /* any NON-ZERO value we are informing the GAP  */
                        /* LE Layer that we are accepting Just Works    */
                        /* Pairing.                                     */
                        GAP_LE_Authentication_Response_Information.Authentication_Data_Length = DWORD_SIZE;

                        Result = GAP_LE_Authentication_Response(BluetoothStackID, Authentication_Event_Data->BD_ADDR, &GAP_LE_Authentication_Response_Information);
                        if(Result)
                        {
                           DisplayFunctionError("GAP_LE_Authentication_Response", Result);
                        }
                     }
                     else
                     {
                        if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtPasskey)
                        {
                           Display(("Call LEPasskeyResponse [PASSCODE].\r\n"));
                        }
                        else
                        {
                           if(Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Request_Type == crtDisplay)
                           {
                              Display(("Passkey: %06ld.\r\n", Authentication_Event_Data->Authentication_Event_Data.Confirmation_Request.Display_Passkey));
                           }
                        }
                     }
                     break;
                  case latSecurityEstablishmentComplete:
                     Display(("Security Re-Establishment Complete: %s.\r\n", BoardStr));
                     Display(("                            Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Security_Establishment_Complete.Status));
                     break;
                  case latPairingStatus:
                     ASSIGN_BD_ADDR(CurrentRemoteBD_ADDR, 0, 0, 0, 0, 0, 0);

                     Display(("Pairing Status: %s.\r\n", BoardStr));
                     Display(("        Status: 0x%02X.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status));

                     if(Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Status == GAP_LE_PAIRING_STATUS_NO_ERROR)
                     {
                        Display(("        Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Pairing_Status.Negotiated_Encryption_Key_Size));
                     }
                     else
                     {
                        /* Failed to pair so delete the key entry for   */
                        /* this device and disconnect the link.         */
                        if((DeviceInfo = DeleteDeviceInfoEntry(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                           FreeDeviceInfoEntryMemory(DeviceInfo);

                        /* Disconnect the Link.                         */
                        GAP_LE_Disconnect(BluetoothStackID, Authentication_Event_Data->BD_ADDR);
                     }
                     break;
                  case latEncryptionInformationRequest:
                     Display(("Encryption Information Request %s.\r\n", BoardStr));

                     /* Generate new LTK,EDIV and Rand and respond with */
                     /* them.                                           */
                     EncryptionInformationRequestResponse(Authentication_Event_Data->BD_ADDR, Authentication_Event_Data->Authentication_Event_Data.Encryption_Request_Information.Encryption_Key_Size, &GAP_LE_Authentication_Response_Information);
                     break;
                  case latEncryptionInformation:
                     /* Display the information from the event.         */
                     Display((" Encryption Information from RemoteDevice: %s.\r\n", BoardStr));
                     Display(("                             Key Size: %d.\r\n", Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size));

                     /* ** NOTE ** If we are the Slave we will NOT      */
                     /*            store the LTK that is sent to us by  */
                     /*            the Master.  However if it was ever  */
                     /*            desired that the Master and Slave    */
                     /*            switch roles in a later connection   */
                     /*            we could store that information at   */
                     /*            this point.                          */
                     if(LocalDeviceIsMaster)
                     {
                        /* Search for the entry for this slave to store */
                        /* the information into.                        */
                        if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, Authentication_Event_Data->BD_ADDR)) != NULL)
                        {
                           BTPS_MemCopy(&(DeviceInfo->LTK), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.LTK), LONG_TERM_KEY_SIZE);
                           BTPS_MemCopy(&(DeviceInfo->Rand), &(Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Rand), RANDOM_NUMBER_DATA_SIZE);

                           DeviceInfo->EDIV              = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.EDIV;
                           DeviceInfo->EncryptionKeySize = Authentication_Event_Data->Authentication_Event_Data.Encryption_Information.Encryption_Key_Size;
                           DeviceInfo->Flags            |= DEVICE_INFO_FLAGS_LTK_VALID;
                        }
                        else
                        {
                           Display(("No Key Info Entry for this Slave.\r\n"));
                        }
                     }
                     break;
               }
            }
            break;
      }

      /* Display the command prompt.                                    */
      DisplayPrompt();
   }
}

   /* The following is a ANP Server Event Callback.  This function will */
   /* be called whenever an ANP Server Profile Event occurs that is     */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the ANS Event Data   */
   /* that occurred and the ANS Event Callback Parameter that was       */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the ANS Event Data ONLY in the context of this*/
   /* callback.  If the caller requires the Data for a longer period of */
   /* time, then the callback function MUST copy the data into another  */
   /* Data Buffer This function is guaranteed NOT to be invoked more    */
   /* than once simultaneously for the specified installed callback     */
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another ANS Event    */
   /* will not be processed while this function call is outstanding).   */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving ANS Event Packets.  */
   /*            A Deadlock WILL occur because NO ANS Event Callbacks   */
   /*            will be issued while this function is currently        */
   /*            outstanding.                                           */
static void BTPSAPI ANS_EventCallback(unsigned int BluetoothStackID, ANS_Event_Data_t *ANS_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;
   BoardStr_t    BoardStr;
   Word_t        SupportedCategories;
   int           Result;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (ANS_Event_Data))
   {
      switch(ANS_Event_Data->Event_Data_Type)
      {
         case etANS_Server_Read_Client_Configuration_Request:
            Display(("etANS_Server_Read_Client_Configuration_Request with size %u.\r\n", ANS_Event_Data->Event_Data_Size));

            if(ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data)
            {
               BD_ADDRToStr(ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->RemoteDevice, BoardStr);
               Display(("   Instance ID:      %u.\r\n", ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->InstanceID));
               Display(("   Connection ID:    %u.\r\n", ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->ConnectionID));
               Display(("   Transaction ID:   %u.\r\n", ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->TransactionID));
               Display(("   Connection Type:  %s.\r\n", ((ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->RemoteDevice)) != NULL)
               {
                  /* Validate event parameters                          */
                  if(ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->InstanceID == ANSInstanceID)
                  {
                     Result = 0;

                     switch(ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->ClientConfigurationType)
                     {
                        case ctNewAlert:
                           Display(("   Config Type:      ctNewAlert.\r\n"));

                           Result = ANS_Read_Client_Configuration_Response(BluetoothStackID, ANSInstanceID, ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->ServerInfo.New_Alert_Client_Configuration);
                           break;
                        case ctUnreadAlertStatus:
                           Display(("   Config Type:      ctUnreadAlertStatus.\r\n"));

                           Result = ANS_Read_Client_Configuration_Response(BluetoothStackID, ANSInstanceID, ANS_Event_Data->Event_Data.ANS_Read_Client_Configuration_Data->TransactionID, DeviceInfo->ServerInfo.Unread_Alert_Status_Client_Configuration);
                           break;
                        default:
                           Display(("   Config Type:      Unknown.\r\n"));
                           break;
                     }

                     if(Result)
                        DisplayFunctionError("ANS_Read_Client_Configuration_Response", Result);
                  }
                  else
                  {
                     Display(("\r\nInvalid Event data.\r\n"));
                  }
               }
               else
               {
                  Display(("\r\nUnknown Client.\r\n"));
               }
            }
            break;
         case etANS_Server_Client_Configuration_Update:
            Display(("etANS_Server_Client_Configuration_Update with size %u.\r\n", ANS_Event_Data->Event_Data_Size));

            if(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data)
            {
               BD_ADDRToStr(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->RemoteDevice, BoardStr);
               Display(("   Instance ID:      %u.\r\n", ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->InstanceID));
               Display(("   Connection ID:    %u.\r\n", ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->ConnectionID));
               Display(("   Connection Type:  %s.\r\n", ((ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->RemoteDevice)) != NULL)
               {
                  /* Validate event parameters                          */
                  if(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->InstanceID == ANSInstanceID)
                  {
                     switch(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->ClientConfigurationType)
                     {
                        case ctNewAlert:
                           Display(("   Config Type:      ctNewAlert.\r\n"));

                           if(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->NotificationsEnabled)
                              DeviceInfo->ServerInfo.New_Alert_Client_Configuration |= GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                           else
                              DeviceInfo->ServerInfo.New_Alert_Client_Configuration &= ~GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                           break;
                        case ctUnreadAlertStatus:
                           Display(("   Config Type:      ctUnreadAlertStatus.\r\n"));

                           if(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->NotificationsEnabled)
                              DeviceInfo->ServerInfo.Unread_Alert_Status_Client_Configuration |= GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                           else
                              DeviceInfo->ServerInfo.Unread_Alert_Status_Client_Configuration &= ~GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE;
                           break;
                        default:
                           Display(("   Config Type:      Unknown.\r\n"));
                           break;
                     }

                     Display(("   Value:            "));
                     if(ANS_Event_Data->Event_Data.ANS_Client_Configuration_Update_Data->NotificationsEnabled)
                        Display(("0x%04X.\r\n", GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE));
                     else
                        Display(("0x0000.\r\n"));
                  }
                  else
                  {
                     Display(("\r\nInvalid Event data.\r\n"));
                  }
               }
               else
               {
                  Display(("\r\nUnknown Client.\r\n"));
               }
            }
            break;
         case etANS_Server_Control_Point_Command_Indication:
            Display(("etANS_Server_Control_Point_Command_Indication with size %u.\r\n", ANS_Event_Data->Event_Data_Size));
            if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data)
            {
               BD_ADDRToStr(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->RemoteDevice, BoardStr);
               Display(("   Instance ID:      %u.\r\n", ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->InstanceID));
               Display(("   Connection ID:    %u.\r\n", ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->ConnectionID));
               Display(("   Connection Type:  %s.\r\n", ((ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));

               /* Get the device info for the connection device.        */
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
               {
                  /* The client is making a request to change the Ringer*/
                  /* Setting and possibly the Ringer State.  The server */
                  /* application can choose whether or not to change the*/
                  /* state, it is not required.  For the purposes of    */
                  /* this demo, the states will be changed at the       */
                  /* client's request.                                  */
                  switch(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Command)
                  {
                     case pcEnable_New_Alert_Notifications:
                        Display(("   Command:          pcEnable_New_Alert_Notifications.\r\n\r\n"));

                        if((Result = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, &SupportedCategories)) == 0)
                        {
                           /* Check if the client wants to enable all   */
                           /* supported categories                      */
                           if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                           {
                              /* Verify that the server supports at     */
                              /* least one category                     */
                              if(SupportedCategories)
                              {
                                 /* Enable all categories for this      */
                                 /* client.                             */
                                 DeviceInfo->ServerInfo.Enabled_New_Alert_Categories = SUPPORTED_CATEGORIES_MASK & SupportedCategories;

                                 Display(("   Enabling all New Alert categories.\r\n"));
                              }
                              else
                                 Display(("Server does not support ANY New Alert categories!\r\n"));
                           }
                           else
                           {
                              /* Verify that the server supports the    */
                              /* category                               */
                              if(SupportedCategories & (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category))
                              {
                                 /* Enable the selected category for    */
                                 /* this client.                        */
                                 DeviceInfo->ServerInfo.Enabled_New_Alert_Categories |= (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category);

                                 Display(("   Enabling '%s' New Alert category.\r\n", AlertCategories[ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category]));
                              }
                              else
                                 Display(("Server does not support desired New Alert category.\r\n"));
                           }
                        }
                        else
                           DisplayFunctionError("ANS_Query_Supported_Categories", Result);
                        break;

                     case pcEnable_Unread_Category_Notifications:
                        Display(("   Command:          pcEnable_Unread_Category_Notifications.\r\n\r\n"));

                        if((Result = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, &SupportedCategories)) == 0)
                        {
                           /* Check if the client wants to enable all   */
                           /* supported categories                      */
                           if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                           {
                              /* Verify that the server supports at     */
                              /* least one category                     */
                              if(SupportedCategories)
                              {
                                 /* Enable all categories for this      */
                                 /* client.                             */
                                 DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories = SUPPORTED_CATEGORIES_MASK & SupportedCategories;

                                 Display(("   Enabling all Unread Alert categories.\r\n"));
                              }
                              else
                                 Display(("Server does not support ANY Unread Alert categories!\r\n"));
                           }
                           else
                           {
                              /* Verify that the server supports the    */
                              /* category                               */
                              if(SupportedCategories & (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category))
                              {
                                 /* Enable the selected category for    */
                                 /* this client.                        */
                                 DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories |= (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category);

                                 Display(("   Enabling '%s' Unread Alert category.\r\n", AlertCategories[ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category]));
                              }
                              else
                                 Display(("Server does not support desired Unread Alert category.\r\n"));
                           }
                        }
                        else
                           DisplayFunctionError("ANS_Query_Supported_Categories", Result);
                        break;

                     case pcDisable_New_Alert_Notifications:
                        Display(("   Command:          pcDisable_New_Alert_Notifications.\r\n\r\n"));

                        /* Check if the client wants to disable all     */
                        /* supported categories                         */
                        if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                        {
                           DeviceInfo->ServerInfo.Enabled_New_Alert_Categories = 0;

                           Display(("   Disabling all New Alert categories.\r\n"));
                        }
                        else
                        {
                           /* Disable the selected category for this    */
                           /* client.                                   */
                           DeviceInfo->ServerInfo.Enabled_New_Alert_Categories &= ~(0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category);

                           Display(("   Disabling '%s' New Alert category.\r\n", AlertCategories[ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category]));
                        }
                        break;

                     case pcDisable_Unread_Category_Notifications:
                        Display(("   Command:          pcDisable_Unread_Category_Notifications.\r\n\r\n"));

                        /* Check if the client wants to enable all      */
                        /* supported categories                         */
                        if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                        {
                           /* Disable all categories for this client.   */
                           DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories = 0;

                           Display(("   Disabling all Unread Alert categories.\r\n"));
                        }
                        else
                        {
                           /* Disable the selected category for this    */
                           /* client.                                   */
                           DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories &= ~(0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category);

                           Display(("\r\nDisabling '%s' Unread Alert category.\r\n", AlertCategories[ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category]));
                        }
                        break;

                     case pcNotify_New_Alert_Immediately:
                        Display(("   Command:          pcNotify_New_Alert_Immediately.\r\n\r\n"));

                        /* Verify that the client has registered for New*/
                        /* Alert Notifications.                         */
                        if(DeviceInfo->ServerInfo.New_Alert_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                        {
                           /* Check if the new alert is supported.      */
                           if((Result = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scNewAlert, &SupportedCategories)) == 0)
                           {
                              /* Check if the client wants to alert all */
                              /* supported categories                   */
                              if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                              {
                                 /* Verify that the server supports at  */
                                 /* least one category                  */
                                 if(SupportedCategories)
                                 {
                                    /* Verify that the client has       */
                                    /* enabled at least one of the      */
                                    /* supported categories.            */
                                    if(DeviceInfo->ServerInfo.Enabled_New_Alert_Categories & SupportedCategories)
                                    {
                                       /* Mask the disabled categories. */
                                       SupportedCategories &= DeviceInfo->ServerInfo.Enabled_New_Alert_Categories;

                                       /* Send an alert for all         */
                                       /* supported categories to this  */
                                       /* client.                       */
                                       SendNewAlertNotification(SupportedCategories, "   ");
                                    }
                                    else
                                       Display(("Client has not enabled any supported New Alert categories!\r\n"));
                                 }
                                 else
                                    Display(("Server does not support ANY New Alert categories!\r\n"));
                              }
                              else
                              {
                                 /* Verify that the server supports the */
                                 /* category                            */
                                 if(SupportedCategories & (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category))
                                 {
                                    /* Verify that the client has       */
                                    /* enabled the category it is       */
                                    /* requesting.                      */
                                    if(DeviceInfo->ServerInfo.Enabled_New_Alert_Categories & SupportedCategories)
                                    {
                                       /* Enable the selected category  */
                                       /* for this client.              */
                                       SendNewAlertNotification(0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category, "   ");
                                    }
                                    else
                                       Display(("Client has not enabled the requested New Alert category!\r\n"));
                                 }
                                 else
                                    Display(("Server does not support desired New Alert category.\r\n"));
                              }
                           }
                           else
                              DisplayFunctionError("ANS_Query_Supported_Categories", Result);
                        }
                        else
                           Display(("Client has not registered for New Alert notifications.\r\n"));
                        break;

                     case pcNotify_Unread_Category_Immediately:
                        Display(("   Command:          pcNotify_Unread_Category_Immediately.\r\n\r\n"));

                        /* Verify that the client has registered for    */
                        /* Unread Alert Notifications.                  */
                        if(DeviceInfo->ServerInfo.Unread_Alert_Status_Client_Configuration & GATT_CLIENT_CONFIGURATION_CHARACTERISTIC_NOTIFY_ENABLE)
                        {
                           /* Check if the new alert is supported.      */
                           if((Result = ANS_Query_Supported_Categories(BluetoothStackID, ANSInstanceID, scUnreadAlertStatus, &SupportedCategories)) == 0)
                           {
                              /* Check if the client wants to alert all */
                              /* supported categories                   */
                              if(ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category == ciAllCategories)
                              {
                                 /* Verify that the server supports at  */
                                 /* least one category                  */
                                 if(SupportedCategories)
                                 {
                                    /* Verify that the client has       */
                                    /* enabled at least one of the      */
                                    /* supported categories.            */
                                    if(DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories & SupportedCategories)
                                    {
                                       /* Mask the disabled categories. */
                                       SupportedCategories &= DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories;

                                       /* Send an alert for all         */
                                       /* supported categories to this  */
                                       /* client.                       */
                                       SendUnreadAlertNotification(SupportedCategories, "   ");
                                    }
                                    else
                                       Display(("Client has not enabled any supported Unread Alert categories!\r\n"));
                                 }
                                 else
                                    Display(("Server does not support ANY Unread Alert categories!\r\n"));
                              }
                              else
                              {
                                 /* Verify that the server supports the */
                                 /* category                            */
                                 if(SupportedCategories & (0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category))
                                 {
                                    /* Verify that the client has       */
                                    /* enabled the category it is       */
                                    /* requesting.                      */
                                    if(DeviceInfo->ServerInfo.Enabled_Unread_Alert_Categories & SupportedCategories)
                                    {
                                       /* Enable the selected category  */
                                       /* for this client.              */
                                       SendUnreadAlertNotification(0x1 << ANS_Event_Data->Event_Data.ANS_Control_Point_Command_Data->Category, "   ");
                                    }
                                    else
                                       Display(("Client has not enabled the requested Unread Alert category!\r\n"));
                                 }
                                 else
                                    Display(("Server does not support desired Unread Alert category.\r\n"));
                              }
                           }
                           else
                              DisplayFunctionError("ANS_Query_Supported_Categories", Result);
                        }
                        else
                           Display(("Client has not registered for Unread Alert notifications.\r\n"));
                        break;

                     default:
                        Display(("   Command:          Unknown.\r\n"));
                        break;
                  }
               }
               else
               {
                  Display(("\r\nUnknown Client.\r\n"));
               }
            }
            break;
         default:
            Display(("Unknown ANS Event\r\n"));
            break;
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("ANS Callback Data: Event_Data = NULL.\r\n"));
   }

   DisplayPrompt();
}

   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_ANP(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;
   unsigned int  i;
   BoardStr_t    BoardStr;
   Word_t        SupportedCategories;
   int           Result;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               Display(("\r\nError Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout"));

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  Display(("   Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode));
                  Display(("   Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle));
                  Display(("   Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode));
                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
                  {
                     Display(("   Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]));
                  }
                  else
                  {
                     if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode == ANS_ERROR_CODE_COMMAND_NOT_SUPPORTED)
                     {
                        Display(("   Error Mesg:      Not Supported.\r\n"));
                     }
                     else
                        Display(("   Error Mesg:      Unknown.\r\n"));
                  }
               }
            }
            else
               Display(("Error - Null Error Response Data.\r\n"));
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               Display(("\r\nRead Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Data Length:     %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength));

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what read response this */
               /* is.                                                   */
               if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength != 0)
               {
                  if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter))
                  {
                     if(CallbackParameter == DeviceInfo->ClientInfo.Supported_New_Alert_Category)
                     {
                        if((Result = ANS_Decode_Supported_Categories(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, &SupportedCategories)) == 0)
                        {
                           Display(("\r\n"));
                           DisplaySupportedCategories(SupportedCategories, "   ", "New");
                        }
                        else
                           DisplayFunctionError("ANS_Decode_Supported_Categories", Result);
                     }
                     else
                     {
                        if(CallbackParameter == DeviceInfo->ClientInfo.Supported_Unread_Alert_Category)
                        {
                           if((Result = ANS_Decode_Supported_Categories(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, &SupportedCategories)) == 0)
                           {
                              Display(("\r\n"));
                              DisplaySupportedCategories(SupportedCategories, "   ", "Unread");
                           }
                           else
                              DisplayFunctionError("ANS_Decode_Supported_Categories", Result);
                        }
                        else
                        {
                           /* Could not find a descriptor to match the  */
                           /* read response, so display raw data        */
                           CallbackParameter = 0;
                        }
                     }
                  }

                  /* If the data has not been decoded and displayed,    */
                  /* then just display the raw data                     */
                  if((!DeviceInfo) || (!CallbackParameter))
                  {
                     Display(("   Data:            { "));
                     for(i = 0; i < (GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength - 1); i++)
                        Display(("0x%02x, ", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue[i]));

                     Display(("0x%02x }\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue[i]));
                  }
               }
            }
            else
               Display(("\r\nError - Null Read Response Data.\r\n"));
            break;
         case etGATT_Client_Exchange_MTU_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data)
            {
               Display(("\r\nExchange MTU Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   MTU:             %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Exchange_MTU_Response_Data->ServerMTU));
            }
            else
               Display(("\r\nError - Null Write Response Data.\r\n"));
            break;
         case etGATT_Client_Write_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data)
            {
               Display(("\r\nWrite Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Bytes Written:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->BytesWritten));

               /* If we know about this device and a callback parameter */
               /* exists, then check if we know what write response this*/
               /* is.                                                   */
               if(((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Write_Response_Data->RemoteDevice)) != NULL) && (CallbackParameter))
               {
                  if(((Word_t)CallbackParameter) == DeviceInfo->ClientInfo.New_Alert_Client_Configuration)
                  {
                     Display(("\r\nWrite New Alert CC Compete.\r\n"));
                  }
                  else
                  {
                     if(((Word_t)CallbackParameter) == DeviceInfo->ClientInfo.Unread_Alert_Status_Client_Configuration)
                     {
                        Display(("\r\nWrite Unread Alert Status CC Compete.\r\n"));
                     }
                     else
                     {
                        if(((Word_t)CallbackParameter) == DeviceInfo->ClientInfo.Control_Point)
                        {
                           Display(("\r\nControl Point Command Complete.\r\n"));
                        }
                     }
                  }
               }
            }
            else
               Display(("\r\nError - Null Write Response Data.\r\n"));
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}


   /* The following function is for an GATT Client Event Callback.  This*/
   /* function will be called whenever a GATT Response is received for a*/
   /* request that was made when this function was registered.  This    */
   /* function passes to the caller the GATT Client Event Data that     */
   /* occurred and the GATT Client Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the GATT Client Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer.  This function is guaranteed NOT to be       */
   /* invoked more than once simultaneously for the specified installed */
   /* callback (i.e.  this function DOES NOT have be reentrant).  It    */
   /* Needs to be noted however, that if the same Callback is installed */
   /* more than once, then the callbacks will be called serially.       */
   /* Because of this, the processing in this function should be as     */
   /* efficient as possible.  It should also be noted that this function*/
   /* is called in the Thread Context of a Thread that the User does NOT*/
   /* own.  Therefore, processing in this function should be as         */
   /* efficient as possible (this argument holds anyway because another */
   /* GATT Event (Server/Client or Connection) will not be processed    */
   /* while this function call is outstanding).                         */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_ClientEventCallback_GAPS(unsigned int BluetoothStackID, GATT_Client_Event_Data_t *GATT_Client_Event_Data, unsigned long CallbackParameter)
{
   char         *NameBuffer;
   Word_t        Appearance;
   BoardStr_t    BoardStr;
   DeviceInfo_t *DeviceInfo;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Client_Event_Data))
   {
      /* Determine the event that occurred.                             */
      switch(GATT_Client_Event_Data->Event_Data_Type)
      {
         case etGATT_Client_Error_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data)
            {
               Display(("\r\nError Response.\r\n"));
               BD_ADDRToStr(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionID));
               Display(("   Transaction ID:  %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->TransactionID));
               Display(("   Connection Type: %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ConnectionType == gctLE)?"LE":"BR/EDR"));
               Display(("   BD_ADDR:         %s.\r\n", BoardStr));
               Display(("   Error Type:      %s.\r\n", (GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)?"Response Error":"Response Timeout"));

               /* Only print out the rest if it is valid.               */
               if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorType == retErrorResponse)
               {
                  Display(("   Request Opcode:  0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestOpCode));
                  Display(("   Request Handle:  0x%04X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->RequestHandle));
                  Display(("   Error Code:      0x%02X.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode));
                  if(GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode < NUMBER_OF_ERROR_CODES)
                  {
                     Display(("   Error Mesg:      %s.\r\n", ErrorCodeStr[GATT_Client_Event_Data->Event_Data.GATT_Request_Error_Data->ErrorCode]));
                  }
                  else
                     Display(("   Error Mesg:      Unknown.\r\n"));
               }
            }
            else
               Display(("Error - Null Error Response Data.\r\n"));
            break;
         case etGATT_Client_Read_Response:
            if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data)
            {
               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->RemoteDevice)) != NULL)
               {
                  if((Word_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceNameHandle)
                  {
                     /* Display the remote device name.                 */
                     if((NameBuffer = (char *)BTPS_AllocateMemory(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1)) != NULL)
                     {
                        BTPS_MemInitialize(NameBuffer, 0, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength+1);
                        BTPS_MemCopy(NameBuffer, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue, GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength);

                        Display(("\r\nRemote Device Name: %s.\r\n", NameBuffer));

                        BTPS_FreeMemory(NameBuffer);
                     }
                  }
                  else
                  {
                     if((Word_t)CallbackParameter == DeviceInfo->GAPSClientInfo.DeviceAppearanceHandle)
                     {
                        if(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength == GAP_DEVICE_APPEARENCE_VALUE_LENGTH)
                        {
                           Appearance = READ_UNALIGNED_WORD_LITTLE_ENDIAN(GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValue);
                           if(AppearanceToString(Appearance, &NameBuffer))
                              Display(("\r\nRemote Device Appearance: %s(%u).\r\n", NameBuffer, Appearance));
                           else
                              Display(("\r\nRemote Device Appearance: Unknown(%u).\r\n", Appearance));
                        }
                        else
                           Display(("Invalid Remote Appearance Value Length %u.\r\n", GATT_Client_Event_Data->Event_Data.GATT_Read_Response_Data->AttributeValueLength));
                     }
                  }
               }
            }
            else
               Display(("\r\nError - Null Read Response Data.\r\n"));
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Connection Event Callback.  */
   /* This function is called for GATT Connection Events that occur on  */
   /* the specified Bluetooth Stack.  This function passes to the caller*/
   /* the GATT Connection Event Data that occurred and the GATT         */
   /* Connection Event Callback Parameter that was specified when this  */
   /* Callback was installed.  The caller is free to use the contents of*/
   /* the GATT Client Event Data ONLY in the context of this callback.  */
   /* If the caller requires the Data for a longer period of time, then */
   /* the callback function MUST copy the data into another Data Buffer.*/
   /* This function is guaranteed NOT to be invoked more than once      */
   /* simultaneously for the specified installed callback (i.e.  this   */
   /* function DOES NOT have be reentrant).  It Needs to be noted       */
   /* however, that if the same Callback is installed more than once,   */
   /* then the callbacks will be called serially.  Because of this, the */
   /* processing in this function should be as efficient as possible.   */
   /* It should also be noted that this function is called in the Thread*/
   /* Context of a Thread that the User does NOT own.  Therefore,       */
   /* processing in this function should be as efficient as possible    */
   /* (this argument holds anyway because another GATT Event            */
   /* (Server/Client or Connection) will not be processed while this    */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_Connection_Event_Callback(unsigned int BluetoothStackID, GATT_Connection_Event_Data_t *GATT_Connection_Event_Data, unsigned long CallbackParameter)
{
   ANS_Un_Read_Alert_Data_t  UnReadAlert;
   ANS_New_Alert_Data_t     *NewAlertData;
   DeviceInfo_t             *DeviceInfo;
   BoardStr_t                BoardStr;
   int                       Result;

   /* Verify that all parameters to this callback are Semi-Valid.       */
   if((BluetoothStackID) && (GATT_Connection_Event_Data))
   {
      /* Determine the Connection Event that occurred.                  */
      switch(GATT_Connection_Event_Data->Event_Data_Type)
      {
         case etGATT_Connection_Device_Connection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data)
            {
               /* Save the Connection ID for later use.                 */
               ConnectionID = GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID;

               Display(("\r\netGATT_Connection_Device_Connection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionID));
               Display(("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:   %s.\r\n", BoardStr));
               Display(("   Connection MTU:  %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->MTU));

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Device_Connection_Data->RemoteDevice)) != NULL)
               {
                  if(LocalDeviceIsMaster)
                  {
                     /* Attempt to update the MTU to the maximum        */
                     /* supported.                                      */
                     GATT_Exchange_MTU_Request(BluetoothStackID, ConnectionID, BTPS_CONFIGURATION_GATT_MAXIMUM_SUPPORTED_MTU_SIZE, GATT_ClientEventCallback_ANP, 0);
                  }
               }
            }
            else
               Display(("Error - Null Connection Data.\r\n"));
            break;
         case etGATT_Connection_Device_Disconnection:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data)
            {
               /* Clear the Connection ID.                              */
               ConnectionID = 0;

               Display(("\r\netGATT_Connection_Device_Disconnection with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:   %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionID));
               Display(("   Connection Type: %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Device_Disconnection_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:   %s.\r\n", BoardStr));
            }
            else
               Display(("Error - Null Disconnection Data.\r\n"));
            break;
         case etGATT_Connection_Server_Notification:
            if(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data)
            {
               Display(("\r\netGATT_Connection_Server_Notification with size %u: \r\n", GATT_Connection_Event_Data->Event_Data_Size));
               BD_ADDRToStr(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice, BoardStr);
               Display(("   Connection ID:    %u.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionID));
               Display(("   Connection Type:  %s.\r\n", ((GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->ConnectionType == gctLE)?"LE":"BR/EDR")));
               Display(("   Remote Device:    %s.\r\n", BoardStr));
               Display(("   Attribute Handle: 0x%04X.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle));
               Display(("   Attribute Length: %d.\r\n", GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength));

               if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->RemoteDevice)) != NULL)
               {
                  if(DeviceInfo->ClientInfo.New_Alert == GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle)
                  {
                     /* Decode and display the data.                    */
                     if((NewAlertData = ANS_Decode_New_Alert_Notification(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue)) != NULL)
                     {
                        DisplayNewAlertStatus(NewAlertData->CategoryID, NewAlertData->NumberOfNewAlerts, NewAlertData->LastAlertString, "   ");

                        ANS_Free_New_Alert_Data(NewAlertData);
                     }
                  }
                  else
                  {
                     if(DeviceInfo->ClientInfo.Unread_Alert_Status == GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeHandle)
                     {
                        /* Decode and display the data.                 */
                        if((Result = ANS_Decode_Un_Read_Alert_Status_Notification(GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValueLength, GATT_Connection_Event_Data->Event_Data.GATT_Server_Notification_Data->AttributeValue, &UnReadAlert)) == 0)
                        {
                           DisplayUnreadAlertStatus(UnReadAlert.CategoryID, UnReadAlert.NumberOfUnreadAlerts, "   ");
                        }
                        else
                           DisplayFunctionError("ANS_Decode_Un_Read_Alert_Status_Notification", Result);
                     }
                     else
                        Display(("Error - Unknown Notification Atrribute Handle.\r\n"));
                  }
               }
               else
                  Display(("Error - Remote Server Unknown.\r\n"));
            }
            else
               Display(("Error - Null Server Notification Data.\r\n"));
            break;
      }

      /* Print the command line prompt.                                 */
      DisplayPrompt();
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Connection Callback Data: Event_Data = NULL.\r\n"));

      DisplayPrompt();
   }
}

   /* The following function is for an GATT Discovery Event Callback.   */
   /* This function will be called whenever a GATT Service is discovered*/
   /* or a previously started service discovery process is completed.   */
   /* This function passes to the caller the GATT Discovery Event Data  */
   /* that occurred and the GATT Client Event Callback Parameter that   */
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the GATT Discovery Event Data ONLY in */
   /* the context of this callback.  If the caller requires the Data for*/
   /* a longer period of time, then the callback function MUST copy the */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e.  this function DOES NOT have be          */
   /* reentrant).  It Needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another GATT Discovery Event will not be processed while this     */
   /* function call is outstanding).                                    */
   /* * NOTE * This function MUST NOT Block and wait for Events that can*/
   /*          only be satisfied by Receiving a Bluetooth Event         */
   /*          Callback.  A Deadlock WILL occur because NO Bluetooth    */
   /*          Callbacks will be issued while this function is currently*/
   /*          outstanding.                                             */
static void BTPSAPI GATT_Service_Discovery_Event_Callback(unsigned int BluetoothStackID, GATT_Service_Discovery_Event_Data_t *GATT_Service_Discovery_Event_Data, unsigned long CallbackParameter)
{
   DeviceInfo_t *DeviceInfo;

   /* Verify that the input parameters are semi-valid.                  */
   if((BluetoothStackID) && (GATT_Service_Discovery_Event_Data))
   {
      if((DeviceInfo = SearchDeviceInfoEntryByBD_ADDR(&DeviceInfoList, ConnectionBD_ADDR)) != NULL)
      {
         switch(GATT_Service_Discovery_Event_Data->Event_Data_Type)
         {
            case etGATT_Service_Discovery_Indication:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data)
               {
                  Display(("\r\n"));
                  Display(("Service 0x%04X - 0x%04X, UUID: ", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.Service_Handle, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.End_Group_Handle));
                  DisplayUUID(&(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data->ServiceInformation.UUID));
                  Display(("\r\n"));

                  if(((Service_Discovery_Type_t)CallbackParameter) == sdGAPS)
                  {
                     /* Attempt to populate the handles for the GAP     */
                     /* Service.                                        */
                     GAPSPopulateHandles(&(DeviceInfo->GAPSClientInfo), GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                  }

                  /* Check the service discovery type for ANS           */
                  if(((Service_Discovery_Type_t)CallbackParameter) == sdANS)
                  {
                     /* Attempt to populate the handles for the ANP     */
                     /* Service.                                        */
                     ANSPopulateHandles(DeviceInfo, GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Indication_Data);
                  }
               }
               break;
            case etGATT_Service_Discovery_Complete:
               /* Verify the event data.                                */
               if(GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data)
               {
                  Display(("\r\n"));
                  Display(("Service Discovery Operation Complete, Status 0x%02X.\r\n", GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status));

                  /* Flag that no service discovery operation is        */
                  /* outstanding for this device.                       */
                  DeviceInfo->Flags &= ~DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_OUTSTANDING;

                  /* Check the service discovery status and type for    */
                  /* ANS.                                               */
                  if((GATT_Service_Discovery_Event_Data->Event_Data.GATT_Service_Discovery_Complete_Data->Status == GATT_SERVICE_DISCOVERY_STATUS_SUCCESS) && (((Service_Discovery_Type_t)CallbackParameter) == sdANS))
                  {
                     /* Flag that service discovery has been performed  */
                     /* on for this connection.                         */
                     DeviceInfo->Flags |= DEVICE_INFO_FLAGS_SERVICE_DISCOVERY_COMPLETE;

                     /* Print a summary of what descriptor were found   */
                     Display(("\r\nANS Service Discovery Summary\r\n"));
                     Display(("   Supported New Alert Category:    %s\r\n", (DeviceInfo->ClientInfo.Supported_New_Alert_Category             ? "Supported" : "Not Supported")));
                     Display(("   New Alert:                       %s\r\n", (DeviceInfo->ClientInfo.New_Alert                                ? "Supported" : "Not Supported")));
                     Display(("   New Alert CC:                    %s\r\n", (DeviceInfo->ClientInfo.New_Alert_Client_Configuration           ? "Supported" : "Not Supported")));
                     Display(("   Supported Unread Alert Category: %s\r\n", (DeviceInfo->ClientInfo.Supported_Unread_Alert_Category          ? "Supported" : "Not Supported")));
                     Display(("   Unread Alert Status:             %s\r\n", (DeviceInfo->ClientInfo.Unread_Alert_Status                      ? "Supported" : "Not Supported")));
                     Display(("   Unread Alert Status CC:          %s\r\n", (DeviceInfo->ClientInfo.Unread_Alert_Status_Client_Configuration ? "Supported" : "Not Supported")));
                     Display(("   Control Point:                   %s\r\n", (DeviceInfo->ClientInfo.Control_Point                            ? "Supported" : "Not Supported")));
                  }
               }
               break;
         }

         DisplayPrompt();
      }
   }
   else
   {
      /* There was an error with one or more of the input parameters.   */
      Display(("\r\n"));

      Display(("GATT Callback Data: Event_Data = NULL.\r\n"));

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
   int ret_val = APPLICATION_ERROR_UNABLE_TO_OPEN_STACK;

   /* Next, makes sure that the Driver Information passed appears to be */
   /* semi-valid.                                                       */
   if((HCI_DriverInformation) && (BTPS_Initialization))
   {
      /* Try to Open the stack and check if it was successful.          */
      if(!OpenStack(HCI_DriverInformation, BTPS_Initialization))
      {
         /* First, attempt to set the Device to be Connectable.         */
         ret_val = SetConnect();

         /* Next, check to see if the Device was successfully made      */
         /* Connectable.                                                */
         if(!ret_val)
         {
            /* Now that the device is Connectable attempt to make it    */
            /* Discoverable.                                            */
            ret_val = SetDisc();

            /* Next, check to see if the Device was successfully made   */
            /* Discoverable.                                            */
            if(!ret_val)
            {
               /* Now that the device is discoverable attempt to make it*/
               /* pairable.                                             */
               ret_val = SetPairable();
               if(!ret_val)
               {
                  /* Set up the Selection Interface.                    */
                  UserInterface();

                  /* Display a list of available commands.              */
                  DisplayHelp(NULL);

                  /* Display the first command prompt.                  */
                  DisplayPrompt();

                  /* Return success to the caller.                      */
                  ret_val = (int)BluetoothStackID;
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
            /* Close the Bluetooth Stack.                               */
            CloseStack();
         }
      }
      else
      {
         /* There was an error while attempting to open the Stack.      */
         Display(("Unable to open the stack.\r\n"));
      }
   }
   else
      ret_val = APPLICATION_ERROR_INVALID_PARAMETERS;

   return(ret_val);
}

   /* The following function is used to process a command line string.  */
   /* This function takes as it's only parameter the command line string*/
   /* to be parsed and returns TRUE if a command was parsed and executed*/
   /* or FALSE otherwise.                                               */
Boolean_t ProcessCommandLine(char *String)
{
   return(CommandLineInterpreter(String));
}
