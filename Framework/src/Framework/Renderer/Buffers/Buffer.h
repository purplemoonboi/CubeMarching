#pragma once
#include "Framework/Core/Log/Log.h"
#include "Framework/Camera/MainCamera.h"


namespace Foundation::Graphics
{

	class GraphicsContext;

	enum class ShaderDataType
	{
		None = 0, 

		Float,
		Float2, 
		Float3, 
		Float4, 

		Mat3, 
		Mat4, 

		Int, 
		Int2, 
		Int3,
		Int4, 
		
		Bool
	};


	static INT32 ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:  return 4;
		case ShaderDataType::Float2: return 4 * 2;
		case ShaderDataType::Float3: return 4 * 3;
		case ShaderDataType::Float4: return 4 * 4;

		case ShaderDataType::Mat3:   return 4 * 3 * 3;
		case ShaderDataType::Mat4:   return 4 * 4 * 4;

		case ShaderDataType::Int:    return 4;
		case ShaderDataType::Int2:   return 4 * 2;
		case ShaderDataType::Int3:   return 4 * 3;
		case ShaderDataType::Int4:   return 4 * 4;

		case ShaderDataType::Bool:   return 1;
		}


		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		UINT Size;
		UINT Offset;
		UINT32 Slot;
		bool PerInstance;
		bool Normalised;

		BufferElement() = default;

		BufferElement(std::string&& name, ShaderDataType type, bool normalised = false, bool perInstance = false)
			:
				Name(std::move(name))
			,	Type(type)
			,	Size(ShaderDataTypeSize(type))
			,	Offset(0)
			,	Slot(0)
			,	Normalised(normalised)
			,	PerInstance(perInstance)
		{
		
		}

		BufferElement(std::string&& name, ShaderDataType type, UINT alignedByteOffset, INT32 slot = 0, bool normalised = false, bool perInstance = false)
			:
				Name(std::move(name))
			,	Type(type)
			,	Size(ShaderDataTypeSize(type))
			,	Offset(alignedByteOffset)
			,	Slot(slot)
			,	Normalised(normalised)
			,	PerInstance(false)
		{

		}

		[[nodiscard]] UINT GetComponentCount() const
		{
			switch (Type)
			{
				case ShaderDataType::Int:	   return 1;
				case ShaderDataType::Int2:	   return 2;
				case ShaderDataType::Int3:	   return 3;
				case ShaderDataType::Int4:	   return 4;

				case ShaderDataType::Mat3:	   return 3 * 3;
				case ShaderDataType::Mat4:	   return 4 * 4;

				case ShaderDataType::Float:    return 1;
				case ShaderDataType::Float2:   return 2;
				case ShaderDataType::Float3:   return 3;
				case ShaderDataType::Float4:   return 4;

				case ShaderDataType::Bool:     return 1;

				default: return 0;
			}
		}

	};

	class BufferLayout
	{
	public:

		BufferLayout() = default;

		BufferLayout(const std::initializer_list<BufferElement>& element)
		: 
			Elements(element)
		{
			CalculateOffsetAndStride();
		}

		BufferLayout(const std::vector<BufferElement>& element)
			:
			Elements(element)
		{
			CalculateOffsetAndStride();
		}

		UINT GetStride() const { return Stride; }

		const std::vector<BufferElement>& GetElements() const { return Elements; }

		std::vector<BufferElement>::iterator begin() { return Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return Elements.end(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator begin() const { return Elements.begin(); }
		[[nodiscard]] std::vector<BufferElement>::const_iterator end() const { return Elements.end(); }

	private:

		void CalculateOffsetAndStride()
		{
			UINT offset = 0;
			Stride = 0;

			for (auto& element : Elements)
			{
				element.Offset = offset;
				offset += element.Size;
				Stride += element.Size;
			}
		}

	private:
		std::vector<BufferElement> Elements;
		UINT Stride;
	};

	// @brief The buffer event system is exclusive for modern APIs such as Direct X12 and Vulkan.
	//	      Because it is the programmer's responsibility for ensuring safe allocation and deallocation
	//		  of GPU resources, a simple event system for scheduling commands was devised.
	enum class BufferEventsFlags : INT8
	{
		Idle = 0x0000,		// Buffer remains in this state until....
		DirtyBuffer = 0x0001,		//....a change has been requested....
		QueueDeletion = 0x0010,		//....a request for buffer deletion.
		InFlight = 0x0011		//....buffer is in use on GPU.
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() = default;

		virtual void SetData(const void* data, INT32 size, INT32 count) = 0;
		virtual void SetBuffer(const void* bufferAddress)= 0;
		virtual void OnDestroy() = 0;
		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual UINT GetCount() = 0;
		[[nodiscard]] virtual const BufferLayout& GetLayout() const = 0;
		[[nodiscard]] virtual const void* GetData() const = 0;

		static ScopePointer<VertexBuffer> Create(UINT size, UINT vertexCount);
		static ScopePointer<VertexBuffer> Create(const void* vertices, UINT size, UINT vertexCount, bool isDynamic);

	protected:
		BufferLayout Layout;
		BufferEventsFlags BufferState = BufferEventsFlags::Idle;

		UINT64 Size;
		UINT Count;
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() = default;

		virtual void OnDestroy() = 0;
		virtual void SetData(const UINT16* data, INT32 count) = 0;
		virtual void SetBuffer(const void* bufferAddress) = 0;
		[[nodiscard]] virtual UINT GetCount() const = 0;
		[[nodiscard]] virtual UINT16* GetData() const = 0;

		static ScopePointer<IndexBuffer> Create(UINT16* indices, UINT64 size, UINT indexCount);
	};

	class GPUResource
	{
	public:
		GPUResource(const std::string_view& registeredName)
		{
			memset(&RegisteredName[0], char(0), sizeof(char) * 64u);
			memcpy(&RegisteredName[0], &registeredName[0], static_cast<UINT32>((registeredName.size()) * sizeof(char)));
		}

	protected:
		char RegisteredName[64];
	};

	class DynamicBuffer : public GPUResource
	{
	public:
		DynamicBuffer(const std::string_view& registeredName);



	};

}

