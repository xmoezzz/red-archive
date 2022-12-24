#pragma once
#ifndef CRYPTMT_H
#define CRYPTMT_H
/**
* @file cryptmt.h
* CryptMT version 3.0 header file.
*/
/**
* \mainpage
*
* CryptMT version 3.0 Stream Cipher.
*
* CryptMT is a software stream cypher, which participates <a
* href="http://www.ecrypt.eu.org/stream/project.html">eSTREAM</a>
* competition held by <a
* href="http://www.ecrypt.eu.org/index.html">ECRYPT</a>.  CryptMT was
* selected one of the candidates of final stage, but was not selected
* one of <a href="http://www.ecrypt.eu.org/stream/portfolio.pdf">
* portfolio of eSTREAM</a>:
* <blockquote cite="http://www.ecrypt.eu.org/stream/portfolio.pdf">
* The cipher CryptMT has a very unusual design which delivers very
* reasonable performance. While there have been no negative
* cryptanalytic results against the cipher in the last phase of
* eSTREAM, we are somewhat concerned that the security of the cipher,
* in particular the non-linear filter component, might not yet be as
* well-understood as some of the other finalists. We anticipate that
* elements of CryptMT will continue to be of interest to the
* cryptographic community, and we hope that the full advantages of
* the approach embodied in CryptMT v3 can be evaluated.However, we
* are currently not suf- ficiently confident in the design and
* security of this algorithm for us to include it in the final
* portfolio.
* </blockquote>
*
* @author Makoto Matsumoto (Hiroshima University)
* @author Mutsuo Saito (Hiroshima University)
* @author Takuji Nishimura (Yamagata University)
* @author Mariko Hagita (Ochanomizu University)
*
* Copyright (C) 2006 -- 2013 Mutsuo Saito, Makoto Matsumoto,
* Takuji Nishimura, Mariko Hagita, Hiroshima University,
* Yamagata University and Ochanomizu University.
* All rights reserved.
*
* Usage:
* <ol>
*   <li> make instance of CryptMT.
*   <li> can repeat some times.
*   <ol>
*     <li> call IVSetUp()
*     <li> can repeat more than zero times.
*       <ol>
*       <li> call encryptBlocks() to encrypt text.
*       </ol>
*     <li> call encrypt() to encrypt remainder text.
*   </ol>
* </ol>
* Free use for noncommercial or academic research.  Contact us if
* you want to use this software as a part of commercial software.
* See LICENSE.txt for more detail.
*/
#include <stdint.h>
#include <exception>
#include <stdexcept>

/**
* namespace for CriptMT
*/
namespace cryptmt {
	/**
	* This function allocate memory for SIMD function.
	* Users should use aligned_free() to release the memory allocated by
	* this function.
	* Users can use the memory allocated by this function as if it were
	* allocated by malloc().
	*
	* @param[in] size byte size of memory to allocate.
	* @return the pointer to the memory which can be used for SIMD function.
	*/
	void *aligned_alloc(size_t size);

	/**
	* This function releases memory allocated by aligned_alloc().
	* Action is not defined when the argument is not allocated by
	* alligned_alloc().
	*
	* @param[in] ptr the pointer to the memory allocated by aligned_alloc().
	*/
	void aligned_free(void * ptr);

	/**
	* This function returns the maximum key size measured in bits.
	* @return the maximum key size measured in bits.
	*/
	uint32_t maxKeySize();

	/**
	* This function returns the unit size of key measured in bits.
	* The key size and IV size must be multiple of the unit size.
	* This function always returns 128.
	*
	* @return the unit size of key measured in bits.
	*/
	uint32_t keySizeUnit();

	/**
	* @class CryptMT
	* @brief CryptMT Stream Cipher
	*/
	class CryptMT {
	public:
		/**
		* Error about calling sequence of methods.
		*/
		class stage_exception : std::runtime_error {
		public:
			/**
			* Constructor of stage_exception.
			*/
			explicit stage_exception(const char *message) :
				std::runtime_error(message) {}
		};

		/**
		* \b keysize and \b ivsize should be multiple of keySizeUnit()
		* and less or equal to maxKeySize().
		* Invalid_argument exception will be thrown when \b keysize or
		* ivsize violate this condition.
		* Bad_alloc exception will be thrown when fail to allocalte
		* internal memory.
		*
		* @param[in] key encryption key
		* @param[in] keysize size of key measured by bits.
		* @param[in] ivsize size of IV measured by bits.
		* @exception std::bad_alloc when fails allocating internal memory.
		* @exception std::invalid_argument \b keysize or \b ivsize is not
		* appropriate.
		*/
		explicit CryptMT(const uint8_t * key,
			int keysize,
			int ivsize)
			throw(std::bad_alloc, std::invalid_argument);

		~CryptMT();

		/**
		* Setup Initial Vector
		*
		* IVSetUp() should be called before encryption.
		* \b stage_exception will be thrown when the condition is broken.
		* @param[in] iv Initial Vector
		*/
		void IVSetUp(const uint8_t * iv);

		/**
		* Encrypts \b plaintext and set the result in \b ciphertext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* After calling this method, following methods should not be
		* called until IVSetUp() is called; encrypt(), decrypt(),
		* encryptBlocks(), decryptBlocks().
		*
		* @param[in] plaintext plain text
		* @param[out] ciphertext encrypted text
		* @param[in] msglen length of \b plaintext in bytes.
		* @exception stage_exception when IVSetUp() was not called before
		* this method.
		*/
		void encrypt(const uint8_t * plaintext,
			uint8_t * ciphertext,
			uint64_t msglen)
			throw(stage_exception);

		/**
		* Decrypt \b ciphertext and set the result in \b platintext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* After calling this method, following methods should not be
		* called until IVSetUp() is called; encrypt(), decrypt(),
		* encryptBlocks(), decryptBlocks().
		*
		* @param[in] ciphertext plain text
		* @param[out] plaintext encrypted text
		* @param[in] msglen length of \b ciphertext in bytes.
		* \exception stage_exception when IVSetUp() was not called before
		* this method.
		*/
		void decrypt(const uint8_t * ciphertext,
			uint8_t * plaintext,
			uint64_t msglen)
			throw(stage_exception)
		{
			encrypt(ciphertext, plaintext, msglen);
		}

		/**
		* First, call IVSetUp() using \b iv and then
		* encrypts \b plaintext and set the result in \b ciphertext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* After calling this method, following methods should not be
		* called until IVSetUp() is called; encrypt(), decrypt(),
		* encryptBlocks(), decryptBlocks().
		*
		* @param[in] iv Initial Vector
		* @param[in] plaintext plain text
		* @param[out] ciphertext encrypted text
		* @param[in] msglen length of \b plaintext in bytes.
		*/
		void encryptPacket(const uint8_t * iv,
			const uint8_t * plaintext,
			uint8_t * ciphertext,
			uint64_t msglen)
		{
			IVSetUp(iv);
			encrypt(plaintext, ciphertext, msglen);
		}

		/**
		* First, call IVSetUp() using \b iv and then
		* decrypts \b ciphertext and set the result in \b plaintext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* After calling this method, following methods should not be
		* called until IVSetUp() is called; encrypt(), decrypt(),
		* encryptBlocks(), decryptBlocks().
		*
		* @param[in] iv Initial Vector
		* @param[in] plaintext plain text
		* @param[out] ciphertext encrypted text
		* @param[in] msglen length of \b plaintext in bytes.
		*/
		void decryptPacket(const uint8_t * iv,
			const uint8_t * ciphertext,
			uint8_t * plaintext,
			uint64_t msglen)
		{
			encryptPacket(iv, ciphertext, plaintext, msglen);
		}

		/**
		* Encrypts \b plaintext and set the result in \b ciphertext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* @param[in] plaintext plain text
		* @param[out] ciphertext encrypted text
		* @param[in] blocks length of \b plaintext in blocks.
		* @exception stage_exception when IVSetUp() was not called before
		* this method.
		*/
		void encryptBlocks(const uint8_t * plaintext,
			uint8_t * ciphertext,
			uint32_t blocks) throw(stage_exception);

		/**
		* Decrypt \b ciphertext and set the result in \b platintext.
		* \b the pointers of plaintext and \b ciphertext should be aligned.
		* aligned_alloc() can be used to get aligned pointer.
		*
		* @param[in] ciphertext plain text
		* @param[out] plaintext encrypted text
		* @param[in] blocks length of \b ciphertext in blocks.
		* \exception stage_exception when IVSetUp() was not called before
		* this method.
		*/
		void decryptBlocks(const uint8_t * ciphertext,
			uint8_t * plaintext,
			uint32_t blocks)
			throw(stage_exception)
		{
			encryptBlocks(ciphertext, plaintext, blocks);
		}

		/**
		* Returns block length needed by encryptBlocks() and decryptBlocks().
		* @returns block length
		*/
		uint32_t blockLength();
#if defined(DEBUG)
		void debug_print();
#endif
	private:
		class Impl;
		Impl * impl;
		CryptMT(const CryptMT&);
		void operator=(const CryptMT&);
	};
}
#endif /* CRYPTMT_H */

