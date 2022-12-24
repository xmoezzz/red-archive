//---------------------------------------------------------------------------
/**
 * 
 */
//---------------------------------------------------------------------------
//!@file ���C���[�c���[�I�[�i�[
//---------------------------------------------------------------------------
#ifndef LayerTreeOwnerImple_H
#define LayerTreeOwnerImple_H

#include "LayerTreeOwner.h"

/**
 * �ŏ�����LayerTreeOwner�@�\��񋟂���B
 * �ق� iTVPLayerManager �֘A���\�b�h�̂�
 * �������̃��\�b�h�͖������Ȃ̂ŁA�p��������Ŏ�������K�v������
 */
class tTVPLayerTreeOwner : public iTVPLayerTreeOwner
{
protected:
	size_t PrimaryLayerManagerIndex; //!< �v���C�}�����C���}�l�[�W��
	std::vector<iTVPLayerManager *> Managers; //!< ���C���}�l�[�W���̔z��
	tTVPRect DestRect; //!< �`���ʒu

protected:
	iTVPLayerManager* GetLayerManagerAt(size_t index);
	const iTVPLayerManager* GetLayerManagerAt(size_t index) const;
	bool TransformToPrimaryLayerManager(tjs_int &x, tjs_int &y);
	bool TransformToPrimaryLayerManager(tjs_real &x, tjs_real &y);
	bool TransformFromPrimaryLayerManager(tjs_int &x, tjs_int &y);

	void GetPrimaryLayerSize( tjs_int &w, tjs_int &h ) const;

public:
	tTVPLayerTreeOwner();

	// LayerManager/Layer -> LTO
	virtual void TJS_INTF_METHOD RegisterLayerManager( class iTVPLayerManager* manager );
	virtual void TJS_INTF_METHOD UnregisterLayerManager( class iTVPLayerManager* manager );

	/* ���ۂ̕`��
	virtual void TJS_INTF_METHOD StartBitmapCompletion(iTVPLayerManager * manager) = 0;
	virtual void TJS_INTF_METHOD NotifyBitmapCompleted(class iTVPLayerManager * manager,
		tjs_int x, tjs_int y, const void * bits, const class BitmapInfomation * bitmapinfo,
		const tTVPRect &cliprect, tTVPLayerType type, tjs_int opacity) = 0;
	virtual void TJS_INTF_METHOD EndBitmapCompletion(iTVPLayerManager * manager) = 0;
	*/

	// �ȉ��͉������Ȃ�
	virtual void TJS_INTF_METHOD SetMouseCursor(class iTVPLayerManager* manager, tjs_int cursor);
	virtual void TJS_INTF_METHOD GetCursorPos(class iTVPLayerManager* manager, tjs_int &x, tjs_int &y);
	virtual void TJS_INTF_METHOD SetCursorPos(class iTVPLayerManager* manager, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD ReleaseMouseCapture(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetHint(class iTVPLayerManager* manager, iTJSDispatch2* sender, const ttstr &hint);

	virtual void TJS_INTF_METHOD NotifyLayerResize(class iTVPLayerManager* manager);
	virtual void TJS_INTF_METHOD NotifyLayerImageChange(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetAttentionPoint(class iTVPLayerManager* manager, tTJSNI_BaseLayer *layer, tjs_int x, tjs_int y);
	virtual void TJS_INTF_METHOD DisableAttentionPoint(class iTVPLayerManager* manager);

	virtual void TJS_INTF_METHOD SetImeMode( class iTVPLayerManager* manager, tjs_int mode );
	virtual void TJS_INTF_METHOD ResetImeMode( class iTVPLayerManager* manager );

	// virtual iTJSDispatch2 * TJS_INTF_METHOD GetOwnerNoAddRef() const = 0;

	// �ȉ��͏�q�̃��\�b�h���R�[�����ꂽ��ɁA���ۂɒl��ݒ肷�邽�߂ɌĂ΂��
	// cursor == 0 �� default
	virtual void OnSetMouseCursor( tjs_int cursor ) = 0;
	virtual void OnGetCursorPos(tjs_int &x, tjs_int &y) = 0;
	virtual void OnSetCursorPos(tjs_int x, tjs_int y) = 0;
	virtual void OnReleaseMouseCapture() = 0;
	virtual void OnSetHintText(iTJSDispatch2* sender, const ttstr &hint) = 0;

	/**
	 * �v���C�}���[���C���[�̃T�C�Y���ύX���ꂽ���ɌĂ΂��
	 */
	virtual void OnResizeLayer( tjs_int w, tjs_int h ) = 0;
	/**
	 * �v���C�}���[���C���[�̉摜���ύX���ꂽ�̂ŁA�K�v�ɉ����čĕ`����s��
	 * NotifyLayerImageChange ����Ă΂�Ă���
	 */
	virtual void OnChangeLayerImage() = 0;

	virtual void OnSetAttentionPoint(tTJSNI_BaseLayer *layer, tjs_int x, tjs_int y) = 0;
	virtual void OnDisableAttentionPoint() = 0;
	virtual void OnSetImeMode(tjs_int mode) = 0;
	virtual void OnResetImeMode() = 0;

	// LTO -> LayerManager/Layer
	// LayerManager �ɑ΂��ăC�x���g��ʒm���邽�߂̃��\�b�h
	void FireClick(tjs_int x, tjs_int y);
	void FireDoubleClick(tjs_int x, tjs_int y);
	void FireMouseDown(tjs_int x, tjs_int y, enum tTVPMouseButton mb, tjs_uint32 flags);
	void FireMouseUp(tjs_int x, tjs_int y, enum tTVPMouseButton mb, tjs_uint32 flags);
	void FireMouseMove(tjs_int x, tjs_int y, tjs_uint32 flags);
	void FireMouseWheel(tjs_uint32 shift, tjs_int delta, tjs_int x, tjs_int y);

	void FireReleaseCapture();
	void FireMouseOutOfWindow();

	void FireTouchDown( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void FireTouchUp( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void FireTouchMove( tjs_real x, tjs_real y, tjs_real cx, tjs_real cy, tjs_uint32 id );
	void FireTouchScaling( tjs_real startdist, tjs_real curdist, tjs_real cx, tjs_real cy, tjs_int flag );
	void FireTouchRotate( tjs_real startangle, tjs_real curangle, tjs_real dist, tjs_real cx, tjs_real cy, tjs_int flag );
	void FireMultiTouch();

	void FireKeyDown(tjs_uint key, tjs_uint32 shift);
	void FireKeyUp(tjs_uint key, tjs_uint32 shift);
	void FireKeyPress(tjs_char key);

	void FireDisplayRotate( tjs_int orientation, tjs_int rotate, tjs_int bpp, tjs_int hresolution, tjs_int vresolution );

	void FireRecheckInputState();

	// ���C���[�Ǘ��⏕
	tTJSNI_BaseLayer* GetPrimaryLayer();
	tTJSNI_BaseLayer* GetFocusedLayer();
	void SetFocusedLayer(tTJSNI_BaseLayer * layer);
};

#endif
