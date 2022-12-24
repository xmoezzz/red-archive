
#ifndef __GRAPHICS_LOAD_THREAD_H__
#define __GRAPHICS_LOAD_THREAD_H__

#include <queue>
#include <vector>
#include "ThreadIntf.h"
#include "NativeEventQueue.h"
#include "GraphicsLoaderIntf.h"

// BaseBitmap ���g���ƃ��G���g�����g�ł͂Ȃ��̂ŁA�ʂ̍\���̂ɓƎ��Ƀ��[�h����K�v������
struct tTVPTmpBitmapImage {
	tjs_uint32 w;
	tjs_uint32 h;
	tjs_int pitch;
	tjs_uint32* buf;
	std::vector<tTVPGraphicMetaInfoPair> * MetaInfo;
	tTVPTmpBitmapImage();
	~tTVPTmpBitmapImage();
// �p���b�g�֘A�͌���ǂ܂Ȃ��A�t�@�C���ɏ]���̂ł͂Ȃ��A���O�w������Ȃ̂�
};

struct tTVPImageLoadCommand {
	iTJSDispatch2*			owner_;	// send to event
	class tTJSNI_Bitmap*	bmp_;	// set bitmap image
	ttstr					path_;
	tTVPTmpBitmapImage*		dest_;
	ttstr					result_;
	tTVPImageLoadCommand();
	~tTVPImageLoadCommand();
};

class tTVPAsyncImageLoader : public tTVPThread {
	/** �Ǎ��ݗv���R�}���h�̃L���[�pCS */
	tTJSCriticalSection CommandQueueCS;
	/** �Ǎ��ݍς݉摜�L���[�pCS */
	tTJSCriticalSection ImageQueueCS;

	/** ���[�h�����チ�C���X���b�h�ŏ������邽�߂̃��b�Z�[�W�L���[ */
	NativeEventQueue<tTVPAsyncImageLoader> EventQueue;
	/**  �Ǎ��݃X���b�h�֓Ǎ��ݗv�������������Ƃ�`����C�x���g */
	tTVPThreadEvent PushCommandQueueEvent;

	/** �Ǎ��ݗv���R�}���h�L���[ */
	std::queue<tTVPImageLoadCommand*> CommandQueue;
	/** �Ǎ��݊����摜�L���[ */
	std::queue<tTVPImageLoadCommand*> LoadedQueue;

private:
	/**
	 * �Ǎ��݃X���b�h���烁�C���X���b�h�֓Ǎ��݂������������Ƃ�ʒm����
	 */
	void SendToLoadFinish();
	/**
	 * �Ǎ��݊��������摜�����C���X���b�h��Bitmap�֊i�[���āA�C�x���g�ʒm����
	 */
	void HandleLoadedImage();

	/**
	 * �Ǎ��݂�Ǎ��݃X���b�h�ɗv������(�L���[�֓����)
	 */
	void PushLoadQueue( iTJSDispatch2 *owner, tTJSNI_Bitmap *bmp, const ttstr &nname );
	
	/**
	 * �Ǎ��݃X���b�h����
	 * �L���[�ɃR�}���h������̂�҂��A�C�x���g��������L���[����R�}���h�����o���ēǍ��ݏ��������s
	 * �Ǎ��݂�����������Ǎ��ݍς݉摜�L���[�ɓ���ă��C���X���b�h�֊�����ʒm����
	 */
	void LoadingThread();
	
	/**
	 * �摜�Ǎ��ݏ���
	 */
	void LoadImageFromCommand( tTVPImageLoadCommand* cmd );
	
	/**
	 * �Ǎ��݃X���b�h���C��
	 */
	void Execute();

	/**
	 * ���C���X���b�h�n���h��
	 * ���C���X���b�h�ւ̃C�x���g(���b�Z�[�W)�ʒm���󂯂�
	 */
	void Proc( NativeEvent& ev );

public:
	tTVPAsyncImageLoader();
	~tTVPAsyncImageLoader();

	/**
	 �Ǎ��݃X���b�h�̏I����v������(�I���͑҂��Ȃ�)
	 */
	void ExitRequest();
	
	/**
	 * �Ǎ��ݗv��
	 * ���C���X���b�h����Ǎ��݃X���b�h�֓Ǎ��݂�v������B
	 * �Ǎ��ݑO�ɃG���[�����������ꍇ��L���b�V����ɉ摜���������ꍇ�͗v���͍s��ꂸ
	 * �����ɏI�����AonLoaded �C�x���g�𔭐�������B
	 */
	void LoadRequest( iTJSDispatch2 *owner, tTJSNI_Bitmap* bmp, const ttstr &name );
};

#endif // __GRAPHICS_LOAD_THREAD_H__
