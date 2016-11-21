/******************************************************************************
*
* Copyright (C) 2015 - 2016 Xilinx, Inc. All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdprxss_intr.c
* @addtogroup dprxss_v4_0
* @{
*
* This file contains interrupt related functions of Xilinx DisplayPort RX
* Subsystem core. Please see xdprxss.h for more details of the core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 sha 05/18/15 Initial release.
* 2.00 sha 10/05/15 Removed setting HDCP callbacks.
*                   Added HDCP and Timer Counter interrupt handler.
* 3.0  sha 02/19/16 Added switch case for XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT
*                   to register callback with HDCP.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xdprxss.h"

/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the DisplayPort RX core operating
* in RX mode.
*
* The application is responsible for connecting this function to the interrupt
* system. Application beyond this driver is also responsible for providing
* callbacks to handle interrupts and installing the callbacks using
* XDpRxSs_SetCallBack() during initialization phase.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_DpIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* DisplayPort RX interrupt handler */
	XDp_InterruptHandler(XDpRxSsPtr->DpPtr);
}

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
/*****************************************************************************/
/**
*
* This function is the interrupt handler for the HDCP Cipher core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_HdcpIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* HDCP Cipher interrupt handler */
	XHdcp1x_CipherIntrHandler(XDpRxSsPtr->Hdcp1xPtr);
}

/*****************************************************************************/
/**
*
* This function is the interrupt handler for the Timer Counter core.
*
* The application is responsible for connecting this function to the interrupt
* system.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance that
*		just interrupted.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_TmrCtrIntrHandler(void *InstancePtr)
{
	XDpRxSs *XDpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify arguments. */
	Xil_AssertVoid(XDpRxSsPtr != NULL);
	Xil_AssertVoid(XDpRxSsPtr->IsReady == XIL_COMPONENT_IS_READY);

	/* Timer Counter interrupt handler */
	XTmrCtr_InterruptHandler(XDpRxSsPtr->TmrCtrPtr);
}
#endif

/*****************************************************************************/
/**
*
* This function installs an asynchronous callback function for the given
* HandlerType:
*
* <pre>
* HandlerType                              Callback Function Type
* ---------------------------------------- -----------------------------------
* XDPRXSS_HANDLER_DP_VM_CHG_EVENT          XDp_RxSetIntrVmChangeHandler
* XDPRXSS_HANDLER_DP_PWR_CHG_EVENT         XDp_RxSetIntrPowerStateHandler
* XDPRXSS_HANDLER_DP_NO_VID_EVENT          XDp_RxSetIntrNoVideoHandler
* XDPRXSS_HANDLER_DP_VBLANK_EVENT          XDp_RxSetIntrVBlankHandler
* XDPRXSS_HANDLER_DP_TLOST_EVENT           XDp_RxSetIntrTrainingLostHandler
* XDPRXSS_HANDLER_DP_VID_EVENT             XDp_RxSetIntrVideoHandler
* XDPRXSS_HANDLER_DP_INFO_PKT_EVENT        XDp_RxSetIntrInfoPktHandler
* XDPRXSS_HANDLER_DP_EXT_PKT_EVENT         XDp_RxSetIntrExtPktHandler
* XDPRXSS_HANDLER_DP_TDONE_EVENT           XDp_RxSetIntrTrainingDoneHandler
* XDPRXSS_HANDLER_DP_BW_CHG_EVENT          XDp_RxSetIntrBwChangeHandler
* XDPRXSS_HANDLER_DP_DWN_REQ_EVENT         XDp_RxSetIntrDownReqHandler
* XDPRXSS_HANDLER_DP_DWN_REP_EVENT         XDp_RxSetIntrDownReplyHandler
* XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT      XDp_RxSetIntrAudioOverHandler
* XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT   XDp_RxSetIntrPayloadAllocHandler
* XDPRXSS_HANDLER_DP_ACT_RX_EVENT          XDp_RxSetIntrActRxHandler
* XDPRXSS_HANDLER_DP_CRC_TEST_EVENT        XDp_RxSetIntrCrcTestHandler
* XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT     XHdcp1x_SetCallBack
* XDPRXSS_HANDLER_UNPLUG_EVENT             UnplugCallback
* XDPRXSS_HANDLER_LINKBW_EVENT             LinkBwCallback
* XDPRXSS_HANDLER_PLL_RESET_EVENT          PllResetCallback
* </pre>
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
* @param	HandlerType specifies the type of handler.
* @param	CallbackFunc is the address of the callback function.
* @param	CallbackRef is a user data item that will be passed to the
*		callback function when it is invoked.
*
* @return
*		- XST_SUCCESS if callback function installed successfully.
*		- XST_INVALID_PARAM when HandlerType is invalid.
*
* @note		Invoking this function for a handler that already has been
*		installed replaces it with the new handler.
*
******************************************************************************/
u32 XDpRxSs_SetCallBack(XDpRxSs *InstancePtr, u32 HandlerType,
			void *CallbackFunc, void *CallbackRef)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(HandlerType >= XDPRXSS_HANDLER_DP_VM_CHG_EVENT);
	Xil_AssertNonvoid(CallbackFunc != NULL);
	Xil_AssertNonvoid(CallbackRef != NULL);

	/* Assign callback based on handler type */
	switch (HandlerType) {
		case XDPRXSS_HANDLER_DP_VM_CHG_EVENT:
			XDp_RxSetIntrVmChangeHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_PWR_CHG_EVENT:
			XDp_RxSetIntrPowerStateHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_NO_VID_EVENT:
			XDp_RxSetIntrNoVideoHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VBLANK_EVENT:
			XDp_RxSetIntrVBlankHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_TLOST_EVENT:
			XDp_RxSetIntrTrainingLostHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_VID_EVENT:
			XDp_RxSetIntrVideoHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_INFO_PKT_EVENT:
			XDp_RxSetIntrInfoPktHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_EXT_PKT_EVENT:
			XDp_RxSetIntrExtPktHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_TDONE_EVENT:
			XDp_RxSetIntrTrainingDoneHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_BW_CHG_EVENT:
			XDp_RxSetIntrBwChangeHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_DWN_REQ_EVENT:
			XDp_RxSetIntrDownReqHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_DWN_REP_EVENT:
			XDp_RxSetIntrDownReplyHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_AUD_OVRFLW_EVENT:
			XDp_RxSetIntrAudioOverHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_PAYLOAD_ALLOC_EVENT:
			XDp_RxSetIntrPayloadAllocHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_ACT_RX_EVENT:
			XDp_RxSetIntrActRxHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_DP_CRC_TEST_EVENT:
			XDp_RxSetIntrCrcTestHandler(InstancePtr->DpPtr,
				CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;

#if (XPAR_XHDCP_NUM_INSTANCES > 0)
		case XDPRXSS_HANDLER_HDCP_RPTR_TDSA_EVENT:
			XHdcp1x_SetCallBack(InstancePtr->Hdcp1xPtr,
				XHDCP1X_RPTR_HDLR_TRIG_DOWNSTREAM_AUTH,
					CallbackFunc, CallbackRef);
			Status = XST_SUCCESS;
			break;
#endif

		case XDPRXSS_HANDLER_UNPLUG_EVENT:
			InstancePtr->UnplugCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->UnplugRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_LINKBW_EVENT:
			InstancePtr->LinkBwCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->LinkBwRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		case XDPRXSS_HANDLER_PLL_RESET_EVENT:
			InstancePtr->PllResetCallback =
				(XDpRxSs_Callback)((void *)CallbackFunc);
			InstancePtr->PllResetRef = CallbackRef;
			Status = XST_SUCCESS;
			break;

		default:
			Status = XST_INVALID_PARAM;
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
*
* This function installs a custom delay/sleep function to be used by the
* DisplayPort RX Subsystem.
*
* @param	InstancePtr is a pointer to the XDpRxSs core instance.
* @param	CallbackFunc is the address to the callback function.
* @param	CallbackRef is the user data item (microseconds to delay) that
*		will be passed to the custom sleep/delay function when it is
*		invoked.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XDpRxSs_SetUserTimerHandler(XDpRxSs *InstancePtr,
		XDpRxSs_TimerHandler CallbackFunc, void *CallbackRef)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(CallbackFunc != NULL);
	Xil_AssertVoid(CallbackRef != NULL);

	/* Set custom timer wait handler */
	XDp_SetUserTimerHandler(InstancePtr->DpPtr, CallbackFunc, CallbackRef);
}
/** @} */
