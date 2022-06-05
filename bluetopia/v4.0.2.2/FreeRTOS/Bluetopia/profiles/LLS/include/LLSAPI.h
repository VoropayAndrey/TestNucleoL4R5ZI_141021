/*****< llsapi.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  LLSAPI - Stonestreet One Bluetooth Link Loss Service (GATT                */
/*           based) API Type Definitions, Constants, and Prototypes.          */
/*                                                                            */
/*  Author:  Ajay Parashar                                                    */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/25/12  A. Parashar      Initial creation.                             */
/******************************************************************************/
#ifndef __LLSAPIH__
#define __LLSAPIH__

#include "SS1BTPS.h"  /* Bluetooth Stack API Prototypes/Constants.            */
#include "SS1BTGAT.h" /* Bluetooth Stack GATT API Prototypes/Constants.       */
#include "LLSTypes.h" /* Link Loss Notification Service Types                 */

   /* Error Return Codes.                                               */

   /* Error Codes that are smaller than these (less than -1000) are     */
   /* related to the Bluetooth Protocol Stack itself (see BTERRORS.H).  */
#define LLS_ERROR_INVALID_PARAMETER                      (-1000)
#define LLS_ERROR_INVALID_BLUETOOTH_STACK_ID             (-1001)
#define LLS_ERROR_INSUFFICIENT_RESOURCES                 (-1002)
#define LLS_ERROR_SERVICE_ALREADY_REGISTERED             (-1003)
#define LLS_ERROR_INVALID_INSTANCE_ID                    (-1004)
#define LLS_ERROR_MALFORMATTED_DATA                      (-1005)
#define LLS_ERROR_UNKNOWN_ERROR                          (-1006)

   /* The following structure contains the Handles that will need to be */
   /* cached by a LLS client in order to only do service discovery once.*/
typedef struct _tagLLS_Client_Information_t
{
   Word_t Alert_Level;
} LLS_Client_Information_t;

#define LLS_CLIENT_INFORMATION_DATA_SIZE                (sizeof( LLS_Client_Information_t))

   /* The following structure contains the Handles that will need to be */
   /* cached by a LLS Server in order to only do service discovery once.*/
typedef struct _tagLLS_Server_Information_t
{
   Word_t Alert_Level;
} LLS_Server_Information_t;

#define LLS_SERVER_INFORMATION_DATA_SIZE              (sizeof( LLS_Server_Information_t))

   /* The following enumeration covers all the events generated by the  */
   /* LLS Profile.  These are used to determine the type of each event  */
   /* generated, and to ensure the proper union element is accessed for */
   /* the LLS_Event_Data_t structure.                                   */
typedef enum
{
   etLLS_Alert_Level_Update
} LLS_Event_Type_t;

   /* The following LLS Profile Event is dispatched to a LLS Server when*/
   /* a LLS Client is attempting to write the Alert Level               */
   /* characteristic.  The ConnectionID, ConnectionType, and            */
   /* RemoteDevice specifiy the Client that is making the update.  The  */
   /* final member is the new Alert Level.                              */
typedef struct _tagLLS_Alert_Level_Update_Data_t
{
   unsigned int           InstanceID;
   unsigned int           ConnectionID;
   GATT_Connection_Type_t ConnectionType;
   BD_ADDR_t              RemoteDevice;
   Byte_t                 AlertLevel;
} LLS_Alert_Level_Update_Data_t;

#define LLS_ALERT_LEVEL_UPDATE_DATA_SIZE              (sizeof(LLS_Alert_Level_Update_Data_t))

   /* The following structure represents the container structure for    */
   /* holding all LLS Profile Event Data.  This structure is received   */
   /* for each event generated.  The Event_Data_Type member is used to  */
   /* determine the appropriate union member element to access the      */
   /* contained data.  The Event_Data_Size member contains the total    */
   /* size of the data contained in this event.                         */
typedef struct _tagLLS_Event_Data_t
{
   LLS_Event_Type_t Event_Data_Type;
   Word_t           Event_Data_Size;
   union
   {
      LLS_Alert_Level_Update_Data_t *LLS_Alert_Level_Update_Data;
   } Event_Data;
} LLS_Event_Data_t;

#define LLS_EVENT_DATA_SIZE                             (sizeof(LLS_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a LLS Profile Event Receive Data Callback.  This function will be */
   /* called whenever an LLS Profile Event occurs that is associated    */
   /* with the specified Bluetooth Stack ID.  This function passes to   */
   /* the caller the Bluetooth Stack ID, the LLS Event Data that        */
   /* occurred and the LLS Profile Event Callback Parameter that was    */
   /* specified when this Callback was installed.  The caller is free to*/
   /* use the contents of the LLS Profile Event Data ONLY in the context*/
   /* of this callback.  If the caller requires the Data for a longer   */
   /* period of time, then the callback function MUST copy the data into*/
   /* another Data Buffer This function is guaranteed NOT to be invoked */
   /* more than once simultaneously for the specified installed callback*/
   /* (i.e.  this function DOES NOT have be re-entrant).  It needs to be*/
   /* noted however, that if the same Callback is installed more than   */
   /* once, then the callbacks will be called serially.  Because of     */
   /* this, the processing in this function should be as efficient as   */
   /* possible.  It should also be noted that this function is called in*/
   /* the Thread Context of a Thread that the User does NOT own.        */
   /* Therefore, processing in this function should be as efficient as  */
   /* possible (this argument holds anyway because another LLS Profile  */
   /* Event will not be processed while this function call is           */
   /* outstanding).                                                     */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving LLS Profile Event   */
   /*            Packets.  A Deadlock WILL occur because NO LLS Event   */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *LLS_Event_Callback_t)(unsigned int BluetoothStackID, LLS_Event_Data_t *LLS_Event_Data, unsigned long CallbackParameter);

   /* LLS Server API.                                                   */

   /* The following function is responsible for opening a LLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The final parameter is a      */
   /* pointer to store the GATT Service ID of the registered LLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  This function returns the positive, non-zero, Instance*/
   /* ID or a negative error code.                                      */
   /* * NOTE * Only 1 LLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI LLS_Initialize_Service(unsigned int BluetoothStackID, LLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LLS_Initialize_Service_t)(unsigned int BluetoothStackID, LLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID);
#endif

   /* The following function is responsible for opening a LLS Server.   */
   /* The first parameter is the Bluetooth Stack ID on which to open the*/
   /* server.  The second parameter is the Callback function to call    */
   /* when an event occurs on this Server Port.  The third parameter is */
   /* a user-defined callback parameter that will be passed to the      */
   /* callback function with each event.  The fourth parameter is a     */
   /* pointer to store the GATT Service ID of the registered LLS        */
   /* service.  This can be used to include the service registered by   */
   /* this call.  The final parameter is a pointer, that on input can be*/
   /* used to control the location of the service in the GATT database, */
   /* and on ouput to store the service handle range.  This function    */
   /* returns the positive, non-zero, Instance ID or a negative error   */
   /* code.                                                             */
   /* * NOTE * Only 1 LLS Server may be open at a time, per Bluetooth   */
   /*          Stack ID.                                                */
   /* * NOTE * All Client Requests will be dispatch to the EventCallback*/
   /*          function that is specified by the second parameter to    */
   /*          this function.                                           */
BTPSAPI_DECLARATION int BTPSAPI LLS_Initialize_Service_Handle_Range(unsigned int BluetoothStackID, LLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LLS_Initialize_Service_Handle_Range_t)(unsigned int BluetoothStackID, LLS_Event_Callback_t EventCallback, unsigned long CallbackParameter, unsigned int *ServiceID, GATT_Attribute_Handle_Group_t *ServiceHandleRange);
#endif

   /* The following function is responsible for closing a previously    */
   /* opened LLS Server.  The first parameter is the Bluetooth Stack ID */
   /* on which to close the server.  The second parameter is the        */
   /* InstanceID that was returned from a successfully call to          */
   /* LLS_Initialize_Service().  This function returns a zero if        */
   /* successful or a negative return error code if an error occurs.    */
BTPSAPI_DECLARATION int BTPSAPI LLS_Cleanup_Service(unsigned int BluetoothStackID, unsigned int InstanceID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LLS_Cleanup_Service_t)(unsigned int BluetoothStackID, unsigned int InstanceID);
#endif

   /* The following function is responsible for querying the number of  */
   /* attributes that are contained in the LLS Service that is          */
   /* registered with a call to LLS_Initialize_Service().  This function*/
   /* returns the non-zero number of attributes that are contained in a */
   /* LLS Server or zero on failure.                                    */
BTPSAPI_DECLARATION unsigned int BTPSAPI LLS_Query_Number_Attributes(void);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef unsigned int (BTPSAPI *PFN_LLS_Query_Number_Attributes_t)(void);
#endif

   /* The following function is responsible for setting the Alert Level */
   /* on the specified LLS Instance.  The first parameter is            */
   /* Bluetooth Stack ID of the Bluetooth Device. The second parameter  */
   /* is the InstanceID returned from a successful call to              */
   /* LLS_Initialize_Server().  The final parameter is the              */
   /* to set for the specified LLS Instance.  This function returns     */
   /* a zero if successful or a negative return error code if an error  */
   /* occurs.                                                           */
BTPSAPI_DECLARATION int BTPSAPI LLS_Set_Alert_Level(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Alert_Level);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LLS_Set_Alert_Level_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t Alert_Level);
#endif

   /* The following function is responsible for querying the Alert Level*/
   /* on the specified LLS Instance.The first parameter is              */
   /* Bluetooth Stack ID of  Bluetooth Device. The second parameter is  */
   /* InstanceID returned from a successful call LLS_Initialize_Server  */
   /* The final parameter is a pointer to return the Alert Level        */
   /* specified LLS Instance. This function returns a zero if successful*/
   /* or a negative return error code if an error occurs.               */
BTPSAPI_DECLARATION int BTPSAPI LLS_Query_Alert_Level(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Alert_Level);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_LLS_Query_Alert_Level_t)(unsigned int BluetoothStackID, unsigned int InstanceID, Byte_t *Alert_Level);
#endif

#endif
