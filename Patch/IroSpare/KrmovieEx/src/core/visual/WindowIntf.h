//---------------------------------------------------------------------------
/*
	TVP2 ( T Visual Presenter 2 )  A script authoring tool
	Copyright (C) 2000 W.Dee <dee@kikyou.info> and contributors

	See details of license at "license.txt"
*/
//---------------------------------------------------------------------------
// "Window" TJS Class implementation
//---------------------------------------------------------------------------
#ifndef WindowIntfH
#define WindowIntfH

#include "tjsNative.h"
#include "drawable.h"
#include "ComplexRect.h"
#include "tvpinputdefs.h"
#include "EventIntf.h"
#include "ObjectList.h"
#include "DrawDevice.h"
#include "LayerTreeOwner.h"



//---------------------------------------------------------------------------
// Window List Management
//---------------------------------------------------------------------------
extern void TVPClearAllWindowInputEvents();
extern bool TVPIsWaitVSync();
//---------------------------------------------------------------------------




/*[*/
//---------------------------------------------------------------------------
// Window related constants
//---------------------------------------------------------------------------
enum tTVPUpdateType
{
	utNormal, // only needed region
	utEntire // entire of window
};
//---------------------------------------------------------------------------
enum tTVPBorderStyle
{
	bsNone=0,  bsSingle=1,  bsSizeable=2,  bsDialog=3,  bsToolWindow=4,
	bsSizeToolWin =5
};
//---------------------------------------------------------------------------
enum tTVPMouseCursorState
{
	mcsVisible, // the mouse cursor is visible
	mcsTempHidden, // the mouse cursor is temporarily hidden
	mcsHidden // the mouse cursor is invisible
};
//---------------------------------------------------------------------------
/*]*/

/*[*/
//---------------------------------------------------------------------------
//! @brief Window basic interface
//---------------------------------------------------------------------------
class iTVPWindow
{
public:
	//! @brief	���摜�̃T�C�Y���ύX���ꂽ
	//! @note	�`��f�o�C�X���A���摜�̃T�C�Y���ύX���ꂽ���Ƃ�ʒm���邽�߂ɌĂԁB
	//!			�E�B���h�E�� iTVPDrawDevice::GetSrcSize() ���Ăяo���Č��摜��
	//!			�T�C�Y���擾������A�Y�[���Ȃǂ̌v�Z���s���Ă��� 
	//!			iTVPDrawDevice::SetTargetWindow() ���Ăяo���B
	virtual void TJS_INTF_METHOD NotifySrcResize() = 0;

	//! @brief		�}�E�X�J�[�\���̌`����f�t�H���g�ɖ߂�
	//! @note		�}�E�X�J�[�\���̌`����f�t�H���g�̕��ɖ߂������ꍇ�ɌĂ�
	virtual void TJS_INTF_METHOD SetDefaultMouseCursor() = 0; // set window mouse cursor to default

	//! @brief		�}�E�X�J�[�\���̌`���ݒ肷��
	//! @param		cursor		�}�E�X�J�[�\���`��ԍ�
	virtual void TJS_INTF_METHOD SetMouseCursor(tjs_int cursor) = 0; // set window mouse cursor

	//! @brief		�}�E�X�J�[�\���̈ʒu���擾����
	//! @param		x			�`���`���̍��W�ɂ�����}�E�X�J�[�\����x�ʒu
	//! @param		y			�`���`���̍��W�ɂ�����}�E�X�J�[�\����y�ʒu
	virtual void TJS_INTF_METHOD GetCursorPos(tjs_int &x, tjs_int &y) = 0;
		// get mouse cursor position in primary layer's coordinates

	//! @brief		�}�E�X�J�[�\���̈ʒu��ݒ肷��
	//! @param		x			�`���`���̍��W�ɂ�����}�E�X�J�[�\����x�ʒu
	//! @param		y			�`���`���̍��W�ɂ�����}�E�X�J�[�\����y�ʒu
	virtual void TJS_INTF_METHOD SetCursorPos(tjs_int x, tjs_int y) = 0;

	//! @brief		�E�B���h�E�̃}�E�X�L���v�`�����������
	//! @note		�E�B���h�E�̃}�E�X�L���v�`����������ׂ��ꍇ�ɌĂԁB
	//! @note		���̃��\�b�h�ł͊�{�I�ɂ� ::ReleaseCapture() �Ȃǂ�
	//!				�}�E�X�̃L���v�`�����J�����邱�ƁB
	virtual void TJS_INTF_METHOD WindowReleaseCapture() = 0;

	//! @brief		�c�[���`�b�v�q���g��ݒ肷��
	//! @param		text		�q���g�e�L�X�g(�󕶎���̏ꍇ�̓q���g�̕\�����L�����Z������)
	virtual void TJS_INTF_METHOD SetHintText(iTJSDispatch2* sender, const ttstr & text) = 0;

	//! @brief		�����|�C���g�̐ݒ�
	//! @param		layer		�t�H���g���̊܂܂�郌�C��
	//! @param		x			�`���`���̍��W�ɂ����钍���|�C���g��x�ʒu
	//! @param		y			�`���`���̍��W�ɂ����钍���|�C���g��y�ʒu
	virtual void TJS_INTF_METHOD SetAttentionPoint(tTJSNI_BaseLayer *layer,
		tjs_int l, tjs_int t) = 0;

	//! @brief		�����|�C���g�̉���
	virtual void TJS_INTF_METHOD DisableAttentionPoint() = 0;

	//! @brief		IME���[�h�̐ݒ�
	//! @param		mode		IME���[�h
	virtual void TJS_INTF_METHOD SetImeMode(tTVPImeMode mode) = 0;

	//! @brief		IME���[�h�̃��Z�b�g
	virtual void TJS_INTF_METHOD ResetImeMode() = 0;

	//! @brief		iTVPWindow::Update() �̌Ăяo����v������
	//! @note		�E�B���h�E�ɑ΂��� iTVPWindow::Update() �����̓K���ȃ^�C�~���O��
	//!				�Ăяo�����Ƃ�v������B
	//!				iTVPWindow::Update() ���Ăяo�����܂ł͉��� RequestUpdate() ��
	//!				�Ă�ł����ʂ͓����ł���B�܂��A��x iTVPWindow::Update() ��
	//!				�Ăяo�����ƁA�Ă� RequestUpdate() ���Ă΂Ȃ������
	//!				iTVPWindow::Update() �͌Ă΂�Ȃ��B
	virtual void TJS_INTF_METHOD RequestUpdate() = 0;


	//! @brief		Window��iTJSDispatch2�C���^�[�t�F�[�X���擾����
	virtual iTJSDispatch2 * GetWindowDispatch() = 0;

};
//---------------------------------------------------------------------------
/*]*/


//---------------------------------------------------------------------------
// tTJSNI_BaseWindow
//---------------------------------------------------------------------------
class tTVPBaseBitmap;
class tTJSNI_BaseLayer;
class tTJSNI_BaseVideoOverlay;
class tTJSNI_BaseWindow : public tTJSNativeInstance, public iTVPWindow, public iTVPLayerTreeOwner
{
	typedef tTJSNativeInstance inherited;

private:
	std::vector<tTJSVariantClosure> ObjectVector;
	bool ObjectVectorLocked;

protected:
	iTJSDispatch2 *Owner;
public:
	iTJSDispatch2 * TJS_INTF_METHOD GetOwnerNoAddRef() const { return Owner; }

public:
	tTJSNI_BaseWindow();
	~tTJSNI_BaseWindow();
	tjs_error TJS_INTF_METHOD
	Construct(tjs_int numparams, tTJSVariant **param,
		iTJSDispatch2 *tjs_obj);
	void TJS_INTF_METHOD Invalidate();

	bool IsMainWindow() const;
	virtual bool GetWindowActive() = 0;
	void FireOnActivate(bool activate_or_deactivate);


	//-- interface to draw device
public:
	tTJSVariant DrawDeviceObject; //!< Current Draw Device TJS2 Object
	iTVPDrawDevice * DrawDevice; //!< Current Draw Device
	void SetDrawDeviceObject(const tTJSVariant & val);
	const tTJSVariant & GetDrawDeviceObject() const { return DrawDeviceObject; }
	iTVPDrawDevice * GetDrawDevice() const { return DrawDevice ; }
	virtual void ResetDrawDevice() = 0;
	virtual iTJSDispatch2 * GetWindowDispatch() { if(Owner) Owner->AddRef(); return Owner; }


	//----- event dispatching
public:
	virtual bool CanDeliverEvents() const = 0; // implement this in each platform


public:

	void OnClose();
	void OnResize();
	void OnClick(tjs_int x, tjs_int y);
	void OnDoubleClick(tjs_int x, tjs_int y);
	void OnMouseDown(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseUp(tjs_int x, tjs_int y, tTVPMouseButton mb, tjs_uint32 flags);
	void OnMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	void OnReleaseCapture();
	void OnMouseOutOfWindow();
	void OnMouseEnter();
	void OnMouseLeave();
	void OnKeyDown(tjs_uint key, tjs_uint32 shift);
	void OnKeyUp(tjs_uint key, tjs_uint32 shift);
	void OnKeyPress(tjs_char key);
	void OnFileDrop(const tTJSVariant &array);
	void OnMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);
	void OnPopupHide();
	void OnActivate(bool activate_or_deactivate);

	void OnTouchDown( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void OnTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void OnTouchMove( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	
	void OnTouchScaling( tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag );
	void OnTouchRotate( tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag );
	void OnMultiTouch();

	void OnHintChange( const ttstr& text, tjs_int x, tjs_int y, bool isshow );

	void OnDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution );

	void ClearInputEvents();

	void PostReleaseCaptureEvent();

	//----- layer managermant
public:
	void TJS_INTF_METHOD RegisterLayerManager(iTVPLayerManager * manager);
	void TJS_INTF_METHOD UnregisterLayerManager(iTVPLayerManager * manager);


protected:
	tTVPRect WindowExposedRegion;
	tTVPBaseBitmap * DrawBuffer;

	bool WindowUpdating; // window is in updating

public:
	void NotifyWindowExposureToLayer(const tTVPRect &cliprect);

public:
	void NotifyUpdateRegionFixed(const tTVPComplexRect &updaterects); // is called by layer manager
	void UpdateContent(); // is called from event dispatcher
	void DeliverDrawDeviceShow();
	virtual void BeginUpdate(const tTVPComplexRect & rects);
	virtual void EndUpdate();
	virtual void TJS_INTF_METHOD RequestUpdate();
	virtual void TJS_INTF_METHOD NotifySrcResize(); // is called from primary layer
	virtual tTVPImeMode GetDefaultImeMode() const = 0;

	void DumpPrimaryLayerStructure();

	void RecheckInputState(); // slow timer tick (about 1 sec interval, inaccurate)

	void SetShowUpdateRect(bool b);


	//----- methods
	void Add(tTJSVariantClosure clo);
	void Remove(tTJSVariantClosure clo);

	//----- interface to video overlay object
protected:
	tObjectList<tTJSNI_BaseVideoOverlay> VideoOverlay;

public:
	void RegisterVideoOverlayObject(tTJSNI_BaseVideoOverlay *ovl);
	void UnregisterVideoOverlayObject(tTJSNI_BaseVideoOverlay *ovl);

	//----- vsync
protected:
	bool WaitVSync;
	virtual void UpdateVSyncThread() = 0;

public:
	void SetWaitVSync( bool enable );
	bool GetWaitVSync() const;
};
//---------------------------------------------------------------------------

#include "WindowImpl.h" // must define tTJSNI_Window class

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Window List
//---------------------------------------------------------------------------
class tTJSNI_Window;
extern tTJSNI_Window * TVPGetWindowListAt(tjs_int idx);
extern tjs_int TVPGetWindowCount();
extern tTJSNI_Window * TVPMainWindow; //  = NULL; // main window

//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
// tTJSNC_Window : TJS Window class
//---------------------------------------------------------------------------
class tTJSNC_Window : public tTJSNativeClass
{
public:
	tTJSNC_Window();
	static tjs_uint32 ClassID;

protected:
	tTJSNativeInstance *CreateNativeInstance();
	/*
		implement this in each platform.
		this must return a proper instance of tTJSNI_Window.
	*/
};
//---------------------------------------------------------------------------
extern tTJSNativeClass * TVPCreateNativeClass_Window();
	/*
		implement this in each platform.
		this must return a proper instance of tTJSNI_Window.
		usually simple returns: new tTJSNC_Window();
	*/
//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
// Input Events
//---------------------------------------------------------------------------
class tTVPOnCloseInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnCloseInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnClose(); }
};
//---------------------------------------------------------------------------
class tTVPOnResizeInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnResizeInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnResize(); }
};
//---------------------------------------------------------------------------
class tTVPOnClickInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
public:
	tTVPOnClickInputEvent(tTJSNI_BaseWindow *win, tjs_int x, tjs_int y) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnClick(X, Y); }
};
//---------------------------------------------------------------------------
class tTVPOnDoubleClickInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
public:
	tTVPOnDoubleClickInputEvent(tTJSNI_BaseWindow *win, tjs_int x, tjs_int y) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnDoubleClick(X, Y); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tTVPMouseButton Buttons;
	tjs_uint32 Flags;
public:
	tTVPOnMouseDownInputEvent(tTJSNI_BaseWindow *win, tjs_int x, tjs_int y,
		tTVPMouseButton buttons, tjs_uint32 flags) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), Buttons(buttons), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseDown(X, Y, Buttons, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tTVPMouseButton Buttons;
	tjs_uint32 Flags;
public:
	tTVPOnMouseUpInputEvent(tTJSNI_BaseWindow *win, tjs_int x, tjs_int y,
		tTVPMouseButton buttons, tjs_uint32 flags) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), Buttons(buttons), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseUp(X, Y, Buttons, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseMoveInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int X;
	tjs_int Y;
	tjs_uint32 Flags;
public:
	tTVPOnMouseMoveInputEvent(tTJSNI_BaseWindow *win, tjs_int x, tjs_int y,
		tjs_uint32 flags) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), Flags(flags) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseMove(X, Y, Flags); }
};
//---------------------------------------------------------------------------
class tTVPOnReleaseCaptureInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnReleaseCaptureInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnReleaseCapture(); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseOutOfWindowInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnMouseOutOfWindowInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseOutOfWindow(); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseEnterInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnMouseEnterInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseEnter(); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseLeaveInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnMouseLeaveInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseLeave(); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint Key;
	tjs_uint32 Shift;
public:
	tTVPOnKeyDownInputEvent(tTJSNI_BaseWindow *win, tjs_uint key, tjs_uint32 shift) :
		tTVPBaseInputEvent(win, Tag), Key(key), Shift(shift) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnKeyDown(Key, Shift); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint Key;
	tjs_uint32 Shift;
public:
	tTVPOnKeyUpInputEvent(tTJSNI_BaseWindow *win, tjs_uint key, tjs_uint32 shift) :
		tTVPBaseInputEvent(win, Tag), Key(key), Shift(shift) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnKeyUp(Key, Shift); }
};
//---------------------------------------------------------------------------
class tTVPOnKeyPressInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_char Key;
public:
	tTVPOnKeyPressInputEvent(tTJSNI_BaseWindow *win, tjs_char key) :
		tTVPBaseInputEvent(win, Tag), Key(key) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnKeyPress(Key); }
};
//---------------------------------------------------------------------------
class tTVPOnFileDropInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tTJSVariant Array;
public:
	tTVPOnFileDropInputEvent(tTJSNI_BaseWindow *win, const tTJSVariant & val) :
		tTVPBaseInputEvent(win, Tag), Array(val) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnFileDrop(Array); }
};
//---------------------------------------------------------------------------
class tTVPOnMouseWheelInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_uint32 Shift;
	tjs_int WheelDelta;
	tjs_int X;
	tjs_int Y;
public:
	tTVPOnMouseWheelInputEvent(tTJSNI_BaseWindow *win, tjs_uint32 shift,
		tjs_int wheeldelta, tjs_int x, tjs_int y) :
		tTVPBaseInputEvent(win, Tag), Shift(shift), WheelDelta(wheeldelta),
		X(x), Y(y) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMouseWheel(Shift, WheelDelta, X, Y); }
};
//---------------------------------------------------------------------------
class tTVPOnPopupHideInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
public:
	tTVPOnPopupHideInputEvent(tTJSNI_BaseWindow *win) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnPopupHide(); }
};
//---------------------------------------------------------------------------
class tTVPOnWindowActivateEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	bool ActivateOrDeactivate;
public:
	tTVPOnWindowActivateEvent(tTJSNI_BaseWindow *win, bool activate_or_deactivate) :
		tTVPBaseInputEvent(win, Tag), ActivateOrDeactivate(activate_or_deactivate) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnActivate(ActivateOrDeactivate); }
};
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
class tTVPOnTouchDownInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchDownInputEvent(tTJSNI_BaseWindow *win, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnTouchDown(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchUpInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchUpInputEvent(tTJSNI_BaseWindow *win, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnTouchUp(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchMoveInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real X;
	tjs_real Y;
	tjs_real CX;
	tjs_real CY;
	tjs_uint32 ID;
public:
	tTVPOnTouchMoveInputEvent(tTJSNI_BaseWindow *win, tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id ) :
		tTVPBaseInputEvent(win, Tag), X(x), Y(y), CX(cx), CY(cy), ID(id) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnTouchMove(X, Y, CX, CY, ID); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchScalingInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real StartDistance;
	tjs_real CurrentDistance;
	tjs_real CX;
	tjs_real CY;
	tjs_int Flag;
public:
	tTVPOnTouchScalingInputEvent(tTJSNI_BaseWindow *win, tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag ) :
		tTVPBaseInputEvent(win, Tag), StartDistance(startdist), CurrentDistance(curdist), CX(cx), CY(cy), Flag(flag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnTouchScaling( StartDistance, CurrentDistance, CX, CY, Flag ); }
};
//---------------------------------------------------------------------------
class tTVPOnTouchRotateInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_real StartAngle;
	tjs_real CurrentAngle;
	tjs_real Distance;
	tjs_real CX;
	tjs_real CY;
	tjs_int Flag;
public:
	tTVPOnTouchRotateInputEvent(tTJSNI_BaseWindow *win, tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag ) :
		tTVPBaseInputEvent(win, Tag), StartAngle(startangle), CurrentAngle(curangle), Distance(dist), CX(cx), CY(cy), Flag(flag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnTouchRotate( StartAngle, CurrentAngle, Distance, CX, CY, Flag ); }
};
//---------------------------------------------------------------------------
class tTVPOnMultiTouchInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;

public:
	tTVPOnMultiTouchInputEvent(tTJSNI_BaseWindow *win ) :
		tTVPBaseInputEvent(win, Tag) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnMultiTouch(); }
};
//---------------------------------------------------------------------------
class tTVPOnHintChangeInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	ttstr HintMessage;
	tjs_int HintX;
	tjs_int HintY;
	bool IsShow;
public:
	tTVPOnHintChangeInputEvent(tTJSNI_BaseWindow *win, const ttstr& text, tjs_int x, tjs_int y, bool isshow ) :
		tTVPBaseInputEvent(win, Tag)
		, HintMessage(text), HintX(x), HintY(y), IsShow(isshow)	{};

	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnHintChange( HintMessage, HintX, HintY, IsShow ); }
};
//---------------------------------------------------------------------------
class tTVPOnDisplayRotateInputEvent : public tTVPBaseInputEvent
{
	static tTVPUniqueTagForInputEvent Tag;
	tjs_int Orientation;
	tjs_int Rotate;
	tjs_int BPP;
	tjs_int HorizontalResolution;
	tjs_int VerticalResolution;
public:
	tTVPOnDisplayRotateInputEvent(tTJSNI_BaseWindow *win, tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution ) :
		tTVPBaseInputEvent(win, Tag), Orientation(orientation), Rotate(rotate), BPP(bpp),
		HorizontalResolution(hresolution), VerticalResolution(vresolution) {};
	void Deliver() const
	{ ((tTJSNI_BaseWindow*)GetSource())->OnDisplayRotate( Orientation, Rotate, BPP, HorizontalResolution, VerticalResolution ); }
};


#endif
