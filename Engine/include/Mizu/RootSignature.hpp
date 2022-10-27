#pragma once

namespace Mizu
{
	class RootSignature
	{
	public:
		RootSignature();

		RootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION version);

		virtual ~RootSignature();

		void Destroy();

		Microsoft::WRL::ComPtr<ID3D12RootSignature> getRootSignature() const;

		void setRootSignature(const D3D12_ROOT_SIGNATURE_DESC1& rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION version);

		const D3D12_ROOT_SIGNATURE_DESC1& getRootSignatureDesc() const
		{
			return m_rootSignatureDesc;
		}

		uint32_t getDescriptorTableBitMask(D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType) const;
		uint32_t getDescriptorsCount(uint32_t rootIndex) const;

	private:

		D3D12_ROOT_SIGNATURE_DESC1 m_rootSignatureDesc;

		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_rootSignature;

		// to know the number of descriptors per descriptor table (maximum is 32 because we have 32 bitmask representing)
		uint32_t descriptorsPerTableCount[32];

		// bitmask representing which root parameters are descriptor tables for samplers
		uint32_t samplerTableBitMask;

		// bitmask representing which root parameters are descriptor tables for CBV, SRV, UAV
		uint32_t m_descriptorTableBitMask;
	};
}
