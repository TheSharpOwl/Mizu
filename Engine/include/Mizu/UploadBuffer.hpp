#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include "d3dx12.h"

#include "Utils.hpp"

#include <deque>
#include <memory>
#include <mutex>

namespace Mizu 
{
	class UploadBuffer
	{
		struct Allocation
		{
			void* CPU;
			D3D12_GPU_VIRTUAL_ADDRESS GPU;
		};

		explicit UploadBuffer(size_t pageSize = mb(2));

		virtual ~UploadBuffer() = default;

		size_t getPageSize() const { return m_pageSize; }

		/// <summary>
		/// Allocate memory in the upload heap
		/// </summary>
		/// <param name="sizeInBytes"> must not exceed the size of the page </param>
		/// <param name="alignment"></param>
		/// <returns></returns>
		Allocation Allocate(size_t sizeInBytes, size_t alignment);

		// TODO add copy method using memcpy

		void Reset();


	protected:

		struct Page
		{
			Page(size_t sizeInBytes);
			virtual ~Page();

			bool hasSpace(size_t sizeInBytes, size_t alignment) const;

		/// <summary>
		/// Allocate memory in the page
		/// </summary>
		/// <param name="sizeInBytes"> must not exceed the size of the page </param>
		/// <param name="alignment"></param>
		/// <returns></returns>
		Allocation Allocate(size_t sizeInBytes, size_t alignment);

		void Reset();

		protected:
			Microsoft::WRL::ComPtr<ID3D12Resource> m_resource;

			void* m_pCpu;
			D3D12_GPU_VIRTUAL_ADDRESS m_pGpu;

			const size_t m_pageSize;

			size_t m_offset;
		};

		using PagePool = std::deque<std::shared_ptr<Page>>;

		std::shared_ptr<Page> requestPage();

		PagePool m_pagePool;
		PagePool m_freePages;

		std::shared_ptr<Page> m_curPage;

		const size_t m_pageSize;

		std::mutex m_mutex;
	};

}
